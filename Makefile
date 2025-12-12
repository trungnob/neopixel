SHELL := /bin/bash

# Default configuration (can be overridden: make PIO_ENV=d1_mini PORT=/dev/ttyUSB0 upload)
PIO_ENV ?= d1_mini
PORT ?=
BAUD ?= 115200
FLASH_BAUD ?= 921600
FLASH_SIZE ?= 0x400000
OUT_DIR ?= artifacts
OTA_HOST ?=
OUT ?=

DEVICE_ENV := PIO_ENV="$(PIO_ENV)" PORT="$(PORT)" BAUD="$(BAUD)" FLASH_BAUD="$(FLASH_BAUD)" FLASH_SIZE="$(FLASH_SIZE)" OUT_DIR="$(OUT_DIR)"

.PHONY: help deps build upload upload-ota monitor clean download ota-init sim-build-wasm mechanical

help:
	@echo "Common targets:"
	@echo "  make deps             # Create .venv/ with PlatformIO + esptool"
	@echo "  make build            # Compile firmware for $(PIO_ENV)"
	@echo "  make upload           # Build and flash over serial (PORT=/dev/ttyUSB0)"
	@echo "  make upload-ota HOST=192.168.1.42   # OTA upload"
	@echo "  make monitor          # Open PlatformIO serial monitor"
	@echo "  make clean            # Remove build artifacts"
	@echo "  make download [OUT=flash.bin PORT=...]  # Dump flash via esptool"
	@echo "  make ota-init [HOST=...]   # Generate config/ota.env with random password"
	@echo "  make mechanical       # Generate STLs from SCAD"


build:
	$(DEVICE_ENV) scripts/device.sh build

upload:
	$(DEVICE_ENV) scripts/device.sh upload

upload-ota:
	@if [ -z "$(HOST)" ] && [ -z "$(OTA_HOST)" ]; then \
		echo "Set HOST=ip (or OTA_HOST env) when running make upload-ota" >&2; \
		exit 1; \
	fi
	$(DEVICE_ENV) OTA_HOST="$(if $(HOST),$(HOST),$(OTA_HOST))" scripts/device.sh upload-ota "$(if $(HOST),$(HOST),$(OTA_HOST))"

monitor:
	$(DEVICE_ENV) scripts/device.sh monitor

clean:
	$(DEVICE_ENV) scripts/device.sh clean

download:
	@if [ -z "$(PORT)" ]; then \
		echo "Set PORT=/dev/ttyUSB0 (or similar) before make download" >&2; \
		exit 1; \
	fi
	$(DEVICE_ENV) scripts/device.sh download $(OUT)

deps:
	scripts/bootstrap.sh

ota-init:
	$(DEVICE_ENV) HOST="$(HOST)" scripts/device.sh ota-init "$(HOST)"

mechanical:
	@echo "Generating STLs..."
	openscad -D 'render_mode=0' -o mechanical/stl/8x32_housing_left.stl mechanical/scad/8x32_housing.scad
	openscad -D 'render_mode=1' -o mechanical/stl/8x32_housing_right.stl mechanical/scad/8x32_housing.scad
	openscad -D 'render_mode=3' -o mechanical/stl/8x32_housing_stack.stl mechanical/scad/8x32_housing.scad
	@echo "Done! Files in mechanical/stl/"



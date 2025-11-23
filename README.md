# ESP8266 WS2812B Controller

This PlatformIO project drives a WS2812B/NeoPixel strip from an ESP8266 (NodeMCU form factor). It exposes a captive-style HTTP interface with dozens of animation presets plus OTA updates, so you can dial in animations from your phone without recompiling.

## Features
- Control up to 200 LEDs (defaults to 144) with FastLED.
- Simple mobile-friendly web UI with 40+ pre-defined patterns, color wipes, rainbows, breathing, etc.
- On-boot LED status indicators so you know whether Wi-Fi and HTTP server are running.
- OTA and HTTP API endpoints so you can reflash or integrate it elsewhere.

## Simulator (WASM)
- There is a WebAssembly simulator that runs the real 2D patterns (100–121) in the browser using the C++ code. It preserves physical strip spacing and zigzag wiring so you can preview layout and timing without hardware.
- Build the simulator artifacts:
  ```bash
  source third_party/emsdk/emsdk_env.sh
  make sim-build-wasm
  ```
- Serve the repo root and open the viewer:
  ```bash
  python3 -m http.server 8000
  # then open http://localhost:8000/sim/wasm/index.html
  ```
- Controls include pattern select, play/pause/step, random seed, scrolling text + speed (pattern 120), FPS and lit-pixel counts. Pattern 121 is a single-pixel test card for mapping checks.

## Getting Started
1. Clone this repo (or copy `platformio.ini`/`src` into an existing PlatformIO workspace).
2. Run `make deps` (or `scripts/bootstrap.sh`) to create `.venv/` with PlatformIO + esptool. Requires `python3`, `pip`, and internet access.
3. Generate an OTA secret + host file (ignored by git) once:
   ```bash
   make ota-init HOST=192.168.1.129   # pick the host/IP that will OTA upload
   ```
   This writes `config/ota.env` with a random password so both the firmware build and OTA uploader use the same secret without hardcoding it.
4. Create `config/secrets.env` (copy from `config/secrets.env.example`) and fill in `WIFI_SSID` / `WIFI_PASSWORD`.
5. Connect the NeoPixel data-in to `D4` (GPIO2) and 5V/GND appropriately.
6. Build and upload:
   ```bash
   make upload
   ```
7. Open the serial monitor (`make monitor PORT=/dev/ttyUSB0`) to grab the IP address, then hit the HTTP interface in a browser.

## Automation Scripts
`scripts/device.sh` (and the `Makefile` wrapper) cover the most common PlatformIO flows so you do not have to remember the exact CLI flags. Run `make deps` once so they can use the repo-local PlatformIO if you do not have it installed globally. Everything runs inside the workspace virtualenv, so none of the `make` targets require `sudo`.

```bash
# Generate OTA credentials (writes config/ota.env, ignored by git)
make ota-init HOST=192.168.1.129

# Copy Wi-Fi secrets template and edit values
cp config/secrets.env.example config/secrets.env

# Compile (PIO_ENV defaults to d1_mini)
scripts/device.sh build
make build

# Flash over serial (set PORT=/dev/ttyUSB0 if PlatformIO cannot auto-detect)
PORT=/dev/ttyUSB0 scripts/device.sh upload
make upload PORT=/dev/ttyUSB0

# OTA upload (pass host/IP or export OTA_HOST)
scripts/device.sh upload-ota 192.168.1.42
make upload-ota HOST=192.168.1.42

# Serial monitor
PORT=/dev/ttyUSB0 BAUD=115200 scripts/device.sh monitor
make monitor PORT=/dev/ttyUSB0 BAUD=115200

# Dump the flash contents to artifacts/flash_dump.bin (requires esptool.py)
PORT=/dev/ttyUSB0 scripts/device.sh download
make download PORT=/dev/ttyUSB0 OUT=my_backup.bin
```

The generated `config/ota.env` and `config/secrets.env` are ignored by git—keep them safe (regenerate via `make ota-init --force` or edit the secrets file as needed).

Other helpful variables:
- `FLASH_SIZE` (default `0x400000`) controls how much flash `download` reads.
- `FLASH_BAUD` (default `921600`) tunes the backup speed.
- `OUT_DIR` (default `artifacts`) selects where flash dumps are written.

Make also exposes those environment overrides (`PIO_ENV`, `PORT`, `FLASH_SIZE`, etc.), so you can chain multiple commands with consistent settings: e.g. `make PIO_ENV=d1_mini_lite PORT=/dev/ttyUSB0 upload monitor`.

The `.gitignore` already excludes build directories (`.pio/`, `.venv/`, `artifacts/`, etc.) so the repo never ships binaries—only the sources and helper scripts.

## Notes
- `BRIGHTNESS` defaults to 64; bump it carefully to avoid overdrawing your power supply.
- OTA uses ArduinoOTA with the default hostname `esp8266-ws2812`; adjust if you run multiple units.
- OTA credentials live in `config/ota.env` and Wi-Fi credentials in `config/secrets.env` (both ignored by git). Run `make ota-init --force HOST=<new-ip>` anytime you want to rotate the OTA password or change the target.
- If you add/remove LED patterns, keep the `currentPattern` switch in sync with the web button IDs.

## License
Released under the [MIT License](LICENSE). Feel free to use, modify, and distribute as long as the copyright notice is preserved.

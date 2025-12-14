# ESP8266 LED Matrix Controller & Pattern Designer

This project drives a **32x32 LED Matrix** (composed of four 8x32 WS2812B panels) using an ESP8266 (Wemos D1 Mini). It features a web-based **Pattern Designer** for creating and uploading custom pixel art and animations.

## Features
- **Hardware Support**: Configured for a 32x32 grid (1024 LEDs) using 4x 8x32 panels in a vertical stack.
- **Web Interface**: 
  - **Control Panel**: Switch between 40+ pre-defined patterns (Rainbow, Fire, Matrix, etc.).
  - **Pattern Designer**: Draw custom pixel art, create animations, and scroll text.
  - **Static Mode**: Display static images (Speed 0).
- **OTA Updates**: Flash new firmware wirelessly.
- **Mechanical Parts**: 3D printable housing for the panels.

## Directory Structure
- `src/`: Firmware source code (C++).
- `web/`: Web interface (`pattern-designer.html`).
- `mechanical/`:
  - `stl/`: 3D printable files (Housing, Mounts).
  - `scad/`: OpenSCAD source files for the housing.
- `scripts/`: Build and utility scripts.

## Getting Started

### 🚀 Quick Start (Recommended)
Run the automated setup script to configure Wi-Fi, install dependencies, and flash the device:
```bash
./scripts/quickstart.sh
```

### Manual Setup
If you prefer to set up everything manually:

### Prerequisites
- Python 3
- `pip`
- `make` (optional, but recommended)

### 1. Setup Environment
Run the bootstrap script to create a virtual environment and install PlatformIO:
```bash
make deps
# Or manually: ./scripts/bootstrap.sh
```

### 2. Configure Secrets
1. Copy the secrets template:
   ```bash
   cp config/secrets.env.example config/secrets.env
   ```
2. Edit `config/secrets.env` and add your Wi-Fi credentials (`WIFI_SSID` and `WIFI_PASSWORD`).
3. Generate OTA credentials:
   ```bash
   make ota-init HOST=192.168.1.130
   ```

### 3. Build and Upload Firmware
**First time (USB):**
Connect your ESP8266 via USB.
```bash
make upload PORT=/dev/ttyUSB0
```
*(Note: `PORT` might vary, e.g., `/dev/ttyACM0`)*

**Subsequent updates (OTA):**
```bash
make upload-ota HOST=192.168.1.130
```

### 4. Run the Pattern Designer
The Pattern Designer runs locally on your computer and communicates with the ESP8266.
```bash
python3 server.py
```
Open your browser to: [http://localhost:8000/pattern-designer.html](http://localhost:8000/pattern-designer.html)

**Note:** Ensure your computer is on the same Wi-Fi network as the ESP8266. The web interface proxies requests to the ESP8266 IP defined in `server.py` (default `192.168.1.130`).

## Mechanical Parts
3D printable files are located in `mechanical/stl/`.
- **Housing**: `8x32_housing_left.stl`, `8x32_housing_right.stl`
- **Stacked Batch**: `8x32_housing_stack.stl` (3x Left Half, matches original production file)
- **Source**: OpenSCAD files in `mechanical/scad/`.

To regenerate the STLs from source:
```bash
make mechanical
```

## Development
- **Build**: `make build`
- **Clean**: `make clean`
- **Monitor Serial**: `make monitor PORT=/dev/ttyUSB0`

## License
MIT License.

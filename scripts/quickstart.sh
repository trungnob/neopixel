#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
SECRETS_FILE="$PROJECT_ROOT/config/secrets.env"
OTA_FILE="$PROJECT_ROOT/config/ota.env"

echo "========================================"
echo "   ESP8266 LED Controller Quickstart"
echo "========================================"

# 1. Check/Prompt for Wi-Fi Credentials
if [[ -f "$SECRETS_FILE" ]]; then
    echo "✅ Wi-Fi credentials found in config/secrets.env"
else
    echo "⚠️  Wi-Fi credentials not found."
    echo "Please enter your router credentials to connect the device to your network."
    
    read -p "Wi-Fi SSID (Name): " wifi_ssid
    read -s -p "Wi-Fi Password: " wifi_password
    echo ""
    
    mkdir -p "$PROJECT_ROOT/config"
    cat > "$SECRETS_FILE" <<EOF
WIFI_SSID="${wifi_ssid}"
WIFI_PASSWORD="${wifi_password}"
EOF
    echo "✅ Credentials saved to config/secrets.env"
fi

# 2. Check/Init OTA Configuration
if [[ -f "$OTA_FILE" ]]; then
    echo "✅ OTA configuration found in config/ota.env"
else
    echo "⚠️  OTA configuration not found. Generating..."
    # Default to a likely IP or let the user change it later. 
    # For now, we just init with a placeholder or ask?
    # The make ota-init command requires a HOST argument usually, but device.sh defaults to 192.168.1.129
    "$SCRIPT_DIR/device.sh" ota-init
    echo "✅ OTA config generated."
fi

# 3. Install Dependencies
echo ""
echo "📦 Installing dependencies..."
"$SCRIPT_DIR/bootstrap.sh"

# 4. Build and Upload
echo ""
echo "🚀 Building and Uploading Firmware..."
echo "Please ensure your ESP8266 is connected via USB."
read -p "Press Enter to continue (or Ctrl+C to cancel)..."

# Detect port if possible, or let make upload handle it (it defaults to auto-detect or user must specify)
# We'll just run make upload. If PORT is needed, make upload might fail if not set, 
# but device.sh upload usually tries to find it or PlatformIO handles it.
# Actually device.sh require_port checks for PORT env var.
# We should try to detect it or ask.

# Simple auto-detection attempt for Linux
DETECTED_PORT=$(ls /dev/ttyUSB* /dev/ttyACM* 2>/dev/null | head -n 1 || true)

if [[ -n "$DETECTED_PORT" ]]; then
    echo "Detected Serial Port: $DETECTED_PORT"
    export PORT="$DETECTED_PORT"
else
    echo "⚠️  No serial port detected automatically."
    read -p "Please enter serial port (e.g. /dev/ttyUSB0): " manual_port
    export PORT="$manual_port"
fi

make upload PORT="$PORT"

echo ""
echo "✅ Done! Device should be restarting."
echo "You can now run 'python3 server.py' to start the controller."

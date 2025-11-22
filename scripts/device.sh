#!/usr/bin/env bash
set -euo pipefail

ENVIRONMENT="${PIO_ENV:-d1_mini}"
PORT="${PORT:-}"
BAUD="${BAUD:-115200}"
FLASH_BAUD="${FLASH_BAUD:-921600}"
FLASH_SIZE="${FLASH_SIZE:-0x400000}"
OUT_DIR="${OUT_DIR:-artifacts}"

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
VENV_DIR="$PROJECT_ROOT/.venv"
VENV_PIO="$VENV_DIR/bin/pio"
VENV_ESPTOOL="$VENV_DIR/bin/esptool.py"
PIO_BIN="${PIO_BIN:-}"
PIO_HOME="${PIO_HOME:-$PROJECT_ROOT/.platformio}"

export PLATFORMIO_CORE_DIR="$PIO_HOME"
export PLATFORMIO_GLOBALLIB_DIR="$PIO_HOME/lib"
export PLATFORMIO_PACKAGES_DIR="$PIO_HOME/packages"
export PLATFORMIO_CACHE_DIR="$PIO_HOME/.cache"
export PLATFORMIO_STATE_DIR="$PIO_HOME/.cache"
CONFIG_DIR="$PROJECT_ROOT/config"
CONFIG_FILE="$CONFIG_DIR/ota.env"
SECRETS_FILE="$CONFIG_DIR/secrets.env"
USER_DEFINED_OTA_PASSWORD="${OTA_PASSWORD:-}"
USER_DEFINED_OTA_HOST="${OTA_HOST:-}"
if [[ -f "$CONFIG_FILE" ]]; then
  set -a
  # shellcheck disable=SC1090
  source "$CONFIG_FILE"
  set +a
fi
if [[ -f "$SECRETS_FILE" ]]; then
  set -a
  # shellcheck disable=SC1090
  source "$SECRETS_FILE"
  set +a
fi
if [[ -n "$USER_DEFINED_OTA_PASSWORD" ]]; then
  OTA_PASSWORD="$USER_DEFINED_OTA_PASSWORD"
fi
if [[ -n "$USER_DEFINED_OTA_HOST" ]]; then
  OTA_HOST="$USER_DEFINED_OTA_HOST"
fi
export OTA_PASSWORD OTA_HOST

usage() {
  cat <<USAGE
Usage: scripts/device.sh <command> [args]

Commands:
  build            Compile the firmware for ${ENVIRONMENT} (override with PIO_ENV)
  upload           Build and flash over serial (set PORT for a custom port)
  upload-ota <ip>  OTA upload using the provided host or \$OTA_HOST
  monitor          Open a serial monitor (uses PORT/BAUD env vars)
  clean            Remove PlatformIO build artifacts for the env
  download [file]  Dump flash via esptool.py (defaults to ${OUT_DIR}/flash_dump.bin)
  ota-init [host]  Generate config/ota.env with random password (host optional)

Environment overrides:
  PIO_ENV       PlatformIO environment name (default: d1_mini)
  PORT          Serial device for upload/monitor/download (e.g. /dev/ttyUSB0)
  BAUD          Serial monitor baud (default: 115200)
  FLASH_BAUD    Baud rate used for flash dumps (default: 921600)
  FLASH_SIZE    Flash size passed to esptool read_flash (default: 0x400000)
  OUT_DIR       Directory for download artifacts (default: artifacts)
  OTA_HOST      Default host/IP for OTA uploads (only used if no argument)
  OTA_PASSWORD  OTA password injected into firmware + OTA uploader
  WIFI_SSID     Wi-Fi SSID injected into firmware
  WIFI_PASSWORD Wi-Fi password injected into firmware
USAGE
}

log() {
  printf '[device.sh] %s\n' "$*" >&2
}

ensure_pio() {
  if [[ -n "$PIO_BIN" && -x "$PIO_BIN" ]]; then
    return
  fi
  if command -v pio >/dev/null 2>&1; then
    PIO_BIN="$(command -v pio)"
    return
  fi
  if [[ -x "$VENV_PIO" ]]; then
    PIO_BIN="$VENV_PIO"
    return
  fi
  cat <<'ERR' >&2
PlatformIO CLI (pio) not found. Run 'make deps' (or scripts/bootstrap.sh) to install it locally,
or install PlatformIO globally so 'pio' is on PATH.
ERR
  exit 1
}

resolve_esptool() {
  if command -v esptool.py >/dev/null 2>&1; then
    ESPTOOL=(esptool.py)
    return
  fi
  if [[ -x "$VENV_ESPTOOL" ]]; then
    ESPTOOL=("$VENV_ESPTOOL")
    return
  fi
  if python3 -c 'import esptool' >/dev/null 2>&1; then
    ESPTOOL=(python3 -m esptool)
    return
  fi
  local embedded="$HOME/.platformio/packages/tool-esptoolpy/esptool.py"
  if [[ -x "$embedded" ]]; then
    ESPTOOL=(python3 "$embedded")
    return
  fi
  echo "esptool.py not found. Install it (pip install esptool) or ensure PlatformIO is installed." >&2
  exit 1
}

ensure_python3() {
  if ! command -v python3 >/dev/null 2>&1; then
    echo "python3 is required for generating OTA credentials" >&2
    exit 1
  fi
}

generate_password() {
  ensure_python3
  python3 - <<'PY'
import secrets
import string

alphabet = string.ascii_letters + string.digits
print(''.join(secrets.choice(alphabet) for _ in range(24)))
PY
}

ensure_ota_password() {
  if [[ -z "${OTA_PASSWORD:-}" ]]; then
    cat <<'ERR' >&2
OTA password is not configured. Run `make ota-init` (or scripts/device.sh ota-init)
to create config/ota.env, or export OTA_PASSWORD before running this command.
ERR
    exit 1
  fi
}

ensure_ota_host() {
  if [[ -z "${OTA_HOST:-}" ]]; then
    echo "Set OTA_HOST in config/ota.env or export it before running this command." >&2
    exit 1
  fi
}

ensure_wifi_secrets() {
  if [[ -z "${WIFI_SSID:-}" || -z "${WIFI_PASSWORD:-}" ]]; then
    cat <<'ERR' >&2
Wi-Fi credentials are missing. Create config/secrets.env (copy from config/secrets.env.example)
or export WIFI_SSID and WIFI_PASSWORD before building.
ERR
    exit 1
  fi
}

require_port() {
  if [[ -z "$PORT" ]]; then
    echo "Set PORT environment variable (e.g. PORT=/dev/ttyUSB0) before running this command." >&2
    exit 1
  fi
}

command=${1:-}
if [[ -z "$command" || "$command" == "-h" || "$command" == "--help" ]]; then
  usage
  exit 0
fi

shift
case "$command" in
  build)
    ensure_pio
    ensure_ota_password
    ensure_wifi_secrets
    log "Compiling ($ENVIRONMENT)"
    "$PIO_BIN" run -e "$ENVIRONMENT"
    ;;
  upload)
    ensure_pio
    ensure_ota_password
    ensure_wifi_secrets
    ensure_ota_host
    log "Uploading over serial ($ENVIRONMENT)"
    cmd=("$PIO_BIN" run -e "$ENVIRONMENT" -t upload)
    if [[ -n "$PORT" ]]; then
      cmd+=(--upload-port "$PORT")
    fi
    "${cmd[@]}"
    ;;
  upload-ota)
    ensure_pio
    ensure_ota_password
    ensure_wifi_secrets
    target_host=${1:-${OTA_HOST:-}}
    if [[ -z "$target_host" ]]; then
      echo "Provide OTA host/IP as argument or set OTA_HOST env variable." >&2
      exit 1
    fi
    log "Uploading OTA to $target_host ($ENVIRONMENT)"
    "$PIO_BIN" run -e "$ENVIRONMENT" -t upload --upload-port "$target_host"
    ;;
  monitor)
    ensure_pio
    log "Opening serial monitor"
    cmd=("$PIO_BIN" device monitor --baud "$BAUD")
    if [[ -n "$PORT" ]]; then
      cmd+=(--port "$PORT")
    fi
    "${cmd[@]}"
    ;;
  clean)
    ensure_pio
    log "Cleaning build artifacts"
    "$PIO_BIN" run -e "$ENVIRONMENT" -t clean
    ;;
  download)
    require_port
    resolve_esptool
    mkdir -p "$OUT_DIR"
    output=${1:-"$OUT_DIR/flash_dump.bin"}
    log "Downloading flash from $PORT -> $output"
    "${ESPTOOL[@]}" \
      --chip esp8266 --port "$PORT" --baud "$FLASH_BAUD" \
      read_flash 0 "$FLASH_SIZE" "$output"
    log "Flash dump saved to $output"
    ;;
  ota-init)
    force=0
    if [[ "${1:-}" == "--force" ]]; then
      force=1
      shift
    fi
    desired_host=${1:-${OTA_HOST:-192.168.1.129}}
    if [[ -f "$CONFIG_FILE" && "$force" -ne 1 ]]; then
      echo "config/ota.env already exists. Use 'scripts/device.sh ota-init --force' to overwrite." >&2
      exit 1
    fi
    mkdir -p "$CONFIG_DIR"
    password=$(generate_password)
    cat > "$CONFIG_FILE" <<EOF
OTA_HOST=${desired_host}
OTA_PASSWORD=${password}
EOF
    echo "[device.sh] Generated OTA credentials at $CONFIG_FILE"
    echo "[device.sh] Host: ${desired_host}"
    ;;
  *)
    echo "Unknown command: $command" >&2
    usage
    exit 1
    ;;
esac

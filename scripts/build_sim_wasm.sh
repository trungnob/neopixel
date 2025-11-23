#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
OUT_DIR="${ROOT_DIR}/artifacts/simulator"
EMSDK_DIR="${EMSDK_DIR:-${ROOT_DIR}/third_party/emsdk}"

# Prefer EMCC env, then PATH, then submodule's installed toolchain
if [[ -n "${EMCC:-}" ]]; then
  EMCC_BIN="${EMCC}"
elif command -v emcc >/dev/null 2>&1; then
  EMCC_BIN="$(command -v emcc)"
elif [[ -x "${EMSDK_DIR}/upstream/emscripten/emcc" ]]; then
  EMCC_BIN="${EMSDK_DIR}/upstream/emscripten/emcc"
else
  cat >&2 <<'EOF'
emcc not found.
- If you have emsdk checked out as a submodule, install + activate it:
    third_party/emsdk/emsdk install 4.0.20
    third_party/emsdk/emsdk activate 4.0.20
    source third_party/emsdk/emsdk_env.sh
- Or set EMCC=/path/to/emcc before running this script.
EOF
  exit 1
fi

mkdir -p "${OUT_DIR}"

echo "[sim-wasm] Building sim core with ${EMCC_BIN}"

PATTERN_SRCS=$(ls "${ROOT_DIR}"/src/patterns/pattern_*.cpp | tr '\n' ' ')
FONT_SRC="${ROOT_DIR}/src/patterns/font.cpp"

"${EMCC_BIN}" \
  -std=c++17 -O2 \
  -DSIMULATOR -DSIM_WASM \
  -I"${ROOT_DIR}/src" \
  "${ROOT_DIR}/sim/wasm/sim_core.cpp" \
  ${PATTERN_SRCS} \
  "${FONT_SRC}" \
  -o "${OUT_DIR}/sim-core.js" \
  -sALLOW_MEMORY_GROWTH=1 \
  -sMODULARIZE=1 \
  -sEXPORT_ES6=1 \
  -sEXPORT_NAME=createSimModule \
  -sENVIRONMENT=web,worker \
  -sEXPORTED_FUNCTIONS='[_sim_init,_sim_set_pattern,_sim_set_scroll_speed,_sim_set_text,_sim_seed,_sim_step,_sim_get_buffer,_sim_get_buffer_length,_sim_get_led_count,_sim_get_grid_width,_sim_get_grid_height]' \
  -sEXPORTED_RUNTIME_METHODS='[cwrap,ccall,HEAPU8]' \
  -sFORCE_FILESYSTEM=0

echo "[sim-wasm] Output:"
echo "  ${OUT_DIR}/sim-core.js"
echo "  ${OUT_DIR}/sim-core.wasm"

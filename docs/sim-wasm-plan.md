# WASM Simulator Plan

Goal: replace the old C++→JS pattern extraction with a WebAssembly simulator that runs the real pattern code. Keep firmware (ESP8266 + OTA/WebServer) separate from the sim core so the browser talks to WASM, not translated JS.

## Current state (May 2025)
- `make simulator` runs `scripts/extract_patterns.py`, converts 2D pattern cases from `src/main.cpp` into JS, and drops them into `cpp-simulator-generated.html` (legacy flow to be removed).
- Patterns 100–120 live in `src/patterns/pattern_*.cpp`; many 1D patterns are inline in the `switch` inside `src/main.cpp`. Shared helpers/constants are in `src/platform.h` (already has a `SIMULATOR` shim for native builds: `CRGB`, `CHSV`, `millis`, `random8/16`, `beat` functions, `HeatColor`, `fadeToBlackBy`, etc.).
- Firmware concerns (WiFi/OTA/WebServer/HTML) are mixed into `main.cpp`; these must not compile for WASM.

## Target architecture
- **Sim core (C++)**: pattern logic compiled with Emscripten. Expose a small C ABI: `init(width,height)`, `set_pattern(id)`, `set_config(speed, brightness, seed)`, `step(delta_ms)`, and `get_buffer()` returning RGB888 data. Internal state owns the LED buffer and timing.
- **Shims**: lightweight replacements for Arduino/FastLED pieces (`CRGB/CHSV`, `fill_solid`, `fadeToBlackBy`, `HeatColor`, `sin8/cos8/beatsin8/16`, `qadd8/qsub8`, `inoise8`, `random8/16`, `millis`, `XY`, and `EVERY_N_*` equivalents). Reuse/adapt `src/platform.h` under a `SIM_WASM`/`SIMULATOR` guard; ensure pure C++ with no ESP headers.
- **Frontend**: static app (TS/JS) that loads the WASM module, calls the ABI, reads the framebuffer from WASM memory, and renders to `<canvas>`. Controls: pattern select, play/pause, speed/brightness, palette, FPS stats.
- **Build flow**: remove `make simulator`; add `make sim-build-wasm` (Emscripten build of sim core + frontend bundle) and `make sim-dev` (frontend dev server). Outputs to `artifacts/simulator/`.

## Carving steps
1) **Dependency map**: list pattern-time dependencies used in `main.cpp` and `src/patterns/*` (helpers, macros, globals). Verify all have shim-able equivalents and no hidden ESP/WiFi references in pattern code.
2) **Isolate sim core**: split firmware vs. render logic. Move the pattern switch + shared state into a new sim entry (`src/sim_core.cpp` or similar) built under `SIM_WASM`; guard firmware-only pieces in `main.cpp`.
3) **Define ABI**: implement the exported functions, allocate the LED buffer, and document framebuffer layout (row-major, using `XY` zigzag). Decide randomness (fixed seed for determinism), and how `EVERY_N_*` maps to `step(delta_ms)`.
4) **Implement shims**: ensure `platform.h` (or a new `sim_platform.h`) compiles without Arduino/FastLED; add any missing helpers used by patterns (e.g., `ColorFromPalette`, `fill_rainbow`, `blur1d` if needed).
5) **Emscripten wiring**: add a build script/Make target that compiles the sim core to `.wasm` + JS glue with the ABI exports; stash artifacts in `artifacts/simulator/`.
6) **Frontend hook-up**: build a minimal loader that calls the ABI, drives `step` via `requestAnimationFrame`, and paints the framebuffer to canvas. Add basic controls and stats.
7) **Decommission legacy**: remove or archive the Python extractor and `cpp-simulator*.html`; update docs/Makefile to the new targets.

## Open questions
- Determinism vs. entropy: should we seed RNG from JS and keep deterministic playback for captures?
- Palette support: do we need `ColorFromPalette`/FastLED palettes, or is RGB-only acceptable initially?
- Frame pacing: lock to real time (`delta_ms` from JS) or allow a sim speed multiplier?
- Build constraints: can we vendor a minimal Emscripten toolchain locally, or rely on a system install?

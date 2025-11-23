# WASM Simulator (patterns 100–120)

This builds a WebAssembly module that runs the real 2D pattern code (cases 100–120) without translating C++ to JS. The exported API lets a frontend load the module, pick a pattern, step frames, and read the RGB buffer.

## Exported C ABI
- `void sim_init(int width, int height)` – initialize buffer (defaults to 144×9 if width/height are 0).
- `void sim_set_pattern(int pattern)` – choose pattern (100–120).
- `void sim_set_scroll_speed(int ms)` – clamp 20–200.
- `void sim_set_text(const char* txt)` – update scrolling text, reset offset.
- `void sim_seed(uint32_t seed)` – seed `rand()`.
- `void sim_step(uint32_t delta_ms)` – advance one frame (delta currently unused; patterns rely on `millis()` shims).
- `uint8_t* sim_get_buffer()` / `int sim_get_buffer_length()` – RGB888 data in strip order.
- `int sim_get_led_count()`, `int sim_get_grid_width()`, `int sim_get_grid_height()`.

## Building
Requires Emscripten (`emcc`) on PATH or via the bundled submodule.
```bash
# using submodule pinned to 4.0.20
third_party/emsdk/emsdk install 4.0.20
third_party/emsdk/emsdk activate 4.0.20
source third_party/emsdk/emsdk_env.sh

# then build
make sim-build-wasm
# outputs: artifacts/simulator/sim-core.{js,wasm}
```
If `emcc` is missing the script will fail with a helpful message.

## Runtime notes
- Uses the `SIMULATOR` shims in `src/platform.h` (CRGB/CHSV, sin/beats, random, etc.).
- `sim_step` currently ignores `delta_ms` because pattern code calls `millis()` internally; time comes from the shim’s steady clock. If deterministic stepping is needed, adjust `platform.h` to allow overriding `millis()` and feed `delta_ms` into a custom implementation.
- Framebuffer is RGB888, length `sim_get_buffer_length()` bytes; layout uses the zigzag `XY()` mapping baked into the pattern functions.

## Minimal UI
- `sim/wasm/index.html` is a static viewer for patterns 100–120. Serve the repo (e.g. `python3 -m http.server 8000`) and open `http://localhost:8000/sim/wasm/index.html`.
- Controls: pattern select, play/pause/step, seed randomizer, text + scroll speed for pattern 120, FPS and lit-pixel readout. Canvas uses `sim-core.js/wasm` directly (no bundler needed).
- Physical scale: the viewer draws each LED as a small square and inserts a vertical gap between rows based on physical spacing (6.9 mm horizontal, 50 mm vertical). Zigzag wiring is respected when rendering.

## Adding / tweaking patterns
- Patterns live under `src/patterns/` and are exposed via `pattern_*.cpp` plus declarations in `src/patterns.h`.
- To light a single LED at `(x, y)`: `int idx = XY(x, y); if (idx >= 0 && idx < activeLeds) leds[idx] = CRGB::Red;`.
- To cycle color over time: use the shared `hue` reference, e.g. `leds[idx] = CHSV(hue, 255, 255); hue++;`.
- Clear pixels explicitly when you want only specific LEDs on: `fill_solid(leds, activeLeds, CRGB::Black);` before setting your pixels.
- After adding a pattern, re-run `make build` (firmware) and `make sim-build-wasm` (simulator) to see it in the UI.

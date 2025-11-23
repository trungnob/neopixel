// WASM simulator core: runs 2D patterns (100-120) and exposes a C ABI for JS.
// This compiles with Emscripten using the SIMULATOR shims in platform.h.

#define SIMULATOR

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <string>

#include "../../src/patterns.h"

static CRGB leds[MAX_LEDS];
static int activeLeds = GRID_WIDTH * GRID_HEIGHT;
static int currentPattern = 100;
static uint8_t hue = 0;
static std::string scrollText = "HELLO WORLD";
static int scrollOffset = 0;
static int scrollSpeed = 80; // ms
unsigned long (*sim_millis_fn)() = nullptr;
static uint64_t sim_time_ms = 0;
static unsigned long wasm_millis() {
  return static_cast<unsigned long>(sim_time_ms);
}

static inline void clearTail() {
  if (activeLeds < MAX_LEDS) {
    for (int i = activeLeds; i < MAX_LEDS; i++) {
      leds[i] = CRGB::Black;
    }
  }
}

static void runPattern() {
  switch (currentPattern) {
    case 100: pattern_horizontal_bars(leds, activeLeds, hue); break;
    case 101: pattern_vertical_ripple(leds, activeLeds, hue); break;
    case 102: pattern_fire_rising(leds, activeLeds, hue); break;
    case 103: pattern_rain_drops(leds, activeLeds, hue); break;
    case 104: pattern_vertical_equalizer(leds, activeLeds, hue); break;
    case 105: pattern_scanning_lines(leds, activeLeds, hue); break;
    case 106: pattern_checkerboard(leds, activeLeds, hue); break;
    case 107: pattern_diagonal_sweep(leds, activeLeds, hue); break;
    case 108: pattern_vertical_wave(leds, activeLeds, hue); break;
    case 109: pattern_plasma_2d(leds, activeLeds, hue); break;
    case 110: pattern_matrix_rain(leds, activeLeds, hue); break;
    case 111: pattern_game_of_life(leds, activeLeds, hue); break;
    case 112: pattern_wave_pool(leds, activeLeds, hue); break;
    case 113: pattern_aurora_2d(leds, activeLeds, hue); break;
    case 114: pattern_lava_lamp(leds, activeLeds, hue); break;
    case 115: pattern_ripple_2d(leds, activeLeds, hue); break;
    case 116: pattern_starfield(leds, activeLeds, hue); break;
    case 117: pattern_side_fire(leds, activeLeds, hue); break;
    case 118: pattern_scrolling_rainbow(leds, activeLeds, hue); break;
    case 119: pattern_particle_fountain(leds, activeLeds, hue); break;
    case 120:
      pattern_scrolling_text(leds, activeLeds, hue, scrollText.c_str(), scrollOffset, scrollSpeed);
      break;
    case 121: pattern_test_card(leds, activeLeds, hue); break;
    default:
      fill_solid(leds, activeLeds, CRGB::Black);
      break;
  }

  clearTail();
}

extern "C" {

void sim_init(int width, int height) {
  if (width > 0 && height > 0) {
    activeLeds = std::min(width * height, MAX_LEDS);
  } else {
    activeLeds = GRID_WIDTH * GRID_HEIGHT;
  }
  sim_time_ms = 0;
  sim_millis_fn = wasm_millis;
  clearTail();
}

void sim_set_pattern(int pattern) {
  currentPattern = pattern;
}

void sim_set_scroll_speed(int speed_ms) {
  scrollSpeed = std::clamp(speed_ms, 20, 200);
}

void sim_set_text(const char* txt) {
  scrollText = txt ? std::string(txt) : std::string();
  scrollOffset = 0;
}

void sim_seed(uint32_t seed) {
  srand(seed);
}

// Run one frame; advance simulated millis by delta (fallback to ~60 FPS if delta is 0).
void sim_step(uint32_t delta_ms) {
  sim_time_ms += (delta_ms > 0) ? delta_ms : 16;
  runPattern();
}

// Raw RGB buffer (RGB888) in strip order (zigzagged via XY mapping inside patterns).
uint8_t* sim_get_buffer() {
  return reinterpret_cast<uint8_t*>(leds);
}

int sim_get_buffer_length() {
  return activeLeds * 3;
}

int sim_get_led_count() {
  return activeLeds;
}

int sim_get_grid_width() { return GRID_WIDTH; }
int sim_get_grid_height() { return GRID_HEIGHT; }

} // extern "C"

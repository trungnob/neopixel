#ifndef PATTERNS_H
#define PATTERNS_H

#include "platform.h"

// XY mapping function (zigzag wiring)
inline int XY(int x, int y) {
  if (x < 0 || x >= GRID_WIDTH || y < 0 || y >= GRID_HEIGHT) return -1;

  if (y % 2 == 0) {
    // Even rows (0, 2, 4, 6, 8): left to right
    return y * GRID_WIDTH + x;
  } else {
    // Odd rows (1, 3, 5, 7): right to left (reversed)
    return y * GRID_WIDTH + (GRID_WIDTH - 1 - x);
  }
}

// Font data for scrolling text
extern const uint8_t FONT_WIDTH;
extern const uint8_t FONT_HEIGHT;
extern const uint8_t font5x7[][5];

// Font helper function
int getFontIndex(char c);

// Pattern function declarations
// Each pattern is a standalone function that can be called from main.cpp or simulator

void pattern_horizontal_bars(CRGB* leds, int activeLeds, uint8_t& hue);
void pattern_vertical_ripple(CRGB* leds, int activeLeds, uint8_t& hue);
void pattern_fire_rising(CRGB* leds, int activeLeds, uint8_t& hue);
void pattern_rain_drops(CRGB* leds, int activeLeds, uint8_t& hue);
void pattern_vertical_equalizer(CRGB* leds, int activeLeds, uint8_t& hue);
void pattern_scanning_lines(CRGB* leds, int activeLeds, uint8_t& hue);
void pattern_checkerboard(CRGB* leds, int activeLeds, uint8_t& hue);
void pattern_diagonal_sweep(CRGB* leds, int activeLeds, uint8_t& hue);
void pattern_vertical_wave(CRGB* leds, int activeLeds, uint8_t& hue);
void pattern_plasma_2d(CRGB* leds, int activeLeds, uint8_t& hue);
void pattern_matrix_rain(CRGB* leds, int activeLeds, uint8_t& hue);
void pattern_game_of_life(CRGB* leds, int activeLeds, uint8_t& hue);
void pattern_wave_pool(CRGB* leds, int activeLeds, uint8_t& hue);
void pattern_aurora_2d(CRGB* leds, int activeLeds, uint8_t& hue);
void pattern_lava_lamp(CRGB* leds, int activeLeds, uint8_t& hue);
void pattern_ripple_2d(CRGB* leds, int activeLeds, uint8_t& hue);
void pattern_starfield(CRGB* leds, int activeLeds, uint8_t& hue);
void pattern_side_fire(CRGB* leds, int activeLeds, uint8_t& hue);
void pattern_scrolling_rainbow(CRGB* leds, int activeLeds, uint8_t& hue);
void pattern_particle_fountain(CRGB* leds, int activeLeds, uint8_t& hue);
void pattern_scrolling_text(CRGB* leds, int activeLeds, uint8_t& hue,
                           const char* text, int& scrollOffset, int scrollSpeed);

#endif // PATTERNS_H

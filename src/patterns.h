#ifndef PATTERNS_H
#define PATTERNS_H

#include "platform.h"

// Forward declarations from main.cpp
extern int GRID_WIDTH;
extern int GRID_HEIGHT;
extern int
    currentLayout; // 0 = LAYOUT_9X144_ROW_ZIGZAG, 1 = LAYOUT_8X32_COL_ZIGZAG
extern int panelsWide; // Number of panels horizontally

// XY mapping function - supports row zigzag, column zigzag, and multi-panel
// layouts
inline int XY(int x, int y) {
  if (x < 0 || x >= GRID_WIDTH || y < 0 || y >= GRID_HEIGHT)
    return -1;

  // Layout 0: Nx8x32 Multi-Panel (Default)
  // Supports 1 or more panels.
  // Each panel is 8x32, Column-Zigzag.

  if (true) { // Always use Multi-Panel logic since it's the only one left
    // MULTI-PANEL LAYOUT (Generic N panels, W panels wide)

    // Supports 2D Panel Serpentine:
    // Row 0: Panel 0 -> Panel 1 -> ...
    // Row 1: Panel 3 <- Panel 2 <- ...

    // 1. Determine which Panel (px, py) the global (x, y) falls into
    int panelCol = x / 32;
    int panelRow = y / 8;

    // 2. Determine Local coordinates within that panel
    int localX = x % 32;
    int localY = y % 8;

    // Handle 180-degree rotation for odd panel rows (Serpentine Vertical)
    if (panelRow % 2 == 1) {
      localX = 31 - localX;
      localY = 7 - localY;
    }

    // 3. Calculate Panel Index based on Serpentine Path
    int panelIndex;
    if (panelRow % 2 == 0) {
      // Even Panel Row: Left to Right
      panelIndex = (panelRow * panelsWide) + panelCol;
    } else {
      // Odd Panel Row: Right to Left
      // e.g. Width=2. Row 1. Cols 0,1.
      // Col 0 -> Index 3. Col 1 -> Index 2.
      // Index = Start + (Width - 1 - Col)
      panelIndex = (panelRow * panelsWide) + (panelsWide - 1 - panelCol);
    }

    // 4. Calculate LED Offset
    int panelOffset = panelIndex * 256;

    // 5. Map Local Coordinates (Standard 8x32 Column ZigZag)
    // Assuming standard orientation (no rotation)
    if (localX % 2 == 0) {
      // Even columns: TOP to BOTTOM
      return panelOffset + (localX * 8 + localY);
    } else {
      // Odd columns: BOTTOM to TOP (reversed)
      return panelOffset + (localX * 8 + (7 - localY));
    }
  }
}

// Font data for scrolling text
extern const uint8_t FONT_WIDTH;
extern const uint8_t FONT_HEIGHT;
extern const uint8_t font5x7[][5];

// Font helper function
int getFontIndex(char c);

// Pattern function declarations
// Each pattern is a standalone function that can be called from main.cpp or
// simulator

void pattern_horizontal_bars(CRGB *leds, int activeLeds, uint8_t &hue);
void pattern_vertical_ripple(CRGB *leds, int activeLeds, uint8_t &hue);
void pattern_fire_rising(CRGB *leds, int activeLeds, uint8_t &hue);
void pattern_rain_drops(CRGB *leds, int activeLeds, uint8_t &hue);
void pattern_vertical_equalizer(CRGB *leds, int activeLeds, uint8_t &hue);
void pattern_scanning_lines(CRGB *leds, int activeLeds, uint8_t &hue);
void pattern_checkerboard(CRGB *leds, int activeLeds, uint8_t &hue);
void pattern_diagonal_sweep(CRGB *leds, int activeLeds, uint8_t &hue);
void pattern_vertical_wave(CRGB *leds, int activeLeds, uint8_t &hue);
void pattern_plasma_2d(CRGB *leds, int activeLeds, uint8_t &hue);
void pattern_matrix_rain(CRGB *leds, int activeLeds, uint8_t &hue);
void pattern_game_of_life(CRGB *leds, int activeLeds, uint8_t &hue);
void pattern_wave_pool(CRGB *leds, int activeLeds, uint8_t &hue);
void pattern_aurora_2d(CRGB *leds, int activeLeds, uint8_t &hue);
void pattern_lava_lamp(CRGB *leds, int activeLeds, uint8_t &hue);
void pattern_ripple_2d(CRGB *leds, int activeLeds, uint8_t &hue);
void pattern_starfield(CRGB *leds, int activeLeds, uint8_t &hue);
void pattern_side_fire(CRGB *leds, int activeLeds, uint8_t &hue);
void pattern_scrolling_rainbow(CRGB *leds, int activeLeds, uint8_t &hue);
void pattern_particle_fountain(CRGB *leds, int activeLeds, uint8_t &hue);
void pattern_scrolling_text(CRGB *leds, int activeLeds, uint8_t &hue,
                            const char *text, int &scrollOffset,
                            int scrollSpeed);
void pattern_enumerate(CRGB *leds, int activeLeds, uint8_t &hue);
void pattern_hypnotic_rings(CRGB *leds, int activeLeds, uint8_t &hue);
void pattern_noise_lava(CRGB *leds, int activeLeds, uint8_t &hue);
void pattern_diagonal_plasma(CRGB *leds, int activeLeds, uint8_t &hue);
void pattern_spiral_galaxy(CRGB *leds, int activeLeds, uint8_t &hue);
void pattern_shooting_stars(CRGB *leds, int activeLeds, uint8_t &hue);
void pattern_fireworks(CRGB *leds, int activeLeds, uint8_t &hue);
void pattern_dna(CRGB *leds, int activeLeds, uint8_t &hue);
void pattern_radar(CRGB *leds, int activeLeds, uint8_t &hue);
void pattern_snake(CRGB *leds, int activeLeds, uint8_t &hue);
void pattern_spectrum(CRGB *leds, int activeLeds, uint8_t &hue);
void pattern_sinewave_3d(CRGB *leds, int activeLeds, uint8_t &hue);
void pattern_confetti(CRGB *leds, int activeLeds, uint8_t &hue);
void pattern_breathing(CRGB *leds, int activeLeds, uint8_t &hue);
void pattern_test_card(CRGB *leds, int activeLeds, uint8_t &hue);
void pattern_fireflies(CRGB *leds, int activeLeds, uint8_t &hue);
void pattern_clock(CRGB *leds, int activeLeds, uint8_t &hue, int &scrollOffset,
                   int scrollSpeed);

// Gravity Patterns
void pattern_bouncing_balls(CRGB *leds, int activeLeds, uint8_t &hue);
void pattern_fountain(CRGB *leds, int activeLeds, uint8_t &hue);
void pattern_gravity_rain(CRGB *leds, int activeLeds, uint8_t &hue);

// 32x32 Extras
void pattern_tunnel(CRGB *leds, int activeLeds, uint8_t &hue);
void pattern_kaleidoscope(CRGB *leds, int activeLeds, uint8_t &hue);
void pattern_lissajous(CRGB *leds, int activeLeds, uint8_t &hue);
void pattern_clouds(CRGB *leds, int activeLeds, uint8_t &hue);
void pattern_tartan(CRGB *leds, int activeLeds, uint8_t &hue);
void pattern_polar_waves(CRGB *leds, int activeLeds, uint8_t &hue);
void pattern_swirl(CRGB *leds, int activeLeds, uint8_t &hue);
void pattern_zoom(CRGB *leds, int activeLeds, uint8_t &hue);
void pattern_liquid(CRGB *leds, int activeLeds, uint8_t &hue);
void pattern_rorschach(CRGB *leds, int activeLeds, uint8_t &hue);

// 8x32 Extras
void pattern_cylon(CRGB *leds, int activeLeds, uint8_t &hue);
void pattern_pacman(CRGB *leds, int activeLeds, uint8_t &hue);
void pattern_windy_rain(CRGB *leds, int activeLeds, uint8_t &hue);
void pattern_traffic(CRGB *leds, int activeLeds, uint8_t &hue);
void pattern_ping_pong(CRGB *leds, int activeLeds, uint8_t &hue);
void pattern_plasma_horizontal(CRGB *leds, int activeLeds, uint8_t &hue);
void pattern_equalizer(CRGB *leds, int activeLeds, uint8_t &hue);
void pattern_knight_rider(CRGB *leds, int activeLeds, uint8_t &hue);
void pattern_police(CRGB *leds, int activeLeds, uint8_t &hue);
void pattern_dna_horizontal(CRGB *leds, int activeLeds, uint8_t &hue);

// 32x32 Wow Patterns
void pattern_metaballs(CRGB *leds, int activeLeds, uint8_t &hue);
void pattern_julia(CRGB *leds, int activeLeds, uint8_t &hue);
void pattern_voronoi(CRGB *leds, int activeLeds, uint8_t &hue);
void pattern_star_warp(CRGB *leds, int activeLeds, uint8_t &hue);
void pattern_perlin_fire(CRGB *leds, int activeLeds, uint8_t &hue);
void pattern_water_caustics(CRGB *leds, int activeLeds, uint8_t &hue);
void pattern_hypnotic_squares(CRGB *leds, int activeLeds, uint8_t &hue);
void pattern_spiral_illusion(CRGB *leds, int activeLeds, uint8_t &hue);
void pattern_glitch(CRGB *leds, int activeLeds, uint8_t &hue);

// Unified Gravity Patterns (Top-Down: 0 -> 31)
void pattern_bouncing_balls(CRGB *leds, int activeLeds, uint8_t &hue);
void pattern_fountain(CRGB *leds, int activeLeds, uint8_t &hue);
void pattern_gravity_rain(CRGB *leds, int activeLeds, uint8_t &hue);
void pattern_gravity_sand(CRGB *leds, int activeLeds, uint8_t &hue);
void pattern_gravity_snow(CRGB *leds, int activeLeds, uint8_t &hue);

#endif // PATTERNS_H

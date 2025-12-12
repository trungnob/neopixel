// pattern_116_starfield.cpp
#include "../patterns.h"

// Starfield Parallax - Stars moving at different speeds
// Starfield 3D - Warp speed effect
void pattern_starfield(CRGB* leds, int activeLeds, uint8_t& hue) {
  const int NUM_STARS = 40;
  static float stars[NUM_STARS][3]; // x, y, z
  static bool initialized = false;

  if (!initialized) {
    for(int i=0; i<NUM_STARS; i++) {
      stars[i][0] = random16(-100, 100); // x relative to center
      stars[i][1] = random16(-100, 100); // y relative to center
      stars[i][2] = random16(1, 255);    // z (depth)
    }
    initialized = true;
  }

  fadeToBlackBy(leds, activeLeds, 60); // Trails

  int centerX = GRID_WIDTH / 2;
  int centerY = GRID_HEIGHT / 2;

  for(int i=0; i<NUM_STARS; i++) {
    // Move star closer
    stars[i][2] -= 4.0; // Speed
    
    // Reset if passed viewer
    if (stars[i][2] <= 1) {
      stars[i][0] = random16(-100, 100);
      stars[i][1] = random16(-100, 100);
      stars[i][2] = 255;
    }

    // Project 3D to 2D
    // x2d = x / z
    float k = 128.0 / stars[i][2];
    int x = centerX + (stars[i][0] * k);
    int y = centerY + (stars[i][1] * k);

    if (x >= 0 && x < GRID_WIDTH && y >= 0 && y < GRID_HEIGHT) {
      int led = XY(x, y);
      if (led >= 0) {
        uint8_t brightness = 255 - stars[i][2];
        leds[led] = CRGB(brightness, brightness, brightness);
      }
    }
  }
}

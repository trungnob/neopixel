#include "../patterns.h"

// Fireflies - Wandering, fading yellow-green particles
void pattern_fireflies(CRGB* leds, int activeLeds, uint8_t& hue) {
  const int NUM_FLIES = 15;
  struct Firefly {
    float x, y;
    float vx, vy;
    uint8_t brightness;
    int8_t fadeDir; // 1 = in, -1 = out, 0 = inactive
    uint8_t hue;
  };

  static Firefly flies[NUM_FLIES];
  static bool initialized = false;

  if (!initialized) {
    for(int i=0; i<NUM_FLIES; i++) {
      flies[i].fadeDir = 0; // Start inactive
      flies[i].brightness = 0;
    }
    initialized = true;
  }

  fadeToBlackBy(leds, activeLeds, 20); // Trails

  for(int i=0; i<NUM_FLIES; i++) {
    Firefly& fly = flies[i];

    // State Machine
    if (fly.fadeDir == 0) {
      // Inactive: Chance to spawn
      if (random8() < 5) {
        fly.x = random16(GRID_WIDTH);
        fly.y = random16(GRID_HEIGHT);
        fly.vx = (random8(20) - 10) / 20.0; // Slow random velocity
        fly.vy = (random8(20) - 10) / 20.0;
        fly.brightness = 0;
        fly.fadeDir = 1; // Start fading in
        fly.hue = 45 + random8(20); // Yellow-Green (Chartreuse)
      }
    } else {
      // Active: Move and Fade
      
      // Move
      fly.x += fly.vx;
      fly.y += fly.vy;
      
      // Bounce off walls (or wrap? Bounce is better for flies)
      if (fly.x < 0 || fly.x >= GRID_WIDTH) fly.vx = -fly.vx;
      if (fly.y < 0 || fly.y >= GRID_HEIGHT) fly.vy = -fly.vy;
      
      // Add some jitter
      fly.vx += (random8(10) - 5) / 100.0;
      fly.vy += (random8(10) - 5) / 100.0;
      
      // Clamp velocity
      if (fly.vx > 0.5) fly.vx = 0.5;
      if (fly.vx < -0.5) fly.vx = -0.5;
      if (fly.vy > 0.5) fly.vy = 0.5;
      if (fly.vy < -0.5) fly.vy = -0.5;

      // Fade
      if (fly.fadeDir == 1) {
        if (fly.brightness >= 250) fly.fadeDir = -1; // Start fading out
        else fly.brightness += 5;
      } else {
        if (fly.brightness <= 5) {
          fly.brightness = 0;
          fly.fadeDir = 0; // Reset to inactive
        } else {
          fly.brightness -= 3;
        }
      }

      // Draw
      int led = XY((int)fly.x, (int)fly.y);
      if (led >= 0) {
        leds[led] += CHSV(fly.hue, 255, fly.brightness);
      }
    }
  }
}

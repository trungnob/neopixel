#include "../patterns.h"
#include <Arduino.h>

// Helper for Perlin Noise (simplified or use FastLED's noise)
// FastLED has inoise8(x,y,z)

// 1. 3D Tunnel
// A classic demo effect: a tunnel flying towards you
void pattern_tunnel(CRGB *leds, int activeLeds, uint8_t &hue) {
  static uint16_t zDist = 0;
  zDist += 4; // Speed

  for (int y = 0; y < GRID_HEIGHT; y++) {
    for (int x = 0; x < GRID_WIDTH; x++) {
      // Calculate angle and distance from center
      int dx = x - GRID_WIDTH / 2;
      int dy = y - GRID_HEIGHT / 2;
      int dist = sqrt(dx * dx + dy * dy);
      int angle = (atan2(dy, dx) * 128.0 / PI) + 128; // 0-255

      // Map to texture coordinates
      // u = angle, v = 1/dist
      // Add zDist to v to move forward

      if (dist == 0)
        dist = 1; // Avoid divide by zero

      uint8_t u = angle + hue;
      uint8_t v = (2550 / dist) - zDist; // Perspective projection

      // Checkerboard texture
      uint8_t col = ((u >> 4) ^ (v >> 4)) & 1;

      int idx = XY(x, y);
      if (idx >= 0 && idx < activeLeds) {
        if (col)
          leds[idx] = CHSV(v + hue, 255, 255 - (dist * 4)); // Fade center
        else
          leds[idx] = CRGB::Black;
      }
    }
  }
}

// 2. Kaleidoscope
// 8-way symmetry
void pattern_kaleidoscope(CRGB *leds, int activeLeds, uint8_t &hue) {
  // Draw to one octant and mirror
  static uint16_t z = 0;
  z += 10;

  for (int y = 0; y <= GRID_HEIGHT / 2; y++) {
    for (int x = 0; x <= GRID_WIDTH / 2; x++) {
      if (x >= y) { // First octant
        uint8_t noise = inoise8(x * 30, y * 30, z);
        CRGB color = CHSV(hue + noise, 255, noise);

        // Mirror to all 8 octants
        int pts[8][2] = {{x, y},
                         {y, x},
                         {GRID_WIDTH - 1 - x, y},
                         {GRID_WIDTH - 1 - y, x},
                         {x, GRID_HEIGHT - 1 - y},
                         {y, GRID_HEIGHT - 1 - x},
                         {GRID_WIDTH - 1 - x, GRID_HEIGHT - 1 - y},
                         {GRID_WIDTH - 1 - y, GRID_HEIGHT - 1 - x}};

        for (int i = 0; i < 8; i++) {
          int idx = XY(pts[i][0], pts[i][1]);
          if (idx >= 0 && idx < activeLeds)
            leds[idx] = color;
        }
      }
    }
  }
}

// 3. Lissajous Curves
void pattern_lissajous(CRGB *leds, int activeLeds, uint8_t &hue) {
  fadeToBlackBy(leds, activeLeds, 20);

  static uint16_t t = 0;
  t += 5;

  // 2 curves
  for (int i = 0; i < 2; i++) {
    int x = (GRID_WIDTH / 2) +
            (sin8(t * (2 + i) / 3) - 128) * (GRID_WIDTH / 2 - 2) / 128;
    int y = (GRID_HEIGHT / 2) +
            (cos8(t * (3 - i) / 2) - 128) * (GRID_HEIGHT / 2 - 2) / 128;

    int idx = XY(x, y);
    if (idx >= 0 && idx < activeLeds) {
      leds[idx] += CHSV(hue + i * 128, 255, 255);
    }
  }
}

// 4. Perlin Noise Clouds
void pattern_clouds(CRGB *leds, int activeLeds, uint8_t &hue) {
  static uint16_t z = 0;
  z += 2;

  for (int x = 0; x < GRID_WIDTH; x++) {
    for (int y = 0; y < GRID_HEIGHT; y++) {
      uint8_t noise = inoise8(x * 20, y * 20, z);
      int idx = XY(x, y);
      if (idx >= 0 && idx < activeLeds) {
        // Map noise to blue-white palette
        leds[idx] = ColorFromPalette(CloudColors_p, noise, 255);
      }
    }
  }
}

// 5. Tartan / Plaid
void pattern_tartan(CRGB *leds, int activeLeds, uint8_t &hue) {
  static uint16_t pos = 0;
  pos++;

  for (int x = 0; x < GRID_WIDTH; x++) {
    for (int y = 0; y < GRID_HEIGHT; y++) {
      uint8_t col = sin8(x * 16 + pos) + cos8(y * 16 - pos);
      int idx = XY(x, y);
      if (idx >= 0 && idx < activeLeds) {
        leds[idx] = CHSV(col + hue, 200, 255);
      }
    }
  }
}

// 6. Polar Waves
void pattern_polar_waves(CRGB *leds, int activeLeds, uint8_t &hue) {
  static uint16_t offset = 0;
  offset -= 5;

  int cx = GRID_WIDTH / 2;
  int cy = GRID_HEIGHT / 2;

  for (int x = 0; x < GRID_WIDTH; x++) {
    for (int y = 0; y < GRID_HEIGHT; y++) {
      int dx = x - cx;
      int dy = y - cy;
      int dist = sqrt(dx * dx + dy * dy);

      uint8_t val = sin8(dist * 16 + offset);
      int idx = XY(x, y);
      if (idx >= 0 && idx < activeLeds) {
        leds[idx] = CHSV(hue + dist * 4, 255, val);
      }
    }
  }
}

// 7. Swirl
void pattern_swirl(CRGB *leds, int activeLeds, uint8_t &hue) {
  static uint16_t t = 0;
  t += 4;

  int cx = GRID_WIDTH / 2;
  int cy = GRID_HEIGHT / 2;

  for (int x = 0; x < GRID_WIDTH; x++) {
    for (int y = 0; y < GRID_HEIGHT; y++) {
      int dx = x - cx;
      int dy = y - cy;
      int angle = (atan2(dy, dx) * 128.0 / PI) + 128;
      int dist = sqrt(dx * dx + dy * dy);

      // Swirl equation: angle + dist
      uint8_t val = sin8(angle * 3 + dist * 8 - t);

      int idx = XY(x, y);
      if (idx >= 0 && idx < activeLeds) {
        leds[idx] = CHSV(hue + val, 255, val);
      }
    }
  }
}

// 8. Zoom Texture
void pattern_zoom(CRGB *leds, int activeLeds, uint8_t &hue) {
  static uint16_t scale = 0;
  scale += 2; // Oscillating scale would be better but linear zoom is okay

  uint8_t s = sin8(scale);            // 0-255
  int zoom = map(s, 0, 255, 10, 100); // 1.0x to 10.0x

  for (int x = 0; x < GRID_WIDTH; x++) {
    for (int y = 0; y < GRID_HEIGHT; y++) {
      // Zoom into center
      int cx = (x - GRID_WIDTH / 2) * 100 / zoom;
      int cy = (y - GRID_HEIGHT / 2) * 100 / zoom;

      // Checkerboard pattern in zoomed space
      uint8_t col = ((cx + 1000) / 4 ^ (cy + 1000) / 4) & 1;

      int idx = XY(x, y);
      if (idx >= 0 && idx < activeLeds) {
        leds[idx] = col ? CRGB(CHSV(hue + cx + cy, 255, 255)) : CRGB::Black;
      }
    }
  }
}

// 9. Liquid / Blobs
void pattern_liquid(CRGB *leds, int activeLeds, uint8_t &hue) {
  static uint16_t t = 0;
  t += 3;

  for (int x = 0; x < GRID_WIDTH; x++) {
    for (int y = 0; y < GRID_HEIGHT; y++) {
      // Sum of 3 sine waves
      uint8_t val =
          sin8(x * 10 + t) + sin8(y * 10 - t) + sin8((x + y) * 10 + t);

      // Threshold for blob effect
      int idx = XY(x, y);
      if (idx >= 0 && idx < activeLeds) {
        if (val > 200)
          leds[idx] = CHSV(hue, 255, 255);
        else if (val > 180)
          leds[idx] = CHSV(hue, 255, 100); // Edge
        else
          leds[idx] = CRGB::Black;
      }
    }
  }
}

// 10. Rorschach (Symmetrical Noise)
void pattern_rorschach(CRGB *leds, int activeLeds, uint8_t &hue) {
  static uint16_t z = 0;
  z += 1;

  for (int x = 0; x <= GRID_WIDTH / 2; x++) {
    for (int y = 0; y < GRID_HEIGHT; y++) {
      uint8_t noise = inoise8(x * 30, y * 10, z);

      // Threshold
      CRGB color =
          (noise > 128) ? CRGB(CHSV(hue + noise, 255, 255)) : CRGB::Black;

      // Mirror X
      int idx1 = XY(x, y);
      int idx2 = XY(GRID_WIDTH - 1 - x, y);

      if (idx1 >= 0 && idx1 < activeLeds)
        leds[idx1] = color;
      if (idx2 >= 0 && idx2 < activeLeds)
        leds[idx2] = color;
    }
  }
}

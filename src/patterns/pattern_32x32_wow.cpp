#include "../patterns.h"
#include <Arduino.h>

// Pre-calculated maps for radial patterns (32x32 max)
static uint8_t angleMap[32][32];
static uint8_t distMap[32][32];
static bool mapsInitialized = false;

void initMaps() {
  if (mapsInitialized)
    return;
  int cx = GRID_WIDTH / 2;
  int cy = GRID_HEIGHT / 2;
  for (int x = 0; x < GRID_WIDTH && x < 32; x++) {
    for (int y = 0; y < GRID_HEIGHT && y < 32; y++) {
      int dx = x - cx;
      int dy = y - cy;
      angleMap[x][y] = (atan2(dy, dx) * 128.0 / PI) + 128;
      distMap[x][y] = sqrt(dx * dx + dy * dy);
    }
  }
  mapsInitialized = true;
}

// 1. Metaballs (Optimized)
void pattern_metaballs(CRGB *leds, int activeLeds, uint8_t &hue) {
  static uint16_t t = 0;
  t += 5;

  // 3 balls
  int x1 = (GRID_WIDTH / 2) + (sin8(t) - 128) * (GRID_WIDTH / 2 - 2) / 128;
  int y1 = (GRID_HEIGHT / 2) + (cos8(t) - 128) * (GRID_HEIGHT / 2 - 2) / 128;

  int x2 = (GRID_WIDTH / 2) + (sin8(t + 80) - 128) * (GRID_WIDTH / 2 - 2) / 128;
  int y2 =
      (GRID_HEIGHT / 2) + (cos8(t * 2) - 128) * (GRID_HEIGHT / 2 - 2) / 128;

  int x3 = (GRID_WIDTH / 2) + (sin8(t * 3) - 128) * (GRID_WIDTH / 2 - 2) / 128;
  int y3 =
      (GRID_HEIGHT / 2) + (cos8(t + 120) - 128) * (GRID_HEIGHT / 2 - 2) / 128;

  for (int x = 0; x < GRID_WIDTH; x++) {
    for (int y = 0; y < GRID_HEIGHT; y++) {
      // Approximation: Use Manhattan distance for speed? No, metaballs need
      // smooth. Use 1/d approximation or just simple distance sum To avoid
      // sqrt, we can use FastLED's qadd8 or similar, but let's just try to
      // minimize float ops. Actually, standard int sqrt is fast enough if we
      // don't do it too much. But for 1024 pixels * 3 balls, it's a lot. Let's
      // use a simpler field function: 255 / (dx*dx + dy*dy + 1)

      long d1 = (x - x1) * (x - x1) + (y - y1) * (y - y1);
      long d2 = (x - x2) * (x - x2) + (y - y2) * (y - y2);
      long d3 = (x - x3) * (x - x3) + (y - y3) * (y - y3);

      // Avoid divide by zero
      int val = (1000 / (d1 + 1)) + (1000 / (d2 + 1)) + (1000 / (d3 + 1));

      int idx = XY(x, y);
      if (idx >= 0 && idx < activeLeds) {
        leds[idx] = CHSV(hue + val * 4, 255, constrain(val, 0, 255));
      }
    }
    yield(); // Prevent WDT reset
  }
}

// 2. Julia Set (Simplified)
void pattern_julia(CRGB *leds, int activeLeds, uint8_t &hue) {
  static float t = 0;
  t += 0.02;

  float cx = -0.8 + 0.15 * sin(t);
  float cy = 0.156 + 0.15 * cos(t);

  for (int x = 0; x < GRID_WIDTH; x++) {
    for (int y = 0; y < GRID_HEIGHT; y++) {
      // Optimization: Reduce max iterations or resolution
      float zx = 1.5 * (x - GRID_WIDTH / 2) / (0.5 * GRID_WIDTH);
      float zy = (y - GRID_HEIGHT / 2) / (0.5 * GRID_HEIGHT);

      int i = 0;
      for (; i < 12; i++) { // Reduced iterations for speed
        float xt = zx * zx - zy * zy + cx;
        zy = 2.0 * zx * zy + cy;
        zx = xt;
        if (zx * zx + zy * zy > 4.0)
          break;
      }

      int idx = XY(x, y);
      if (idx >= 0 && idx < activeLeds) {
        leds[idx] = CHSV(hue + i * 16, 255, i * 16);
      }
    }
    yield(); // Prevent WDT reset
  }
}

// 3. Voronoi (Optimized - Manhattan Distance)
void pattern_voronoi(CRGB *leds, int activeLeds, uint8_t &hue) {
  static uint16_t t = 0;
  t += 2;

  // 4 moving points
  int px[4], py[4];
  for (int i = 0; i < 4; i++) {
    px[i] = (GRID_WIDTH / 2) +
            (sin8(t * (i + 1) + i * 40) - 128) * (GRID_WIDTH / 2) / 128;
    py[i] = (GRID_HEIGHT / 2) +
            (cos8(t * (i + 1) + i * 40) - 128) * (GRID_HEIGHT / 2) / 128;
  }

  for (int x = 0; x < GRID_WIDTH; x++) {
    for (int y = 0; y < GRID_HEIGHT; y++) {
      int minDist = 255;
      int closest = 0;

      for (int i = 0; i < 4; i++) {
        // Manhattan distance: |x1-x2| + |y1-y2| (Much faster than sqrt)
        int d = abs(x - px[i]) + abs(y - py[i]);
        if (d < minDist) {
          minDist = d;
          closest = i;
        }
      }

      int idx = XY(x, y);
      if (idx >= 0 && idx < activeLeds) {
        // Color based on closest point
        leds[idx] = CHSV(hue + closest * 64, 255, 255 - minDist * 4);
      }
    }
    yield(); // Prevent WDT reset
  }
}

// 4. Star Warp (Fixed & Optimized)
void pattern_star_warp(CRGB *leds, int activeLeds, uint8_t &hue) {
  initMaps();
  static uint16_t z = 0;
  z += 20;

  for (int x = 0; x < GRID_WIDTH; x++) {
    for (int y = 0; y < GRID_HEIGHT; y++) {
      // Use pre-calculated maps
      uint8_t angle = angleMap[x % 32][y % 32];
      uint8_t dist = distMap[x % 32][y % 32];

      if (dist == 0)
        dist = 1;
      uint16_t depth = (2000 / dist) - z;

      uint8_t noise = inoise8(angle, depth);

      int idx = XY(x, y);
      if (idx >= 0 && idx < activeLeds) {
        // Threshold for stars - Lowered to 200 for better visibility
        leds[idx] = (noise > 200) ? CRGB::White : CRGB::Black;
      }
    }
    yield(); // Prevent WDT reset
  }
}

// 5. Perlin Fire
void pattern_perlin_fire(CRGB *leds, int activeLeds, uint8_t &hue) {
  static uint16_t t = 0;
  t += 10;
  static uint16_t y_shift = 0;
  y_shift += 5;

  for (int x = 0; x < GRID_WIDTH; x++) {
    for (int y = 0; y < GRID_HEIGHT; y++) {
      // Noise moves up
      uint8_t noise = inoise8(x * 20, (GRID_HEIGHT - y) * 20 + y_shift, t);

      // Fade out near top
      uint8_t fade = 255 - (y * 255 / GRID_HEIGHT);
      noise = scale8(noise, fade);

      int idx = XY(x, y);
      if (idx >= 0 && idx < activeLeds) {
        leds[idx] = ColorFromPalette(HeatColors_p, noise);
      }
    }
  }
}

// 6. Water Caustics
void pattern_water_caustics(CRGB *leds, int activeLeds, uint8_t &hue) {
  static uint16_t t = 0;
  t += 2;

  for (int x = 0; x < GRID_WIDTH; x++) {
    for (int y = 0; y < GRID_HEIGHT; y++) {
      uint8_t noise = inoise8(x * 20, y * 20, t);
      uint8_t noise2 = inoise8(x * 20 + 1000, y * 20 + 1000, t + 1000);

      // Multiply noise layers
      uint8_t val = scale8(noise, noise2) * 2; // Brighten

      int idx = XY(x, y);
      if (idx >= 0 && idx < activeLeds) {
        leds[idx] = CHSV(160, 255 - val / 2, val); // Blue-ish
      }
    }
  }
}

// 7. Hypnotic Squares
void pattern_hypnotic_squares(CRGB *leds, int activeLeds, uint8_t &hue) {
  static uint16_t t = 0;
  t += 4;

  int cx = GRID_WIDTH / 2;
  int cy = GRID_HEIGHT / 2;

  for (int x = 0; x < GRID_WIDTH; x++) {
    for (int y = 0; y < GRID_HEIGHT; y++) {
      // Chebyshev distance (max(|dx|, |dy|)) gives squares
      int dist = max(abs(x - cx), abs(y - cy));

      uint8_t val = sin8(dist * 20 - t);

      int idx = XY(x, y);
      if (idx >= 0 && idx < activeLeds) {
        leds[idx] =
            (val > 128) ? CRGB(CHSV(hue + dist * 10, 255, 255)) : CRGB::Black;
      }
    }
  }
}

// 8. Spiral Illusion (Optimized)
void pattern_spiral_illusion(CRGB *leds, int activeLeds, uint8_t &hue) {
  initMaps();
  static uint16_t t = 0;
  t -= 8; // Rotate

  for (int x = 0; x < GRID_WIDTH; x++) {
    for (int y = 0; y < GRID_HEIGHT; y++) {
      // Use pre-calculated maps
      uint8_t angle = angleMap[x % 32][y % 32];
      uint8_t dist = distMap[x % 32][y % 32];

      // Spiral: angle + dist
      uint8_t val = sin8(angle * 4 + dist * 4 + t);

      int idx = XY(x, y);
      if (idx >= 0 && idx < activeLeds) {
        leds[idx] = (val > 128) ? CRGB::White : CRGB::Black;
      }
    }
  }
}

// 9. Glitch Art
void pattern_glitch(CRGB *leds, int activeLeds, uint8_t &hue) {
  // Random blocks
  if (random8() < 20) {
    int w = random(5, 15);
    int h = random(1, 4);
    int x = random(GRID_WIDTH - w);
    int y = random(GRID_HEIGHT - h);
    CRGB col = CHSV(random8(), 255, 255);

    for (int i = x; i < x + w; i++) {
      for (int j = y; j < y + h; j++) {
        int idx = XY(i, j);
        if (idx >= 0 && idx < activeLeds)
          leds[idx] = col;
      }
    }
  }

  // Shift lines randomly
  if (random8() < 40) {
    int y = random(GRID_HEIGHT);
    int shift = random(1, 5);
    for (int x = 0; x < GRID_WIDTH; x++) {
      int idx = XY((x + shift) % GRID_WIDTH, y);
      if (idx >= 0 && idx < activeLeds)
        leds[idx] = (random8() > 128) ? CRGB::White : CRGB::Black;
    }
  }

  fadeToBlackBy(leds, activeLeds, 10);
}

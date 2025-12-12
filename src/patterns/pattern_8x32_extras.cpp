#include "../patterns.h"
#include <Arduino.h>

// 1. Cylon / Scanner
void pattern_cylon(CRGB *leds, int activeLeds, uint8_t &hue) {
  fadeToBlackBy(leds, activeLeds, 20);
  int pos = beatsin16(30, 0, GRID_WIDTH - 1);

  for (int y = 0; y < GRID_HEIGHT; y++) {
    int idx = XY(pos, y);
    if (idx >= 0 && idx < activeLeds)
      leds[idx] += CHSV(hue, 255, 255);
  }
}

// 2. Pacman
void pattern_pacman(CRGB *leds, int activeLeds, uint8_t &hue) {
  static int pos = 0;
  static int mouthOpen = 0;
  static int dir = 1;

  EVERY_N_MILLISECONDS(100) {
    pos += dir;
    mouthOpen = !mouthOpen;
    if (pos >= GRID_WIDTH)
      pos = 0;
  }

  fill_solid(leds, activeLeds, CRGB::Black);

  // Draw Dots
  for (int x = 0; x < GRID_WIDTH; x += 4) {
    if (x > pos || (pos > GRID_WIDTH - 5 && x < 5)) { // Wrap logic simplified
      int idx = XY(x, GRID_HEIGHT / 2);
      if (idx >= 0)
        leds[idx] = CRGB::White;
    }
  }

  // Draw Pacman
  int idx = XY(pos, GRID_HEIGHT / 2);
  if (idx >= 0)
    leds[idx] = CRGB::Yellow;
  // Make him bigger (2x2)
  if (GRID_HEIGHT >= 2) {
    int idx2 = XY(pos, GRID_HEIGHT / 2 - 1);
    int idx3 = XY(pos, GRID_HEIGHT / 2 + 1);
    int idx4 = XY(pos - 1, GRID_HEIGHT / 2);
    if (idx2 >= 0)
      leds[idx2] = CRGB::Yellow;
    if (idx3 >= 0)
      leds[idx3] = CRGB::Yellow;
    if (idx4 >= 0)
      leds[idx4] = CRGB::Yellow;
  }
}

// 3. Windy Rain (Horizontal)
void pattern_windy_rain(CRGB *leds, int activeLeds, uint8_t &hue) {
  fadeToBlackBy(leds, activeLeds, 20);
  if (random8() < 40) {
    int y = random(GRID_HEIGHT);
    int idx = XY(0, y);
    if (idx >= 0)
      leds[idx] = CHSV(hue, 255, 255);
  }

  // Shift everything right
  for (int y = 0; y < GRID_HEIGHT; y++) {
    for (int x = GRID_WIDTH - 1; x > 0; x--) {
      int idx1 = XY(x, y);
      int idx2 = XY(x - 1, y);
      if (idx1 >= 0 && idx2 >= 0)
        leds[idx1] = leds[idx2];
    }
    int idx0 = XY(0, y);
    if (idx0 >= 0)
      leds[idx0] = CRGB::Black; // Clear start
  }
}

// 4. Traffic
void pattern_traffic(CRGB *leds, int activeLeds, uint8_t &hue) {
  fadeToBlackBy(leds, activeLeds, 40);

  // Lane 1: Right
  if (random8() < 10) {
    int idx = XY(0, 2);
    if (idx >= 0)
      leds[idx] += CRGB::Red;
  }
  // Lane 2: Left
  if (random8() < 10) {
    int idx = XY(GRID_WIDTH - 1, 5);
    if (idx >= 0)
      leds[idx] += CRGB::White;
  }

  EVERY_N_MILLISECONDS(50) {
    // Move Lane 1 Right
    for (int x = GRID_WIDTH - 1; x > 0; x--) {
      int idx1 = XY(x, 2);
      int idx2 = XY(x - 1, 2);
      if (idx1 >= 0 && idx2 >= 0)
        leds[idx1] = leds[idx2];
    }
    int idx0 = XY(0, 2);
    if (idx0 >= 0)
      leds[idx0] = CRGB::Black;

    // Move Lane 2 Left
    for (int x = 0; x < GRID_WIDTH - 1; x++) {
      int idx1 = XY(x, 5);
      int idx2 = XY(x + 1, 5);
      if (idx1 >= 0 && idx2 >= 0)
        leds[idx1] = leds[idx2];
    }
    int idxEnd = XY(GRID_WIDTH - 1, 5);
    if (idxEnd >= 0)
      leds[idxEnd] = CRGB::Black;
  }
}

// 5. Ping Pong
void pattern_ping_pong(CRGB *leds, int activeLeds, uint8_t &hue) {
  fadeToBlackBy(leds, activeLeds, 30);
  static float x = 0;
  static float y = GRID_HEIGHT / 2.0;
  static float vx = 0.5;
  static float vy = 0.2;

  x += vx;
  y += vy;

  if (x <= 0 || x >= GRID_WIDTH - 1)
    vx = -vx;
  if (y <= 0 || y >= GRID_HEIGHT - 1)
    vy = -vy;

  int idx = XY((int)x, (int)y);
  if (idx >= 0 && idx < activeLeds)
    leds[idx] = CRGB::White;

  // Paddles
  int padY = (int)y;
  int idxP1 = XY(0, padY);
  int idxP2 = XY(GRID_WIDTH - 1, padY);
  if (idxP1 >= 0)
    leds[idxP1] = CRGB::Blue;
  if (idxP2 >= 0)
    leds[idxP2] = CRGB::Red;
}

// 6. Horizontal Plasma
void pattern_plasma_horizontal(CRGB *leds, int activeLeds, uint8_t &hue) {
  static uint16_t t = 0;
  t += 2;
  for (int x = 0; x < GRID_WIDTH; x++) {
    uint8_t col = sin8(x * 10 + t);
    for (int y = 0; y < GRID_HEIGHT; y++) {
      uint8_t col2 = sin8(y * 30 + t * 2);
      int idx = XY(x, y);
      if (idx >= 0 && idx < activeLeds) {
        leds[idx] = CHSV(hue + col + col2, 255, 255);
      }
    }
  }
}

// 7. Fake Equalizer
void pattern_equalizer(CRGB *leds, int activeLeds, uint8_t &hue) {
  fadeToBlackBy(leds, activeLeds, 50);
  EVERY_N_MILLISECONDS(50) {
    for (int x = 0; x < GRID_WIDTH; x++) {
      int height = random(GRID_HEIGHT);
      for (int y = 0; y < height; y++) {
        int idx = XY(x, GRID_HEIGHT - 1 - y); // Grow from bottom
        if (idx >= 0 && idx < activeLeds) {
          leds[idx] = CHSV(hue + (x * 5), 255, 255);
        }
      }
    }
  }
}

// 8. Knight Rider (Dual)
void pattern_knight_rider(CRGB *leds, int activeLeds, uint8_t &hue) {
  fadeToBlackBy(leds, activeLeds, 40);
  int pos = beatsin16(20, 0, GRID_WIDTH - 1);
  int pos2 = beatsin16(25, 0, GRID_WIDTH - 1); // Faster second scanner

  for (int y = 0; y < GRID_HEIGHT; y++) {
    if (y == 2 || y == 3) {
      int idx = XY(pos, y);
      if (idx >= 0)
        leds[idx] += CRGB::Red;
    }
    if (y == 5 || y == 6) {
      int idx = XY(pos2, y);
      if (idx >= 0)
        leds[idx] += CRGB::Blue;
    }
  }
}

// 9. Police Strobes
void pattern_police(CRGB *leds, int activeLeds, uint8_t &hue) {
  static int state = 0;
  EVERY_N_MILLISECONDS(100) {
    state++;
    if (state > 3)
      state = 0;
  }

  fill_solid(leds, activeLeds, CRGB::Black);

  if (state == 0) {
    // Left Red
    for (int x = 0; x < GRID_WIDTH / 2; x++) {
      for (int y = 0; y < GRID_HEIGHT; y++) {
        int idx = XY(x, y);
        if (idx >= 0)
          leds[idx] = CRGB::Red;
      }
    }
  } else if (state == 2) {
    // Right Blue
    for (int x = GRID_WIDTH / 2; x < GRID_WIDTH; x++) {
      for (int y = 0; y < GRID_HEIGHT; y++) {
        int idx = XY(x, y);
        if (idx >= 0)
          leds[idx] = CRGB::Blue;
      }
    }
  }
}

// 10. Horizontal DNA
void pattern_dna_horizontal(CRGB *leds, int activeLeds, uint8_t &hue) {
  fadeToBlackBy(leds, activeLeds, 20);
  static uint16_t t = 0;
  t += 5;

  for (int x = 0; x < GRID_WIDTH; x++) {
    int y1 = (GRID_HEIGHT / 2) +
             (sin8(x * 16 + t) - 128) * (GRID_HEIGHT / 2 - 1) / 128;
    int y2 = (GRID_HEIGHT / 2) +
             (sin8(x * 16 + t + 128) - 128) * (GRID_HEIGHT / 2 - 1) / 128;

    int idx1 = XY(x, y1);
    int idx2 = XY(x, y2);

    if (idx1 >= 0 && idx1 < activeLeds)
      leds[idx1] = CRGB::Red;
    if (idx2 >= 0 && idx2 < activeLeds)
      leds[idx2] = CRGB::Blue;
  }
}

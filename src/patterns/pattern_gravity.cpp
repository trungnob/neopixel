#include "../patterns.h"
#include <Arduino.h>

// 1. Bouncing Balls (Top-Down)
// Balls bounce on the bottom (Panel 31)
void pattern_bouncing_balls(CRGB *leds, int activeLeds, uint8_t &hue) {
  fadeToBlackBy(leds, activeLeds, 20);

  const int NUM_BALLS = 5;
  static struct Ball {
    float y;
    float v;
    float x;
    uint8_t color;
    float gravity;
    float elasticity;
  } balls[NUM_BALLS];

  static bool initialized = false;
  if (!initialized) {
    for (int i = 0; i < NUM_BALLS; i++) {
      balls[i].x = random(GRID_WIDTH);
      balls[i].y = random(GRID_HEIGHT / 2); // Start top half
      balls[i].v = 0;
      balls[i].gravity = 0.2 + (random(10) / 50.0); // Positive gravity (Down)
      balls[i].elasticity = 0.8 + (random(10) / 100.0);
      balls[i].color = i * (255 / NUM_BALLS);
    }
    initialized = true;
  }

  for (int i = 0; i < NUM_BALLS; i++) {
    // Physics
    balls[i].v += balls[i].gravity;
    balls[i].y += balls[i].v;

    // Bounce Floor (Bottom = GRID_HEIGHT-1)
    if (balls[i].y >= GRID_HEIGHT - 1) {
      balls[i].y = GRID_HEIGHT - 1;
      balls[i].v = -balls[i].v * balls[i].elasticity;

      if (abs(balls[i].v) < 0.5) {
        balls[i].y = 0; // Reset to top
        balls[i].v = 0;
        balls[i].x = random(GRID_WIDTH);
      }
    }

    int y = (int)(balls[i].y + 0.5);
    int x = (int)balls[i].x;

    int idx = XY(x, y);
    if (idx >= 0 && idx < activeLeds)
      leds[idx] = CHSV(balls[i].color + hue, 255, 255);
  }
  yield();
}

// 2. Fountain / Water Jet (Top-Down)
// Shoots from Top (0) downwards
void pattern_fountain(CRGB *leds, int activeLeds, uint8_t &hue) {
  fadeToBlackBy(leds, activeLeds, 30);

  const int NUM_PARTICLES = 20;
  static struct Particle {
    float x, y;
    float vx, vy;
    uint8_t hue;
    bool active;
  } parts[NUM_PARTICLES];

  static bool initialized = false;
  if (!initialized) {
    for (int i = 0; i < NUM_PARTICLES; i++)
      parts[i].active = false;
    initialized = true;
  }

  // Spawn from Top Center
  if (random8() < 50) {
    for (int i = 0; i < NUM_PARTICLES; i++) {
      if (!parts[i].active) {
        parts[i].active = true;
        parts[i].x = GRID_WIDTH / 2.0;
        parts[i].y = 0;                          // Start at top
        parts[i].vx = (random(100) - 50) / 50.0; // Random spread X
        parts[i].vy = random(10, 30) / 10.0;     // Downward velocity (Positive)
        parts[i].hue = hue + random(32);
        break;
      }
    }
  }

  // Update
  float gravity = 0.15; // Pulls down (Positive)
  for (int i = 0; i < NUM_PARTICLES; i++) {
    if (parts[i].active) {
      parts[i].x += parts[i].vx;
      parts[i].y += parts[i].vy;
      parts[i].vy += gravity; // Accelerate down

      // Deactivate if out of bounds
      if (parts[i].y >= GRID_HEIGHT || parts[i].x < 0 ||
          parts[i].x >= GRID_WIDTH) {
        parts[i].active = false;
      } else {
        int idx = XY((int)parts[i].x, (int)parts[i].y);
        if (idx >= 0 && idx < activeLeds) {
          leds[idx] = CHSV(parts[i].hue, 255, 255);
        }
      }
    }
  }
  yield();
}

// 3. Gravity Rain (Top-Down)
// Falls from 0 to 31
void pattern_gravity_rain(CRGB *leds, int activeLeds, uint8_t &hue) {
  fadeToBlackBy(leds, activeLeds, 20);

  const int NUM_DROPS = 10;
  static struct Drop {
    float y;
    float v;
    int x;
    bool active;
  } drops[NUM_DROPS];

  static bool initialized = false;
  if (!initialized) {
    for (int i = 0; i < NUM_DROPS; i++)
      drops[i].active = false;
    initialized = true;
  }

  // Spawn
  if (random8() < 30) {
    for (int i = 0; i < NUM_DROPS; i++) {
      if (!drops[i].active) {
        drops[i].active = true;
        drops[i].x = random(GRID_WIDTH);
        drops[i].y = 0; // Start at top
        drops[i].v = 0;
        break;
      }
    }
  }

  // Update
  float gravity = 0.2; // Positive (Down)

  for (int i = 0; i < NUM_DROPS; i++) {
    if (drops[i].active) {
      drops[i].v += gravity;
      drops[i].y += drops[i].v; // y increases (goes down)

      if (drops[i].y >= GRID_HEIGHT) {
        drops[i].active = false; // Splash/Die
        // Splash effect at bottom
        int idx = XY(drops[i].x, GRID_HEIGHT - 1);
        if (idx >= 0 && idx < activeLeds)
          leds[idx] = CRGB::White;
      } else {
        int idx = XY(drops[i].x, (int)drops[i].y);
        if (idx >= 0 && idx < activeLeds) {
          leds[idx] = CHSV(hue, 255, 255);
        }
      }
    }
  }
  yield();
}

// 4. Gravity Sand (Top-Down)
// Particles fall and pile up at bottom
void pattern_gravity_sand(CRGB *leds, int activeLeds, uint8_t &hue) {
  // We need a grid to track piled sand
  static uint8_t grid[32][32];      // 0=empty, 1=sand
  static uint8_t colorGrid[32][32]; // Store hue
  static bool init = false;

  if (!init) {
    memset(grid, 0, sizeof(grid));
    init = true;
  }

  // Spawn
  if (random8() < 100) {
    int x = random(GRID_WIDTH);
    if (grid[x][0] == 0) {
      grid[x][0] = 1; // Active falling sand
      colorGrid[x][0] = hue + random(32);
    }
  }

  // Update (Bottom to Top to avoid overwriting)
  // We scan from bottom-1 up to 0
  for (int y = GRID_HEIGHT - 2; y >= 0; y--) {
    for (int x = 0; x < GRID_WIDTH; x++) {
      if (grid[x][y] == 1) { // If sand is here
        // Try to move down
        if (grid[x][y + 1] == 0) {
          grid[x][y + 1] = 1;
          colorGrid[x][y + 1] = colorGrid[x][y];
          grid[x][y] = 0;
        }
        // Try down-left
        else if (x > 0 && grid[x - 1][y + 1] == 0) {
          grid[x - 1][y + 1] = 1;
          colorGrid[x - 1][y + 1] = colorGrid[x][y];
          grid[x][y] = 0;
        }
        // Try down-right
        else if (x < GRID_WIDTH - 1 && grid[x + 1][y + 1] == 0) {
          grid[x + 1][y + 1] = 1;
          colorGrid[x + 1][y + 1] = colorGrid[x][y];
          grid[x][y] = 0;
        }
        // Else stay (pile up)
        else {
          grid[x][y] = 2; // Resting sand
        }
      }
    }
  }

  // Render
  fill_solid(leds, activeLeds, CRGB::Black);
  for (int x = 0; x < GRID_WIDTH; x++) {
    for (int y = 0; y < GRID_HEIGHT; y++) {
      if (grid[x][y] > 0) {
        int idx = XY(x, y);
        if (idx >= 0 && idx < activeLeds) {
          leds[idx] = CHSV(colorGrid[x][y], 255, 255);
        }
      }
    }
  }

  // Reset if full
  if (grid[GRID_WIDTH / 2][0] == 2) {
    memset(grid, 0, sizeof(grid));
  }

  yield();
}

// 5. Gravity Snow (Top-Down)
// Drifting snow
void pattern_gravity_snow(CRGB *leds, int activeLeds, uint8_t &hue) {
  fadeToBlackBy(leds, activeLeds, 10);

  const int NUM_FLAKES = 30;
  static struct Flake {
    float x, y;
    float speed;
    bool active;
  } flakes[NUM_FLAKES];

  static bool init = false;
  if (!init) {
    for (int i = 0; i < NUM_FLAKES; i++)
      flakes[i].active = false;
    init = true;
  }

  // Spawn
  if (random8() < 20) {
    for (int i = 0; i < NUM_FLAKES; i++) {
      if (!flakes[i].active) {
        flakes[i].active = true;
        flakes[i].x = random(GRID_WIDTH);
        flakes[i].y = 0;
        flakes[i].speed = 0.05 + random(10) / 100.0;
        break;
      }
    }
  }

  // Update
  for (int i = 0; i < NUM_FLAKES; i++) {
    if (flakes[i].active) {
      flakes[i].y += flakes[i].speed;
      flakes[i].x += (random(3) - 1) * 0.1; // Drift

      if (flakes[i].y >= GRID_HEIGHT) {
        flakes[i].active = false;
      } else {
        int idx = XY((int)flakes[i].x, (int)flakes[i].y);
        if (idx >= 0 && idx < activeLeds) {
          leds[idx] = CRGB::White;
        }
      }
    }
  }
  yield();
}

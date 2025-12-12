#include "../patterns.h"
#include <Arduino.h>

// Helper for Hypnotic Rings
void pattern_hypnotic_rings(CRGB *leds, int activeLeds, uint8_t &hue) {
  fadeToBlackBy(leds, activeLeds, 20);

  int centerX = GRID_WIDTH / 2;
  int centerY = GRID_HEIGHT / 2;

  // Use system time for expansion
  uint16_t t = millis() / 50;

  for (int x = 0; x < GRID_WIDTH; x++) {
    for (int y = 0; y < GRID_HEIGHT; y++) {
      // Calculate distance from center
      int dx = x - centerX;
      int dy = y - centerY;
      int dist = sqrt(dx * dx + dy * dy);

      // Create ring effect
      // (dist * factor) - t creates expanding rings
      uint8_t colorIndex = (dist * 16) - t;

      // Map to LED
      int ledIdx = XY(x, y);
      if (ledIdx >= 0 && ledIdx < activeLeds) {
        // Only light up if it's a "ring" (using sine wave logic or simple
        // modulo) Here we use the colorIndex directly for a smooth rainbow
        // spiral
        leds[ledIdx] += CHSV(colorIndex + hue, 255, 128);
      }
    }
  }
}

// Helper for Noise Lava
// Requires FastLED noise functions
void pattern_noise_lava(CRGB *leds, int activeLeds, uint8_t &hue) {
  // Scale determines how "zoomed in" the noise is
  uint16_t scale = 30;
  // Speed determines how fast the noise moves
  uint16_t speed = 10;

  uint32_t realTime = millis() * speed;

  for (int x = 0; x < GRID_WIDTH; x++) {
    for (int y = 0; y < GRID_HEIGHT; y++) {
      // 16-bit noise function
      uint16_t noise = inoise16(x * scale * 256, y * scale * 256, realTime);

      // Map noise (0-65535) to hue (0-255)
      uint8_t pixelHue = noise >> 8;

      int ledIdx = XY(x, y);
      if (ledIdx >= 0 && ledIdx < activeLeds) {
        // Use the noise value for brightness too to give it texture
        uint8_t bri = map(noise, 0, 65535, 50, 255);
        leds[ledIdx] = CHSV(pixelHue + hue, 255, bri);
      }
    }
  }
}

// Helper for Diagonal Plasma
void pattern_diagonal_plasma(CRGB *leds, int activeLeds, uint8_t &hue) {
  uint16_t t = millis() / 4;

  for (int x = 0; x < GRID_WIDTH; x++) {
    for (int y = 0; y < GRID_HEIGHT; y++) {
      // Combine sine waves
      uint8_t v = sin8(x * 16 + t) + cos8(y * 16 + t) + sin8((x + y) * 16 + t);

      int ledIdx = XY(x, y);
      if (ledIdx >= 0 && ledIdx < activeLeds) {
        leds[ledIdx] = CHSV(v + hue, 255, 255);
      }
    }
  }
}

// --- 10 NEW PATTERNS ---

// 1. Spiral Galaxy
void pattern_spiral_galaxy(CRGB *leds, int activeLeds, uint8_t &hue) {
  fadeToBlackBy(leds, activeLeds, 20);
  int centerX = GRID_WIDTH / 2;
  int centerY = GRID_HEIGHT / 2;
  uint16_t t = millis() / 20;

  for (int i = 0; i < GRID_WIDTH; i++) {
    // 2 spiral arms
    float angle = (float)i * 0.5 + (t * 0.1);
    int x = centerX + (cos(angle) * i / 2);
    int y = centerY + (sin(angle) * i / 2);

    int ledIdx = XY(x, y);
    if (ledIdx >= 0 && ledIdx < activeLeds) {
      leds[ledIdx] = CHSV(hue + i * 5, 255, 255);
    }

    // Second arm
    x = centerX + (cos(angle + PI) * i / 2);
    y = centerY + (sin(angle + PI) * i / 2);
    ledIdx = XY(x, y);
    if (ledIdx >= 0 && ledIdx < activeLeds) {
      leds[ledIdx] = CHSV(hue + i * 5 + 128, 255, 255);
    }
  }
}

// 2. Shooting Stars (Fast Diagonal)
void pattern_shooting_stars(CRGB *leds, int activeLeds, uint8_t &hue) {
  fadeToBlackBy(leds, activeLeds, 20); // Trail length

  const int NUM_STARS = 6;
  static struct Star {
    int x, y;
    int startX, startY; // To calculate path
    uint8_t speed;      // Lower is faster (delay in ms)
    uint32_t lastMove;
    bool active;
  } stars[NUM_STARS];

  static bool initialized = false;
  if (!initialized) {
    for (int i = 0; i < NUM_STARS; i++)
      stars[i].active = false;
    initialized = true;
  }

  // Spawn new stars
  if (random8() < 15) { // Spawn chance
    for (int i = 0; i < NUM_STARS; i++) {
      if (!stars[i].active) {
        stars[i].active = true;
        // Spawn at Top or Left
        if (random8() > 128) {
          stars[i].x = random(GRID_WIDTH);
          stars[i].y = 0;
        } else {
          stars[i].x = 0;
          stars[i].y = random(GRID_HEIGHT);
        }
        stars[i].speed = random(10, 40); // Speed in ms delay
        stars[i].lastMove = millis();
        break;
      }
    }
  }

  // Update and draw
  uint32_t now = millis();
  for (int i = 0; i < NUM_STARS; i++) {
    if (stars[i].active) {
      // Draw Head (White)
      int ledIdx = XY(stars[i].x, stars[i].y);
      if (ledIdx >= 0 && ledIdx < activeLeds) {
        leds[ledIdx] = CRGB::White;
        // Add a little sparkle/tint to the head
        if (random8() < 100)
          leds[ledIdx] += CHSV(hue, 50, 255);
      }

      // Move logic
      if (now - stars[i].lastMove > stars[i].speed) {
        stars[i].lastMove = now;
        stars[i].x++;
        stars[i].y++;

        // Deactivate if off screen
        if (stars[i].x >= GRID_WIDTH || stars[i].y >= GRID_HEIGHT) {
          stars[i].active = false;
        }
      }
    }
  }
}

// ... (Fireworks pattern remains unchanged) ...

// 6. Snake (Auto with Collision & Smarter AI)
void pattern_snake(CRGB *leds, int activeLeds, uint8_t &hue) {
  fadeToBlackBy(leds, activeLeds, 100); // Clear trail

  const int MAX_SNAKE_LENGTH = 50;
  static int snakeX[MAX_SNAKE_LENGTH];
  static int snakeY[MAX_SNAKE_LENGTH];
  static int snakeLen = 3;
  static int dirX = 1, dirY = 0;
  static int foodX = 10, foodY = 4;
  static bool initialized = false;
  static bool gameOver = false;
  static uint32_t gameOverTime = 0;

  // Reset Game Function
  auto resetGame = [&]() {
    for (int i = 0; i < MAX_SNAKE_LENGTH; i++) {
      snakeX[i] = -1;
      snakeY[i] = -1;
    }
    snakeX[0] = 5;
    snakeY[0] = 5;
    snakeLen = 3;
    dirX = 1;
    dirY = 0;
    foodX = random(GRID_WIDTH);
    foodY = random(GRID_HEIGHT);
    gameOver = false;
  };

  if (!initialized) {
    resetGame();
    initialized = true;
  }

  if (gameOver) {
    // Flash Red on Game Over
    fill_solid(leds, activeLeds, CRGB::Red);
    if (millis() - gameOverTime > 1000) {
      resetGame();
    }
    return;
  }

  EVERY_N_MILLISECONDS(100) {
    // AI Logic: Determine next direction
    int nextDirX = dirX;
    int nextDirY = dirY;

    // Try to move towards food
    if (snakeX[0] < foodX) {
      nextDirX = 1;
      nextDirY = 0;
    } else if (snakeX[0] > foodX) {
      nextDirX = -1;
      nextDirY = 0;
    } else if (snakeY[0] < foodY) {
      nextDirX = 0;
      nextDirY = 1;
    } else if (snakeY[0] > foodY) {
      nextDirX = 0;
      nextDirY = -1;
    }

    // Check if this move kills us
    auto isSafe = [&](int dx, int dy) {
      int nx = snakeX[0] + dx;
      int ny = snakeY[0] + dy;
      // Wall Collision
      if (nx < 0 || nx >= GRID_WIDTH || ny < 0 || ny >= GRID_HEIGHT)
        return false;
      // Self Collision
      for (int i = 0; i < snakeLen - 1; i++) { // Don't check tail as it moves
        if (nx == snakeX[i] && ny == snakeY[i])
          return false;
      }
      return true;
    };

    // If preferred move is unsafe, try others
    if (!isSafe(nextDirX, nextDirY)) {
      // Try current direction
      if (isSafe(dirX, dirY)) {
        nextDirX = dirX;
        nextDirY = dirY;
      } else {
        // Try all 4 directions
        if (isSafe(1, 0)) {
          nextDirX = 1;
          nextDirY = 0;
        } else if (isSafe(-1, 0)) {
          nextDirX = -1;
          nextDirY = 0;
        } else if (isSafe(0, 1)) {
          nextDirX = 0;
          nextDirY = 1;
        } else if (isSafe(0, -1)) {
          nextDirX = 0;
          nextDirY = -1;
        }
        // If still unsafe, we accept death
      }
    }

    // Prevent 180 turns
    if (nextDirX == -dirX && nextDirY == -dirY) {
      // Keep current if valid, or pick another
    } else {
      dirX = nextDirX;
      dirY = nextDirY;
    }

    // Move body
    for (int i = snakeLen - 1; i > 0; i--) {
      snakeX[i] = snakeX[i - 1];
      snakeY[i] = snakeY[i - 1];
    }

    // Move head
    snakeX[0] += dirX;
    snakeY[0] += dirY;

    // Check Collisions (Game Over)
    bool collision = false;
    // Wall
    if (snakeX[0] < 0 || snakeX[0] >= GRID_WIDTH || snakeY[0] < 0 ||
        snakeY[0] >= GRID_HEIGHT)
      collision = true;
    // Self
    for (int i = 1; i < snakeLen; i++) {
      if (snakeX[0] == snakeX[i] && snakeY[0] == snakeY[i])
        collision = true;
    }

    if (collision) {
      gameOver = true;
      gameOverTime = millis();
      return;
    }

    // Eat food
    if (snakeX[0] == foodX && snakeY[0] == foodY) {
      foodX = random(GRID_WIDTH);
      foodY = random(GRID_HEIGHT);
      if (snakeLen < MAX_SNAKE_LENGTH)
        snakeLen++;
    }
  }

  // Draw Snake
  for (int i = 0; i < snakeLen; i++) {
    if (snakeX[i] != -1) {
      int idx = XY(snakeX[i], snakeY[i]);
      if (idx >= 0 && idx < activeLeds) {
        if (i == 0)
          leds[idx] = CRGB::Green; // Head
        else
          leds[idx] = CRGB::DarkGreen; // Body
      }
    }
  }

  // Draw Food
  int foodIdx = XY(foodX, foodY);
  if (foodIdx >= 0)
    leds[foodIdx] = CRGB::Red;
}

// 3. Fireworks
void pattern_fireworks(CRGB *leds, int activeLeds, uint8_t &hue) {
  fadeToBlackBy(leds, activeLeds, 30);

  static int fx = -1, fy = -1, fState = 0; // 0=rising, 1=exploding
  static uint8_t fHue = 0;

  if (fState == 0) {
    if (fx == -1) {
      fx = random(GRID_WIDTH);
      fy = GRID_HEIGHT - 1;
      fHue = random8();
    }

    // Draw rocket
    int ledIdx = XY(fx, fy);
    if (ledIdx >= 0)
      leds[ledIdx] = CHSV(fHue, 255, 255);

    fy--;
    if (fy < random(GRID_HEIGHT / 3)) { // Explode height
      fState = 1;
      fy = 0; // Use fy as radius counter
    }
  } else {
    // Explode
    int centerX = fx;
    int centerY = fy + random(GRID_HEIGHT / 3); // Approximate center
    int radius = fy;                            // reusing fy

    for (int i = 0; i < 8; i++) {
      int x = centerX + (cos8(i * 32) * radius) / 255;
      int y = centerY + (sin8(i * 32) * radius) / 255;

      // Simple offset approximation for small radius
      int ox = 0, oy = 0;
      if (i == 0)
        oy = -radius;
      if (i == 1) {
        ox = radius;
        oy = -radius;
      }
      if (i == 2)
        ox = radius;
      if (i == 3) {
        ox = radius;
        oy = radius;
      }
      if (i == 4)
        oy = radius;
      if (i == 5) {
        ox = -radius;
        oy = radius;
      }
      if (i == 6)
        ox = -radius;
      if (i == 7) {
        ox = -radius;
        oy = -radius;
      }

      int ledIdx = XY(centerX + ox, centerY + oy);
      if (ledIdx >= 0 && ledIdx < activeLeds)
        leds[ledIdx] = CHSV(fHue, 255, 255);
    }

    fy++; // Increase radius
    if (fy > 4) {
      fState = 0;
      fx = -1;
    }
  }
  EVERY_N_MILLISECONDS(100) {}
}

// 4. DNA Helix
void pattern_dna(CRGB *leds, int activeLeds, uint8_t &hue) {
  fadeToBlackBy(leds, activeLeds, 50);
  uint16_t t = millis() / 20;

  for (int y = 0; y < GRID_HEIGHT; y++) {
    int x1 =
        (GRID_WIDTH / 2) + (sin8(y * 16 + t) - 128) * (GRID_WIDTH / 4) / 128;
    int x2 = (GRID_WIDTH / 2) +
             (sin8(y * 16 + t + 128) - 128) * (GRID_WIDTH / 4) / 128;

    int ledIdx1 = XY(x1, y);
    int ledIdx2 = XY(x2, y);

    if (ledIdx1 >= 0)
      leds[ledIdx1] = CHSV(hue, 255, 255);
    if (ledIdx2 >= 0)
      leds[ledIdx2] = CHSV(hue + 128, 255, 255);

    // Draw bars connecting them
    if (y % 4 == 0) {
      int minX = min(x1, x2);
      int maxX = max(x1, x2);
      for (int x = minX; x <= maxX; x += 2) {
        int idx = XY(x, y);
        if (idx >= 0)
          leds[idx] = CHSV(hue + y * 8, 255, 100);
      }
    }
  }
}

// 5. Radar
void pattern_radar(CRGB *leds, int activeLeds, uint8_t &hue) {
  fadeToBlackBy(leds, activeLeds, 10);

  int centerX = GRID_WIDTH / 2;
  int centerY = GRID_HEIGHT / 2;
  static uint8_t angle = 0;

  // Draw sweep line
  for (int r = 0; r < max(GRID_WIDTH, GRID_HEIGHT) / 2; r++) {
    int x = centerX + (cos8(angle) - 128) * r / 128;
    int y = centerY + (sin8(angle) - 128) * r / 128;
    int ledIdx = XY(x, y);
    if (ledIdx >= 0)
      leds[ledIdx] = CHSV(96, 255, 255); // Green
  }

  // Random blips
  if (random8() < 5) {
    int bx = random(GRID_WIDTH);
    int by = random(GRID_HEIGHT);
    int idx = XY(bx, by);
    if (idx >= 0)
      leds[idx] = CRGB::Red;
  }

  angle += 4;
}

// 7. Spectrum (Fake)
void pattern_spectrum(CRGB *leds, int activeLeds, uint8_t &hue) {
  fadeToBlackBy(leds, activeLeds, 60);

  for (int x = 0; x < GRID_WIDTH; x++) {
    // Simulate bar height with noise
    int height = inoise8(x * 30, millis() / 5) * GRID_HEIGHT / 255;

    for (int y = GRID_HEIGHT - 1; y >= GRID_HEIGHT - 1 - height; y--) {
      int idx = XY(x, y);
      if (idx >= 0) {
        // Color gradient from bottom (green) to top (red)
        int h = map(y, 0, GRID_HEIGHT, 0, 96); // 0=Red, 96=Green
        leds[idx] = CHSV(h, 255, 255);
      }
    }
  }
}

// 8. Sinewave 3D
void pattern_sinewave_3d(CRGB *leds, int activeLeds, uint8_t &hue) {
  fadeToBlackBy(leds, activeLeds, 20);
  uint16_t t = millis() / 10;

  for (int x = 0; x < GRID_WIDTH; x++) {
    int y =
        (GRID_HEIGHT / 2) + (sin8(x * 10 + t) - 128) * (GRID_HEIGHT / 3) / 128;
    int idx = XY(x, y);
    if (idx >= 0)
      leds[idx] = CHSV(x * 5 + hue, 255, 255);
  }
}

// 9. Confetti
void pattern_confetti(CRGB *leds, int activeLeds, uint8_t &hue) {
  fadeToBlackBy(leds, activeLeds, 10);
  int pos = random16(activeLeds);
  leds[pos] += CHSV(random8(64) + hue, 200, 255);
}

// 10. Breathing
void pattern_breathing(CRGB *leds, int activeLeds, uint8_t &hue) {
  float breath = (exp(sin(millis() / 2000.0 * PI)) - 0.36787944) * 108.0;
  fill_solid(leds, activeLeds, CHSV(hue, 255, breath));
}

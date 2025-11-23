// pattern_119_particle_fountain.cpp
#include "../patterns.h"

// Particle Fountain - Particles shoot up from bottom
void pattern_particle_fountain(CRGB* leds, int activeLeds, uint8_t& hue) {
#define NUM_PARTICLES 30
          static float particles[NUM_PARTICLES][4]; // x, y, vx, vy
          static bool initialized119 = false;

          if (!initialized119) {
            for(int i=0; i<NUM_PARTICLES; i++) {
              particles[i][0] = GRID_WIDTH / 2;
              particles[i][1] = 0;
              particles[i][2] = (random8(200) - 100) / 10.0;
              particles[i][3] = random8(10, 30) / 10.0;
            }
            initialized119 = true;
          }

          fadeToBlackBy(leds, activeLeds, 40);

          for(int i=0; i<NUM_PARTICLES; i++) {
            // Draw particle
            int led = XY((int)particles[i][0], (int)particles[i][1]);
            if (led >= 0) {
              leds[led] = CHSV(hue + i*8, 255, 255);
            }

            // Update physics (corrected for aspect ratio)
            particles[i][2] *= 0.99; // Air resistance
            particles[i][3] -= 0.15; // Gravity (adjusted for vertical spacing)
            particles[i][0] += particles[i][2];
            particles[i][1] += particles[i][3] / ASPECT_RATIO;

            // Reset if out of bounds
            if (particles[i][1] < 0 || particles[i][0] < 0 || particles[i][0] >= GRID_WIDTH) {
              particles[i][0] = GRID_WIDTH / 2 + random8(40) - 20;
              particles[i][1] = 0;
              particles[i][2] = (random8(200) - 100) / 10.0;
              particles[i][3] = random8(10, 30) / 10.0;
            }
          }
          hue++;
}

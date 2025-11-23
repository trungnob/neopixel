#ifndef PLATFORM_H
#define PLATFORM_H

#ifdef SIMULATOR
  // Simulator platform (native C++)
  #include <cstdint>
  #include <cmath>
  #include <cstdlib>
  #include <cstring>

  // Mock PROGMEM for simulator
  #define PROGMEM
  #define pgm_read_byte(addr) (*(const uint8_t *)(addr))

  // Mock millis() for simulator
  #include <chrono>
inline unsigned long millis() {
  static auto start = std::chrono::steady_clock::now();
  auto now = std::chrono::steady_clock::now();
  return std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count();
}

// Timer helpers (lightweight stand-ins for FastLED EVERY_N_* macros)
#ifndef EVERY_N_MILLISECONDS
#define _EVERY_N_HELPER(token, interval_ms) \
  for (static unsigned long _last_##token = 0; ((millis() - _last_##token) >= (interval_ms)) ? (_last_##token = millis(), true) : false; )
#define EVERY_N_MILLISECONDS(x) _EVERY_N_HELPER(__LINE__, (x))
#define EVERY_N_SECONDS(x) _EVERY_N_HELPER(__LINE__, (uint32_t)(x) * 1000UL)
#endif

  // Mock random for simulator
  inline uint8_t random8() {
    return rand() % 256;
  }

  inline uint8_t random8(uint8_t lim) {
    return rand() % lim;
  }

  inline uint8_t random8(uint8_t min, uint8_t max) {
    return min + (rand() % (max - min));
  }

  inline uint16_t random16() {
    return rand() % 65536;
  }

inline uint16_t random16(uint16_t lim) {
  return rand() % lim;
}

// Arduino-compatible aliases
using byte = uint8_t;

#else
  // ESP8266 platform
  #include <Arduino.h>
  #include <FastLED.h>

  // Arduino provides these already

#endif

// Common types and constants (work on both platforms)
#define MAX_LEDS    1500
#define GRID_WIDTH  144
#define GRID_HEIGHT 9

#define LED_SPACING_H 6.9
#define LED_SPACING_V 50.0
#define ASPECT_RATIO 7.25

// CRGB type for both platforms
#ifndef SIMULATOR
  // Use FastLED's CRGB
  // (already included above)
#else
  // Define our own CRGB for simulator
  struct CRGB {
    uint8_t r;
    uint8_t g;
    uint8_t b;

    CRGB() : r(0), g(0), b(0) {}
    CRGB(uint8_t r, uint8_t g, uint8_t b) : r(r), g(g), b(b) {}

    inline void fadeToBlackBy(uint8_t amount) {
      r = (amount > r) ? 0 : static_cast<uint8_t>(r - amount);
      g = (amount > g) ? 0 : static_cast<uint8_t>(g - amount);
      b = (amount > b) ? 0 : static_cast<uint8_t>(b - amount);
    }

    // Named colors
    static const CRGB Black;
    static const CRGB Red;
    static const CRGB Green;
    static const CRGB Blue;
    static const CRGB Yellow;
    static const CRGB Orange;
    static const CRGB Purple;
    static const CRGB Cyan;
    static const CRGB Magenta;
    static const CRGB White;
  };

  // Define named colors
  inline const CRGB CRGB::Black   = CRGB(0, 0, 0);
  inline const CRGB CRGB::Red     = CRGB(255, 0, 0);
  inline const CRGB CRGB::Green   = CRGB(0, 255, 0);
  inline const CRGB CRGB::Blue    = CRGB(0, 0, 255);
  inline const CRGB CRGB::Yellow  = CRGB(255, 255, 0);
  inline const CRGB CRGB::Orange  = CRGB(255, 165, 0);
  inline const CRGB CRGB::Purple  = CRGB(128, 0, 128);
  inline const CRGB CRGB::Cyan    = CRGB(0, 255, 255);
  inline const CRGB CRGB::Magenta = CRGB(255, 0, 255);
  inline const CRGB CRGB::White   = CRGB(255, 255, 255);

  // CHSV to RGB conversion
  inline CRGB CHSV(uint8_t h, uint8_t s, uint8_t v) {
    float hue = h / 255.0f;
    float sat = s / 255.0f;
    float val = v / 255.0f;

    float r, g, b;
    int i = (int)(hue * 6);
    float f = hue * 6 - i;
    float p = val * (1 - sat);
    float q = val * (1 - f * sat);
    float t = val * (1 - (1 - f) * sat);

    switch (i % 6) {
      case 0: r = val; g = t; b = p; break;
      case 1: r = q; g = val; b = p; break;
      case 2: r = p; g = val; b = t; break;
      case 3: r = p; g = q; b = val; break;
      case 4: r = t; g = p; b = val; break;
      case 5: r = val; g = p; b = q; break;
    }

    return CRGB(r * 255, g * 255, b * 255);
  }

  // FastLED helper functions
  inline uint8_t sin8(uint8_t theta) {
    return (sin(theta * M_PI / 128.0) + 1.0) * 127.5;
  }

  inline uint8_t cos8(uint8_t theta) {
    return (cos(theta * M_PI / 128.0) + 1.0) * 127.5;
  }

  inline uint8_t beatsin8(uint8_t bpm, uint8_t lowest = 0, uint8_t highest = 255,
                          uint32_t timebase = 0, uint8_t phase_offset = 0) {
    uint32_t ms = millis() + timebase;
    uint16_t beats = (ms * bpm) / 60000;
    uint8_t phase = (beats & 0xFF) + phase_offset;
    uint8_t sinval = sin8(phase);
    return lowest + ((sinval * (highest - lowest)) >> 8);
  }

  inline uint16_t beatsin16(uint16_t bpm, uint16_t lowest = 0, uint16_t highest = 65535,
                            uint32_t timebase = 0, uint16_t phase_offset = 0) {
    return beatsin8(bpm, lowest >> 8, highest >> 8, timebase, phase_offset >> 8) << 8;
  }

  inline uint8_t qadd8(uint8_t a, uint8_t b) {
    unsigned int sum = a + b;
    if (sum > 255) sum = 255;
    return sum;
  }

  inline uint8_t qsub8(uint8_t a, uint8_t b) {
    if (a < b) return 0;
    return a - b;
  }

  inline void fill_solid(CRGB* leds, int numLeds, const CRGB& color) {
    for (int i = 0; i < numLeds; i++) {
      leds[i] = color;
    }
  }

  inline void fadeToBlackBy(CRGB* leds, int numLeds, uint8_t amount) {
    for (int i = 0; i < numLeds; i++) {
      leds[i].r = qsub8(leds[i].r, amount);
      leds[i].g = qsub8(leds[i].g, amount);
      leds[i].b = qsub8(leds[i].b, amount);
    }
  }

  inline CRGB HeatColor(uint8_t temperature) {
    // Heat ramp: black -> red -> yellow -> white
    uint8_t t192 = (temperature * 191) / 255;
    uint8_t heatramp = t192 & 0x3F;
    heatramp <<= 2;

    if (t192 & 0x80) {
      return CRGB(255, 255, heatramp);
    } else if (t192 & 0x40) {
      return CRGB(255, heatramp, 0);
    } else {
      return CRGB(heatramp, 0, 0);
    }
  }

  inline uint8_t inoise8(uint16_t x, uint16_t y = 0, uint16_t z = 0) {
    // Simplified noise function
    float nx = x * 0.01f;
    float ny = y * 0.01f;
    float nz = z * 0.01f;
    float n = sin(nx) * cos(ny) * sin(nz);
    return ((n + 1.0f) * 0.5f) * 255;
  }

  inline void blur1d(CRGB* leds, int numLeds, uint8_t amount) {
    // Simple 3-point blur
    for (int i = 1; i < numLeds - 1; i++) {
      leds[i].r = (leds[i-1].r + leds[i].r + leds[i+1].r) / 3;
      leds[i].g = (leds[i-1].g + leds[i].g + leds[i+1].g) / 3;
      leds[i].b = (leds[i-1].b + leds[i].b + leds[i+1].b) / 3;
    }
  }
#endif

#endif // PLATFORM_H

// pattern_121_test_card.cpp
#include "../patterns.h"

// Test Card - one lit pixel sweeping across all LEDs to validate mapping/orientation
void pattern_test_card(CRGB* leds, int activeLeds, uint8_t& hue) {
  static int pos = 0;
  fill_solid(leds, activeLeds, CRGB::Black);

  if (pos >= 0 && pos < activeLeds) {
    leds[pos] = CHSV(hue, 255, 255);  // set one LED bright; change hue for color cycling
  }

  pos = (pos + 1) % activeLeds;
  hue += 4;
}

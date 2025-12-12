#include "../patterns.h"
#include <Arduino.h>
#include <stdio.h>

extern int numPanels;
extern int panelsWide;

void pattern_enumerate(CRGB *leds, int activeLeds, uint8_t &hue) {
  fill_solid(leds, activeLeds, CRGB::Black);

  // Loop through each panel
  for (int i = 0; i < numPanels; i++) {
    // Calculate panel position in the grid
    int pCol = i % panelsWide;
    int pRow = i / panelsWide;

    int startX = pCol * 32;
    int startY = pRow * 8;

    // Convert number to string
    char numStr[4];
    sprintf(numStr, "%d", i);
    int len = strlen(numStr);

    // Calculate text width (5px char + 1px space)
    int textWidth = (len * 5) + (len - 1);

    // Center text within the 32x8 panel
    int xOffset = (32 - textWidth) / 2;
    int yOffset = (8 - 7) / 2; // 7px height, centered in 8px

    // Draw each character
    for (int c = 0; c < len; c++) {
      int charIdx = getFontIndex(numStr[c]);
      int charX = startX + xOffset + (c * 6); // Global X

      for (int col = 0; col < 5; col++) {
        uint8_t colData = pgm_read_byte(&font5x7[charIdx][col]);
        for (int row = 0; row < 7; row++) {
          if (colData & (1 << row)) {
            int x = charX + col;
            int y = startY + yOffset + row;

            // Map to LED index
            int ledIdx = XY(x, y);
            if (ledIdx >= 0 && ledIdx < activeLeds) {
              // Use a different color for each panel to make it distinct
              leds[ledIdx] = CHSV(i * 32, 255, 255);
            }
          }
        }
      }
    }

    // Optional: Draw a border or corner pixel to verify boundaries?
    // Let's keep it simple for now, just the number.
  }
}

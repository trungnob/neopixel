// pattern_120_scrolling_text.cpp
#include "../patterns.h"
#include <string.h>

// Scrolling Text - Aspect-ratio corrected for 7.2:1 physical spacing
void pattern_scrolling_text(CRGB* leds, int activeLeds, uint8_t& hue,
                           const char* text, int& scrollOffset, int scrollSpeed) {
fill_solid(leds, activeLeds, CRGB::Black);

          // Aspect ratio compensation: vertical is 7.2x taller than horizontal
          // To make letters look square, make them ~7x wider in LED count
          const int scaleX = 7;  // Horizontal scale (to compensate for tight spacing)
          const int scaleY = 1;  // Vertical scale (keep it 1:1 since spacing is already wide)
          const int charSpacing = 5;  // Gap between characters

          // Calculate scaled dimensions
          int scaledFontWidth = FONT_WIDTH * scaleX;
          int scaledFontHeight = FONT_HEIGHT * scaleY;
          int charWidth = scaledFontWidth + charSpacing;

          // Calculate total text width
          int textLen = strlen(text);
          int textWidth = textLen * charWidth;

          // Draw each character
          for(int charIdx = 0; charIdx < textLen; charIdx++) {
            char c = text[charIdx];
            int fontIdx = getFontIndex(c);

            // Starting X position for this character
            int charX = GRID_WIDTH + charIdx * charWidth - scrollOffset;

            // Only draw if character is visible on screen
            if (charX < GRID_WIDTH && charX + scaledFontWidth > 0) {
              // Draw each column of the character
              for(int col = 0; col < FONT_WIDTH; col++) {
                uint8_t columnData = pgm_read_byte(&font5x7[fontIdx][col]);

                // Draw each row of the column
                for(int row = 0; row < FONT_HEIGHT; row++) {
                  if (columnData & (1 << row)) {
                    // Scale pixels with aspect ratio compensation
                    for(int dy = 0; dy < scaleY; dy++) {
                      for(int dx = 0; dx < scaleX; dx++) {
                        // Calculate position on grid (use all 9 rows, no centering)
                        int y = row * scaleY + dy + 1;  // Start at row 1
                        int x = charX + col * scaleX + dx;

                        // Only draw if within grid bounds
                        if (y >= 0 && y < GRID_HEIGHT && x >= 0 && x < GRID_WIDTH) {
                          int led = XY(x, y);
                          if (led >= 0) {
                            leds[led] = CHSV(hue, 255, 255);
                          }
                        }
                      }
                    }
                  }
                }
              }
            }
          }

          // Scroll the text using configurable speed
          static unsigned long lastScrollTime = 0;
          if (millis() - lastScrollTime > scrollSpeed) {
            scrollOffset++;
            // Reset when text has fully scrolled off screen
            if (scrollOffset > GRID_WIDTH + textWidth) {
              scrollOffset = 0;
            }
            lastScrollTime = millis();
          }

          hue++;
}

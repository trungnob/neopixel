// pattern_120_scrolling_text.cpp
#include "../patterns.h"
#include <string.h>

// Scrolling Text - Aspect-ratio corrected for 7.2:1 physical spacing
void pattern_scrolling_text(CRGB *leds, int activeLeds, uint8_t &hue,
                            const char *text, int &scrollOffset,
                            int scrollSpeed) {
  fill_solid(leds, activeLeds, CRGB::Black);

  // Aspect ratio compensation:
  // 9x144 layout has 7.25:1 spacing (needs horizontal stretching)
  // 8x32/32x32 layouts have 1:1 spacing (no stretching needed)

  int scaleX = 1;
  // Only stretch if aspect ratio is significantly non-square (like the 7.25:1
  // of 9x144) Explicitly force 1x for Multi-Panel (layout 2) and Single Panel
  // (layout 1)
  if (currentLayout == 0 && ASPECT_RATIO > 4.0) {
    scaleX = 7; // Stretch ONLY for 9x144
  }

  const int scaleY = 1;                         // Vertical scale usually 1:1
  const int charSpacing = (scaleX > 1) ? 5 : 1; // Gap between characters

  // Calculate scaled dimensions
  int scaledFontWidth = FONT_WIDTH * scaleX;
  int scaledFontHeight = FONT_HEIGHT * scaleY;

  int charWidth = scaledFontWidth + charSpacing;

  // Calculate total text width
  int textLen = strlen(text);
  int textWidth = textLen * charWidth;

  // Vertical centering
  int yOffset = (GRID_HEIGHT - scaledFontHeight) / 2;

  // Smooth Scrolling Logic
  // Use a static float to accumulate sub-pixel movements
  static float smoothScrollPos = 0.0f;
  static unsigned long lastUpdate = 0;

  int currentScrollX = 0;

  // Auto-center if text fits on screen
  if (textWidth <= GRID_WIDTH) {
    // Center the text and don't scroll
    currentScrollX = (GRID_WIDTH - textWidth) / 2;
    // Reset scroll pos so if text changes to longer, it starts correctly
    smoothScrollPos = -GRID_WIDTH;
  } else {
    // Text is longer than screen, scroll it

    // Detect external reset (e.g. from Web UI setting scrollOffset = 0)
    if (abs(scrollOffset - smoothScrollPos) > 10.0f) {
      if (scrollOffset == 0) {
        smoothScrollPos = -GRID_WIDTH; // Reset to start (entering from right)
      } else {
        smoothScrollPos = scrollOffset;
      }
    }

    unsigned long now = millis();
    float dt = (now - lastUpdate) / 1000.0f; // Delta time in seconds
    lastUpdate = now;

    // Prevent huge jumps if loop was blocked or first run
    if (dt > 0.1f)
      dt = 0.0f;

    // Map scrollSpeed (1-100) to pixels per second
    float pixelsPerSecond = 2.0f + (scrollSpeed * 0.5f);

    smoothScrollPos += pixelsPerSecond * dt;

    // Wrap around
    if (smoothScrollPos > textWidth + 20) {
      smoothScrollPos = -GRID_WIDTH;
    }

    currentScrollX = (int)smoothScrollPos;
  }

  // Update the global scrollOffset just for state consistency
  scrollOffset = currentScrollX;

  // Draw each character
  for (int charIdx = 0; charIdx < textLen; charIdx++) {
    char c = text[charIdx];
    int fontIdx = getFontIndex(c);

    // Starting X position for this character
    int charX = charIdx * charWidth - currentScrollX;

    // Only draw if character is visible on screen
    if (charX < GRID_WIDTH && charX + scaledFontWidth > 0) {
      // Draw each column of the character
      for (int col = 0; col < FONT_WIDTH; col++) {
        uint8_t columnData = pgm_read_byte(&font5x7[fontIdx][col]);

        // Draw each row of the column
        for (int row = 0; row < FONT_HEIGHT; row++) {
          if (columnData & (1 << row)) {
            // Draw scaled pixel(s)
            for (int sx = 0; sx < scaleX; sx++) {
              for (int sy = 0; sy < scaleY; sy++) {
                int x = charX + col * scaleX + sx;
                int y = yOffset + row * scaleY + sy;

                // Rainbow effect: Hue depends on X position + time (hue
                // variable) This makes the rainbow flow across the text
                uint8_t pixelHue = hue + (x * 2);

                int ledIdx = XY(x, y);
                if (ledIdx >= 0) {
                  leds[ledIdx] = CHSV(pixelHue, 255, 255);
                }
              }
            }
          }
        }
      }
    }
  }

  // Cycle the base hue to make the rainbow move even if text is stationary
  hue++;
}

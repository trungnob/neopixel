#include "../patterns.h"
#include <Arduino.h> // Required for millis() and map()
#include <stdio.h>
#include <string.h>
#include <time.h>

void pattern_clock(CRGB *leds, int activeLeds, uint8_t &hue, int &scrollOffset,
                   int scrollSpeed) {
  fill_solid(leds, activeLeds, CRGB::Black);

  time_t now = time(nullptr);
  struct tm *timeinfo = localtime(&now);

  char timeStr[16];
  // Format: 12:34:56 PM
  int hour = timeinfo->tm_hour;
  const char *ampm = ""; // Not used in 24h mode

  // 24-hour format requested (e.g. 20:00 instead of 08:00 PM)
  // No conversion needed.

  // Custom 3x7 font for digits and colon (0-9, :)
  // Format: 3 bytes per character (columns), LSB at top
  // Fills the 8-pixel height much better than 3x5
  static const uint8_t font3x7[][3] = {
      {0x7F, 0x41, 0x7F}, // 0
      {0x00, 0x7F, 0x00}, // 1
      {0x79, 0x49, 0x4F}, // 2
      {0x41, 0x49, 0x7F}, // 3
      {0x0F, 0x08, 0x7F}, // 4
      {0x4F, 0x49, 0x79}, // 5
      {0x7F, 0x49, 0x79}, // 6
      {0x01, 0x01, 0x7F}, // 7
      {0x7F, 0x49, 0x7F}, // 8
      {0x4F, 0x49, 0x7F}, // 9
      {0x00, 0x36, 0x00}, // : (centered dots)
      {0x00, 0x00, 0x00}  // space
  };

  auto getTinyFontIdx = [](char c) -> int {
    if (c >= '0' && c <= '9')
      return c - '0';
    if (c == ':')
      return 10;
    return 11; // space or other
  };

  // "12:34:56" is 8 chars.
  // With 3x5 font + 1px space = 4px per char.
  // 8 * 4 = 32 pixels. Fits exactly on 32 width!

  if (GRID_WIDTH < 40 && GRID_HEIGHT < 15) {
    // 8x32 Mode: Use Standard 5x7 Font for HH:MM
    // "12:34" is 5 chars. 5 * 5px + 4 spaces = 29 pixels wide.
    // Fits in 32px with 3px margin.

    char timeStr[6];
    // Force 2-digit hour (e.g. 09:00)
    sprintf(timeStr, "%02d:%02d", hour, timeinfo->tm_min);

    int len = strlen(timeStr);
    int totalW = (len * 5) + (len - 1); // 5px width + 1px space
    int startX = (GRID_WIDTH - totalW) / 2;
    int startY = 0; // Top align to leave room for seconds bar at bottom

    // Draw HH:MM
    for (int i = 0; i < len; i++) {
      // Blinking Colon Logic:
      // If char is ':' and we are in the "off" phase of the blink
      // (500ms-1000ms)
      if (timeStr[i] == ':' && (millis() % 1000 > 500)) {
        continue; // Skip drawing
      }

      int idx = getFontIndex(timeStr[i]);
      int charX = startX + (i * 6); // 5px char + 1px space

      for (int col = 0; col < 5; col++) {
        uint8_t colData = pgm_read_byte(&font5x7[idx][col]);
        for (int row = 0; row < 7; row++) {
          if (colData & (1 << row)) {
            int x = charX + col;
            int y = startY + row;
            int ledIdx = XY(x, y);
            if (ledIdx >= 0)
              leds[ledIdx] = CHSV(hue + (x * 8), 255, 255);
          }
        }
      }
    }

    // Draw Seconds Bar at the bottom (Row 7)
    // Map 0-59 seconds to 0-32 pixels
    int secWidth = map(timeinfo->tm_sec, 0, 59, 0, GRID_WIDTH);
    for (int x = 0; x < secWidth; x++) {
      int ledIdx = XY(x, 7);
      // Make the bar a contrasting color or same rainbow
      if (ledIdx >= 0)
        leds[ledIdx] = CHSV(hue + (x * 8) + 128, 255, 255);
    }

  } else if (GRID_WIDTH < 40) {
    // 32x32 Mode: Stacked 5x7 Font
    char line1[16];
    char line2[16];
    sprintf(line1, "%d:%02d", hour, timeinfo->tm_min);
    sprintf(line2, ":%02d", timeinfo->tm_sec);

    // Helper to draw standard font line
    auto drawLine = [&](const char *str, int yBase) {
      int len = strlen(str);
      int width = len * (FONT_WIDTH + 1);
      int startX = (GRID_WIDTH - width) / 2;
      for (int i = 0; i < len; i++) {
        int fontIdx = getFontIndex(str[i]);
        int charX = startX + i * (FONT_WIDTH + 1);
        for (int col = 0; col < FONT_WIDTH; col++) {
          uint8_t colData = pgm_read_byte(&font5x7[fontIdx][col]);
          for (int row = 0; row < FONT_HEIGHT; row++) {
            if (colData & (1 << row)) {
              int x = charX + col;
              int y = yBase + row;
              int ledIdx = XY(x, y);
              if (ledIdx >= 0)
                leds[ledIdx] = CHSV(hue + (x * 4) + (y * 4), 255, 255);
            }
          }
        }
      }
    };

    int totalH = 15;
    int startY = (GRID_HEIGHT - totalH) / 2;
    drawLine(line1, startY + 8);
    drawLine(line2, startY);

  } else {
    // Wide Mode: Standard 5x7 Font
    char line1[16];
    sprintf(line1, "%d:%02d:%02d %s", hour, timeinfo->tm_min, timeinfo->tm_sec,
            ampm);

    int len = strlen(line1);
    int width = len * (FONT_WIDTH + 1);
    int startX = (GRID_WIDTH - width) / 2;
    int startY = (GRID_HEIGHT - FONT_HEIGHT) / 2;

    for (int i = 0; i < len; i++) {
      int fontIdx = getFontIndex(line1[i]);
      int charX = startX + i * (FONT_WIDTH + 1);
      for (int col = 0; col < FONT_WIDTH; col++) {
        uint8_t colData = pgm_read_byte(&font5x7[fontIdx][col]);
        for (int row = 0; row < FONT_HEIGHT; row++) {
          if (colData & (1 << row)) {
            int x = charX + col;
            int y = startY + row;
            int ledIdx = XY(x, y);
            if (ledIdx >= 0)
              leds[ledIdx] = CHSV(hue + (x * 4), 255, 255);
          }
        }
      }
    }
  }

  hue++;
}

#include "../patterns.h"
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

// Pattern 200: IP Address (Scrolling) + Clock
// Panel 1 (rows 0-7): Full IP Address scrolling horizontally
// Panel 2 (rows 8-15): Clock HH:MM
// Panel 3 & 4: Off

void pattern_ip_clock(CRGB *leds, int activeLeds, uint8_t &hue,
                      int &scrollOffset, int scrollSpeed) {
  fill_solid(leds, activeLeds, CRGB::Black);

  // Get IP address
  IPAddress ip = WiFi.localIP();
  char ipStr[20];
  sprintf(ipStr, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);

  // Get time
  time_t now = time(nullptr);
  struct tm *timeinfo = localtime(&now);
  int hour = timeinfo->tm_hour;
  int minute = timeinfo->tm_min;

  // --- Panel 1: Scrolling IP (Rows 0-7) ---
  int ipLen = strlen(ipStr);
  int charWidth = FONT_WIDTH + 1; // 5px + 1px spacing
  int totalIpWidth = ipLen * charWidth;

  // Update scroll logic
  // Use a static variable for timing to control speed independent of frame rate
  static unsigned long lastScrollTime = 0;
  unsigned long msPerStep =
      map(scrollSpeed, 1, 100, 200, 10); // 1=slow, 100=fast

  if (millis() - lastScrollTime > msPerStep) {
    scrollOffset++;
    // Reset scroll when text has fully scrolled off screen
    if (scrollOffset > totalIpWidth) {
      scrollOffset = -GRID_WIDTH;
    }
    lastScrollTime = millis();
  }

  int startX =
      -scrollOffset; // Start drawing from negative offset to scroll left
  // If we want it to start from right and move left:
  // startX = GRID_WIDTH - scrollOffset;
  // But let's stick to standard scrolling text behavior.
  // Actually, standard is usually startX = GRID_WIDTH and decrement, or startX
  // = 0 and decrement. Let's make it scroll right-to-left. If scrollOffset
  // starts at 0 and increases: startX = GRID_WIDTH - scrollOffset;

  startX = GRID_WIDTH - scrollOffset;

  for (int i = 0; i < ipLen; i++) {
    int fontIdx = getFontIndex(ipStr[i]);
    int charX = startX + i * charWidth;

    // Optimization: Skip characters completely off screen
    if (charX < -charWidth || charX >= GRID_WIDTH)
      continue;

    for (int col = 0; col < FONT_WIDTH; col++) {
      uint8_t colData = pgm_read_byte(&font5x7[fontIdx][col]);
      for (int row = 0; row < FONT_HEIGHT; row++) {
        if (colData & (1 << row)) {
          int x = charX + col;
          int y = 0 + row; // Panel 1 starts at y=0
          if (x >= 0 && x < GRID_WIDTH && y >= 0 && y < GRID_HEIGHT) {
            int ledIdx = XY(x, y);
            if (ledIdx >= 0 && ledIdx < activeLeds) {
              leds[ledIdx] = CHSV(hue + (x * 4), 255, 255);
            }
          }
        }
      }
    }
  }

  // --- Panel 2: Clock HH:MM (Rows 8-15) ---
  char clockStr[8];
  sprintf(clockStr, "%02d:%02d", hour, minute);
  if (millis() % 1000 > 500) {
    clockStr[2] = ' '; // Blinking colon
  }

  int clockLen = strlen(clockStr);
  int totalClockWidth = clockLen * charWidth - 1;
  int clockStartX = (GRID_WIDTH - totalClockWidth) / 2;

  for (int i = 0; i < clockLen; i++) {
    int fontIdx = getFontIndex(clockStr[i]);
    int charX = clockStartX + i * charWidth;
    for (int col = 0; col < FONT_WIDTH; col++) {
      uint8_t colData = pgm_read_byte(&font5x7[fontIdx][col]);
      for (int row = 0; row < FONT_HEIGHT; row++) {
        if (colData & (1 << row)) {
          int x = charX + col;
          int y = 8 + row; // Panel 2 starts at y=8
          if (x >= 0 && x < GRID_WIDTH && y >= 0 && y < GRID_HEIGHT) {
            int ledIdx = XY(x, y);
            if (ledIdx >= 0 && ledIdx < activeLeds) {
              leds[ledIdx] = CHSV(hue + 64 + (x * 4), 255, 255);
            }
          }
        }
      }
    }
  }

  // --- Panel 3: WiFi Signal Strength (Rows 16-23) ---
  int rssi = WiFi.RSSI();
  // Map RSSI to bars: -30 dBm = excellent (5 bars), -90 dBm = poor (1 bar)
  // RSSI typically ranges from -30 (excellent) to -90 (poor)
  int bars = 0;
  if (rssi > -50)
    bars = 5;
  else if (rssi > -60)
    bars = 4;
  else if (rssi > -70)
    bars = 3;
  else if (rssi > -80)
    bars = 2;
  else
    bars = 1;

  // Draw WiFi icon/label "WiFi" and signal bars
  // Position: centered on panel 3
  int barWidth = 4;
  int barSpacing = 2;
  int totalBarsWidth = 5 * barWidth + 4 * barSpacing; // 5 bars
  int barsStartX = (GRID_WIDTH - totalBarsWidth) / 2;
  int panelY = 16; // Panel 3 starts at row 16

  // Draw signal bars
  for (int b = 0; b < 5; b++) {
    int barHeight = (b + 1) * 1 + 2; // Heights: 3,4,5,6,7
    int barX = barsStartX + b * (barWidth + barSpacing);

    // Color based on bar position and if active
    CRGB barColor;
    if (b < bars) {
      // Active bar - color based on signal strength
      if (bars >= 4)
        barColor = CRGB::Green;
      else if (bars >= 2)
        barColor = CRGB::Yellow;
      else
        barColor = CRGB::Red;
    } else {
      // Inactive bar - dim gray
      barColor = CRGB(30, 30, 30);
    }

    // Draw bar from bottom of panel upward
    for (int x = barX; x < barX + barWidth && x < GRID_WIDTH; x++) {
      for (int h = 0; h < barHeight; h++) {
        int y = panelY + 7 - h; // Start from bottom of panel (row 23) going up
        if (y >= panelY && y < panelY + 8) {
          int ledIdx = XY(x, y);
          if (ledIdx >= 0 && ledIdx < activeLeds) {
            leds[ledIdx] = barColor;
          }
        }
      }
    }
  }

  // --- Panel 4: RSSI value in dBm (Rows 24-31) ---
  char rssiStr[10];
  sprintf(rssiStr, "%ddBm", rssi);
  int rssiLen = strlen(rssiStr);
  int totalRssiWidth = rssiLen * charWidth - 1;
  int rssiStartX = (GRID_WIDTH - totalRssiWidth) / 2;

  for (int i = 0; i < rssiLen; i++) {
    int fontIdx = getFontIndex(rssiStr[i]);
    int charX = rssiStartX + i * charWidth;
    for (int col = 0; col < FONT_WIDTH; col++) {
      uint8_t colData = pgm_read_byte(&font5x7[fontIdx][col]);
      for (int row = 0; row < FONT_HEIGHT; row++) {
        if (colData & (1 << row)) {
          int x = charX + col;
          int y = 24 + row; // Panel 4 starts at y=24
          if (x >= 0 && x < GRID_WIDTH && y >= 0 && y < GRID_HEIGHT) {
            int ledIdx = XY(x, y);
            if (ledIdx >= 0 && ledIdx < activeLeds) {
              // Color based on signal quality
              CRGB textColor;
              if (bars >= 4)
                textColor = CRGB::Green;
              else if (bars >= 2)
                textColor = CRGB::Yellow;
              else
                textColor = CRGB::Red;
              leds[ledIdx] = textColor;
            }
          }
        }
      }
    }
  }

  hue++;
}

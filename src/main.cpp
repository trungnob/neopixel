#include "pattern_list.h"
#include "patterns.h"
#include <Arduino.h>
#include <ArduinoOTA.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <FastLED.h>
#include <WiFiUdp.h>
#include <time.h>

#ifndef OTA_PASSWORD
#error                                                                         \
    "OTA_PASSWORD is missing. Run `make ota-init` to generate config/ota.env or set OTA_PASSWORD in your environment."
#endif

#ifndef WIFI_SSID
#error "WIFI_SSID is missing. Define it via build flags or secrets header."
#endif

#ifndef WIFI_PASSWORD
#error "WIFI_PASSWORD is missing. Define it via build flags or secrets header."
#endif

#define LED_PIN D4
#define MAX_LEDS 2048 // Support up to 8 panels (8 * 256 = 2048)
#define BRIGHTNESS 76
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB

CRGB leds[MAX_LEDS];
ESP8266WebServer server(80);
WiFiUDP udp;
#define UDP_PORT 4210
unsigned long lastUdpPacketTime = 0;
const unsigned long UDP_TIMEOUT =
    5000; // Switch back after 5s of silence? Optional.

// Layout Presets
enum LayoutType {
  LAYOUT_9X144_ROW_ZIGZAG = 0, // Original: 9 rows × 144 cols, row-wise zigzag
  LAYOUT_8X32_COL_ZIGZAG = 1,  // Single: 8 rows × 32 cols, column-wise zigzag
  LAYOUT_32X32_MULTI_PANEL =
      2 // Multi: 32×32 (4× 8×32 panels stacked vertically)
};

// Current layout configuration (changeable at runtime)
int currentLayout = LAYOUT_32X32_MULTI_PANEL;
int numPanels = 4;     // Default to 4 panels
int panelsWide = 1;    // Default to 1 panel wide (vertical stack)
int GRID_WIDTH = 32;   // 32
int GRID_HEIGHT = 32;  // 32
int activeLeds = 1024; // 4 * 256

// Layout presets configuration
struct LayoutConfig {
  int width;
  int height;
  int totalLeds;
  const char *name;
};

const LayoutConfig layouts[] = {
    {32, 8, 256, "Nx8x32 Multi-Panel"} // Default to N=1 (8x32)
};

// Physical dimensions for aspect-ratio-aware patterns
// Defaulting to 1:1 spacing for 8x32 panels
float LED_SPACING_H = 10.0;
float LED_SPACING_V = 10.0;
float ASPECT_RATIO = 1.0;

// XY function is now in patterns.h (handles both layouts)

// State variables
int currentPattern = 200; // Default to IP + Clock
uint8_t hue = 0;
String scrollText = "HELLO WORLD"; // Global variables
int scrollOffset = 0;
int currentMode = 0;
int scrollSpeed = 20; // Default speed (1-100)

// UDP Stats for debugging
unsigned long udpPacketsReceived = 0;
unsigned long udpPacketsDisplayed = 0;
unsigned long udpStatsLastPrint = 0;

// State Machine Timers
unsigned long lastActivityTime = 0; // Last user interaction or state change
const unsigned long IDLE_TIMEOUT = 60000; // 60 seconds to auto-stream

// Timing profiling (in microseconds)
unsigned long timeParsePacket = 0;
unsigned long timeReadPacket = 0;
unsigned long timeDrainLoop = 0;
unsigned long timeFastLEDShow = 0;
unsigned long timeLoopTotal = 0;
unsigned long timingSamples = 0;

// Custom pattern storage (for pattern designer)
CRGB customPattern[MAX_LEDS];
bool hasCustomPattern = false;

// Font data is now in patterns/font.cpp

// HTML Page
const char htmlPage[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body { font-family: sans-serif; text-align: center; padding: 20px; background: #222; color: #fff; }
    button { display: block; width: 100%; padding: 15px; margin: 10px 0; font-size: 18px; border: none; border-radius: 5px; cursor: pointer; }
    input { padding: 10px; font-size: 16px; width: 60px; text-align: center; }
    .control-group { margin: 20px 0; padding: 15px; background: #333; border-radius: 10px; }
    .rainbow { background: linear-gradient(90deg, red, orange, yellow, green, blue, indigo, violet); color: black; }
    .red { background-color: #ff4444; color: white; }
    .green { background-color: #44ff44; color: black; }
    .blue { background-color: #4444ff; color: white; }
    .cool { background: linear-gradient(45deg, #ff00cc, #3333ff); color: white; }
    .fire { background: linear-gradient(to right, #ff0000, #ffff00); color: black; }
    .special { background: linear-gradient(to right, #00ffff, #ff00ff); color: black; }
    .off { background-color: #555; color: white; }

    /* Tab styles */
    .tabs { display: flex; margin: 20px 0; border-bottom: 2px solid #444; }
    .tab { flex: 1; padding: 15px; background: #333; border: none; color: #aaa; cursor: pointer; font-size: 16px; }
    .tab.active { background: #444; color: #fff; border-bottom: 3px solid #00ffff; }
    .tab-content { display: none; }
    .tab-content.active { display: block; }
    .footer {
      margin-top: 20px;
      font-size: 12px;
      color: #666;
    }
  </style>
  <script>
    function setMode(m) {
      fetch('/set?m=' + m);
    }
    function showTab(tabName) {
      var tabs = document.getElementsByClassName('tab-content');
      for (var i = 0; i < tabs.length; i++) {
        tabs[i].classList.remove('active');
      }
      var tabButtons = document.getElementsByClassName('tab');
      for (var i = 0; i < tabButtons.length; i++) {
        tabButtons[i].classList.remove('active');
      }
      document.getElementById(tabName).classList.add('active');
      event.target.classList.add('active');
    }
    function setLayout(layoutId) {
      fetch('/setLayout?layout=' + layoutId)
        .then(response => response.json())
        .then(data => {
          alert('Layout changed to: ' + data.layout);
          location.reload();  // Reload to refresh LED count
        });
    }
    window.onload = function() {
      // Set the layout dropdown to the current layout
      var layoutSelect = document.querySelector('select');
      if (layoutSelect) {
        layoutSelect.value = "%LAYOUT%";
      }
    }
  </script>
</head>
<body>
  <h1>LED Controller</h1>
  
  <div style="margin: 20px 0; padding: 15px; background: #333; border-radius: 8px;">
    <label style="color: #fff; font-size: 18px; margin-right: 10px;">Layout:</label>
    <select onchange="setLayout(this.value)" style="padding: 8px; font-size: 16px; background: #444; color: #fff; border: 1px solid #666; border-radius: 4px;">
      <option value="0">9x144 Row Zigzag (1296 LEDs)</option>
      <option value="1">8x32 Column Zigzag (256 LEDs)</option>
      <option value="2">32x32 Multi-Panel (1024 LEDs)</option>
    </select>
    <span style="color: #aaa; margin-left: 15px;">Active LEDs: %LEDS%</span>
  </div>


  <div class="control-group">
    <label>Panel Configuration (8x32 panels):</label>
    <form action="/setPanels" method="get" style="display:inline;">
      <label>Rows:</label>
      <input type="number" name="r" id="r" min="1" max="16" value="%ROWS%" style="width: 50px;">
      <label>Cols:</label>
      <input type="number" name="c" id="c" min="1" max="16" value="%COLS%" style="width: 50px;">
      <button type="submit" style="display:inline; width:auto; padding: 10px;">Set</button>
    </form>
    <br><small style="color:#aaa">Grid Size: (Cols*32) x (Rows*8)</small>
  </div>

  <div class="tabs">
    <button class="tab active" onclick="showTab('tab-patterns')">Patterns</button>
    <button class="tab" onclick="showTab('tab-text')">Scrolling Text</button>
  </div>

  <!-- Patterns Tab -->
  <div id="tab-patterns" class="tab-content active">
    <!-- Dynamic Menu Container -->
    <div id="patterns-container">
      <p style="color: #aaa; text-align: center;">Loading patterns...</p>
    </div>
  </div>

  <script>
    // Fetch and render patterns
    fetch('/api/patterns')
      .then(response => response.json())
      .then(patterns => {
        const container = document.getElementById('patterns-container');
        container.innerHTML = ''; // Clear loading message
        
        // Group by category
        const categories = {};
        patterns.forEach(p => {
          if (!categories[p.category]) categories[p.category] = [];
          categories[p.category].push(p);
        });
        
        // Render categories
        for (const [category, items] of Object.entries(categories)) {
          const groupDiv = document.createElement('div');
          groupDiv.className = 'control-group';
          
          const title = document.createElement('h3');
          title.textContent = category;
          groupDiv.appendChild(title);
          
          items.forEach(p => {
            const btn = document.createElement('button');
            btn.textContent = p.name;
            btn.onclick = () => setMode(p.id);
            
            // Assign random color class for variety
            const colors = ['cool', 'warm', 'special', 'fire', 'green', 'blue', 'rainbow', 'red'];
            const colorClass = colors[p.id % colors.length];
            btn.className = colorClass;
            
            groupDiv.appendChild(btn);
          });
          
          container.appendChild(groupDiv);
        }
      })
      .catch(err => {
        console.error('Error loading patterns:', err);
        document.getElementById('patterns-container').innerHTML = '<p style="color:red">Error loading patterns</p>';
      });
  </script>

  <!-- Scrolling Text Tab -->
  <div id="tab-text" class="tab-content">
    <h2>Scrolling Text</h2>
    <div class="control-group">
      <label>Enter text to scroll:</label>
      <div style="margin-top: 10px;">
        <input type="text" id="textInput" value="%TEXT%" style="width: 80%; padding: 10px; font-size: 16px;" maxlength="100">
        <button onclick="updateText()" style="width: 18%; padding: 10px;">Set</button>
      </div>

      <div style="margin-top: 15px;">
        <label>Scroll Speed: <span id="speedDisplay">%SPEED%</span></label><br>
        <input type="range" id="speedSlider" min="1" max="100" value="%SPEED%" style="width: 80%;"
               oninput="document.getElementById('speedDisplay').textContent = this.value"
               onchange="updateSpeed(this.value)">
        <br>
        <small style="color: #888;">1 = Slow, 100 = Fast</small>
      </div>
    </div>
    <p style="color: #aaa; font-size: 14px;">Supports: A-Z, 0-9, space, !, ., -</p>
  </div>

  <script>
    function updateText() {
      var text = document.getElementById('textInput').value;
      fetch('/setText?text=' + encodeURIComponent(text))
        .then(response => console.log('Text updated'));
    }

    function updateSpeed(speed) {
      fetch('/setText?speed=' + speed)
        .then(response => console.log('Speed updated'));
    }
  </script>


  <div class="footer">
    Build: %BUILD%
  </div>
</body>
</html>
)=====";

// The following server.on calls are typically placed in the setup() function.
// For the purpose of this edit, they are placed here as per the instruction's
// context. If this code is part of a larger sketch, ensure these are moved into
// setup().

void setupServerHandlers() { // Encapsulating the server.on calls in a function
                             // for syntactic correctness
  server.on("/setPanels", []() {
    String r = server.arg("r");
    String c = server.arg("c");

    int rows = 1;
    int cols = 1;

    // Default to current values if not provided
    if (panelsWide > 0) {
      cols = panelsWide;
      rows = (numPanels + panelsWide - 1) / panelsWide;
    }

    if (r.length() > 0) {
      int val = r.toInt();
      if (val >= 1 && val <= 16)
        rows = val;
    }

    if (c.length() > 0) {
      int val = c.toInt();
      if (val >= 1 && val <= 16)
        cols = val;
    }

    numPanels = rows * cols;
    panelsWide = cols;

    // Update Grid Dimensions for Multi-Panel Layout
    if (currentLayout == LAYOUT_32X32_MULTI_PANEL) {
      // Ensure valid width
      if (panelsWide > numPanels)
        panelsWide = numPanels;

      GRID_WIDTH = panelsWide * 32;
      int rows = (numPanels + panelsWide - 1) / panelsWide; // Ceiling division
      GRID_HEIGHT = rows * 8;

      activeLeds = numPanels * 256;
      if (activeLeds > MAX_LEDS)
        activeLeds = MAX_LEDS;
    }
    Serial.print("Panels: ");
    Serial.print(numPanels);
    Serial.print(", Wide: ");
    Serial.println(panelsWide);

    server.sendHeader("Location", "/");
    server.send(303);
  });

  server.on("/", []() {
    String html = FPSTR(htmlPage);
    int rows = (numPanels + panelsWide - 1) / panelsWide;
    html.replace("%ROWS%", String(rows));
    html.replace("%COLS%", String(panelsWide));
    html.replace("%LEDS%", String(activeLeds));
    // html.replace("%PANELS%", String(numPanels)); // Removed
    // html.replace("%WIDTH%", String(panelsWide)); // Removed
    html.replace("%LAYOUT%", String(currentLayout));
    html.replace("%TEXT%", scrollText);
    html.replace("%SPEED%", String(scrollSpeed));
    html.replace("%BUILD%", String(__DATE__) + " " + String(__TIME__));
    server.send(200, "text/html", html);
  });
}

// The original handleRoot() function is replaced by the server.on("/", ...)
// call above. If handleRoot() was called elsewhere, those calls will need to be
// updated.

void handleSet() {
  if (server.hasArg("m")) {
    currentPattern = server.arg("m").toInt();
    lastActivityTime = millis(); // User interaction resets timer
  }
  if (server.hasArg("c")) {
    int newCount = server.arg("c").toInt();
    if (newCount > 0 && newCount <= MAX_LEDS) {
      activeLeds = newCount;
      // Clear any LEDs that might be beyond the new count
      fill_solid(leds, MAX_LEDS, CRGB::Black);
      FastLED.show();
    }
  }
  server.send(200, "text/plain", "OK");
}

void handleSetLayout() {
  if (server.hasArg("layout")) {
    int newLayout = server.arg("layout").toInt();
    if (newLayout >= 0 && newLayout < 1) { // Only 1 layout now
      currentLayout = (LayoutType)newLayout;
      GRID_WIDTH = layouts[newLayout].width;
      GRID_HEIGHT = layouts[newLayout].height;

      // Always 1:1 spacing for Nx8x32
      LED_SPACING_H = 10.0;
      LED_SPACING_V = 10.0;
      ASPECT_RATIO = 1.0;

      // Clear LEDs
      fill_solid(leds, MAX_LEDS, CRGB::Black);
      FastLED.show();

      server.send(200, "application/json",
                  "{\"status\":\"success\",\"layout\":\"" +
                      String(layouts[newLayout].name) + "\"}");
    } else {
      server.send(400, "text/plain", "Invalid layout");
    }
  } else {
    server.send(400, "text/plain", "Missing layout parameter");
  }
}

void handleSetText() {
  if (server.hasArg("text")) {
    scrollText = server.arg("text");
    scrollText.toUpperCase(); // Convert to uppercase for font
    scrollOffset = 0;         // Reset scroll position
    currentPattern = 120;     // Switch to scrolling text mode
  }
  if (server.hasArg("speed")) {
    scrollSpeed = server.arg("speed").toInt();
    // Constrain to valid range
    if (scrollSpeed < 20)
      scrollSpeed = 20;
    if (scrollSpeed > 200)
      scrollSpeed = 200;
  }
  server.sendHeader("Location", "/");
  server.send(303); // Redirect back to main page
}

void handleUploadPattern() {
  // Enable CORS for cross-origin requests from pattern designer
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "POST, OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type");

  if (server.method() == HTTP_OPTIONS) {
    // Handle preflight
    server.send(200);
    return;
  }

  if (server.method() != HTTP_POST) {
    server.send(405, "text/plain", "Method Not Allowed");
    return;
  }

  String body = server.arg("plain");

  // Clear pattern to black first
  for (int i = 0; i < MAX_LEDS; i++) {
    customPattern[i] = CRGB::Black;
  }

  // Check for "hex" format: {"hex":"RRGGBB...", "scrollSpeed":80}
  int pixelCount = 0;
  int hexStart = body.indexOf("\"hex\":\"");
  if (hexStart != -1) {
    int pos = hexStart + 7; // Skip "hex":"
    int ledIdx = 0;

    while (pos < (int)body.length() && body[pos] != '"' && ledIdx < MAX_LEDS) {
      if (pos + 6 <= (int)body.length()) {
        // Parse 6 hex chars: RRGGBB
        char hex[7];
        hex[0] = body[pos];
        hex[1] = body[pos + 1];
        hex[2] = body[pos + 2];
        hex[3] = body[pos + 3];
        hex[4] = body[pos + 4];
        hex[5] = body[pos + 5];
        hex[6] = 0;

        long rgb = strtol(hex, NULL, 16);
        // Extract RGB components (pattern designer sends in RGB order)
        uint8_t r = (rgb >> 16) & 0xFF;
        uint8_t g = (rgb >> 8) & 0xFF;
        uint8_t b = rgb & 0xFF;
        // Assign to CRGB using .r, .g, .b to respect COLOR_ORDER (GRB)
        customPattern[ledIdx].r = r;
        customPattern[ledIdx].g = g;
        customPattern[ledIdx].b = b;

        if (customPattern[ledIdx].r > 0 || customPattern[ledIdx].g > 0 ||
            customPattern[ledIdx].b > 0) {
          pixelCount++;
        }

        ledIdx++;
        pos += 6;
      } else {
        break;
      }
    }
  } else {
    // Fallback to sparse format: {"sparse":[[ledIndex,r,g,b],...],
    // "scrollSpeed":80}
    int sparseStart = body.indexOf("\"sparse\":[");
    if (sparseStart != -1) {
      int pos = sparseStart + 10; // Skip to after "sparse":[

      while (pos < (int)body.length()) {
        // Skip whitespace
        while (pos < (int)body.length() &&
               (body[pos] == ' ' || body[pos] == '\n'))
          pos++;

        // Look for [ledIndex,r,g,b]
        if (body[pos] == '[') {
          pos++; // Skip [

          // Parse LED index
          int ledIdx = 0;
          while (pos < (int)body.length() && body[pos] >= '0' &&
                 body[pos] <= '9') {
            ledIdx = ledIdx * 10 + (body[pos] - '0');
            pos++;
          }
          while (pos < (int)body.length() && body[pos] == ',')
            pos++; // Skip comma

          // Parse R
          int r = 0;
          while (pos < (int)body.length() && body[pos] >= '0' &&
                 body[pos] <= '9') {
            r = r * 10 + (body[pos] - '0');
            pos++;
          }
          while (pos < (int)body.length() && body[pos] == ',')
            pos++; // Skip comma

          // Parse G
          int g = 0;
          while (pos < (int)body.length() && body[pos] >= '0' &&
                 body[pos] <= '9') {
            g = g * 10 + (body[pos] - '0');
            pos++;
          }
          while (pos < (int)body.length() && body[pos] == ',')
            pos++; // Skip comma

          // Parse B
          int b = 0;
          while (pos < (int)body.length() && body[pos] >= '0' &&
                 body[pos] <= '9') {
            b = b * 10 + (body[pos] - '0');
            pos++;
          }

          // Store LED color at specific index
          if (ledIdx >= 0 && ledIdx < MAX_LEDS) {
            customPattern[ledIdx] = CRGB(r, g, b);
            pixelCount++;
          }

          // Skip to next element
          while (pos < (int)body.length() && body[pos] != '[' &&
                 body[pos] != ']')
            pos++;
          if (body[pos] == ']')
            pos++; // Skip ]
          if (body[pos] == ',')
            pos++; // Skip comma between array elements
        } else if (body[pos] == ']') {
          // End of sparse array
          break;
        } else {
          pos++;
        }
      }
    }
  }

  if (pixelCount > 0) {
    hasCustomPattern = true;
    currentPattern = 122; // Switch to custom pattern mode

    // Parse scrollSpeed if present
    int speedPos = body.indexOf("\"scrollSpeed\":");
    if (speedPos != -1) {
      int speedVal = 0;
      int p = speedPos + 14; // Skip "scrollSpeed":
      while (p < (int)body.length() && (body[p] < '0' || body[p] > '9'))
        p++; // Skip non-digits
      while (p < (int)body.length() && body[p] >= '0' && body[p] <= '9') {
        speedVal = speedVal * 10 + (body[p] - '0');
        p++;
      }
      if (speedVal >= 0 && speedVal <= 2000) {
        scrollSpeed = speedVal;
      }
    }

    Serial.print("Pattern uploaded. Pixels: ");
    Serial.print(pixelCount);
    Serial.print(", ScrollSpeed: ");
    Serial.println(scrollSpeed);

    server.send(200, "application/json",
                "{\"status\":\"success\",\"pixels\":" + String(pixelCount) +
                    "}");
  } else {
    // Even with 0 pixels, we can show a blank pattern
    hasCustomPattern = true;
    currentPattern = 122;
    server.send(200, "application/json",
                "{\"status\":\"success\",\"pixels\":0}");
  }
}

// API: Get list of patterns
void handleGetPatterns() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "application/json", "");

  server.sendContent("[");

  for (int i = 0; i < numPatterns; i++) {
    String item = "";
    if (i > 0)
      item += ",";
    item += "{\"id\":";
    item += patterns[i].id;
    item += ",\"name\":\"";
    item += patterns[i].name;
    item += "\",\"category\":\"";
    item += patterns[i].category;
    item += "\"}";
    server.sendContent(item);
  }

  server.sendContent("]");
}

// Global flag to track web server status
bool serverRunning = false;

// Global variable to track upload progress
int uploadLedIdx = 0;

void handleUploadRaw() {
  HTTPUpload &upload = server.upload();

  if (upload.status == UPLOAD_FILE_START) {
    // Safety Check: Content-Length
    if (server.hasHeader("Content-Length")) {
      int contentLength = server.header("Content-Length").toInt();
      if (contentLength > MAX_LEDS * 3 + 100) { // +100 for safety/headers
        Serial.println("Upload too large, rejecting.");
        // We can't easily stop the upload stream here without closing
        // connection, but we can set a flag to ignore data.
        uploadLedIdx = -1;
        return;
      }
    }

    // Reset state
    uploadLedIdx = 0;
    hasCustomPattern = true;
    currentPattern = 122; // Switch to custom pattern mode immediately

    // Optional: Check for scrollSpeed in query param
    if (server.hasArg("speed")) {
      scrollSpeed = server.arg("speed").toInt();
    }

    Serial.println("Starting Raw Upload...");

  } else if (upload.status == UPLOAD_FILE_WRITE) {
    // Process chunk
    if (uploadLedIdx == -1)
      return; // Rejected

    for (size_t i = 0; i < upload.currentSize; i++) {
      if (uploadLedIdx < MAX_LEDS * 3) {
        // Unsafe cast to write directly to the LED array memory
        // This treats the array of CRGB structs as a raw byte array
        ((uint8_t *)customPattern)[uploadLedIdx] = upload.buf[i];
        uploadLedIdx++;
      }
    }

  } else if (upload.status == UPLOAD_FILE_END) {
    if (uploadLedIdx == -1) {
      Serial.println("Upload rejected.");
    } else {
      Serial.print("Raw Upload Done. Bytes: ");
      Serial.println(uploadLedIdx);
    }
  }
}

void setup() {
  // Start Serial FIRST for debugging
  Serial.begin(115200);
  delay(100);
  Serial.println("\n\n=================================");
  Serial.println("ESP8266 LED Controller Starting");
  Serial.println("=================================");

  // LEDs - Use activeLeds (1024) not MAX_LEDS (2048) for faster updates
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, activeLeds)
      .setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);

  // Power limit removed per user request
  // FastLED.setMaxPowerInVoltsAndMilliamps(5, 2000);

  Serial.println("LEDs initialized");

  // WiFi - Station Mode (Connect to Home WiFi)
  const char *ssid = WIFI_SSID;
  const char *password = WIFI_PASSWORD;

  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  Serial.print("Password: ");
  Serial.println(password);

  WiFi.mode(WIFI_STA);
  WiFi.setSleepMode(WIFI_NONE_SLEEP); // Disable WiFi sleep for better stability
  WiFi.setAutoReconnect(true);        // Auto-reconnect if connection drops
  WiFi.begin(ssid, password);

  // Wait for connection - indicate with blue flashing
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED &&
         attempts < 40) { // Increased to 40 attempts (20 seconds)
    delay(500);
    leds[0] = CRGB::Blue; // connecting
    FastLED.show();
    delay(100);
    leds[0] = CRGB::Black;
    FastLED.show();

    Serial.print(".");
    attempts++;
    if (attempts % 20 == 0) {
      Serial.println();
      Serial.print("Still trying... Status: ");
      Serial.println(WiFi.status());
    }
  }

  if (WiFi.status() != WL_CONNECTED) {
    // Failed to connect - flash red
    Serial.println("\n*** WiFi FAILED to connect! ***");
    for (int i = 0; i < 5; i++) {
      leds[0] = CRGB::Red;
      FastLED.show();
      delay(200);
      leds[0] = CRGB::Black;
      FastLED.show();
      delay(200);
    }
  } else {
    // Connected - solid green for 500ms
    leds[0] = CRGB::Green;
    FastLED.show();
    delay(500);
    leds[0] = CRGB::Black;
    FastLED.show();
  }

  // Print IP address
  Serial.println("\n\n*** WiFi Connected! ***");
  Serial.print("Connected to: ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.print("RSSI: ");
  Serial.print(WiFi.RSSI());
  Serial.println(" dBm");

  // Configure Time (NTP) for PST/PDT - AFTER WiFi connects!
  // PST8PDT: Standard PST, Daylight PDT
  // M3.2.0: DST starts 2nd Sunday of March at 2am
  // M11.1.0: DST ends 1st Sunday of November at 2am
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  setenv("TZ", "PST8PDT,M3.2.0,M11.1.0", 1);
  tzset();

  // Wait for NTP sync (up to 10 seconds)
  Serial.print("Waiting for NTP sync...");
  time_t now = time(nullptr);
  int ntpAttempts = 0;
  while (now < 24 * 3600 &&
         ntpAttempts < 20) { // Wait until time is after Jan 1 1970 00:00
    delay(500);
    Serial.print(".");
    now = time(nullptr);
    ntpAttempts++;
  }
  if (now > 24 * 3600) {
    Serial.println(" OK!");
    struct tm *timeinfo = localtime(&now);
    Serial.printf("Current time: %02d:%02d:%02d\n", timeinfo->tm_hour,
                  timeinfo->tm_min, timeinfo->tm_sec);
  } else {
    Serial.println(" FAILED (will retry in background)");
  }

  // OTA Setup
  ArduinoOTA.setHostname("LED_Controller");
  ArduinoOTA.setPassword(OTA_PASSWORD);

  // Auto-pause animations during OTA - indicate with yellow flash
  ArduinoOTA.onStart([]() {
    currentPattern = 4; // Turn Off
    fill_solid(leds, MAX_LEDS, CRGB::Black);
    FastLED.show();
    // Yellow flash to show OTA start
    leds[0] = CRGB::Yellow;
    FastLED.show();
    delay(200);
    leds[0] = CRGB::Black;
    FastLED.show();
  });

  ArduinoOTA.onEnd([]() {
    // LEDs will stay off until user selects a pattern
  });

  ArduinoOTA.begin();

  // Web Server
  setupServerHandlers(); // Sets up /, /setPanels
  server.on("/set", handleSet);
  server.on("/setLayout", handleSetLayout); // NEW: Layout switching endpoint
  server.on("/setText", handleSetText);
  server.on("/info", []() {
    String json = "{";
    json += "\"currentPattern\":" + String(currentPattern) + ",";
    json += "\"uptime\":" + String(millis() / 1000) + ",";
    json += "\"heap\":" + String(ESP.getFreeHeap());
    json += "}";
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "application/json", json);
    // Reset activity timer on status check? No, checking status shouldn't delay
    // idle
  });
  server.on("/api/patterns", handleGetPatterns);
  server.on("/uploadPattern", HTTP_POST, handleUploadPattern);
  server.on("/uploadPattern", HTTP_OPTIONS,
            handleUploadPattern); // Handle CORS preflight

  // NEW: Raw Binary Upload for memory efficiency
  // Usage: POST /uploadRaw?speed=80
  // Body: Raw binary bytes (R, G, B, ...)
  server.on(
      "/uploadRaw", HTTP_POST, []() { server.send(200, "text/plain", "OK"); },
      handleUploadRaw); // Register upload handler
  server.on("/uploadRaw", HTTP_OPTIONS, []() {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Access-Control-Allow-Methods", "POST, OPTIONS");
    server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
    server.send(200);
  });

  // Enter streaming mode via HTTP
  server.on("/stream", []() {
    currentPattern = 255;
    lastUdpPacketTime = millis();
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "text/plain",
                "Streaming mode enabled. Send EXIT via UDP to exit.");
  });

  // UDP Stats endpoint (no UART needed!)
  server.on("/udpstats", []() {
    String json = "{";
    json += "\"received\":" + String(udpPacketsReceived) + ",";
    json += "\"displayed\":" + String(udpPacketsDisplayed) + ",";
    json +=
        "\"dropped\":" + String(udpPacketsReceived - udpPacketsDisplayed) + ",";
    json += "\"uptime\":" + String(millis() / 1000);
    json += "}";
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "application/json", json);
  });

  // Timing profiling endpoint (in microseconds)
  server.on("/timing", []() {
    String json = "{";
    if (timingSamples > 0) {
      json += "\"samples\":" + String(timingSamples) + ",";
      json +=
          "\"avgParsePacket_us\":" + String(timeParsePacket / timingSamples) +
          ",";
      json += "\"avgReadPacket_us\":" + String(timeReadPacket / timingSamples) +
              ",";
      json +=
          "\"avgDrainLoop_us\":" + String(timeDrainLoop / timingSamples) + ",";
      json +=
          "\"avgFastLEDShow_us\":" + String(timeFastLEDShow / timingSamples) +
          ",";
      json +=
          "\"avgLoopTotal_us\":" + String(timeLoopTotal / timingSamples) + ",";
      json += "\"maxFPS\":" + String(1000000 / (timeLoopTotal / timingSamples));
    } else {
      json += "\"samples\":0,\"error\":\"No timing data yet\"";
    }
    json += "}";
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "application/json", json);
    // Reset after reading
    timeParsePacket = 0;
    timeReadPacket = 0;
    timeDrainLoop = 0;
    timeFastLEDShow = 0;
    timeLoopTotal = 0;
    timingSamples = 0;
  });

  // Increase max POST body size for pattern uploads (default is ~2KB, we need
  // ~20KB)
  server.setContentLength(25000);

  server.begin();
  serverRunning = true; // server is up

  // Start UDP Listener
  udp.begin(UDP_PORT);
  Serial.print("UDP Listening on port ");
  Serial.println(UDP_PORT);

  // Indicate server ready with cyan flash
  leds[0] = CRGB::Cyan;
  FastLED.show();
  delay(300);
  leds[0] = CRGB::Black;
  FastLED.show();

  // Initialize Activity Timer
  lastActivityTime = millis();
}

void renderPatternFrame(int currentPattern, CRGB *leds, int activeLeds,
                        uint8_t &hue, String &scrollText, int &scrollOffset,
                        int scrollSpeed) {
  // Always clear buffer first to handle changing sizes cleanly
  // Patterns that need fade/trail effect: 5, 6, 8, 10, 13, 14, 23, 26, 29,
  // 32, 33, 37, 43, 44, 51, 52, 54, 58, 60, 61, 65, 66, 68, 73, 74, 75, 82,
  // 86, 90, 94, 96, 98, 99, 103, 105, 110, 116, 119
  if (currentPattern != 0 && currentPattern != 5 && currentPattern != 6 &&
      currentPattern != 8 && currentPattern != 10 && currentPattern != 13 &&
      currentPattern != 14 && currentPattern != 23 && currentPattern != 26 &&
      currentPattern != 29 && currentPattern != 32 && currentPattern != 33 &&
      currentPattern != 37 && currentPattern != 43 && currentPattern != 44 &&
      currentPattern != 51 && currentPattern != 52 && currentPattern != 54 &&
      currentPattern != 58 && currentPattern != 60 && currentPattern != 61 &&
      currentPattern != 65 && currentPattern != 66 && currentPattern != 68 &&
      currentPattern != 73 && currentPattern != 74 && currentPattern != 75 &&
      currentPattern != 82 && currentPattern != 86 && currentPattern != 90 &&
      currentPattern != 94 && currentPattern != 96 && currentPattern != 98 &&
      currentPattern != 99 && currentPattern != 103 && currentPattern != 105 &&
      currentPattern != 110 && currentPattern != 116 && currentPattern != 119 &&
      currentPattern != 125 && currentPattern != 255) {
    fill_solid(leds, MAX_LEDS, CRGB::Black);
  }

  switch (currentPattern) {
  case 0: // Rainbow
    fill_rainbow(leds, activeLeds, hue++, 7);
    break;
  case 1: // Red
    fill_solid(leds, activeLeds, CRGB::Red);
    break;
  case 2: // Green
    fill_solid(leds, activeLeds, CRGB::Green);
    break;
  case 3: // Blue
    fill_solid(leds, activeLeds, CRGB::Blue);
    break;
  case 4: // Off
    fill_solid(leds, MAX_LEDS, CRGB::Black);
    break;

  case 10: // Rainbow Glitter
    fill_rainbow(leds, activeLeds, hue++, 7);
    if (random8() < 80)
      leds[random16(activeLeds)] += CRGB::White;
    break;
  case 11: // Candy Cane
    for (int i = 0; i < activeLeds; i++) {
      if (((i + hue / 4) % 4) < 2)
        leds[i] = CRGB::Red;
      else
        leds[i] = CRGB::White;
    }
    hue++;
    break;
  case 12: // Theater Chase
    for (int i = 0; i < activeLeds; i++) {
      if (((i + hue / 10) % 3) == 0)
        leds[i] = CRGB::Red;
      else
        leds[i] = CRGB::Black;
    }
  case 125: // Digital Clock
    pattern_clock(leds, activeLeds, hue, scrollOffset, scrollSpeed);
    break;
  case 200: // IP + Clock
    pattern_ip_clock(leds, activeLeds, hue, scrollOffset, scrollSpeed);
    break;
  case 255: // Streaming Mode - Do nothing here, handled in loop
    break;
  default:
    fill_solid(leds, activeLeds, CRGB::Black);
    break;
  }

  // Ensure any LEDs beyond active count are always black
  if (activeLeds < MAX_LEDS) {
    for (int i = activeLeds; i < MAX_LEDS; i++)
      leds[i] = CRGB::Black;
  }
}

void loop() {
  // State Machine Logic
  if (currentPattern == 200) {
    if (millis() - lastActivityTime > IDLE_TIMEOUT) {
      Serial.println("Idle timeout -> Switching to Streaming Mode");
      currentPattern = 255;
      lastUdpPacketTime = millis();
    }
  }

  // Streaming optimization: skip HTTP/OTA during active streaming
  bool inStreaming = (currentPattern == 255);
  // Relaxed check: Check HTTP every 200ms even if streaming
  static unsigned long lastHttpCheck = 0;
  if (!inStreaming || (millis() - lastHttpCheck > 200)) {
    ArduinoOTA.handle();
    server.handleClient();
    lastHttpCheck = millis();
  }

  // Auto-exit streaming if no UDP for 5 seconds -> REMOVED per request
  // if (inStreaming && millis() - lastUdpPacketTime > 5000) {
  //   currentPattern = 0;
  // }

  // UDP Streaming - only process if in streaming mode
  if (inStreaming) {
    int packetSize;
    while ((packetSize = udp.parsePacket())) {
      // Check for EXIT magic packet (4 bytes)
      if (packetSize == 4) {
        char buf[5] = {0};
        udp.read(buf, 4);
        if (memcmp(buf, "EXIT", 4) == 0) {
          Serial.println("UDP EXIT received -> Returning to Idle");
          currentPattern = 200;        // Return to IP+Clock
          lastActivityTime = millis(); // Reset idle timer
          break;
        }
        // Not EXIT, treat as small data (unlikely for LED data)
      } else {
        // Normal LED data packet
        udp.read((char *)leds, min(packetSize, (int)(MAX_LEDS * sizeof(CRGB))));
        FastLED.show();
        lastUdpPacketTime = millis();
        udpPacketsReceived++;
        udpPacketsDisplayed++;
      }
    }
  }

  // Non-blocking animation (only when not streaming)
  if (currentPattern != 255) {
    EVERY_N_MILLISECONDS(20) {
      if (!serverRunning) {
        // Flash red to indicate server not up
        static bool flashState = false;
        fill_solid(leds, MAX_LEDS, flashState ? CRGB::Red : CRGB::Black);
        flashState = !flashState;
        FastLED.show();
        return;
      }

      renderPatternFrame(currentPattern, leds, activeLeds, hue, scrollText,
                         scrollOffset, scrollSpeed);
      FastLED.show();
    }
  }
}

#include <Arduino.h>
#include <FastLED.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoOTA.h>
#include "patterns.h"

#ifndef OTA_PASSWORD
#error "OTA_PASSWORD is missing. Run `make ota-init` to generate config/ota.env or set OTA_PASSWORD in your environment."
#endif

#ifndef WIFI_SSID
#error "WIFI_SSID is missing. Define it via build flags or secrets header."
#endif

#ifndef WIFI_PASSWORD
#error "WIFI_PASSWORD is missing. Define it via build flags or secrets header."
#endif

#define LED_PIN     D4
#define MAX_LEDS    1500     // Maximum possible LEDs (memory buffer) - 9 strips × 144 = 1296
#define BRIGHTNESS  64
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB

CRGB leds[MAX_LEDS];
int activeLeds = 1296;       // 9 strips of 144 LEDs each
ESP8266WebServer server(80);

// 2D Grid Configuration
#define GRID_WIDTH  144      // LEDs per strip (horizontal) - 1 meter
#define GRID_HEIGHT 9        // Number of strips (vertical) - 45cm total (5cm spacing)

// Physical dimensions for aspect-ratio-aware patterns
#define LED_SPACING_H 6.9    // ~6.9mm between LEDs horizontally (1000mm / 144)
#define LED_SPACING_V 50.0   // 50mm between strips vertically (5cm)
#define ASPECT_RATIO 7.25    // V/H ratio (50/6.9) for circular patterns

// XY function is now in patterns.h

// State variables
int currentPattern = 0; // 0: Rainbow, 1: Red, 2: Green, 3: Blue, 4: Off
uint8_t hue = 0;
String scrollText = "HELLO WORLD";  // Text to scroll
int scrollOffset = 0;
int scrollSpeed = 80;  // Scroll speed in milliseconds (default 80ms)

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
  </script>
</head>
<body>
  <h1>LED Control (9×144 Grid)</h1>

  <div class="control-group">
    <label>Number of LEDs:</label>
    <form action="/set" method="get" style="display:inline;">
      <input type="number" name="c" id="c" min="1" max="1500" value="%LEDS%">
      <button type="submit" style="display:inline; width:auto; padding: 10px;">Set</button>
    </form>
  </div>

  <div class="tabs">
    <button class="tab active" onclick="showTab('tab-1d')">1D Patterns</button>
    <button class="tab" onclick="showTab('tab-2d')">2D Grid Patterns</button>
    <button class="tab" onclick="showTab('tab-text')">Scrolling Text</button>
    <button class="tab" onclick="showTab('tab-basic')">Basic</button>
  </div>

  <!-- Scrolling Text Tab -->
  <div id="tab-text" class="tab-content">
    <h2>Scrolling Text</h2>
    <div class="control-group">
      <label>Enter text to scroll:</label>
      <form action="/setText" method="get" style="margin-top: 10px;">
        <input type="text" name="text" id="text" value="%TEXT%" style="width: 80%; padding: 10px; font-size: 16px;" maxlength="100">

        <div style="margin-top: 15px;">
          <label>Scroll Speed: <span id="speedDisplay">%SPEED%</span> ms</label><br>
          <input type="range" name="speed" id="speedSlider" min="20" max="200" value="%SPEED%" style="width: 80%;"
                 oninput="document.getElementById('speedDisplay').textContent = this.value">
          <br>
          <small style="color: #888;">Lower = Faster, Higher = Slower</small>
        </div>

        <button type="submit" style="width: 100%; margin-top: 10px;">Update Text & Start Scrolling</button>
      </form>
    </div>
    <p style="color: #aaa; font-size: 14px;">Supports: A-Z, 0-9, space, !, ., -</p>
  </div>

  <!-- 2D Grid Patterns Tab -->
  <div id="tab-2d" class="tab-content">
    <h2>2D Grid Patterns (1m × 45cm)</h2>
    <button class="rainbow" onclick="setMode(100)">Horizontal Bars</button>
    <button class="cool" onclick="setMode(101)">Vertical Ripple</button>
    <button class="fire" onclick="setMode(102)">2D Fire Rising</button>
    <button class="blue" onclick="setMode(103)">Rain Drops</button>
    <button class="rainbow" onclick="setMode(104)">Vertical Equalizer</button>
    <button class="cool" onclick="setMode(105)">Scanning Lines</button>
    <button class="special" onclick="setMode(106)">Checkerboard</button>
    <button class="rainbow" onclick="setMode(107)">Diagonal Sweep</button>
    <button class="cool" onclick="setMode(108)">Vertical Wave</button>
    <button class="special" onclick="setMode(109)">Plasma 2D</button>

    <button class="green" onclick="setMode(110)">Matrix Rain 2D</button>
    <button class="cool" onclick="setMode(111)">Game of Life</button>
    <button class="blue" onclick="setMode(112)">Wave Pool</button>
    <button class="green" onclick="setMode(113)">Aurora 2D</button>
    <button class="fire" onclick="setMode(114)">Lava Lamp 2D</button>
    <button class="special" onclick="setMode(115)">2D Ripple</button>
    <button class="cool" onclick="setMode(116)">Starfield Parallax</button>
    <button class="fire" onclick="setMode(117)">Side Fire</button>
    <button class="rainbow" onclick="setMode(118)">Scrolling Rainbow</button>
    <button class="special" onclick="setMode(119)">Particle Fountain</button>
    <button class="cool" onclick="setMode(121)">Test Card</button>
  </div>

  <!-- 1D Patterns Tab -->
  <div id="tab-1d" class="tab-content active">

  <button class="rainbow" onclick="setMode(0)">Rainbow Loop</button>
  <button class="rainbow" onclick="setMode(10)">Rainbow Glitter</button>
  <button class="special" onclick="setMode(11)">Candy Cane</button>
  <button class="special" onclick="setMode(12)">Theater Chase</button>
  <button class="green" onclick="setMode(13)">Matrix Rain</button>
  <button class="cool" onclick="setMode(14)">Twinkle</button>
  <button class="special" onclick="setMode(15)">Police Lights</button>
  <button class="cool" onclick="setMode(16)">Running Lights</button>
  <button class="cool" onclick="setMode(17)">Snow Sparkle</button>
  <button class="special" onclick="setMode(18)">Color Wipe</button>
  <button class="cool" onclick="setMode(19)">Color Pulse</button>
  
  <button class="special" onclick="setMode(20)">Lightning</button>
  <button class="blue" onclick="setMode(21)">Ocean Waves</button>
  <button class="fire" onclick="setMode(22)">Lava Lamp</button>
  <button class="cool" onclick="setMode(23)">Meteor Rain</button>
  <button class="rainbow" onclick="setMode(24)">Pride</button>
  <button class="red" onclick="setMode(25)">Heartbeat</button>
  <button class="cool" onclick="setMode(26)">Comet</button>
  <button class="special" onclick="setMode(27)">Gradient</button>
  <button class="cool" onclick="setMode(28)">Random Colors</button>
  <button class="red" onclick="setMode(29)">Knight Rider</button>
  
  <button class="cool" onclick="setMode(30)">Breathing</button>
  <button class="special" onclick="setMode(31)">Strobe</button>
  <button class="special" onclick="setMode(32)">Pac-Man</button>
  <button class="cool" onclick="setMode(33)">Bouncing Balls</button>
  <button class="special" onclick="setMode(34)">USA Flag</button>
  <button class="special" onclick="setMode(35)">Christmas</button>
  <button class="rainbow" onclick="setMode(36)">Plasma</button>
  <button class="cool" onclick="setMode(37)">Scanner</button>
  <button class="cool" onclick="setMode(38)">Sparkle</button>
  <button class="rainbow" onclick="setMode(39)">Color Chase</button>
  
  <button class="rainbow" onclick="setMode(40)">Rainbow Wave</button>
  <button class="fire" onclick="setMode(41)">Dragon Breath</button>
  <button class="special" onclick="setMode(42)">Aurora</button>
  <button class="rainbow" onclick="setMode(43)">Disco Ball</button>
  <button class="blue" onclick="setMode(44)">Waterfall</button>
  <button class="special" onclick="setMode(45)">Neon Signs</button>
  <button class="special" onclick="setMode(46)">Traffic Light</button>
  <button class="green" onclick="setMode(47)">Binary Code</button>
  <button class="rainbow" onclick="setMode(48)">Rave</button>
  <button class="fire" onclick="setMode(49)">Sunset</button>
  <button class="fire" onclick="setMode(50)">Campfire</button>
  <button class="special" onclick="setMode(51)">Sparkler</button>
  <button class="blue" onclick="setMode(52)">Lighthouse</button>
  <button class="special" onclick="setMode(53)">SOS Morse</button>
  <button class="cool" onclick="setMode(54)">Meteor Shower</button>
  <button class="rainbow" onclick="setMode(55)">Rainbow Spiral</button>
  <button class="fire" onclick="setMode(56)">Lava Flow</button>
  <button class="blue" onclick="setMode(57)">Ice Cave</button>
  <button class="cool" onclick="setMode(58)">Fireflies</button>
  <button class="rainbow" onclick="setMode(59)">Circus</button>
  
  <button class="cool" onclick="setMode(60)">Warp Speed</button>
  <button class="special" onclick="setMode(61)">Radar Sweep</button>
  <button class="rainbow" onclick="setMode(62)">Equalizer Bars</button>
  <button class="green" onclick="setMode(63)">Snake</button>
  <button class="cool" onclick="setMode(64)">Pulse Wave</button>
  <button class="rainbow" onclick="setMode(65)">Color Explosion</button>
  <button class="green" onclick="setMode(66)">Digital Rain</button>
  <button class="red" onclick="setMode(67)">Heartbeat Wave</button>
  <button class="special" onclick="setMode(68)">Thunderstorm</button>
  <button class="rainbow" onclick="setMode(69)">Rainbow Fade</button>
  <button class="special" onclick="setMode(70)">Disco Strobe</button>
  <button class="special" onclick="setMode(71)">Biohazard</button>
  <button class="blue" onclick="setMode(72)">Ocean Depth</button>
  <button class="cool" onclick="setMode(73)">Pixel Sort</button>
  <button class="special" onclick="setMode(74)">Glitch</button>
  <button class="blue" onclick="setMode(75)">Tron</button>
  <button class="fire" onclick="setMode(76)">Ember</button>
  <button class="green" onclick="setMode(77)">Aurora Borealis</button>
  <button class="cool" onclick="setMode(78)">Neon Pulse</button>
  <button class="rainbow" onclick="setMode(79)">Rainbow Ripple</button>
  
  <button class="rainbow" onclick="setMode(80)">Kaleidoscope</button>
  <button class="special" onclick="setMode(81)">DNA Helix</button>
  <button class="fire" onclick="setMode(82)">Fireworks</button>
  <button class="rainbow" onclick="setMode(83)">VU Meter</button>
  <button class="cool" onclick="setMode(84)">Spinning Wheel</button>
  <button class="rainbow" onclick="setMode(85)">Color Bands</button>
  <button class="special" onclick="setMode(86)">Starfield</button>
  <button class="green" onclick="setMode(87)">Binary Counter</button>
  <button class="rainbow" onclick="setMode(88)">Breathing Rainbow</button>
  <button class="cool" onclick="setMode(89)">Wave Interference</button>
  <button class="cool" onclick="setMode(90)">Bouncing Ball</button>
  <button class="fire" onclick="setMode(91)">Color Temperature</button>
  <button class="special" onclick="setMode(92)">Police Siren</button>
  <button class="special" onclick="setMode(93)">Candy Stripes</button>
  <button class="cool" onclick="setMode(94)">Pixel Rain</button>
  <button class="special" onclick="setMode(95)">Energy Field</button>
  <button class="cool" onclick="setMode(96)">Orbit</button>
  <button class="cool" onclick="setMode(97)">Pulse Ring</button>
  <button class="rainbow" onclick="setMode(98)">Random Walk</button>
  <button class="fire" onclick="setMode(99)">Supernova</button>
  
  <button class="cool" onclick="setMode(5)">Confetti</button>
  <button class="cool" onclick="setMode(6)">Sinelon (Cylon)</button>
  <button class="cool" onclick="setMode(7)">BPM Pulse</button>
  <button class="cool" onclick="setMode(8)">Juggle</button>
  <button class="fire" onclick="setMode(9)">Fire</button>
  </div>

  <!-- Basic Controls Tab -->
  <div id="tab-basic" class="tab-content">
    <h2>Basic Controls</h2>
    <button class="red" onclick="setMode(1)">Red</button>
    <button class="green" onclick="setMode(2)">Green</button>
    <button class="blue" onclick="setMode(3)">Blue</button>
    <button class="off" onclick="setMode(4)">Turn Off</button>
  </div>
</body>
</html>
)=====";

void handleRoot() {
  String page = htmlPage;
  page.replace("%LEDS%", String(activeLeds));
  page.replace("%TEXT%", scrollText);
  page.replace("%SPEED%", String(scrollSpeed));
  server.send(200, "text/html", page);
}

void handleSet() {
  if (server.hasArg("m")) {
    currentPattern = server.arg("m").toInt();
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

void handleSetText() {
  if (server.hasArg("text")) {
    scrollText = server.arg("text");
    scrollText.toUpperCase(); // Convert to uppercase for font
    scrollOffset = 0; // Reset scroll position
    currentPattern = 120; // Switch to scrolling text mode
  }
  if (server.hasArg("speed")) {
    scrollSpeed = server.arg("speed").toInt();
    // Constrain to valid range
    if (scrollSpeed < 20) scrollSpeed = 20;
    if (scrollSpeed > 200) scrollSpeed = 200;
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

  // Parse sparse format: {"sparse":[[ledIndex,r,g,b],...], "scrollSpeed":80}
  int sparseStart = body.indexOf("\"sparse\":[");
  if (sparseStart == -1) {
    server.send(400, "text/plain", "Invalid JSON: missing sparse array");
    return;
  }

  int pixelCount = 0;
  int pos = sparseStart + 10; // Skip to after "sparse":[

  while (pos < body.length()) {
    // Skip whitespace
    while (pos < body.length() && (body[pos] == ' ' || body[pos] == '\n')) pos++;

    // Look for [ledIndex,r,g,b]
    if (body[pos] == '[') {
      pos++; // Skip [

      // Parse LED index
      int ledIdx = 0;
      while (pos < body.length() && body[pos] >= '0' && body[pos] <= '9') {
        ledIdx = ledIdx * 10 + (body[pos] - '0');
        pos++;
      }
      while (pos < body.length() && body[pos] == ',') pos++; // Skip comma

      // Parse R
      int r = 0;
      while (pos < body.length() && body[pos] >= '0' && body[pos] <= '9') {
        r = r * 10 + (body[pos] - '0');
        pos++;
      }
      while (pos < body.length() && body[pos] == ',') pos++; // Skip comma

      // Parse G
      int g = 0;
      while (pos < body.length() && body[pos] >= '0' && body[pos] <= '9') {
        g = g * 10 + (body[pos] - '0');
        pos++;
      }
      while (pos < body.length() && body[pos] == ',') pos++; // Skip comma

      // Parse B
      int b = 0;
      while (pos < body.length() && body[pos] >= '0' && body[pos] <= '9') {
        b = b * 10 + (body[pos] - '0');
        pos++;
      }

      // Store LED color at specific index
      if (ledIdx >= 0 && ledIdx < MAX_LEDS) {
        customPattern[ledIdx] = CRGB(r, g, b);
        pixelCount++;
      }

      // Skip to next element
      while (pos < body.length() && body[pos] != '[' && body[pos] != ']') pos++;
      if (body[pos] == ']') pos++; // Skip ]
      if (body[pos] == ',') pos++; // Skip comma between array elements
    } else if (body[pos] == ']') {
      // End of sparse array
      break;
    } else {
      pos++;
    }
  }

  if (pixelCount > 0) {
    hasCustomPattern = true;
    currentPattern = 122; // Switch to custom pattern mode
    server.send(200, "application/json", "{\"status\":\"success\",\"pixels\":" + String(pixelCount) + "}");
  } else {
    // Even with 0 pixels, we can show a blank pattern
    hasCustomPattern = true;
    currentPattern = 122;
    server.send(200, "application/json", "{\"status\":\"success\",\"pixels\":0}");
  }
}

// Global flag to track web server status
bool serverRunning = false;

void setup() {
  // Start Serial FIRST for debugging
  Serial.begin(115200);
  delay(100);
  Serial.println("\n\n=================================");
  Serial.println("ESP8266 LED Controller Starting");
  Serial.println("=================================");

  // LEDs
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, MAX_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);
  Serial.println("LEDs initialized");

  // WiFi - Station Mode (Connect to Home WiFi)
  const char* ssid = WIFI_SSID;
  const char* password = WIFI_PASSWORD;

  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  Serial.print("Password: ");
  Serial.println(password);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  // Wait for connection - indicate with blue flashing
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED) {
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

  // Connected - solid green for 500ms
  leds[0] = CRGB::Green;
  FastLED.show();
  delay(500);
  leds[0] = CRGB::Black;
  FastLED.show();

  // Print IP address
  Serial.println("\n\n*** WiFi Connected! ***");
  Serial.print("Connected to: ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

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
  server.on("/", handleRoot);
  server.on("/set", handleSet);
  server.on("/setText", handleSetText);
  server.on("/uploadPattern", HTTP_POST, handleUploadPattern);
  server.on("/uploadPattern", HTTP_OPTIONS, handleUploadPattern); // Handle CORS preflight

  // Increase max POST body size for pattern uploads (default is ~2KB, we need ~20KB)
  server.setContentLength(25000);

  server.begin();
  serverRunning = true; // server is up

  // Indicate server ready with cyan flash
  leds[0] = CRGB::Cyan;
  FastLED.show();
  delay(300);
  leds[0] = CRGB::Black;
  FastLED.show();
}

void renderPatternFrame(int currentPattern, CRGB* leds, int activeLeds, uint8_t& hue, String& scrollText, int& scrollOffset, int scrollSpeed) {
  // Always clear buffer first to handle changing sizes cleanly
  // Patterns that need fade/trail effect: 5, 6, 8, 10, 13, 14, 23, 26, 29, 32, 33, 37, 43, 44, 51, 52, 54, 58, 60, 61, 65, 66, 68, 73, 74, 75, 82, 86, 90, 94, 96, 98, 99, 103, 105, 110, 116, 119
  if (currentPattern != 0 && currentPattern != 5 && currentPattern != 6 && currentPattern != 8 && currentPattern != 10 && currentPattern != 13 && currentPattern != 14 && currentPattern != 23 && currentPattern != 26 && currentPattern != 29 && currentPattern != 32 && currentPattern != 33 && currentPattern != 37 && currentPattern != 43 && currentPattern != 44 && currentPattern != 51 && currentPattern != 52 && currentPattern != 54 && currentPattern != 58 && currentPattern != 60 && currentPattern != 61 && currentPattern != 65 && currentPattern != 66 && currentPattern != 68 && currentPattern != 73 && currentPattern != 74 && currentPattern != 75 && currentPattern != 82 && currentPattern != 86 && currentPattern != 90 && currentPattern != 94 && currentPattern != 96 && currentPattern != 98 && currentPattern != 99 && currentPattern != 103 && currentPattern != 105 && currentPattern != 110 && currentPattern != 116 && currentPattern != 119) {
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
      case 5: // Confetti
        {
          fadeToBlackBy(leds, activeLeds, 10);
          int pos = random16(activeLeds);
          leds[pos] += CHSV(hue++ + random8(64), 200, 255);
        }
        break;
      case 6: // Sinelon
        {
          fadeToBlackBy(leds, activeLeds, 20);
          int pos2 = beatsin16(13, 0, activeLeds-1);
          leds[pos2] += CHSV(hue++, 255, 192);
        }
        break;
      case 7: // BPM
        {
          uint8_t beat = beatsin8(62, 64, 255);
          for(int i = 0; i < activeLeds; i++) {
            leds[i] = ColorFromPalette(PartyColors_p, hue+(i*2), beat-hue+(i*10));
          }
          hue++;
        }
        break;
      case 8: // Juggle
        {
          fadeToBlackBy(leds, activeLeds, 20);
          byte dothue = 0;
          for(int i = 0; i < 8; i++) {
            leds[beatsin16(i+7, 0, activeLeds-1)] |= CHSV(dothue, 200, 255);
            dothue += 32;
          }
        }
        break;
      case 9: // Fire
        {
          static byte heat[MAX_LEDS];
          for( int i = 0; i < activeLeds; i++) heat[i] = qsub8( heat[i],  random8(0, ((55 * 10) / activeLeds) + 2));
          for( int k= activeLeds - 1; k >= 2; k--) heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2] ) / 3;
          if( random8() < 120 ) { int y = random8(7); heat[y] = qadd8( heat[y], random8(160,255) ); }
          for( int j = 0; j < activeLeds; j++) leds[j] = HeatColor( heat[j]);
        }
        break;
      case 10: // Rainbow Glitter
        fill_rainbow(leds, activeLeds, hue++, 7);
        if( random8() < 80) leds[ random16(activeLeds) ] += CRGB::White;
        break;
      case 11: // Candy Cane
        for (int i = 0; i < activeLeds; i++) {
          if (((i + hue/4) % 4) < 2) leds[i] = CRGB::Red;
          else leds[i] = CRGB::White;
        }
        hue++;
        break;
      case 12: // Theater Chase
        for (int i = 0; i < activeLeds; i++) {
          if (((i + hue/10) % 3) == 0) leds[i] = CRGB::Red;
          else leds[i] = CRGB::Black;
        }
        hue++;
        break;
      case 13: // Matrix Rain
        fadeToBlackBy(leds, activeLeds, 20);
        if (random8() < 25) leds[random16(activeLeds)] = CRGB::Green;
        break;
      case 14: // Twinkle
        fadeToBlackBy(leds, activeLeds, 10);
        if (random8() < 80) leds[random16(activeLeds)] = CRGB::White;
        break;
      case 15: // Police Lights
        for (int i = 0; i < activeLeds; i++) {
          if (((i + hue/16) % 8) < 4) leds[i] = CRGB::Blue;
          else leds[i] = CRGB::Red;
        }
        hue+=4;
        break;
      case 16: // Running Lights
        for(int i=0; i<activeLeds; i++) {
          leds[i] = CHSV(hue, 255, (sin8(i*10 + hue*4) + 128)/2);
        }
        hue++;
        break;
      case 17: // Snow Sparkle
        fill_solid(leds, activeLeds, CRGB(16, 16, 16)); // Grey background
        if (random8() < 20) leds[random16(activeLeds)] = CRGB::White;
        break;
      case 18: // Color Wipe
        {
          static int wipePos = 0;
          fill_solid(leds, wipePos, CHSV(hue, 255, 255));
          wipePos++;
          if (wipePos >= activeLeds) { wipePos = 0; hue += 32; }
        }
        break;
      case 19: // Color Pulse
        fill_solid(leds, activeLeds, CHSV(hue, 255, beatsin8(30, 50, 255)));
        hue++;
        break;
      case 20: // Lightning
        {
          static unsigned long lastFlash = 0;
          if (millis() - lastFlash > random(100, 1000)) {
            fill_solid(leds, activeLeds, CRGB::White);
            lastFlash = millis();
          } else {
            fill_solid(leds, activeLeds, CRGB::Black);
          }
        }
        break;
      case 21: // Ocean Waves
        for(int i=0; i<activeLeds; i++) {
          uint8_t wave1 = sin8((i * 10) + (hue * 2));
          uint8_t wave2 = sin8((i * 15) + (hue * 3));
          leds[i] = CHSV(160, 255, (wave1 + wave2) / 2);
        }
        hue++;
        break;
      case 22: // Lava Lamp
        for(int i=0; i<activeLeds; i++) {
          uint8_t blob1 = sin8((i * 5) + (hue));
          uint8_t blob2 = sin8((i * 7) + (hue * 2));
          leds[i] = CHSV(hue/4, 255, (blob1 + blob2) / 2);
        }
        hue++;
        break;
      case 23: // Meteor Rain
        {
          fadeToBlackBy(leds, activeLeds, 64);
          int pos = beatsin16(20, 0, activeLeds-1);
          leds[pos] = CHSV(hue, 200, 255);
          hue++;
        }
        break;
      case 24: // Pride
        fill_rainbow(leds, activeLeds, hue, 255/activeLeds);
        hue++;
        break;
      case 25: // Heartbeat
        {
          uint8_t beat1 = beatsin8(60, 0, 255);
          uint8_t beat2 = beatsin8(120, 0, 255);
          uint8_t combined = qadd8(beat1, beat2);
          fill_solid(leds, activeLeds, CRGB(combined, 0, 0));
        }
        break;
      case 26: // Comet
        {
          fadeToBlackBy(leds, activeLeds, 128);
          static int cometPos = 0;
          leds[cometPos] = CHSV(hue, 255, 255);
          if (cometPos > 0) leds[cometPos-1] = CHSV(hue, 255, 128);
          if (cometPos > 1) leds[cometPos-2] = CHSV(hue, 255, 64);
          cometPos++;
          if (cometPos >= activeLeds) { cometPos = 0; hue += 32; }
        }
        break;
      case 27: // Gradient
        fill_gradient_RGB(leds, 0, CHSV(hue, 255, 255), activeLeds-1, CHSV(hue+128, 255, 255));
        hue++;
        break;
      case 28: // Random Colors
        EVERY_N_MILLISECONDS(100) {
          for(int i=0; i<activeLeds; i++) {
            leds[i] = CHSV(random8(), 255, 255);
          }
        }
        break;
      case 29: // Knight Rider
        {
          fadeToBlackBy(leds, activeLeds, 64);
          int pos = beatsin16(13, 0, activeLeds-1);
          leds[pos] = CRGB::Red;
          if (pos > 0) leds[pos-1] = CRGB(64, 0, 0);
          if (pos < activeLeds-1) leds[pos+1] = CRGB(64, 0, 0);
        }
        break;
      case 30: // Breathing
        {
          uint8_t brightness = beatsin8(20, 50, 255);
          fill_solid(leds, activeLeds, CHSV(hue, 255, brightness));
          EVERY_N_SECONDS(5) { hue += 32; }
        }
        break;
      case 31: // Strobe
        {
          static unsigned long lastFlash = 0;
          if (millis() - lastFlash > 100) {
            fill_solid(leds, activeLeds, random8() % 2 ? CRGB::White : CRGB::Black);
            lastFlash = millis();
          }
        }
        break;
      case 32: // Pac-Man
        {
          fadeToBlackBy(leds, activeLeds, 128);
          int pacPos = beatsin16(10, 0, activeLeds-1);
          leds[pacPos] = CRGB::Yellow;
          for(int i=0; i<5; i++) {
            int ghostPos = beatsin16(8+i, 0, activeLeds-1, 0, i*10000);
            if (ghostPos < activeLeds) leds[ghostPos] = CRGB::White;
          }
        }
        break;
      case 33: // Bouncing Balls
        {
          static float positions[3] = {0, activeLeds/3, activeLeds*2/3};
          static float velocities[3] = {0, 0, 0};
          fill_solid(leds, activeLeds, CRGB::Black);
          for(int i=0; i<3; i++) {
            velocities[i] += 0.5; // gravity
            positions[i] += velocities[i];
            if (positions[i] >= activeLeds-1) {
              positions[i] = activeLeds-1;
              velocities[i] *= -0.9; // bounce with damping
            }
            leds[(int)positions[i]] = CHSV(i*85, 255, 255);
          }
        }
        break;
      case 34: // USA Flag
        for(int i=0; i<activeLeds; i++) {
          if (i < activeLeds/3) leds[i] = CRGB::Red;
          else if (i < activeLeds*2/3) leds[i] = CRGB::White;
          else leds[i] = CRGB::Blue;
        }
        break;
      case 35: // Christmas
        for(int i=0; i<activeLeds; i++) {
          if (((i + hue/4) % 2) == 0) leds[i] = CRGB::Red;
          else leds[i] = CRGB::Green;
        }
        hue++;
        break;
      case 36: // Plasma
        for(int i=0; i<activeLeds; i++) {
          uint8_t wave1 = sin8((i * 8) + (hue));
          uint8_t wave2 = sin8((i * 12) + (hue * 2));
          uint8_t wave3 = sin8((i * 16) + (hue * 3));
          leds[i] = CHSV((wave1 + wave2 + wave3) / 3, 255, 255);
        }
        hue++;
        break;
      case 37: // Scanner
        {
          fadeToBlackBy(leds, activeLeds, 64);
          for(int i=0; i<4; i++) {
            int pos = beatsin16(13+i*2, 0, activeLeds-1, 0, i*8192);
            leds[pos] = CHSV(hue + i*64, 255, 255);
          }
          hue++;
        }
        break;
      case 38: // Sparkle
        fill_solid(leds, activeLeds, CHSV(hue, 255, 32));
        if (random8() < 40) leds[random16(activeLeds)] = CRGB::White;
        EVERY_N_SECONDS(3) { hue += 32; }
        break;
      case 39: // Color Chase
        {
          static int chasePos = 0;
          for(int i=0; i<activeLeds; i++) {
            int diff = abs(i - chasePos);
            if (diff < 5) leds[i] = CHSV(hue, 255, 255);
            else leds[i] = CRGB::Black;
          }
          chasePos++;
          if (chasePos >= activeLeds) { chasePos = 0; hue += 32; }
        }
        break;
      case 40: // Rainbow Wave
        for(int i=0; i<activeLeds; i++) {
          leds[i] = CHSV(hue + (i * 256 / activeLeds), 255, beatsin8(10, 128, 255, 0, i*4));
        }
        hue++;
        break;
      case 41: // Dragon Breath
        for(int i=0; i<activeLeds; i++) {
          uint8_t flicker = random8(20);
          leds[i] = CHSV(0, 255, qadd8(220 - flicker, beatsin8(40, 0, 50)));
        }
        break;
      case 42: // Aurora (Northern Lights)
        for(int i=0; i<activeLeds; i++) {
          uint8_t wave = sin8((i * 10) + (hue * 2));
          uint8_t colorIndex = 96 + (wave / 4); // Green-ish to purple
          leds[i] = CHSV(colorIndex, 200, wave);
        }
        hue++;
        break;
      case 43: // Disco Ball
        EVERY_N_MILLISECONDS(50) {
          int spot = random16(activeLeds);
          leds[spot] = CHSV(random8(), 255, 255);
        }
        fadeToBlackBy(leds, activeLeds, 30);
        break;
      case 44: // Waterfall
        {
          static int dropPos = 0;
          for(int i=activeLeds-1; i>0; i--) {
            leds[i] = leds[i-1];
          }
          leds[0] = CHSV(160, 255, beatsin8(20, 100, 255));
        }
        break;
      case 45: // Neon Signs
        for(int i=0; i<activeLeds; i++) {
          if ((i % 10) < 5) leds[i] = CHSV(hue, 255, 255);
          else leds[i] = CHSV(hue + 128, 255, 255);
        }
        EVERY_N_SECONDS(2) { hue += 32; }
        break;
      case 46: // Traffic Light
        {
          static unsigned long lastChange = 0;
          static int phase = 0;
          if (millis() - lastChange > 2000) {
            phase = (phase + 1) % 3;
            lastChange = millis();
          }
          CRGB color = (phase == 0) ? CRGB::Green : (phase == 1) ? CRGB::Yellow : CRGB::Red;
          fill_solid(leds, activeLeds, color);
        }
        break;
      case 47: // Binary Code
        for(int i=0; i<activeLeds; i++) {
          leds[i] = ((random8() % 2) && (i % 2 == (hue/10) % 2)) ? CRGB::Green : CRGB::Black;
        }
        hue++;
        break;
      case 48: // Rave
        for(int i=0; i<activeLeds; i++) {
          leds[i] = CHSV(beatsin8(30 + i, 0, 255), 255, beatsin8(15, 100, 255));
        }
        break;
      case 49: // Sunset
        for(int i=0; i<activeLeds; i++) {
          float pos = (float)i / activeLeds;
          if (pos < 0.5) {
            leds[i] = CRGB(255, 60 + pos*40, pos*200);
          } else {
            leds[i] = CRGB(255 - (pos-0.5)*500, 100 - (pos-0.5)*180, 100 - (pos-0.5)*180);
          }
        }
        break;
      case 50: // Campfire
        for(int i=0; i<activeLeds; i++) {
          uint8_t flicker = random8(60);
          leds[i] = CRGB(200 - flicker, 100 - (flicker/2), 0);
        }
        break;
      case 51: // Sparkler
        fadeToBlackBy(leds, activeLeds, 50);
        for(int i=0; i<10; i++) {
          if (random8() < 50) {
            leds[random16(activeLeds)] = CRGB::White;
          }
        }
        break;
      case 52: // Lighthouse
        {
          fadeToBlackBy(leds, activeLeds, 64);
          int beam = beatsin16(8, 0, activeLeds-1);
          for(int i=beam-2; i<=beam+2; i++) {
            if (i >= 0 && i < activeLeds) {
              leds[i] = CRGB::White;
            }
          }
        }
        break;
      case 53: // SOS Morse Code
        {
          static unsigned long lastBlink = 0;
          static int pattern[] = {1,0,1,0,1,0,0,3,0,3,0,3,0,0,1,0,1,0,1,0,0,0}; // S=..., O=---, S=...
          static int patternIdx = 0;
          int dotTime = 200;
          
          if (millis() - lastBlink > dotTime * pattern[patternIdx]) {
            patternIdx = (patternIdx + 1) % 22;
            lastBlink = millis();
          }
          fill_solid(leds, activeLeds, (pattern[patternIdx] > 0) ? CRGB::Red : CRGB::Black);
        }
        break;
      case 54: // Meteor Shower
        {
          fadeToBlackBy(leds, activeLeds, 30);
          for(int i=0; i<5; i++) {
            int meteor = beatsin16(20 + i*4, 0, activeLeds-1, 0, i*13000);
            if (meteor < activeLeds) leds[meteor] = CHSV(hue + i*50, 200, 255);
          }
          hue++;
        }
        break;
      case 55: // Rainbow Spiral
        for(int i=0; i<activeLeds; i++) {
          leds[i] = CHSV((hue + (i * 10)) % 256, 255, 255);
        }
        hue += 2;
        break;
      case 56: // Lava Flow
        for(int i=0; i<activeLeds; i++) {
          uint8_t heat = qsub8(inoise8(i*20, hue), abs8(i - (activeLeds/2)) * 2);
          leds[i] = HeatColor(heat);
        }
        hue++;
        break;
      case 57: // Ice Cave
        for(int i=0; i<activeLeds; i++) {
          uint8_t brightness = inoise8(i*30, hue);
          leds[i] = CHSV(160, 255, brightness);
        }
        hue++;
        break;
      case 58: // Fireflies
        fadeToBlackBy(leds, activeLeds, 10);
        if (random8() < 20) {
          int pos = random16(activeLeds);
          leds[pos] = CHSV(32, 200, 255);
        }
        break;
      case 59: // Circus
        for(int i=0; i<activeLeds; i++) {
          if (((i + hue/8) % 5) == 0) leds[i] = CHSV(random8(), 255, 255);
          else leds[i] = CRGB::White;
        }
        hue++;
        break;
      case 60: // Warp Speed
        {
          static int warpPos[10];
          for(int i=0; i<10; i++) {
            warpPos[i] += (i+1)*2;
            if (warpPos[i] >= activeLeds) warpPos[i] = 0;
            leds[warpPos[i]] = CHSV(160 + i*10, 255, 255);
          }
          fadeToBlackBy(leds, activeLeds, 100);
        }
        break;
      case 61: // Radar Sweep
        {
          fadeToBlackBy(leds, activeLeds, 20);
          int sweepPos = beatsin16(10, 0, activeLeds-1);
          for(int i=-5; i<=5; i++) {
            int pos = sweepPos + i;
            if (pos >= 0 && pos < activeLeds) {
              leds[pos] = CHSV(96, 255, 255 - abs(i)*40);
            }
          }
        }
        break;
      case 62: // Equalizer Bars
        for(int i=0; i<activeLeds; i++) {
          int bar = i / (activeLeds/8);
          int height = beatsin8(30 + bar*5, 0, 255);
          if (i % (activeLeds/8) < height * (activeLeds/8) / 255) {
            leds[i] = CHSV(bar*32, 255, 255);
          } else {
            leds[i] = CRGB::Black;
          }
        }
        break;
      case 63: // Snake
        {
          static int snakePos = 0;
          static int snakeLen = 10;
          fill_solid(leds, activeLeds, CRGB::Black);
          for(int i=0; i<snakeLen; i++) {
            int pos = (snakePos - i + activeLeds) % activeLeds;
            leds[pos] = CHSV(96, 255, 255 - i*20);
          }
          snakePos = (snakePos + 1) % activeLeds;
        }
        break;
      case 64: // Pulse Wave
        for(int i=0; i<activeLeds; i++) {
          uint8_t wave = sin8((i * 20) + (hue * 3));
          leds[i] = CHSV(hue, 255, wave);
        }
        hue += 2;
        break;
      case 65: // Color Explosion
        {
          static int explosionCenter = activeLeds/2;
          static int explosionRadius = 0;
          fadeToBlackBy(leds, activeLeds, 20);
          for(int i=0; i<activeLeds; i++) {
            int dist = abs(i - explosionCenter);
            if (dist == explosionRadius) {
              leds[i] = CHSV(hue, 255, 255);
            }
          }
          explosionRadius++;
          if (explosionRadius > activeLeds/2) {
            explosionRadius = 0;
            explosionCenter = random16(activeLeds);
            hue += 32;
          }
        }
        break;
      case 66: // Digital Rain
        {
          for(int i=activeLeds-1; i>0; i--) {
            leds[i] = leds[i-1];
            leds[i].fadeToBlackBy(10);
          }
          if (random8() < 30) {
            leds[0] = CRGB::Green;
          } else {
            leds[0] = CRGB::Black;
          }
        }
        break;
      case 67: // Heartbeat Wave
        {
          uint8_t beat = beatsin8(60, 0, 255);
          for(int i=0; i<activeLeds; i++) {
            uint8_t wave = sin8((i * 10) + (hue));
            leds[i] = CRGB(beat, 0, wave/4);
          }
          hue++;
        }
        break;
      case 68: // Thunderstorm
        {
          static unsigned long lastFlash = 0;
          fadeToBlackBy(leds, activeLeds, 30);
          if (random8() < 2) {
            fill_solid(leds, activeLeds, CRGB::White);
            lastFlash = millis();
          } else if (millis() - lastFlash < 100) {
            fill_solid(leds, activeLeds, CRGB(128, 128, 255));
          } else {
            for(int i=0; i<activeLeds; i++) {
              leds[i] = CRGB(0, 0, random8(20));
            }
          }
        }
        break;
      case 69: // Rainbow Fade
        fill_solid(leds, activeLeds, CHSV(hue, 255, 255));
        hue++;
        break;
      case 70: // Disco Strobe
        {
          static unsigned long lastChange = 0;
          if (millis() - lastChange > 100) {
            fill_solid(leds, activeLeds, CHSV(random8(), 255, random8() % 2 ? 255 : 0));
            lastChange = millis();
          }
        }
        break;
      case 71: // Biohazard
        for(int i=0; i<activeLeds; i++) {
          if (((i + hue/4) % 3) == 0) leds[i] = CRGB::Yellow;
          else leds[i] = CRGB::Black;
        }
        hue++;
        break;
      case 72: // Ocean Depth
        for(int i=0; i<activeLeds; i++) {
          uint8_t depth = 255 - (i * 255 / activeLeds);
          uint8_t shimmer = sin8((i * 5) + hue);
          leds[i] = CHSV(160, 255, (depth + shimmer) / 2);
        }
        hue++;
        break;
      case 73: // Pixel Sort
        {
          static uint8_t sortPhase = 0;
          static unsigned long lastSwap = 0;
          static bool initialized = false;
          
          // Initialize with distinct colors only once
          if (!initialized) {
            for(int i=0; i<activeLeds; i++) {
              // Use very distinct hues and medium brightness
              leds[i] = CHSV(random8() & 0xE0, 255, random8(100, 180));
            }
            initialized = true;
            sortPhase = 0;
          }
          
          // Slow down the sorting - only swap every 100ms
          if (millis() - lastSwap > 100) {
            // Bubble sort by HUE - one pass per frame
            for(int i=0; i<activeLeds-1; i++) {
              if ((i + sortPhase) % 2 == 0) {
                // Sort by hue instead of brightness
                CHSV hsv1 = rgb2hsv_approximate(leds[i]);
                CHSV hsv2 = rgb2hsv_approximate(leds[i+1]);
                if (hsv1.hue > hsv2.hue) {
                  CRGB temp = leds[i];
                  leds[i] = leds[i+1];
                  leds[i+1] = temp;
                }
              }
            }
            sortPhase++;
            lastSwap = millis();
          }
          
          // Keep sorted for 3 seconds before scrambling
          if (sortPhase > 200) {
            initialized = false;
            sortPhase = 0;
          }
        }
        break;
      case 74: // Glitch
        {
          static unsigned long lastGlitch = 0;
          if (random8() < 5 || millis() - lastGlitch < 50) {
            int glitchPos = random16(activeLeds);
            int glitchLen = random8(5, 20);
            for(int i=0; i<glitchLen && (glitchPos+i)<activeLeds; i++) {
              leds[glitchPos+i] = CHSV(random8(), 255, 255);
            }
            lastGlitch = millis();
          } else {
            fadeToBlackBy(leds, activeLeds, 50);
          }
        }
        break;
      case 75: // Tron
        {
          static int tronPos = 0;
          fadeToBlackBy(leds, activeLeds, 30);
          leds[tronPos] = CRGB(0, 255, 255);
          if (tronPos > 0) leds[tronPos-1] = CRGB(0, 128, 255);
          if (tronPos > 1) leds[tronPos-2] = CRGB(0, 64, 255);
          tronPos = (tronPos + 1) % activeLeds;
        }
        break;
      case 76: // Ember
        for(int i=0; i<activeLeds; i++) {
          uint8_t heat = qsub8(inoise8(i*15, hue*2), abs8(i - (activeLeds/2)));
          leds[i] = CRGB(heat, heat/4, 0);
        }
        hue++;
        break;
      case 77: // Aurora Borealis
        for(int i=0; i<activeLeds; i++) {
          uint8_t wave1 = sin8((i * 7) + (hue * 2));
          uint8_t wave2 = sin8((i * 11) + (hue * 3));
          uint8_t colorIndex = 80 + (wave1 / 6);
          leds[i] = CHSV(colorIndex, 200, (wave1 + wave2) / 2);
        }
        hue++;
        break;
      case 78: // Neon Pulse
        {
          uint8_t pulse = beatsin8(30, 50, 255);
          for(int i=0; i<activeLeds; i++) {
            uint8_t colorSection = (i * 256) / activeLeds;
            leds[i] = CHSV(colorSection, 255, pulse);
          }
        }
        break;
      case 79: // Rainbow Ripple
        {
          static int rippleCenter = activeLeds/2;
          for(int i=0; i<activeLeds; i++) {
            int dist = abs(i - rippleCenter);
            uint8_t brightness = sin8((dist * 20) - (hue * 3));
            leds[i] = CHSV(hue + dist*5, 255, brightness);
          }
          hue+=2;
          EVERY_N_SECONDS(3) {
            rippleCenter = random16(activeLeds);
          }
        }
        break;
      case 80: // Kaleidoscope
        for(int i=0; i<activeLeds/2; i++) {
          uint8_t color = sin8((i * 10) + hue);
          leds[i] = CHSV(color, 255, 255);
          leds[activeLeds-1-i] = CHSV(color, 255, 255);
        }
        hue+=2;
        break;
      case 81: // DNA Helix
        for(int i=0; i<activeLeds; i++) {
          uint8_t wave1 = sin8((i * 15) + hue);
          uint8_t wave2 = sin8((i * 15) - hue);
          if (wave1 > 128) leds[i] = CRGB::Blue;
          else if (wave2 > 128) leds[i] = CRGB::Green;
          else leds[i] = CRGB::Black;
        }
        hue++;
        break;
      case 82: // Fireworks
        {
          static unsigned long lastBurst = 0;
          static int burstPos = 0;
          static int burstPhase = 0;
          fadeToBlackBy(leds, activeLeds, 20);
          if (millis() - lastBurst > 2000) {
            burstPos = random16(activeLeds);
            burstPhase = 0;
            lastBurst = millis();
          }
          if (burstPhase < 20) {
            for(int i=-burstPhase; i<=burstPhase; i++) {
              int pos = burstPos + i;
              if (pos >= 0 && pos < activeLeds) {
                leds[pos] = CHSV(hue, 255, 255 - burstPhase*10);
              }
            }
            burstPhase++;
          }
        }
        break;
      case 83: // VU Meter
        {
          int level = beatsin8(40, 0, activeLeds);
          for(int i=0; i<activeLeds; i++) {
            if (i < level) {
              if (i < activeLeds/3) leds[i] = CRGB::Green;
              else if (i < activeLeds*2/3) leds[i] = CRGB::Yellow;
              else leds[i] = CRGB::Red;
            } else {
              leds[i] = CRGB::Black;
            }
          }
        }
        break;
      case 84: // Spinning Wheel
        for(int i=0; i<activeLeds; i++) {
          uint8_t spoke = ((i * 8 / activeLeds) + (hue / 32)) % 8;
          if (spoke % 2) {
            leds[i] = CHSV(spoke * 32, 255, 255);
          } else {
            leds[i] = CRGB::Black;
          }
        }
        hue+=2;
        break;
      case 85: // Color Bands
        for(int i=0; i<activeLeds; i++) {
          leds[i] = CHSV(((i + hue) * 256 / activeLeds) % 256, 255, 255);
        }
        hue++;
        break;
      case 86: // Starfield
        fadeToBlackBy(leds, activeLeds, 10);
        if (random8() < 30) {
          leds[random16(activeLeds)] = CRGB::White;
        }
        break;
      case 87: // Binary Counter
        {
          static uint8_t counter = 0;
          for(int i=0; i<min(8, activeLeds); i++) {
            leds[i] = (counter & (1 << i)) ? CRGB::Green : CRGB::Black;
          }
          EVERY_N_MILLISECONDS(200) { counter++; }
        }
        break;
      case 88: // Breathing Rainbow
        {
          uint8_t brightness = beatsin8(20, 50, 255);
          fill_rainbow(leds, activeLeds, hue, 255/activeLeds);
          for(int i=0; i<activeLeds; i++) {
            leds[i].nscale8(brightness);
          }
          hue++;
        }
        break;
      case 89: // Wave Interference
        for(int i=0; i<activeLeds; i++) {
          uint8_t wave1 = sin8((i * 10) + (hue * 2));
          uint8_t wave2 = sin8((i * 15) + (hue * 3));
          leds[i] = CHSV(hue, 255, (wave1 + wave2) / 2);
        }
        hue++;
        break;
      case 90: // Bouncing Ball
        {
          static float ballPos = 0;
          static float ballVel = 0;
          fadeToBlackBy(leds, activeLeds, 100);
          ballVel += 0.5;
          ballPos += ballVel;
          if (ballPos >= activeLeds-1) {
            ballPos = activeLeds-1;
            ballVel *= -0.85;
          }
          leds[(int)ballPos] = CHSV(hue, 255, 255);
          EVERY_N_SECONDS(10) { hue += 32; }
        }
        break;
      case 91: // Color Temperature - Moving Hot Spot (with fade & slower speed)
        {
          static int hotSpot = 0;
          static unsigned long lastMove = 0;
          // Fade trail so the hot spot leaves a subtle glow
          fadeToBlackBy(leds, activeLeds, 20);
          // Move the hot spot every 100 ms for a smoother pace
          if (millis() - lastMove > 100) {
            for(int i=0; i<activeLeds; i++) {
              int dist = abs(i - hotSpot);
              float temp;
              if (dist < 5) {
                // Very hot - white/yellow
                temp = 1.0 - (dist / 5.0);
                leds[i] = CRGB(255, 255, 255 - temp * 100);
              } else if (dist < 15) {
                // Hot - orange/red
                temp = (dist - 5.0) / 10.0;
                leds[i] = CRGB(255, 200 - temp * 150, 50 - temp * 50);
              } else if (dist < 30) {
                // Warm - dark red
                temp = (dist - 15.0) / 15.0;
                leds[i] = CRGB(255 - temp * 205, 50 - temp * 50, 0);
              } else {
                // Cool - blue
                leds[i] = CRGB(0, 0, 100);
              }
            }
            hotSpot++;
            if (hotSpot >= activeLeds) hotSpot = 0;
            lastMove = millis();
          }
        }
        break;
      case 92: // Police Siren
        {
          static unsigned long lastSwitch = 0;
          static bool isRed = true;
          if (millis() - lastSwitch > 300) {
            isRed = !isRed;
            lastSwitch = millis();
          }
          for(int i=0; i<activeLeds; i++) {
            if (i < activeLeds/2) leds[i] = isRed ? CRGB::Red : CRGB::Black;
            else leds[i] = isRed ? CRGB::Black : CRGB::Blue;
          }
        }
        break;
      case 93: // Candy Stripes
        for(int i=0; i<activeLeds; i++) {
          int stripe = (i + hue/4) % 6;
          if (stripe < 3) leds[i] = CRGB::Red;
          else leds[i] = CRGB::White;
        }
        hue++;
        break;
      case 94: // Pixel Rain
        {
          for(int i=activeLeds-1; i>0; i--) {
            leds[i] = leds[i-1];
          }
          if (random8() < 40) {
            leds[0] = CHSV(random8(), 255, 255);
          } else {
            leds[0] = CRGB::Black;
          }
        }
        break;
      case 95: // Energy Field
        for(int i=0; i<activeLeds; i++) {
          uint8_t noise = inoise8(i*30, hue*2);
          leds[i] = CHSV(160, 255, noise);
        }
        hue++;
        break;
      case 96: // Orbit
        {
          fadeToBlackBy(leds, activeLeds, 30);
          int planet1 = beatsin16(10, 0, activeLeds-1);
          int planet2 = beatsin16(13, 0, activeLeds-1, 0, 16384);
          leds[planet1] = CRGB::Yellow;
          leds[planet2] = CRGB::Blue;
        }
        break;
      case 97: // Pulse Ring
        {
          static int ringPos = 0;
          static int ringSize = 5;
          fill_solid(leds, activeLeds, CRGB::Black);
          for(int i=-ringSize; i<=ringSize; i++) {
            int pos = ringPos + i;
            if (pos >= 0 && pos < activeLeds) {
              leds[pos] = CHSV(hue, 255, 255 - abs(i)*40);
            }
          }
          ringPos++;
          if (ringPos >= activeLeds + ringSize) {
            ringPos = -ringSize;
            hue += 32;
          }
        }
        break;
      case 98: // Random Walk
        {
          static int walker = activeLeds/2;
          fadeToBlackBy(leds, activeLeds, 20);
          walker += random8(3) - 1;
          if (walker < 0) walker = 0;
          if (walker >= activeLeds) walker = activeLeds-1;
          leds[walker] = CHSV(hue, 255, 255);
          hue++;
        }
        break;
      case 99: // Supernova
        {
          static unsigned long lastNova = 0;
          static int novaPhase = 0;
          if (millis() - lastNova > 3000 || novaPhase > 0) {
            if (novaPhase == 0) lastNova = millis();
            int brightness = (novaPhase < 10) ? novaPhase * 25 : max(0, 255 - (novaPhase - 10) * 10);
            fill_solid(leds, activeLeds, CRGB(brightness, brightness, brightness/2));
            novaPhase++;
            if (novaPhase > 35) novaPhase = 0;
          } else {
            fadeToBlackBy(leds, activeLeds, 5);
          }
        }
        break;
      case 100: // Horizontal Bars - Each strip a different cycling color
        pattern_horizontal_bars(leds, activeLeds, hue);
        break;

      case 101: // Vertical Ripple - Waves moving vertically
        pattern_vertical_ripple(leds, activeLeds, hue);
        break;

      case 102: // 2D Fire Rising - Fire effect rising from bottom
        pattern_fire_rising(leds, activeLeds, hue);
        break;

      case 103: // Rain Drops - Droplets falling down
        pattern_rain_drops(leds, activeLeds, hue);
        break;

      case 104: // Vertical Equalizer - Each strip is a bar
        pattern_vertical_equalizer(leds, activeLeds, hue);
        break;

      case 105: // Scanning Lines - Horizontal lines moving up/down
        pattern_scanning_lines(leds, activeLeds, hue);
        break;

      case 106: // Checkerboard - Classic 2D pattern
        pattern_checkerboard(leds, activeLeds, hue);
        break;

      case 107: // Diagonal Sweep - Diagonal lines moving
        pattern_diagonal_sweep(leds, activeLeds, hue);
        break;

      case 108: // Vertical Wave - Sine wave across strips
        pattern_vertical_wave(leds, activeLeds, hue);
        break;

      case 109: // Plasma 2D - Full 2D plasma effect
        pattern_plasma_2d(leds, activeLeds, hue);
        break;

      case 110: // Matrix Rain 2D - Proper Matrix effect with columns
        pattern_matrix_rain(leds, activeLeds, hue);
        break;

      case 111: // Game of Life - Conway's cellular automaton
        pattern_game_of_life(leds, activeLeds, hue);
        break;

      case 112: // Wave Pool - Horizontal waves perfect for 1m strips
        pattern_wave_pool(leds, activeLeds, hue);
        break;

      case 113: // Aurora 2D - Optimized for horizontal strips
        pattern_aurora_2d(leds, activeLeds, hue);
        break;

      case 114: // Lava Lamp 2D - Aspect-ratio corrected blobs
        pattern_lava_lamp(leds, activeLeds, hue);
        break;

      case 115: // 2D Ripple - Aspect-ratio corrected circles
        pattern_ripple_2d(leds, activeLeds, hue);
        break;

      case 116: // Starfield Parallax - Stars moving at different speeds
        pattern_starfield(leds, activeLeds, hue);
        break;

      case 117: // Side Fire - Fire from left and right edges
        pattern_side_fire(leds, activeLeds, hue);
        break;

      case 118: // Scrolling Rainbow - Smooth horizontal scroll
        pattern_scrolling_rainbow(leds, activeLeds, hue);
        break;

      case 119: // Particle Fountain - Particles shoot up from bottom
        pattern_particle_fountain(leds, activeLeds, hue);
        break;

      case 121: // Test Card - simple moving hue grid
        pattern_test_card(leds, activeLeds, hue);
        break;

      case 120: // Scrolling Text - Aspect-ratio corrected for 7.2:1 physical spacing
        pattern_scrolling_text(leds, activeLeds, hue, scrollText.c_str(), scrollOffset, scrollSpeed);
        break;

      case 122: // Custom Pattern from Designer
        if (hasCustomPattern) {
          // Display the custom pattern directly
          for (int i = 0; i < activeLeds && i < MAX_LEDS; i++) {
            leds[i] = customPattern[i];
          }
        } else {
          // No custom pattern loaded, show message
          fill_solid(leds, activeLeds, CRGB::Black);
        }
        break;

  }

  // Ensure any LEDs beyond active count are always black
  if (activeLeds < MAX_LEDS) {
      for(int i=activeLeds; i<MAX_LEDS; i++) leds[i] = CRGB::Black;
  }
}

void loop() {
  ArduinoOTA.handle();
  server.handleClient();

  // Non-blocking animation
  EVERY_N_MILLISECONDS(20) {
    // Only run animations if the server is up and running
    if (!serverRunning) {
      // Flash red to indicate server not up
      static bool flashState = false;
      if (flashState) {
        fill_solid(leds, MAX_LEDS, CRGB::Red);
      } else {
        fill_solid(leds, MAX_LEDS, CRGB::Black);
      }
      flashState = !flashState;
      FastLED.show();
      return; // Skip animation logic if server not ready
    }

    renderPatternFrame(currentPattern, leds, activeLeds, hue, scrollText, scrollOffset, scrollSpeed);
    FastLED.show();
  }
}

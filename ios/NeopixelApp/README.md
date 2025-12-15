# Neopixel iOS App

This folder contains the Swift source files for the **NeopixelApp** iOS application.

## How to use
1. Open Xcode on your Mac.
2. Choose **File → New → Project…** → **App** → **SwiftUI**.
3. Name the project **NeopixelApp** and set the Interface to **SwiftUI**, Life Cycle to **UIKit App Delegate**, Language to **Swift**.
4. When the project is created, replace the automatically generated files with the ones in this folder:
   - `AppDelegate.swift`
   - `SceneDelegate.swift`
   - `ContentView.swift`
   - `PatternGrid.swift`
   - `NetworkManager.swift`
5. Add the `Info.plist` entry `NSAppTransportSecurity` → `NSAllowsArbitraryLoads` = `YES` (or edit the existing Info.plist).
6. Build and run on your iPhone (or Simulator). The app will discover ESP8266 devices via Bonjour (`_neopixel._tcp`) and let you control the LED matrix.

## mDNS on the ESP8266
Add the following snippet to `src/main.cpp` **after** `ArduinoOTA.begin();` and re‑upload the firmware:
```cpp
if (MDNS.begin("neopixel-controller")) {
    MDNS.addService("neopixel", "tcp", 80);
    Serial.println("mDNS responder started");
}
```
The device will appear as `neopixel-controller.local` and be listed automatically in the app.

Enjoy!

# ESP8266 LED Matrix - State Machine

## Device States & Inputs

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                         ESP8266 LED MATRIX STATE MACHINE                    │
└─────────────────────────────────────────────────────────────────────────────┘

                                    INPUTS
┌─────────────────────────────────────────────────────────────────────────────┐
│  • UDP Packet (port 4210)     - LED frame data (3072 bytes)                 │
│  • HTTP Request               - /set, /setText, /setLayout, /uploadPattern │
│  • OTA Upload                 - Firmware update via ArduinoOTA             │
│  • Timer                      - millis() for animations and timeouts       │
└─────────────────────────────────────────────────────────────────────────────┘


                              STATE DIAGRAM

                         ┌──────────────────┐
                         │     STARTUP      │
                         │   (serverRunning │
                         │     = false)     │
                         └────────┬─────────┘
                                  │
                                  │ WiFi Connected
                                  │ + Server Started
                                  ▼
         ┌────────────────────────────────────────────────────┐
         │                                                    │
         │                  NORMAL MODE                       │
         │              (currentPattern = 0-254)              │
         │                                                    │
         │  ┌────────────────────────────────────────────┐   │
         │  │ loop() {                                   │   │
         │  │   ArduinoOTA.handle()     [~0-50ms]       │   │
         │  │   server.handleClient()   [~0-100ms]      │   │
         │  │   checkUDP()              [~0.1ms]        │   │
         │  │   EVERY_N_MS(20) {                        │   │
         │  │     renderPattern()       [~1-5ms]        │   │
         │  │     FastLED.show()        [~30ms]         │   │
         │  │   }                                       │   │
         │  │ }                                         │   │
         │  │ Total: 30-180ms per loop                  │   │
         │  └────────────────────────────────────────────┘   │
         │                                                    │
         └───────────────────────┬────────────────────────────┘
                                 │
              ┌──────────────────┼──────────────────┐
              │                  │                  │
              ▼                  ▼                  ▼
    ┌─────────────────┐ ┌───────────────┐ ┌─────────────────┐
    │ HTTP: /set?p=N  │ │  UDP Packet   │ │   OTA Upload    │
    │                 │ │   Received    │ │                 │
    └────────┬────────┘ └───────┬───────┘ └────────┬────────┘
             │                  │                   │
             │                  │                   │
             ▼                  ▼                   ▼
    ┌─────────────────┐ ┌───────────────────┐ ┌─────────────┐
    │ Set Pattern     │ │ Switch to         │ │ Reboot      │
    │ (0-254)         │ │ Streaming Mode    │ │ after flash │
    └────────┬────────┘ │ (pattern=255)     │ └─────────────┘
             │          └───────┬───────────┘
             │                  │
             ▼                  ▼
         ┌──────────────────────────────────────────────────┐
         │                                                   │
         │               STREAMING MODE                      │
         │             (currentPattern = 255)                │
         │                                                   │
         │  ┌────────────────────────────────────────────┐  │
         │  │ loop() {                                   │  │
         │  │   ArduinoOTA.handle()     [~0-50ms] ❌     │  │
         │  │   server.handleClient()   [~0-100ms] ❌    │  │
         │  │   checkUDP() {            [~0.1ms]        │  │
         │  │     read packet into leds                 │  │
         │  │     drain extra packets   [~0.1ms/pkt]    │  │
         │  │     FastLED.show()        [~30ms]         │  │
         │  │   }                                       │  │
         │  │ }                                         │  │
         │  │ Target: ~30ms per frame (~33 FPS max)     │  │
         │  └────────────────────────────────────────────┘  │
         │                                                   │
         │  PROBLEM: OTA/HTTP still run, adding latency!     │
         │                                                   │
         └───────────────────────┬───────────────────────────┘
                                 │
                                 │ No UDP for 5 seconds (timeout)
                                 │ OR HTTP: /set?p=N
                                 ▼
                    ┌────────────────────────┐
                    │  Return to NORMAL MODE │
                    │  (currentPattern = N)  │
                    └────────────────────────┘


                         PATTERNS (0-254)
┌─────────────────────────────────────────────────────────────────────────────┐
│  0-99:    Built-in animations (rainbow, fire, plasma, etc)                  │
│  100-199: Special effects (clock, text scroll, etc)                        │
│  200:     IP + Clock display (default on boot)                             │
│  255:     UDP Streaming mode (external control)                            │
└─────────────────────────────────────────────────────────────────────────────┘


                     FINAL IMPLEMENTATION
┌─────────────────────────────────────────────────────────────────────────────┐
│                                                                             │
│  ┌─────────────────┐                          ┌─────────────────────┐      │
│  │   NORMAL MODE   │  HTTP: /stream           │   STREAMING MODE    │      │
│  │  (pattern 0-254)│ ─────────────────────────▶ (pattern 255)       │      │
│  │                 │                          │                     │      │
│  │  • HTTP ✓       │                          │  • HTTP ✗ (skipped) │      │
│  │  • OTA ✓        │                          │  • OTA ✗ (skipped)  │      │
│  │  • Animations   │◀──────────────────────── │  • ~30 FPS max      │      │
│  │                 │  UDP: "EXIT" or timeout  │                     │      │
│  └─────────────────┘                          └─────────────────────┘      │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘

## Usage

```python
# Enter streaming mode
requests.get("http://192.168.1.130/stream")

# Send LED frames
sock.sendto(led_data, ("192.168.1.130", 4210))

# Exit streaming mode
sock.sendto(b"EXIT", ("192.168.1.130", 4210))
```

## Summary

| State | Enter | Exit | FPS |
|-------|-------|------|-----|
| Normal | Boot or EXIT | HTTP `/stream` | N/A |
| Streaming | HTTP `/stream` | UDP `EXIT` or 5s timeout | ~30 |

┌─────────────────────────────────────────────────────────────────────────────┐
│                                                                             │
│                              ┌─────────────┐                                │
│                              │   STARTUP   │                                │
│                              │  WiFi Init  │                                │
│                              └──────┬──────┘                                │
│                                     │                                       │
│                                     │ WiFi Connected                        │
│                                     ▼                                       │
│  ┌─────────────────────────────────────────────────────────────────────┐   │
│  │                         NORMAL MODE                                  │   │
│  │                      (currentPattern = 0-254)                        │   │
│  │                                                                      │   │
│  │   INPUTS PROCESSED:                                                  │   │
│  │   ├── ArduinoOTA.handle()    [Check for firmware update]            │   │
│  │   ├── server.handleClient()  [Check for HTTP requests]              │   │
│  │   ├── udp.parsePacket()      [Check for UDP packets]                │   │
│  │   └── renderPattern()        [Run current animation]                │   │
│  │                                                                      │   │
│  │   LOOP TIME: 30-180ms (variable due to HTTP/OTA overhead)           │   │
│  │   EFFECTIVE FPS: ~5-30 FPS                                          │   │
│  └──────────────────────────────────┬──────────────────────────────────┘   │
│                                     │                                       │
│                                     │ UDP Packet Received                   │
│                                     │ (any LED data, 3072 bytes)            │
│                                     ▼                                       │
│  ┌─────────────────────────────────────────────────────────────────────┐   │
│  │                       STREAMING MODE                                 │   │
│  │                     (currentPattern = 255)                           │   │
│  │                                                                      │   │
│  │   INPUTS PROCESSED:                                                  │   │
│  │   ├── udp.parsePacket()      [Check for UDP packets]      ✓ FAST    │   │
│  │   └── FastLED.show()         [Update LED strip]           ✓ NEEDED  │   │
│  │                                                                      │   │
│  │   INPUTS SKIPPED (during active streaming):                          │   │
│  │   ├── ArduinoOTA.handle()    ✗ SKIP (saves 0-50ms)                  │   │
│  │   └── server.handleClient()  ✗ SKIP (saves 0-100ms)                 │   │
│  │                                                                      │   │
│  │   LOOP TIME: ~30ms (consistent)                                     │   │
│  │   EFFECTIVE FPS: ~33 FPS (maximum for 1024 LEDs)                    │   │
│  └──────────────────────────────────┬──────────────────────────────────┘   │
│                                     │                                       │
│                  ┌──────────────────┼──────────────────┐                   │
│                  │                  │                  │                   │
│                  ▼                  ▼                  ▼                   │
│         ┌──────────────┐  ┌──────────────┐  ┌──────────────────┐          │
│         │ UDP: "EXIT"  │  │ UDP Timeout  │  │ Brief HTTP Check │          │
│         │  (4 bytes)   │  │  (5 seconds) │  │ (every 500ms)    │          │
│         └──────┬───────┘  └──────┬───────┘  └────────┬─────────┘          │
│                │                 │                   │                     │
│                └─────────────────┴───────────────────┘                     │
│                                  │                                         │
│                                  ▼                                         │
│                         Return to NORMAL MODE                              │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘


                           UDP PACKET TYPES
┌─────────────────────────────────────────────────────────────────────────────┐
│                                                                             │
│  ┌────────────────────────────────────────────────────────────────────┐    │
│  │  LED DATA PACKET (3072 bytes)                                      │    │
│  │  ───────────────────────────────                                   │    │
│  │  Format: [R0][G0][B0][R1][G1][B1]...[R1023][G1023][B1023]          │    │
│  │  Effect: Update LEDs, enter/stay in STREAMING MODE                 │    │
│  └────────────────────────────────────────────────────────────────────┘    │
│                                                                             │
│  ┌────────────────────────────────────────────────────────────────────┐    │
│  │  EXIT PACKET (4 bytes)                                             │    │
│  │  ─────────────────────                                             │    │
│  │  Format: "EXIT" (0x45 0x58 0x49 0x54)                              │    │
│  │  Effect: Return to NORMAL MODE (enable HTTP/OTA)                   │    │
│  └────────────────────────────────────────────────────────────────────┘    │
│                                                                             │
│  ┌────────────────────────────────────────────────────────────────────┐    │
│  │  STATS REQUEST (5 bytes) - Optional                                │    │
│  │  ───────────────────────                                           │    │
│  │  Format: "STATS" (0x53 0x54 0x41 0x54 0x53)                        │    │
│  │  Effect: Reply with JSON stats (received/displayed/dropped)        │    │
│  └────────────────────────────────────────────────────────────────────┘    │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘


                      PYTHON CLIENT USAGE
┌─────────────────────────────────────────────────────────────────────────────┐
│                                                                             │
│  # Normal streaming (just send LED data)                                    │
│  sock.sendto(led_data, (ESP_IP, 4210))                                     │
│                                                                             │
│  # Exit streaming mode (to enable OTA or web UI)                           │
│  sock.sendto(b"EXIT", (ESP_IP, 4210))                                      │
│  time.sleep(0.5)  # Wait for mode switch                                   │
│  # Now HTTP and OTA work normally                                          │
│                                                                             │
│  # Resume streaming                                                         │
│  sock.sendto(led_data, (ESP_IP, 4210))  # Auto-enters streaming mode       │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

## Summary Table

| State | OTA | HTTP | UDP | Loop Time | FPS |
|-------|-----|------|-----|-----------|-----|
| Normal | ✓ | ✓ | ✓ | 30-180ms | 5-30 |
| **Streaming (proposed)** | ✗ | ✗* | ✓ | **~30ms** | **~33** |

*HTTP checked briefly every 500ms during streaming for stats endpoint


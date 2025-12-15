# Audio Visualizer Data Flow

## System Architecture

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                        AUDIO VISUALIZER DATA FLOW                           │
└─────────────────────────────────────────────────────────────────────────────┘

┌──────────────┐
│   YouTube/   │
│   Spotify/   │
│   Any App    │
└──────┬───────┘
       │ Audio Output
       ▼
┌──────────────────────────────────────────────────────────────────────────┐
│                            PipeWire Audio System                          │
│                                                                          │
│  ┌─────────────────────┐                                                 │
│  │  visualizer_sink    │◄─── Apps send audio here (set as default)       │
│  │  (Null Sink)        │                                                 │
│  └──────────┬──────────┘                                                 │
│             │                                                            │
│             │ .monitor (realtime capture)                                │
│             │                                                            │
│             ├───────────────────────────────┐                            │
│             │                               │                            │
│             ▼                               ▼                            │
│  ┌──────────────────────┐     ┌─────────────────────────┐               │
│  │  parec (capture)     │     │  module-loopback        │               │
│  │  --latency-msec=1    │     │  latency_msec=1         │               │
│  └──────────┬───────────┘     └──────────┬──────────────┘               │
│             │                            │                               │
│             │ ◄──── MEASUREMENT POINT    │                               │
│             │       (1.4s includes       │                               │
│             │        human reaction)     │                               │
│             │                            │                               │
└─────────────┼────────────────────────────┼───────────────────────────────┘
              │                            │
              │ Raw PCM Audio              │ Delayed Audio
              ▼                            ▼
┌─────────────────────────┐    ┌─────────────────────────┐
│  lowlatency_visualizer  │    │  HDMI Output            │
│  (Python Script)        │    │  (alsa_output...hdmi)   │
│                         │    │                         │
│  ┌───────────────────┐  │    │  ┌───────────────────┐  │
│  │  FFT Processing   │  │    │  │  Speakers/TV      │  │
│  │  (scipy.fft.rfft) │  │    │  └───────────────────┘  │
│  └─────────┬─────────┘  │    └─────────────────────────┘
│            │            │
│  ┌─────────▼─────────┐  │
│  │  32 Frequency     │  │
│  │  Bins (40-16kHz)  │  │
│  └─────────┬─────────┘  │
│            │            │
│  ┌─────────▼─────────┐  │
│  │  LED Mapping      │  │
│  │  XY() function    │  │
│  └─────────┬─────────┘  │
│            │            │
└────────────┼────────────┘
             │ UDP Packet (3072 bytes)
             ▼
┌─────────────────────────┐
│  ESP8266 (192.168.1.130)│
│  Port 4210              │
│                         │
│  ┌───────────────────┐  │
│  │  FastLED Library  │  │
│  │  WS2812B Driver   │  │
│  └─────────┬─────────┘  │
└────────────┼────────────┘
             │ Data Signal
             ▼
┌─────────────────────────┐
│  32x32 LED Matrix       │
│  (4 × 8x32 Panels)      │
│  1024 WS2812B LEDs      │
└─────────────────────────┘
```

## Latency Measurement

The **1.43 second** was measured using `latency_test.py`:

```
Test data:
1.33s | 0.796 | Music still playing
1.44s | 0.000 | Silence detected
─────────────────────────────────────
Drop time: 1.44 - 1.33 = 0.11 seconds
```

### What the measurement shows:

```
┌─────────────────────────────────────────────────────────────────────┐
│ 0.0s                           1.33s                    1.44s       │
│  │                               │                        │         │
│  │◄─── Human Reaction Time ─────►│◄── Audio Latency ────►│         │
│  │     (ENTER → click pause)     │    (~110ms)           │         │
│  │         ~1.33 seconds         │                        │         │
│  │                               │                        │         │
│  ▼                               ▼                        ▼         │
│ [Press ENTER]              [Click Pause]          [parec sees 0]   │
│              in terminal          on YouTube                        │
└─────────────────────────────────────────────────────────────────────┘
```

### Latency Breakdown

| Component | Latency |
|-----------|---------|
| Human reaction (ENTER → click pause) | ~1330ms |
| **Actual audio latency** | **~110ms** |
| FFT + LED mapping | ~0.2ms |
| UDP over WiFi | ~5-10ms |
| ESP8266 → LEDs | ~30ms |
| **Total audio → LED (theoretical)** | **~150ms** |

## ⚠️ Unresolved Issue: 7 Second Delay

When running `lowlatency_visualizer.py`, there's still a ~7 second delay
from music start/stop to LED response, even though:

- parec captures with ~110ms latency ✓
- FFT processing takes 0.18ms ✓  
- No buffer at startup ✓

The delay might be in:
1. PipeWire's internal routing to visualizer_sink
2. The loopback module buffering
3. Browser audio pipeline

**TODO:** Further investigation needed

## Configuration

To use this setup, run:
```bash
# Set audio output to visualizer
pactl set-default-sink visualizer_sink

# Run the visualizer
python3 lowlatency_visualizer.py --brightness 0.3
```

## Files

- `lowlatency_visualizer.py` - Main visualizer using parec
- `latency_test.py` - Tool to measure audio latency
- `live_visualizer.py` - Alternative using PyAudio

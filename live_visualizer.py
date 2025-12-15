#!/usr/bin/env python3
"""
Real-time System Audio Visualizer for 32x32 LED Matrix
Captures audio from your system (YouTube, Spotify, etc.) and displays FFT spectrum
"""
import numpy as np
import socket
import time
import sys
import pyaudio
from scipy.fft import rfft

# Configuration
ESP_IP = "192.168.1.130"
UDP_PORT = 4210
GRID_WIDTH = 32
GRID_HEIGHT = 32
MAX_LEDS = 1024
PANELS_WIDE = 1

# Audio settings
CHUNK = 1024  # Smaller for lower latency (~23ms)
RATE = 44100  # Sample rate
CHANNELS = 2  # Stereo (required by most devices)

def XY(x, y):
    """Convert x,y to LED index"""
    if x < 0 or x >= GRID_WIDTH or y < 0 or y >= GRID_HEIGHT:
        return -1
    
    panelCol = x // 32
    panelRow = y // 8
    localX = x % 32
    localY = y % 8
    
    if panelRow % 2 == 1:
        localX = 31 - localX
        localY = 7 - localY
    
    if panelRow % 2 == 0:
        panelIndex = (panelRow * PANELS_WIDE) + panelCol
    else:
        panelIndex = (panelRow * PANELS_WIDE) + (PANELS_WIDE - 1 - panelCol)
    
    panelOffset = panelIndex * 256
    
    if localX % 2 == 0:
        return panelOffset + (localX * 8 + localY)
    else:
        return panelOffset + (localX * 8 + (7 - localY))

def hsv_to_rgb(h, s, v):
    """Convert HSV to RGB"""
    if s == 0:
        return (int(v * 255), int(v * 255), int(v * 255))
    
    h = h % 360
    h = h / 60
    i = int(h)
    f = h - i
    p = v * (1 - s)
    q = v * (1 - s * f)
    t = v * (1 - s * (1 - f))
    
    if i == 0:
        r, g, b = v, t, p
    elif i == 1:
        r, g, b = q, v, p
    elif i == 2:
        r, g, b = p, v, t
    elif i == 3:
        r, g, b = p, q, v
    elif i == 4:
        r, g, b = t, p, v
    else:
        r, g, b = v, p, q
    
    return (int(r * 255), int(g * 255), int(b * 255))

def find_monitor_device(p):
    """Find the PulseAudio/PipeWire monitor device for system audio capture"""
    monitor_device = None
    device_name = None
    
    print("\nAvailable audio devices:")
    print("-" * 50)
    
    for i in range(p.get_device_count()):
        dev = p.get_device_info_by_index(i)
        name = dev['name']
        inputs = dev['maxInputChannels']
        
        print(f"  [{i}] {name} (inputs: {inputs})")
        
        # Look for monitor devices (system audio loopback)
        if inputs > 0:
            name_lower = name.lower()
            if 'monitor' in name_lower or 'loopback' in name_lower:
                monitor_device = i
                device_name = name
    
    print("-" * 50)
    
    if monitor_device is not None:
        print(f"\n✓ Found monitor device: [{monitor_device}] {device_name}")
    else:
        print("\n⚠ No monitor device found automatically.")
        print("  You may need to set up a loopback device.")
        print("\n  On PulseAudio/PipeWire, you can use:")
        print("    pactl load-module module-loopback")
        print("\n  Or select a device manually with --device <number>")
    
    return monitor_device

def visualize_realtime(device_index=None, brightness=0.3, sensitivity=1.0):
    """Real-time audio visualization"""
    p = pyaudio.PyAudio()
    
    if device_index is None:
        device_index = find_monitor_device(p)
        
        if device_index is None:
            # Try default input
            print("\nTrying default input device...")
            device_index = p.get_default_input_device_info()['index']
    
    device_info = p.get_device_info_by_index(device_index)
    print(f"\nUsing device [{device_index}]: {device_info['name']}")
    print(f"  Sample rate: {RATE}")
    print(f"  Chunk size: {CHUNK}")
    print(f"  Brightness: {brightness}")
    print(f"  Sensitivity: {sensitivity}")
    print(f"\nStreaming to {ESP_IP}:{UDP_PORT}")
    print("\n🎵 Play some music and watch the LED matrix!")
    print("Press Ctrl+C to stop...\n")
    
    # Open audio stream with minimal buffering
    try:
        stream = p.open(
            format=pyaudio.paFloat32,
            channels=CHANNELS,
            rate=RATE,
            input=True,
            input_device_index=device_index,
            frames_per_buffer=CHUNK,
            stream_callback=None  # Use blocking mode but with small buffer
        )
        # Get the actual latency
        latency = stream.get_input_latency()
        print(f"  Audio latency: {latency*1000:.1f}ms")
    except Exception as e:
        print(f"Error opening audio stream: {e}")
        print("\nTry running with a specific device:")
        print("  python3 live_visualizer.py --device <number>")
        p.terminate()
        return
    
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    
    # Frequency bins (logarithmic)
    num_bins = GRID_WIDTH
    min_freq = 40  # Sub-bass start
    max_freq = 16000
    freq_bins = np.logspace(np.log10(min_freq), np.log10(max_freq), num_bins + 1)
    freqs = np.fft.rfftfreq(CHUNK, 1 / RATE)
    
    # Smoothing
    prev_heights = np.zeros(num_bins)
    peak_heights = np.zeros(num_bins)  # Peak hold for falling dots
    
    frame_count = 0
    
    try:
        while True:
            # Read audio data - skip buffered data to minimize lag
            try:
                # Read available data, only process the most recent chunk
                available = stream.get_read_available()
                if available > CHUNK * 2:
                    # Skip old data (discard buffer buildup)
                    _ = stream.read(available - CHUNK, exception_on_overflow=False)
                
                data = stream.read(CHUNK, exception_on_overflow=False)
                audio = np.frombuffer(data, dtype=np.float32)
                # Convert stereo to mono
                if CHANNELS == 2:
                    audio = audio.reshape(-1, 2).mean(axis=1)
            except Exception as e:
                continue
            
            # Apply window and FFT
            window = np.hanning(len(audio))
            fft_data = np.abs(rfft(audio * window))
            
            # Map to frequency bins
            bin_heights = np.zeros(num_bins)
            for b in range(num_bins):
                mask = (freqs >= freq_bins[b]) & (freqs < freq_bins[b + 1])
                if np.any(mask):
                    # Use max instead of mean to capture peaks better
                    bin_heights[b] = np.max(fft_data[mask])
            
            # Boost low frequencies (they're naturally quieter)
            for b in range(8):
                bin_heights[b] *= (2.0 - b * 0.125)  # 2x boost at bin 0, 1x at bin 8
            
            # Apply sensitivity and normalize per-frame
            bin_heights = bin_heights * sensitivity
            max_val = np.max(bin_heights)
            if max_val > 0:
                bin_heights = bin_heights / max_val  # Normalize to 0-1
            
            # Apply softer log scaling
            bin_heights = np.sqrt(bin_heights)  # Square root for gentler curve
            bin_heights = np.clip(bin_heights, 0, 1)
            
            # Smooth - minimal smoothing for snappy response
            bin_heights = 0.1 * prev_heights + 0.9 * bin_heights
            prev_heights = bin_heights.copy()
            
            # Update peak hold (falling dots)
            peak_heights = np.maximum(peak_heights - 0.02, bin_heights)
            
            # Create LED frame
            pixels = [0] * (MAX_LEDS * 3)
            
            for x in range(GRID_WIDTH):
                height = int(bin_heights[x] * GRID_HEIGHT)
                peak = int(peak_heights[x] * GRID_HEIGHT)
                
                for y in range(GRID_HEIGHT):
                    # Use XY directly like streamer does (no transformation)
                    led_idx = XY(x, y)
                    
                    if led_idx < 0 or led_idx >= MAX_LEDS:
                        continue
                    
                    # Draw bar from bottom (y=31) upward
                    # y=31 is bottom row, y=0 is top row
                    bar_y = GRID_HEIGHT - 1 - y  # bar_y=0 at bottom, bar_y=31 at top
                    
                    if bar_y < height:
                        # Bar color (rainbow based on frequency bin)
                        hue = (x / GRID_WIDTH) * 300  # Rainbow across spectrum
                        # Intensity based on position in bar
                        intensity = 0.5 + 0.5 * (bar_y / GRID_HEIGHT)
                        r, g, b = hsv_to_rgb(hue, 1.0, intensity)
                        
                        r = int(r * brightness)
                        g = int(g * brightness)
                        b = int(b * brightness)
                        
                        pixels[led_idx * 3] = r
                        pixels[led_idx * 3 + 1] = g
                        pixels[led_idx * 3 + 2] = b
                    
                    # Peak dot (white)
                    elif bar_y == peak and peak > 0:
                        bright = int(255 * brightness)
                        pixels[led_idx * 3] = bright
                        pixels[led_idx * 3 + 1] = bright
                        pixels[led_idx * 3 + 2] = bright
            
            # Send to LED
            sock.sendto(bytes(pixels), (ESP_IP, UDP_PORT))
            
            frame_count += 1
            if frame_count % 100 == 0:
                print(f"\rFrames: {frame_count}", end='', flush=True)
    
    except KeyboardInterrupt:
        print(f"\n\nStopped. Total frames: {frame_count}")
    finally:
        stream.stop_stream()
        stream.close()
        p.terminate()
        sock.close()

if __name__ == "__main__":
    import argparse
    
    parser = argparse.ArgumentParser(description="Real-time Audio Visualizer for 32x32 LED Matrix")
    parser.add_argument("--device", type=int, default=None, help="Audio device index")
    parser.add_argument("--ip", default=ESP_IP, help="ESP8266 IP address")
    parser.add_argument("--brightness", type=float, default=0.3, help="LED brightness (0.0-1.0)")
    parser.add_argument("--sensitivity", type=float, default=1.0, help="Audio sensitivity multiplier")
    parser.add_argument("--list", action="store_true", help="List audio devices and exit")
    
    args = parser.parse_args()
    ESP_IP = args.ip
    
    if args.list:
        p = pyaudio.PyAudio()
        find_monitor_device(p)
        p.terminate()
    else:
        visualize_realtime(args.device, args.brightness, args.sensitivity)

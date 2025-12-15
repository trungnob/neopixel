#!/usr/bin/env python3
"""
Low-latency audio visualizer using parec for real-time system audio capture
"""
import numpy as np
import socket
import sys
import subprocess
import requests
import gc
from scipy.fft import rfft

# Configuration
ESP_IP = "192.168.1.130"
UDP_PORT = 4210
GRID_WIDTH = 32
GRID_HEIGHT = 32
MAX_LEDS = 1024
PANELS_WIDE = 1

# Audio settings
RATE = 48000  # Match PipeWire's 48kHz
CHUNK = 2000  # 48000/2000 = 24 FPS
CHANNELS = 2
AMPLITUDE_SCALE = 0.8  # Scale down amplitude

def XY(x, y):
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
    if s == 0:
        return (int(v * 255), int(v * 255), int(v * 255))
    h = h % 360
    h = h / 60
    i = int(h)
    f = h - i
    p = v * (1 - s)
    q = v * (1 - s * f)
    t = v * (1 - s * (1 - f))
    if i == 0: r, g, b = v, t, p
    elif i == 1: r, g, b = q, v, p
    elif i == 2: r, g, b = p, v, t
    elif i == 3: r, g, b = p, q, v
    elif i == 4: r, g, b = t, p, v
    else: r, g, b = v, p, q
    return (int(r * 255), int(g * 255), int(b * 255))

def main(brightness=0.3):
    print("=" * 60)
    print("Low-Latency Audio Visualizer (using parec)")
    print("=" * 60)
    
    # Find monitor source - prefer visualizer_sink for low latency
    result = subprocess.run(
        ['pactl', 'list', 'sources', 'short'],
        capture_output=True, text=True
    )
    monitor = None
    for line in result.stdout.strip().split('\n'):
        if 'visualizer_sink.monitor' in line.lower():
            monitor = line.split()[1]
            break
    
    # Fall back to any monitor
    if not monitor:
        for line in result.stdout.strip().split('\n'):
            if 'monitor' in line.lower():
                monitor = line.split()[1]
                break
    
    if not monitor:
        print("Error: No monitor source found!")
        return
    
    print(f"Using monitor: {monitor}")
    print(f"Sample rate: {RATE} Hz")
    print(f"Chunk size: {CHUNK}")
    print(f"Streaming to: {ESP_IP}:{UDP_PORT}")
    print("\nPress Ctrl+C to stop\n")
    
    # Setup UDP socket
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    
    # FFT setup - optimized for 24Hz FFT resolution
    num_bins = GRID_WIDTH
    min_freq = 40      # Above first FFT bin (24Hz) for better separation
    max_freq = 16000   # Most music content
    freq_bins = np.logspace(np.log10(min_freq), np.log10(max_freq), num_bins + 1)
    freqs = np.fft.rfftfreq(CHUNK, 1 / RATE)
    
    # Per-band normalization history
    band_maxes = np.ones(num_bins)
    prev_heights = np.zeros(num_bins)
    peaks = np.zeros(num_bins)  # Peak hold values
    peak_fall_speed = 0.5  # How fast peaks fall per frame
    frame_count = 0
    
    # Start parec with minimal latency
    parec = subprocess.Popen(
        [
            'parec',
            '--rate', str(RATE),
            '--channels', str(CHANNELS),
            '--format', 's16le',
            '--latency-msec', '1',
            '--device', monitor
        ],
        stdout=subprocess.PIPE,
        stderr=subprocess.DEVNULL
    )
    
    bytes_per_sample = 2 * CHANNELS
    bytes_per_chunk = CHUNK * bytes_per_sample
    
    # Enter streaming mode via HTTP
    print(f"Entering streaming mode via HTTP...")
    try:
        r = requests.get(f'http://{ESP_IP}/stream', timeout=5)
        print(f"  {r.text}")
    except Exception as e:
        print(f"Warning: Could not enter streaming mode: {e}")
    
    print("Starting visualization...")
    
    # Disable garbage collection to prevent pauses
    gc.disable()
    
    try:
        while True:
            raw = parec.stdout.read(bytes_per_chunk)
            if not raw:
                break
            
            audio = np.frombuffer(raw, dtype=np.int16).astype(np.float32) / 32768.0
            
            if CHANNELS == 2:
                audio = audio.reshape(-1, 2).mean(axis=1)
            
            if len(audio) >= CHUNK:
                window = np.hanning(len(audio))
                fft_data = np.abs(rfft(audio * window))
                
                # Map to frequency bins - use nearest bin when range is too small
                bin_heights = np.zeros(num_bins)
                for b in range(num_bins):
                    low_f = freq_bins[b]
                    high_f = freq_bins[b + 1]
                    mask = (freqs >= low_f) & (freqs < high_f)
                    if np.any(mask):
                        bin_heights[b] = np.sqrt(np.mean(fft_data[mask] ** 2))
                    else:
                        # No FFT bins in range - use nearest frequency
                        center_f = (low_f + high_f) / 2
                        nearest_idx = np.argmin(np.abs(freqs - center_f))
                        bin_heights[b] = fft_data[nearest_idx]
                
                # Per-band auto-normalization (adapts to music dynamics)
                band_maxes = np.maximum(band_maxes * 0.995, bin_heights)
                for b in range(num_bins):
                    if band_maxes[b] > 0.001:
                        bin_heights[b] = bin_heights[b] / band_maxes[b]
                
                # Apply gamma for better visual spread
                bin_heights = np.power(bin_heights, 0.7)
                bin_heights = bin_heights * AMPLITUDE_SCALE
                bin_heights = np.clip(bin_heights, 0, 1)
                
                # Light smoothing
                bin_heights = 0.2 * prev_heights + 0.8 * bin_heights
                prev_heights = bin_heights.copy()
                
                # Update peaks (falling dots)
                for x in range(GRID_WIDTH):
                    height = int(bin_heights[x] * GRID_HEIGHT)
                    if height > peaks[x]:
                        peaks[x] = height
                    else:
                        peaks[x] = max(0, peaks[x] - peak_fall_speed)
                
                # Create LED frame
                pixels = [0] * (MAX_LEDS * 3)
                
                for x in range(GRID_WIDTH):
                    height = int(bin_heights[x] * GRID_HEIGHT)
                    peak_y = int(peaks[x])
                    
                    for y in range(GRID_HEIGHT):
                        led_idx = XY(x, y)
                        bar_y = GRID_HEIGHT - 1 - y
                        
                        if led_idx >= 0 and led_idx < MAX_LEDS:
                            if bar_y < height:
                                # Normal bar
                                hue = (x / GRID_WIDTH) * 300
                                intensity = 0.5 + 0.5 * (bar_y / GRID_HEIGHT)
                                r, g, b = hsv_to_rgb(hue, 1.0, intensity)
                                r = int(r * brightness)
                                g = int(g * brightness)
                                b = int(b * brightness)
                            elif bar_y == peak_y and peak_y > 0:
                                # Peak dot - bright white
                                r = int(255 * brightness)
                                g = int(255 * brightness)
                                b = int(255 * brightness)
                            else:
                                r, g, b = 0, 0, 0
                            
                            pixels[led_idx * 3] = r
                            pixels[led_idx * 3 + 1] = g
                            pixels[led_idx * 3 + 2] = b
                
                sock.sendto(bytes(pixels), (ESP_IP, UDP_PORT))
                frame_count += 1
                
                if frame_count % 100 == 0:
                    print(f"\rFrames: {frame_count}", end='', flush=True)
    
    except KeyboardInterrupt:
        print(f"\n\nStopped. Total frames: {frame_count}")
    finally:
        # Send EXIT to exit streaming mode
        print("Exiting streaming mode...")
        sock.sendto(b'EXIT', (ESP_IP, UDP_PORT))
        gc.enable()  # Re-enable garbage collection
        parec.terminate()
        sock.close()

if __name__ == "__main__":
    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument("--brightness", type=float, default=0.3)
    args = parser.parse_args()
    main(args.brightness)

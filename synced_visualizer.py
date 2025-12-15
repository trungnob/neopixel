#!/usr/bin/env python3
"""
Synced Audio Visualizer - Delays audio playback to match LED panel
This captures system audio, feeds it to FFT->LED, and plays it with delay
"""
import numpy as np
import socket
import time
import sys
import argparse
import collections
from scipy.fft import rfft

try:
    import sounddevice as sd
except ImportError:
    print("Installing sounddevice...")
    import subprocess
    subprocess.run([sys.executable, '-m', 'pip', 'install', '--user', 'sounddevice', '--break-system-packages'], 
                   capture_output=True)
    import sounddevice as sd

# Configuration
ESP_IP = "192.168.1.130"
UDP_PORT = 4210
GRID_WIDTH = 32
GRID_HEIGHT = 32
MAX_LEDS = 1024
PANELS_WIDE = 1

# Audio settings
SAMPLE_RATE = 44100
BLOCK_SIZE = 1024
CHANNELS = 2

# Delay buffer (in seconds)
AUDIO_DELAY = 0.15  # 150ms default delay

def XY(x, y):
    """Convert x,y to LED index - same as streamer"""
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

def run_synced_visualizer(audio_file=None, delay=0.15, brightness=0.3):
    """Run visualizer with synced audio playback"""
    
    print("=" * 60)
    print("Synced Audio Visualizer")
    print("=" * 60)
    print(f"Audio delay: {delay*1000:.0f}ms")
    print(f"Brightness: {brightness}")
    print(f"Streaming to: {ESP_IP}:{UDP_PORT}")
    print()
    
    # Setup UDP socket
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    
    # FFT setup
    num_bins = GRID_WIDTH
    min_freq = 40
    max_freq = 16000
    freq_bins = np.logspace(np.log10(min_freq), np.log10(max_freq), num_bins + 1)
    freqs = np.fft.rfftfreq(BLOCK_SIZE, 1 / SAMPLE_RATE)
    
    # Audio delay buffer
    delay_samples = int(delay * SAMPLE_RATE)
    delay_blocks = delay_samples // BLOCK_SIZE + 1
    audio_buffer = collections.deque(maxlen=delay_blocks)
    
    # Pre-fill with silence
    for _ in range(delay_blocks):
        audio_buffer.append(np.zeros((BLOCK_SIZE, CHANNELS), dtype=np.float32))
    
    # State
    prev_heights = np.zeros(num_bins)
    frame_count = [0]
    
    def audio_callback(indata, outdata, frames, time_info, status):
        """Process audio: FFT->LED now, play audio later"""
        if status:
            print(f"Audio status: {status}")
        
        # Add current audio to delay buffer
        audio_buffer.append(indata.copy())
        
        # Get delayed audio for playback
        delayed = audio_buffer[0] if len(audio_buffer) > 0 else np.zeros_like(indata)
        outdata[:] = delayed
        
        # Process current audio for FFT (no delay)
        mono = indata.mean(axis=1) if indata.ndim > 1 else indata.flatten()
        
        # FFT
        window = np.hanning(len(mono))
        fft_data = np.abs(rfft(mono * window))
        
        # Map to frequency bins
        bin_heights = np.zeros(num_bins)
        for b in range(num_bins):
            mask = (freqs >= freq_bins[b]) & (freqs < freq_bins[b + 1])
            if np.any(mask):
                bin_heights[b] = np.max(fft_data[mask])
        
        # Boost low frequencies
        for b in range(8):
            bin_heights[b] *= (2.0 - b * 0.125)
        
        # Normalize
        max_val = np.max(bin_heights)
        if max_val > 0:
            bin_heights = bin_heights / max_val
        
        bin_heights = np.sqrt(bin_heights)
        bin_heights = np.clip(bin_heights, 0, 1)
        
        # Smooth
        nonlocal prev_heights
        bin_heights = 0.1 * prev_heights + 0.9 * bin_heights
        prev_heights = bin_heights.copy()
        
        # Create LED frame
        pixels = [0] * (MAX_LEDS * 3)
        
        for x in range(GRID_WIDTH):
            height = int(bin_heights[x] * GRID_HEIGHT)
            
            for y in range(GRID_HEIGHT):
                led_idx = XY(x, y)
                bar_y = GRID_HEIGHT - 1 - y
                
                if led_idx >= 0 and led_idx < MAX_LEDS and bar_y < height:
                    hue = (x / GRID_WIDTH) * 300
                    intensity = 0.5 + 0.5 * (bar_y / GRID_HEIGHT)
                    r, g, b = hsv_to_rgb(hue, 1.0, intensity)
                    
                    r = int(r * brightness)
                    g = int(g * brightness)
                    b = int(b * brightness)
                    
                    pixels[led_idx * 3] = r
                    pixels[led_idx * 3 + 1] = g
                    pixels[led_idx * 3 + 2] = b
        
        # Send to LED
        sock.sendto(bytes(pixels), (ESP_IP, UDP_PORT))
        frame_count[0] += 1
    
    print("🎵 Starting audio stream with loopback...")
    print(f"Audio will be delayed by {delay*1000:.0f}ms to sync with LEDs")
    print("Press Ctrl+C to stop\n")
    
    try:
        # Open stream for both input (capture) and output (playback)
        with sd.Stream(samplerate=SAMPLE_RATE,
                      blocksize=BLOCK_SIZE,
                      channels=CHANNELS,
                      dtype=np.float32,
                      callback=audio_callback):
            print("Stream opened. Play music on your computer!")
            while True:
                time.sleep(0.1)
                if frame_count[0] % 100 == 0 and frame_count[0] > 0:
                    print(f"\rFrames: {frame_count[0]}", end='', flush=True)
    
    except KeyboardInterrupt:
        print(f"\n\nStopped. Total frames: {frame_count[0]}")
    finally:
        sock.close()

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Synced Audio Visualizer with delayed playback")
    parser.add_argument("--delay", type=float, default=0.15, help="Audio delay in seconds (default: 0.15)")
    parser.add_argument("--brightness", type=float, default=0.3, help="LED brightness 0.0-1.0")
    parser.add_argument("--ip", default=ESP_IP, help="ESP8266 IP address")
    
    args = parser.parse_args()
    ESP_IP = args.ip
    
    run_synced_visualizer(delay=args.delay, brightness=args.brightness)

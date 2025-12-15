#!/usr/bin/env python3
"""
FFT Audio Visualizer for 32x32 LED Matrix
Displays frequency spectrum as vertical bars
"""
import numpy as np
import socket
import time
import sys
import argparse

# Try to import audio libraries
try:
    from scipy.io import wavfile
    from scipy.fft import rfft, rfftfreq
    HAS_SCIPY = True
except ImportError:
    HAS_SCIPY = False
    print("Installing scipy...")
    import subprocess
    subprocess.run([sys.executable, '-m', 'pip', 'install', '--user', 'scipy', '--break-system-packages'], 
                   capture_output=True)
    from scipy.io import wavfile
    from scipy.fft import rfft, rfftfreq

try:
    import soundfile as sf
    HAS_SOUNDFILE = True
except ImportError:
    HAS_SOUNDFILE = False

# Configuration
ESP_IP = "192.168.1.130"
UDP_PORT = 4210
GRID_WIDTH = 32
GRID_HEIGHT = 32
MAX_LEDS = 1024
PANELS_WIDE = 1

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

def load_audio(filename):
    """Load audio file and return samples + sample rate"""
    if filename.endswith('.wav'):
        sample_rate, data = wavfile.read(filename)
        # Convert to mono if stereo
        if len(data.shape) > 1:
            data = data.mean(axis=1)
        # Normalize to float
        if data.dtype == np.int16:
            data = data.astype(np.float32) / 32768.0
        elif data.dtype == np.int32:
            data = data.astype(np.float32) / 2147483648.0
        return sample_rate, data
    elif HAS_SOUNDFILE:
        data, sample_rate = sf.read(filename)
        if len(data.shape) > 1:
            data = data.mean(axis=1)
        return sample_rate, data
    else:
        print(f"Cannot load {filename}. Install soundfile: pip install soundfile")
        return None, None

def visualize_audio(filename, brightness=0.3, fps=30):
    """Visualize audio file as FFT spectrum"""
    print(f"Loading audio: {filename}")
    sample_rate, audio_data = load_audio(filename)
    
    if audio_data is None:
        return
    
    print(f"  Sample rate: {sample_rate} Hz")
    print(f"  Duration: {len(audio_data) / sample_rate:.2f}s")
    print(f"  Streaming to {ESP_IP}:{UDP_PORT}")
    print(f"  FPS: {fps}, Brightness: {brightness}")
    print("\nPress Ctrl+C to stop...\n")
    
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    
    # FFT parameters
    chunk_size = sample_rate // fps  # Samples per frame
    num_bins = GRID_WIDTH  # 32 frequency bins
    
    # Frequency bin setup (logarithmic scaling for better visualization)
    min_freq = 20
    max_freq = min(16000, sample_rate // 2)
    freq_bins = np.logspace(np.log10(min_freq), np.log10(max_freq), num_bins + 1)
    
    frame_count = 0
    total_frames = len(audio_data) // chunk_size
    
    # Smoothing for bars
    prev_heights = np.zeros(num_bins)
    
    try:
        start_time = time.time()
        
        for i in range(0, len(audio_data) - chunk_size, chunk_size):
            frame_start = time.time()
            
            # Get audio chunk
            chunk = audio_data[i:i + chunk_size]
            
            # Apply window function
            window = np.hanning(len(chunk))
            chunk = chunk * window
            
            # FFT
            fft_data = np.abs(rfft(chunk))
            freqs = rfftfreq(len(chunk), 1 / sample_rate)
            
            # Map FFT to frequency bins
            bin_heights = np.zeros(num_bins)
            for b in range(num_bins):
                # Find FFT indices in this frequency range
                mask = (freqs >= freq_bins[b]) & (freqs < freq_bins[b + 1])
                if np.any(mask):
                    bin_heights[b] = np.mean(fft_data[mask])
            
            # Normalize and apply log scaling
            bin_heights = np.log10(bin_heights + 1)
            max_val = np.max(bin_heights) if np.max(bin_heights) > 0 else 1
            bin_heights = bin_heights / max_val
            
            # Smooth with previous frame
            bin_heights = 0.3 * prev_heights + 0.7 * bin_heights
            prev_heights = bin_heights.copy()
            
            # Create LED frame
            pixels = [0] * (MAX_LEDS * 3)
            
            for x in range(GRID_WIDTH):
                # Height of bar (0 to GRID_HEIGHT)
                height = int(bin_heights[x] * GRID_HEIGHT)
                
                for y in range(GRID_HEIGHT):
                    # Flip y so bars go up from bottom
                    y_flipped = GRID_HEIGHT - 1 - y
                    
                    if y >= (GRID_HEIGHT - height):
                        # Color based on height (rainbow from bottom to top)
                        hue = (y_flipped / GRID_HEIGHT) * 120  # Green to red
                        r, g, b = hsv_to_rgb(hue, 1.0, 1.0)
                        
                        # Apply brightness
                        r = int(r * brightness)
                        g = int(g * brightness)
                        b = int(b * brightness)
                        
                        led_idx = XY(x, y_flipped)
                        if led_idx >= 0 and led_idx < MAX_LEDS:
                            pixels[led_idx * 3] = r
                            pixels[led_idx * 3 + 1] = g
                            pixels[led_idx * 3 + 2] = b
            
            # Send to LED matrix
            sock.sendto(bytes(pixels), (ESP_IP, UDP_PORT))
            
            frame_count += 1
            
            # Show progress
            if frame_count % fps == 0:
                elapsed = time.time() - start_time
                print(f"\rFrame {frame_count}/{total_frames} | "
                      f"Time: {elapsed:.1f}s | "
                      f"Audio: {i / sample_rate:.1f}s", end='', flush=True)
            
            # Frame timing
            frame_time = time.time() - frame_start
            sleep_time = (1 / fps) - frame_time
            if sleep_time > 0:
                time.sleep(sleep_time)
        
        print(f"\n\nFinished! Played {frame_count} frames.")
        
    except KeyboardInterrupt:
        print("\n\nStopped by user.")
    finally:
        sock.close()

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="FFT Audio Visualizer for 32x32 LED Matrix")
    parser.add_argument("file", help="Audio file (WAV, MP3, FLAC, etc.)")
    parser.add_argument("--ip", default=ESP_IP, help="ESP8266 IP address")
    parser.add_argument("--brightness", type=float, default=0.3, help="LED brightness (0.0-1.0)")
    parser.add_argument("--fps", type=int, default=30, help="Frames per second")
    
    args = parser.parse_args()
    ESP_IP = args.ip
    
    visualize_audio(args.file, args.brightness, args.fps)

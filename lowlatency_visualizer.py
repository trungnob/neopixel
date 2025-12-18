#!/usr/bin/env python3
"""
Low-latency audio visualizer using parec for real-time system audio capture

Keyboard shortcuts:
  1 = bars mode
  2 = mirror mode  
  3 = wave mode
  4 = waveform mode (oscilloscope)
  5 = radial mode (circular spectrum)
  6 = beat mode (flash on bass)
  7 = spectrogram mode (waterfall)
  8 = vu mode (green-yellow-red bars)
  9 = bars16 mode (16 bands, mirrored)
  
  Amplitude scaling:
  g = gamma (default)
  d = dB (logarithmic)
  l = linear
  s = sqrt
  
  Colors:
  c = cycle color scheme
  
  q = quit
"""
import numpy as np
import socket
import sys
import subprocess
import requests
import gc
import select
import termios
import tty
from scipy.fft import rfft
import time

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
AMPLITUDE_SCALE = 0.8  # Moderate amplitude

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

def get_scheme_color(scheme, value, intensity=1.0):
    """Get RGB color from scheme based on value (0.0-1.0)"""
    r, g, b = 0, 0, 0
    
    if scheme == "rainbow":
        # Classic full spectrum
        hue = value * 300
        r, g, b = hsv_to_rgb(hue, 1.0, intensity)
        
    elif scheme == "fire":
        # Red -> Orange -> Yellow -> White
        # Hue 0 (Red) to 60 (Yellow)
        hue = value * 60
        # Saturation decreases at high values for "white hot" look
        sat = 1.0 - (max(0, value - 0.8) * 5)
        r, g, b = hsv_to_rgb(hue, sat, intensity)
        
    elif scheme == "ocean":
        # Deep Blue -> Cyan -> White
        # Hue 240 (Blue) to 180 (Cyan)
        hue = 240 - (value * 60)
        sat = 1.0 - (max(0, value - 0.8) * 5)
        r, g, b = hsv_to_rgb(hue, sat, intensity)
        
    elif scheme == "matrix":
        # Dark Green -> Bright Green -> White
        # Hue ~120
        hue = 120
        # Saturation drops at high intensity
        sat = 1.0 - (max(0, value - 0.7) * 3.3)
        # Intensity ramps up faster
        val = intensity * (0.3 + 0.7 * value)
        r, g, b = hsv_to_rgb(hue, sat, val)
        
    elif scheme == "vaporwave":
        # Purple -> Pink -> Cyan
        # Hue 270 (Purple) -> 300 (Pink) -> 180 (Cyan)
        if value < 0.5:
            hue = 270 + (value * 2 * 60) # 270->330
        else:
            hue = 330 + ((value - 0.5) * 2 * 210) # 330->540(180)
        r, g, b = hsv_to_rgb(hue, 1.0, intensity)
        
    elif scheme == "xmas":
        # Red and Green alternating bands
        # Use value to determine red or green band
        if (value * 10) % 2 < 1:
            hue = 0 # Red
        else:
            hue = 120 # Green
        r, g, b = hsv_to_rgb(hue, 1.0, intensity)
        
    return r, g, b

def main(brightness=0.3, mode="bars"):
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
    last_fps_time = time.time()
    last_fps_count = 0
    
    # State for new visualization modes
    waveform_history = np.zeros((GRID_WIDTH, CHUNK // (CHUNK // GRID_WIDTH)))  # Scrolling waveform buffer
    spectrogram_history = np.zeros((GRID_HEIGHT, GRID_WIDTH))  # Scrolling spectrogram
    beat_energy_history = []  # For beat detection
    beat_flash = 0.0  # Current flash intensity
    hue_offset = 0  # For color cycling
    
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
    
    # Check state and enter streaming mode
    print(f"Checking device state...")
    try:
        # Check if already in streaming mode
        r = requests.get(f'http://{ESP_IP}/info', timeout=2)
        state = r.json()
        if state.get('currentPattern') == 255:
            print("  Device already in streaming mode (Idle timeout active)")
        else:
            print(f"  Switching to streaming mode...")
            r = requests.get(f'http://{ESP_IP}/stream', timeout=2)
            print(f"  {r.text}")
    except Exception as e:
        print(f"Warning: Could not check/set streaming mode: {e}")
    
    print("Starting visualization...")
    print("Press 1-8 for modes, g/d/l/s for scaling, q=quit")
    
    # Disable garbage collection to prevent pauses
    gc.disable()
    
    # Setup non-blocking keyboard input
    old_settings = termios.tcgetattr(sys.stdin)
    modes = ["bars", "mirror", "wave", "waveform", "radial", "beat", "spectrogram", "vu", "bars16"]
    current_mode = mode
    current_scaling = "gamma"  # gamma, db, linear, sqrt
    
    # Color schemes
    color_schemes = ["rainbow", "fire", "ocean", "matrix", "vaporwave", "xmas"]
    current_color_scheme = "rainbow"
    
    try:
        tty.setcbreak(sys.stdin.fileno())
        
        while True:
            # Check for keyboard input
            if select.select([sys.stdin], [], [], 0)[0]:
                key = sys.stdin.read(1)
                if key == 'q':
                    break
                elif key == '1':
                    current_mode = "bars"
                    print(f"\rMode: bars       ", end='', flush=True)
                elif key == '2':
                    current_mode = "mirror"
                    print(f"\rMode: mirror     ", end='', flush=True)
                elif key == '3':
                    current_mode = "wave"
                    print(f"\rMode: wave       ", end='', flush=True)
                elif key == '4':
                    current_mode = "waveform"
                    print(f"\rMode: waveform   ", end='', flush=True)
                elif key == '5':
                    current_mode = "radial"
                    print(f"\rMode: radial     ", end='', flush=True)
                elif key == '6':
                    current_mode = "beat"
                    print(f"\rMode: beat       ", end='', flush=True)
                elif key == '7':
                    current_mode = "spectrogram"
                    print(f"\rMode: spectrogram", end='', flush=True)
                elif key == '8':
                    current_mode = "vu"
                    print(f"\rMode: vu         ", end='', flush=True)
                # Scaling keys
                elif key == 'g':
                    current_scaling = "gamma"
                    print(f"\rScaling: gamma   ", end='', flush=True)
                elif key == 'd':
                    current_scaling = "db"
                    print(f"\rScaling: dB      ", end='', flush=True)
                elif key == 'l':
                    current_scaling = "linear"
                    print(f"\rScaling: linear  ", end='', flush=True)
                elif key == 's':
                    current_scaling = "sqrt"
                    print(f"\rScaling: sqrt    ", end='', flush=True)
                elif key == '9':
                    current_mode = "bars16"
                    print(f"\rMode: bars16     ", end='', flush=True)
                # Color keys
                elif key == 'c':
                    # Cycle through color schemes
                    try:
                        idx = color_schemes.index(current_color_scheme)
                        idx = (idx + 1) % len(color_schemes)
                        current_color_scheme = color_schemes[idx]
                        print(f"\rColor: {current_color_scheme:<10}", end='', flush=True)
                    except ValueError:
                        current_color_scheme = "rainbow"
            
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
                
                # Apply selected amplitude scaling
                if current_scaling == "gamma":
                    bin_heights = np.power(bin_heights, 0.7)
                elif current_scaling == "db":
                    # Convert to dB scale (-60dB to 0dB range)
                    bin_heights = np.maximum(bin_heights, 1e-6)  # Avoid log(0)
                    bin_heights = 20 * np.log10(bin_heights)
                    bin_heights = (bin_heights + 60) / 60  # Normalize to 0-1
                    bin_heights = np.clip(bin_heights, 0, 1)
                elif current_scaling == "linear":
                    pass  # No transformation
                elif current_scaling == "sqrt":
                    bin_heights = np.sqrt(bin_heights)
                
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
                
                if current_mode == "bars":
                    # Normal bars from bottom
                    for x in range(GRID_WIDTH):
                        height = int(bin_heights[x] * GRID_HEIGHT)
                        peak_y = int(peaks[x])
                        
                        for y in range(GRID_HEIGHT):
                            led_idx = XY(x, y)
                            bar_y = GRID_HEIGHT - 1 - y
                            
                            if led_idx >= 0 and led_idx < MAX_LEDS:
                                if bar_y < height:
                                    r, g, b = get_scheme_color(current_color_scheme, x / GRID_WIDTH, 0.5 + 0.5 * (bar_y / GRID_HEIGHT))
                                    r = int(r * brightness)
                                    g = int(g * brightness)
                                    b = int(b * brightness)
                                elif bar_y == peak_y and peak_y > 0:
                                    r = int(255 * brightness)
                                    g = int(255 * brightness)
                                    b = int(255 * brightness)
                                else:
                                    r, g, b = 0, 0, 0
                                
                                pixels[led_idx * 3] = r
                                pixels[led_idx * 3 + 1] = g
                                pixels[led_idx * 3 + 2] = b
                
                elif current_mode == "mirror":
                    # Mirrored from center - bars grow up and down
                    center_y = GRID_HEIGHT // 2
                    for x in range(GRID_WIDTH):
                        half_height = int(bin_heights[x] * center_y)
                        
                        for y in range(GRID_HEIGHT):
                            led_idx = XY(x, y)
                            dist_from_center = abs(y - center_y)
                            
                            if led_idx >= 0 and led_idx < MAX_LEDS:
                                if dist_from_center < half_height:
                                    r, g, b = get_scheme_color(current_color_scheme, x / GRID_WIDTH, 0.3 + 0.7 * (1 - dist_from_center / center_y))
                                    r = int(r * brightness)
                                    g = int(g * brightness)
                                    b = int(b * brightness)
                                elif dist_from_center == int(peaks[x] * 0.5) and int(peaks[x]) > 0:
                                    # Peak dot - white, mirrored on both sides
                                    r = int(255 * brightness)
                                    g = int(255 * brightness)
                                    b = int(255 * brightness)
                                else:
                                    r, g, b = 0, 0, 0
                                
                                pixels[led_idx * 3] = r
                                pixels[led_idx * 3 + 1] = g
                                pixels[led_idx * 3 + 2] = b
                
                elif current_mode == "wave":
                    # Smooth waveform centered at middle - amplitude controls deviation
                    center_y = GRID_HEIGHT // 2
                    
                    # Smooth the heights for wave mode
                    smooth_heights = np.convolve(bin_heights, np.ones(3)/3, mode='same')
                    
                    for x in range(GRID_WIDTH):
                        # Amplitude determines how far from center (both up and down)
                        amplitude = smooth_heights[x] * (GRID_HEIGHT // 2)
                        wave_y = int(center_y + amplitude * np.sin(x * 0.3))  # Sine pattern
                        wave_y = max(0, min(GRID_HEIGHT - 1, wave_y))
                        
                        # Draw gradient line
                        for y in range(GRID_HEIGHT):
                            led_idx = XY(x, y)
                            if led_idx >= 0 and led_idx < MAX_LEDS:
                                dist = abs(y - wave_y)
                                if dist <= 2:
                                    r, g, b = get_scheme_color(current_color_scheme, x / GRID_WIDTH, 1.0 - dist * 0.35)
                                    r = int(r * brightness)
                                    g = int(g * brightness)
                                    b = int(b * brightness)
                                else:
                                    r, g, b = 0, 0, 0
                                
                                pixels[led_idx * 3] = r
                                pixels[led_idx * 3 + 1] = g
                                pixels[led_idx * 3 + 2] = b
                
                elif current_mode == "waveform":
                    # Oscilloscope-style waveform display (time domain)
                    # Sample the audio at GRID_WIDTH points
                    step = len(audio) // GRID_WIDTH
                    for x in range(GRID_WIDTH):
                        # Get sample value and map to y position
                        sample = audio[x * step]
                        y_pos = int((sample + 1) * (GRID_HEIGHT - 1) / 2)
                        y_pos = max(0, min(GRID_HEIGHT - 1, y_pos))
                        
                        for y in range(GRID_HEIGHT):
                            led_idx = XY(x, y)
                            if led_idx >= 0 and led_idx < MAX_LEDS:
                                dist = abs(y - y_pos)
                                if dist <= 1:
                                    # Color based on x position (time) using current scheme
                                    intensity = 1.0 - dist * 0.5
                                    r, g, b = get_scheme_color(current_color_scheme, x / GRID_WIDTH, intensity)
                                    r = int(r * brightness)
                                    g = int(g * brightness)
                                    b = int(b * brightness)
                                else:
                                    r, g, b = 0, 0, 0
                                
                                pixels[led_idx * 3] = r
                                pixels[led_idx * 3 + 1] = g
                                pixels[led_idx * 3 + 2] = b
                
                elif current_mode == "radial":
                    # Circular spectrum - bars radiate from center
                    center_x = GRID_WIDTH // 2
                    center_y = GRID_HEIGHT // 2
                    max_radius = min(center_x, center_y)
                    
                    for x in range(GRID_WIDTH):
                        for y in range(GRID_HEIGHT):
                            led_idx = XY(x, y)
                            if led_idx >= 0 and led_idx < MAX_LEDS:
                                # Calculate angle and distance from center
                                dx = x - center_x
                                dy = y - center_y
                                distance = np.sqrt(dx*dx + dy*dy)
                                angle = np.arctan2(dy, dx)  # -pi to pi
                                
                                # Map angle to frequency bin (0-31)
                                bin_idx = int(((angle + np.pi) / (2 * np.pi)) * GRID_WIDTH) % GRID_WIDTH
                                
                                # Check if this pixel should be lit
                                bar_length = bin_heights[bin_idx] * max_radius
                                
                                if distance <= bar_length and distance > 1:
                                    r, g, b = get_scheme_color(current_color_scheme, bin_idx / GRID_WIDTH, 0.5 + 0.5 * (1 - distance / max_radius))
                                    r = int(r * brightness)
                                    g = int(g * brightness)
                                    b = int(b * brightness)
                                else:
                                    r, g, b = 0, 0, 0
                                
                                pixels[led_idx * 3] = r
                                pixels[led_idx * 3 + 1] = g
                                pixels[led_idx * 3 + 2] = b
                
                elif current_mode == "beat":
                    # Beat-reactive flash mode
                    # Calculate bass energy (bins 0-4 are ~40-200Hz)
                    bass_energy = np.mean(bin_heights[0:5])
                    
                    # Track energy history for adaptive threshold
                    beat_energy_history.append(bass_energy)
                    if len(beat_energy_history) > 30:
                        beat_energy_history.pop(0)
                    
                    # Detect beat (energy significantly above average) - lowered thresholds
                    avg_energy = np.mean(beat_energy_history) if beat_energy_history else 0.3
                    if bass_energy > avg_energy * 1.3 and bass_energy > 0.15:
                        beat_flash = 1.0  # Trigger flash
                        hue_offset = (hue_offset + 30) % 360  # Change color on beat
                    
                    # Decay flash (faster decay = more distinct beats)
                    beat_flash = max(0, beat_flash - 0.08)
                    
                    # Fill screen with flash color
                    for x in range(GRID_WIDTH):
                        for y in range(GRID_HEIGHT):
                            led_idx = XY(x, y)
                            if led_idx >= 0 and led_idx < MAX_LEDS:
                                if beat_flash > 0.1:
                                    # Radial gradient from center
                                    dx = x - GRID_WIDTH // 2
                                    dy = y - GRID_HEIGHT // 2
                                    dist = np.sqrt(dx*dx + dy*dy) / (GRID_WIDTH // 2)
                                    
                                    val = hue_offset / 360.0 # Map 0-360 to 0.0-1.0
                                    intensity = beat_flash * (1 - dist * 0.5)
                                    r, g, b = get_scheme_color(current_color_scheme, val, intensity)
                                    r = int(r * brightness)
                                    g = int(g * brightness)
                                    b = int(b * brightness)
                                else:
                                    # Show dim spectrum when not flashing
                                    bar_y = GRID_HEIGHT - 1 - y
                                    height = int(bin_heights[x] * GRID_HEIGHT * 0.3)
                                    if bar_y < height:
                                        val = hue_offset / 360.0
                                        r, g, b = get_scheme_color(current_color_scheme, val, 0.2)
                                        r = int(r * brightness)
                                        g = int(g * brightness)
                                        b = int(b * brightness)
                                    else:
                                        r, g, b = 0, 0, 0
                                
                                pixels[led_idx * 3] = r
                                pixels[led_idx * 3 + 1] = g
                                pixels[led_idx * 3 + 2] = b
                
                elif current_mode == "spectrogram":
                    # Scrolling waterfall spectrogram
                    # Shift history up
                    spectrogram_history[:-1] = spectrogram_history[1:]
                    # Add new FFT data at bottom
                    spectrogram_history[-1] = bin_heights
                    
                    # Draw spectrogram
                    for x in range(GRID_WIDTH):
                        for y in range(GRID_HEIGHT):
                            led_idx = XY(x, y)
                            if led_idx >= 0 and led_idx < MAX_LEDS:
                                # y=0 is top (oldest), y=31 is bottom (newest)
                                intensity = spectrogram_history[y, x]
                                
                                if intensity > 0.05:
                                    # Color based on frequency (x) and intensity
                                    r, g, b = get_scheme_color(current_color_scheme, x / GRID_WIDTH, intensity)
                                    r = int(r * brightness)
                                    g = int(g * brightness)
                                    b = int(b * brightness)
                                else:
                                    r, g, b = 0, 0, 0
                                
                                pixels[led_idx * 3] = r
                                pixels[led_idx * 3 + 1] = g
                                pixels[led_idx * 3 + 2] = b
                
                elif current_mode == "vu":
                    # VU meter style - color based on height (green->yellow->red)
                    for x in range(GRID_WIDTH):
                        height = int(bin_heights[x] * GRID_HEIGHT)
                        peak_y = int(peaks[x])
                        
                        for y in range(GRID_HEIGHT):
                            led_idx = XY(x, y)
                            bar_y = GRID_HEIGHT - 1 - y
                            
                            if led_idx >= 0 and led_idx < MAX_LEDS:
                                if bar_y < height:
                                    # Color based on height using current scheme
                                    height_ratio = bar_y / GRID_HEIGHT
                                    r, g, b = get_scheme_color(current_color_scheme, height_ratio, 1.0)
                                    r = int(r * brightness)
                                    g = int(g * brightness)
                                    b = int(b * brightness)
                                elif bar_y == peak_y and peak_y > 0:
                                    # Peak dot - white
                                    r = int(255 * brightness)
                                    g = int(255 * brightness)
                                    b = int(255 * brightness)
                                else:
                                    r, g, b = 0, 0, 0
                                
                                pixels[led_idx * 3] = r
                                pixels[led_idx * 3 + 1] = g
                                pixels[led_idx * 3 + 2] = b
                
                elif current_mode == "bars16":
                    # 16 bands, mirrored around center
                    # Each band is 2 pixels wide (32 / 16 = 2)
                    # Use only first 16 bins, mirror to both sides
                    half_width = GRID_WIDTH // 2  # 16
                    
                    for x in range(GRID_WIDTH):
                        # Map x to one of 16 bins, mirrored around center
                        if x < half_width:
                            # Left side: bin 15 at center, bin 0 at left edge
                            bin_idx = half_width - 1 - x
                        else:
                            # Right side: bin 15 at center, bin 0 at right edge
                            bin_idx = x - half_width
                        
                        # Clamp to 16 bins (use only first 16 of 32)
                        bin_idx = min(bin_idx, 15)
                        height = int(bin_heights[bin_idx] * GRID_HEIGHT)
                        peak_y = int(peaks[bin_idx])
                        
                        for y in range(GRID_HEIGHT):
                            led_idx = XY(x, y)
                            bar_y = GRID_HEIGHT - 1 - y
                            
                            if led_idx >= 0 and led_idx < MAX_LEDS:
                                if bar_y < height:
                                    # Color based on bin using current scheme
                                    r, g, b = get_scheme_color(current_color_scheme, bin_idx / 16.0, 0.5 + 0.5 * (bar_y / GRID_HEIGHT))
                                    r = int(r * brightness)
                                    g = int(g * brightness)
                                    b = int(b * brightness)
                                elif bar_y == peak_y and peak_y > 0:
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
                
                now = time.time()
                if now - last_fps_time >= 1.0:
                    fps = (frame_count - last_fps_count) / (now - last_fps_time)
                    print(f"\rFPS: {fps:5.1f} | Frames: {frame_count}", end='', flush=True)
                    last_fps_time = now
                    last_fps_count = frame_count
    
    except KeyboardInterrupt:
        print(f"\n\nStopped. Total frames: {frame_count}")
    finally:
        # Restore terminal settings
        termios.tcsetattr(sys.stdin, termios.TCSADRAIN, old_settings)
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
    parser.add_argument("--mode", choices=["bars", "mirror", "wave", "waveform", "radial", "beat", "spectrogram", "vu"], 
                        default="bars",
                        help="Visualization mode: bars, mirror, wave, waveform, radial, beat, spectrogram, or vu")
    args = parser.parse_args()
    main(args.brightness, args.mode)

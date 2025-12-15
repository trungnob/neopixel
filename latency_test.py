#!/usr/bin/env python3
"""Latency test - measures exact audio latency"""
import subprocess
import time
import numpy as np

RATE = 48000
CHUNK = 512
CHANNELS = 2
MONITOR = 'visualizer_sink.monitor'

def get_level(parec):
    """Read one chunk and return level"""
    raw = parec.stdout.read(CHUNK * 2 * CHANNELS)
    audio = np.frombuffer(raw, dtype=np.int16).astype(np.float32)
    return np.max(np.abs(audio)) / 32768.0

def main():
    print("=" * 60)
    print("Audio Latency Test")
    print("=" * 60)
    print(f"Monitor: {MONITOR}")
    print()
    
    parec = subprocess.Popen(
        ['parec', '--rate', str(RATE), '--channels', str(CHANNELS),
         '--format', 's16le', '--latency-msec', '1', '--device', MONITOR],
        stdout=subprocess.PIPE, stderr=subprocess.DEVNULL
    )
    
    # Step 1: Measure noise floor (no music)
    print("Step 1: STOP all music. Measuring noise floor for 3 seconds...")
    input("Press ENTER when music is STOPPED...")
    
    noise_levels = []
    start = time.time()
    while time.time() - start < 3:
        level = get_level(parec)
        noise_levels.append(level)
    
    noise_floor = np.mean(noise_levels)
    print(f"  Noise floor: {noise_floor:.3f}")
    print()
    
    # Step 2: Measure music level
    print("Step 2: START playing music. Measuring for 3 seconds...")
    input("Press ENTER when music is PLAYING...")
    
    music_levels = []
    start = time.time()
    while time.time() - start < 3:
        level = get_level(parec)
        music_levels.append(level)
    
    music_avg = np.mean(music_levels)
    print(f"  Music level: {music_avg:.3f}")
    print()
    
    if music_avg < noise_floor * 1.5:
        print("WARNING: Music level is very close to noise floor!")
        print("Make sure audio output is set to 'Visualizer' sink")
    
    # Calculate threshold
    threshold = noise_floor + (music_avg - noise_floor) * 0.3
    print(f"  Detection threshold: {threshold:.3f}")
    print()
    
    # Step 3: Detect pause
    print("Step 3: Press ENTER, then IMMEDIATELY pause the music!")
    print("        I will measure exactly when the audio stops.")
    input("Press ENTER and then PAUSE music...")
    
    start = time.time()
    pause_detected_at = None
    
    samples = []
    while time.time() - start < 10:
        level = get_level(parec)
        elapsed = time.time() - start
        samples.append((elapsed, level))
        
        # Check if we crossed below threshold
        if pause_detected_at is None and level < threshold:
            # Need 3 consecutive low readings to confirm
            pass
    
    parec.terminate()
    
    # Find when level dropped
    print()
    print("=" * 60)
    print("Results:")
    print("=" * 60)
    
    # Find transition point
    window_size = 5
    for i in range(window_size, len(samples)):
        window_avg = np.mean([s[1] for s in samples[i-window_size:i]])
        if window_avg < threshold:
            pause_time = samples[i][0]
            print(f"  Audio level dropped at: {pause_time:.2f} seconds")
            print(f"  This is your TOTAL latency!")
            break
    else:
        print("  Could not detect pause in 10 seconds")
        print("  Make sure you actually paused the music")
    
    print()
    print("Sample data:")
    for t, lvl in samples[::10]:  # Every 10th sample
        bar = '#' * int(lvl * 40)
        marker = " <-- THRESHOLD" if abs(lvl - threshold) < 0.02 else ""
        print(f"  {t:5.2f}s | {lvl:.3f} | {bar}{marker}")

if __name__ == "__main__":
    main()

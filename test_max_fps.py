#!/usr/bin/env python3
"""
Test to measure the maximum FPS the ESP8266 LED matrix can handle.
Sends UDP packets at increasing rates and measures received vs displayed.
"""
import socket
import time
import requests
import sys

ESP_IP = "192.168.1.130"
UDP_PORT = 4210
MAX_LEDS = 1024

def get_stats():
    """Get UDP stats from ESP8266"""
    try:
        r = requests.get(f"http://{ESP_IP}/udpstats", timeout=2)
        return r.json()
    except:
        return None

def send_test_frames(fps, duration=3):
    """Send frames at given FPS for duration seconds"""
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    
    # Create test frame (alternating colors)
    frame_a = bytes([50, 100, 50] * MAX_LEDS)
    frame_b = bytes([100, 50, 100] * MAX_LEDS)
    
    interval = 1.0 / fps
    start = time.time()
    frames_sent = 0
    use_a = True
    
    while time.time() - start < duration:
        frame_start = time.time()
        
        sock.sendto(frame_a if use_a else frame_b, (ESP_IP, UDP_PORT))
        frames_sent += 1
        use_a = not use_a
        
        # Wait for next frame
        elapsed = time.time() - frame_start
        if elapsed < interval:
            time.sleep(interval - elapsed)
    
    sock.close()
    actual_fps = frames_sent / duration
    return frames_sent, actual_fps

def main():
    print("=" * 60)
    print("ESP8266 LED Matrix - Maximum FPS Test")
    print("=" * 60)
    print(f"Target: {ESP_IP}:{UDP_PORT}")
    print()
    
    # Check connectivity
    print("Checking ESP8266 connection...")
    stats = get_stats()
    if stats is None:
        print("ERROR: Cannot connect to ESP8266!")
        print(f"Make sure firmware is updated with /udpstats endpoint")
        print(f"Try: curl http://{ESP_IP}/udpstats")
        sys.exit(1)
    
    print(f"Connected! Uptime: {stats['uptime']}s")
    print()
    
    # Test different FPS rates
    test_rates = [10, 20, 30, 40, 50, 60, 80, 100, 120, 150]
    results = []
    
    print("Testing FPS rates...")
    print("-" * 60)
    print(f"{'Target FPS':>12} | {'Sent':>8} | {'Recv':>8} | {'Disp':>8} | {'Drop%':>8}")
    print("-" * 60)
    
    for target_fps in test_rates:
        # Get initial stats
        before = get_stats()
        if before is None:
            print(f"Lost connection at {target_fps} FPS")
            break
        
        # Send frames
        sent, actual_fps = send_test_frames(target_fps, duration=3)
        
        # Wait a moment for ESP to process
        time.sleep(0.2)
        
        # Get final stats
        after = get_stats()
        if after is None:
            print(f"Lost connection at {target_fps} FPS")
            break
        
        # Calculate deltas
        recv_delta = after['received'] - before['received']
        disp_delta = after['displayed'] - before['displayed']
        drop_pct = (recv_delta - disp_delta) / recv_delta * 100 if recv_delta > 0 else 0
        
        results.append({
            'target': target_fps,
            'sent': sent,
            'received': recv_delta,
            'displayed': disp_delta,
            'drop_pct': drop_pct
        })
        
        print(f"{target_fps:>12} | {sent:>8} | {recv_delta:>8} | {disp_delta:>8} | {drop_pct:>7.1f}%")
    
    print("-" * 60)
    print()
    
    # Find optimal FPS
    if results:
        # Find highest FPS with <10% drops
        optimal = None
        for r in results:
            if r['drop_pct'] < 10:
                optimal = r
        
        if optimal:
            print(f"✓ Optimal FPS: {optimal['target']} (with <10% packet loss)")
        
        # Find actual max displayed FPS
        max_displayed = max(r['displayed'] / 3 for r in results)  # /3 because 3 second test
        print(f"✓ Max displayed FPS: ~{max_displayed:.0f}")
        
        # Theoretical limit
        led_time_ms = MAX_LEDS * 0.03  # ~30μs per LED
        max_theoretical = 1000 / led_time_ms
        print(f"✓ Theoretical max (LED timing): ~{max_theoretical:.0f} FPS")
    
    print()
    print("Done!")

if __name__ == "__main__":
    main()

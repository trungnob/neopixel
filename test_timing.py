#!/usr/bin/env python3
"""
Test ESP8266 timing profile to understand latency breakdown.
"""
import socket
import time
import requests
import json

ESP_IP = "192.168.1.130"
UDP_PORT = 4210
MAX_LEDS = 1024

def main():
    print("=" * 60)
    print("ESP8266 LED Matrix - Timing Profile Test")
    print("=" * 60)
    
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    frame = bytes([50, 100, 50] * MAX_LEDS)
    
    # Wait for streaming to exit
    print("\nWaiting for streaming mode to exit (6 seconds)...")
    time.sleep(6)
    
    # Check connection
    try:
        r = requests.get(f"http://{ESP_IP}/timing", timeout=5)
        print(f"Connected to ESP8266!")
    except Exception as e:
        print(f"ERROR: Cannot connect to ESP8266: {e}")
        return
    
    # Reset timing by reading once
    requests.get(f"http://{ESP_IP}/timing", timeout=5)
    
    # Send frames at a slow rate
    print("\nSending 30 frames at 5 FPS (200ms interval)...")
    for i in range(30):
        sock.sendto(frame, (ESP_IP, UDP_PORT))
        time.sleep(0.2)
    
    # Wait for processing
    print("Waiting for ESP to process...")
    time.sleep(2)
    
    # Get timing data
    print("\nReading timing profile...")
    r = requests.get(f"http://{ESP_IP}/timing", timeout=5)
    timing = r.json()
    
    print("\n" + "=" * 60)
    print("TIMING PROFILE (microseconds)")
    print("=" * 60)
    
    if timing.get("samples", 0) > 0:
        samples = timing["samples"]
        parse_us = timing.get("avgParsePacket_us", 0)
        read_us = timing.get("avgReadPacket_us", 0)
        drain_us = timing.get("avgDrainLoop_us", 0)
        show_us = timing.get("avgFastLEDShow_us", 0)
        total_us = timing.get("avgLoopTotal_us", 0)
        max_fps = timing.get("maxFPS", 0)
        
        # Calculate percentages
        if total_us > 0:
            parse_pct = parse_us / total_us * 100
            read_pct = read_us / total_us * 100
            drain_pct = drain_us / total_us * 100
            show_pct = show_us / total_us * 100
        else:
            parse_pct = read_pct = drain_pct = show_pct = 0
        
        print(f"Samples collected:     {samples}")
        print()
        print(f"{'Step':<25} {'Time (us)':<12} {'Time (ms)':<12} {'%':<8}")
        print("-" * 60)
        print(f"{'parsePacket()':<25} {parse_us:<12} {parse_us/1000:<12.2f} {parse_pct:<8.1f}")
        print(f"{'read() 3072 bytes':<25} {read_us:<12} {read_us/1000:<12.2f} {read_pct:<8.1f}")
        print(f"{'Drain loop':<25} {drain_us:<12} {drain_us/1000:<12.2f} {drain_pct:<8.1f}")
        print(f"{'FastLED.show()':<25} {show_us:<12} {show_us/1000:<12.2f} {show_pct:<8.1f}")
        print("-" * 60)
        print(f"{'TOTAL':<25} {total_us:<12} {total_us/1000:<12.2f} {'100.0':<8}")
        print()
        print(f"Max sustainable FPS:   {max_fps}")
        print(f"At 10 FPS (100ms):     {'OK' if total_us < 100000 else 'TOO SLOW'}")
        print(f"At 25 FPS (40ms):      {'OK' if total_us < 40000 else 'TOO SLOW'}")
        print(f"At 30 FPS (33ms):      {'OK' if total_us < 33000 else 'TOO SLOW'}")
    else:
        print("No timing data collected!")
        print(f"Raw response: {timing}")
    
    sock.close()
    print("\nDone!")

if __name__ == "__main__":
    main()

import socket
import time
import sys
import argparse
import cv2
import numpy as np

# Configuration
ESP_IP = "192.168.1.130"
UDP_PORT = 4210
GRID_WIDTH = 32
GRID_HEIGHT = 32
PANELS_WIDE = 1
MAX_LEDS = 1024

def XY(x, y):
    if x < 0 or x >= GRID_WIDTH or y < 0 or y >= GRID_HEIGHT:
        return -1

    # Layout 0: Nx8x32 Multi-Panel (Default)
    # Supports 1 or more panels.
    # Each panel is 8x32, Column-Zigzag.

    # 1. Determine which Panel (px, py) the global (x, y) falls into
    panelCol = x // 32
    panelRow = y // 8
    
    # 2. Determine Local coordinates within that panel
    localX = x % 32
    localY = y % 8
    
    # Handle 180-degree rotation for odd panel rows (Serpentine Vertical)
    if panelRow % 2 == 1:
        localX = 31 - localX
        localY = 7 - localY
        
    # 3. Calculate Panel Index based on Serpentine Path
    if panelRow % 2 == 0:
        # Even Panel Row: Left to Right
        panelIndex = (panelRow * PANELS_WIDE) + panelCol
    else:
        # Odd Panel Row: Right to Left
        panelIndex = (panelRow * PANELS_WIDE) + (PANELS_WIDE - 1 - panelCol)
        
    # 4. Calculate LED Offset
    panelOffset = panelIndex * 256
    
    # 5. Map Local Coordinates (Standard 8x32 Column ZigZag)
    if localX % 2 == 0:
        # Even columns: TOP to BOTTOM
        return panelOffset + (localX * 8 + localY)
    else:
        # Odd columns: BOTTOM to TOP (reversed)
        return panelOffset + (localX * 8 + (7 - localY))

def stream_video(video_path, brightness=1.0, fps=None, loop=False):
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    
    # Open video file
    cap = cv2.VideoCapture(video_path)
    
    if not cap.isOpened():
        print(f"Error: Could not open video file: {video_path}")
        return
    
    # Get video properties
    total_frames = int(cap.get(cv2.CAP_PROP_FRAME_COUNT))
    video_fps = cap.get(cv2.CAP_PROP_FPS)
    video_width = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH))
    video_height = int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))
    
    # Use video FPS if not specified
    if fps is None:
        fps = video_fps
    
    frame_delay = 1.0 / fps if fps > 0 else 0.033  # Default to ~30fps
    
    print(f"Video: {video_path}")
    print(f"  Resolution: {video_width}x{video_height}")
    print(f"  FPS: {video_fps:.2f} (streaming at {fps:.2f} FPS)")
    print(f"  Total frames: {total_frames}")
    print(f"  Brightness: {brightness}")
    print(f"  Streaming to {ESP_IP}:{UDP_PORT}")
    print(f"  Loop: {'Yes' if loop else 'No'}")
    print("\nPress Ctrl+C to stop streaming...\n")
    
    frame_count = 0
    start_time = time.time()
    
    try:
        while True:
            ret, frame = cap.read()
            
            # If end of video
            if not ret:
                if loop:
                    # Reset to beginning
                    cap.set(cv2.CAP_PROP_POS_FRAMES, 0)
                    continue
                else:
                    print("\nEnd of video reached.")
                    break
            
            # Convert BGR to RGB
            frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
            
            # Resize to grid
            frame = cv2.resize(frame, (GRID_WIDTH, GRID_HEIGHT), interpolation=cv2.INTER_LANCZOS4)
            
            # Prepare buffer
            pixels = [0] * (MAX_LEDS * 3)
            
            for y in range(GRID_HEIGHT):
                for x in range(GRID_WIDTH):
                    r, g, b = frame[y, x]
                    
                    # Apply brightness
                    r = int(r * brightness)
                    g = int(g * brightness)
                    b = int(b * brightness)
                    
                    led_idx = XY(x, y)
                    if led_idx != -1 and led_idx < MAX_LEDS:
                        pixels[led_idx * 3] = r
                        pixels[led_idx * 3 + 1] = g
                        pixels[led_idx * 3 + 2] = b
            
            # Send packet
            data = bytes(pixels)
            sock.sendto(data, (ESP_IP, UDP_PORT))
            
            frame_count += 1
            
            # Display progress
            if frame_count % 30 == 0:
                elapsed = time.time() - start_time
                actual_fps = frame_count / elapsed if elapsed > 0 else 0
                print(f"\rFrame {frame_count}/{total_frames if not loop else '∞'} | "
                      f"Actual FPS: {actual_fps:.2f}", end='', flush=True)
            
            # Frame timing
            time.sleep(frame_delay)
                
    except KeyboardInterrupt:
        print("\n\nStopping stream.")
        elapsed = time.time() - start_time
        actual_fps = frame_count / elapsed if elapsed > 0 else 0
        print(f"Streamed {frame_count} frames in {elapsed:.2f}s ({actual_fps:.2f} FPS)")
    finally:
        cap.release()
        sock.close()

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Stream video files to ESP8266 LED Matrix")
    parser.add_argument("file", help="Path to video file (mp4, avi, mov, etc.)")
    parser.add_argument("--ip", default=ESP_IP, help="ESP8266 IP Address")
    parser.add_argument("--fps", type=float, default=None, help="Target FPS (default: use video's FPS)")
    parser.add_argument("--brightness", type=float, default=1.0, help="Brightness multiplier (0.0 - 1.0)")
    parser.add_argument("--loop", action="store_true", help="Loop video continuously")
    
    args = parser.parse_args()
    
    ESP_IP = args.ip
    stream_video(args.file, args.brightness, args.fps, args.loop)

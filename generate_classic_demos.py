#!/usr/bin/env python3
"""
Create classic demo-style videos for LED matrix
"""
import cv2
import numpy as np
import math

WIDTH = 32
HEIGHT = 32
FPS = 30

def create_classic_tunnel(filename="tunnel_demo.mp4", duration=30):
    """Classic tunnel effect demo"""
    fourcc = cv2.VideoWriter_fourcc(*'mp4v')
    out = cv2.VideoWriter(filename, fourcc, FPS, (WIDTH, HEIGHT))
    
    total_frames = duration * FPS
    print(f"Creating classic tunnel effect ({duration}s)...")
    
    # Pre-calculate tunnel lookup table
    tunnel_distance = np.zeros((HEIGHT, WIDTH))
    tunnel_angle = np.zeros((HEIGHT, WIDTH))
    
    for y in range(HEIGHT):
        for x in range(WIDTH):
            dx = x - WIDTH / 2
            dy = y - HEIGHT / 2
            distance = math.sqrt(dx * dx + dy * dy)
            tunnel_distance[y, x] = 32 * HEIGHT / (distance + 1)
            tunnel_angle[y, x] = math.atan2(dy, dx) * 180 / math.pi
    
    for frame_num in range(total_frames):
        frame = np.zeros((HEIGHT, WIDTH, 3), dtype=np.uint8)
        t = frame_num / FPS
        
        for y in range(HEIGHT):
            for x in range(WIDTH):
                # Animated tunnel
                u = int(tunnel_distance[y, x] + t * 20) % 32
                v = int(tunnel_angle[y, x] + t * 30) % 32
                
                # Checkerboard pattern
                if (u + v) % 16 < 8:
                    # Rainbow color based on depth
                    hue = int((tunnel_distance[y, x] + t * 50) % 180)
                    hsv = np.uint8([[[hue, 255, 255]]])
                    rgb = cv2.cvtColor(hsv, cv2.COLOR_HSV2BGR)
                    frame[y, x] = rgb[0, 0]
                else:
                    frame[y, x] = (20, 20, 20)
        
        out.write(frame)
        if frame_num % 60 == 0:
            print(f"  {int(100 * frame_num / total_frames)}%")
    
    out.release()
    print(f"✓ {filename}")

def create_classic_plasma(filename="plasma_demo.mp4", duration=30):
    """Classic plasma effect"""
    fourcc = cv2.VideoWriter_fourcc(*'mp4v')
    out = cv2.VideoWriter(filename, fourcc, FPS, (WIDTH, HEIGHT))
    
    total_frames = duration * FPS
    print(f"\nCreating classic plasma effect ({duration}s)...")
    
    for frame_num in range(total_frames):
        frame = np.zeros((HEIGHT, WIDTH, 3), dtype=np.uint8)
        t = frame_num / FPS
        
        for y in range(HEIGHT):
            for x in range(WIDTH):
                # Multiple sine waves for plasma
                value = 0
                value += math.sin(x / 4.0 + t)
                value += math.sin(y / 3.0 + t * 1.5)
                value += math.sin((x + y) / 5.0 + t * 0.5)
                value += math.sin(math.sqrt(x*x + y*y) / 4.0 + t * 2)
                
                # Map to hue
                hue = int((value * 20 + t * 30) % 180)
                saturation = 255
                brightness = int(128 + 127 * math.sin(value))
                
                hsv = np.uint8([[[hue, saturation, brightness]]])
                rgb = cv2.cvtColor(hsv, cv2.COLOR_HSV2BGR)
                frame[y, x] = rgb[0, 0]
        
        out.write(frame)
        if frame_num % 60 == 0:
            print(f"  {int(100 * frame_num / total_frames)}%")
    
    out.release()
    print(f"✓ {filename}")

def create_classic_rotozoom(filename="rotozoom_demo.mp4", duration=30):
    """Classic rotozoom effect with pattern"""
    fourcc = cv2.VideoWriter_fourcc(*'mp4v')
    out = cv2.VideoWriter(filename, fourcc, FPS, (WIDTH, HEIGHT))
    
    total_frames = duration * FPS
    print(f"\nCreating classic rotozoom effect ({duration}s)...")
    
    # Create pattern texture
    pattern_size = 64
    pattern = np.zeros((pattern_size, pattern_size, 3), dtype=np.uint8)
    for y in range(pattern_size):
        for x in range(pattern_size):
            if ((x // 8) + (y // 8)) % 2:
                hue = (x * 3 + y * 3) % 180
                hsv = np.uint8([[[hue, 255, 255]]])
                rgb = cv2.cvtColor(hsv, cv2.COLOR_HSV2BGR)
                pattern[y, x] = rgb[0, 0]
    
    for frame_num in range(total_frames):
        frame = np.zeros((HEIGHT, WIDTH, 3), dtype=np.uint8)
        t = frame_num / FPS
        
        # Rotation and zoom
        angle = t * 50
        zoom = 1.0 + 0.5 * math.sin(t * 0.5)
        
        # Rotation matrix
        cos_a = math.cos(math.radians(angle))
        sin_a = math.sin(math.radians(angle))
        
        for y in range(HEIGHT):
            for x in range(WIDTH):
                # Center and rotate
                dx = (x - WIDTH / 2) / zoom
                dy = (y - HEIGHT / 2) / zoom
                
                rx = dx * cos_a - dy * sin_a
                ry = dx * sin_a + dy * cos_a
                
                # Map to pattern
                px = int(rx + pattern_size / 2 + t * 20) % pattern_size
                py = int(ry + pattern_size / 2 + t * 15) % pattern_size
                
                frame[y, x] = pattern[py, px]
        
        out.write(frame)
        if frame_num % 60 == 0:
            print(f"  {int(100 * frame_num / total_frames)}%")
    
    out.release()
    print(f"✓ {filename}")

def create_scrolling_credits(filename="credits_demo.mp4", duration=20):
    """Star Wars style scrolling credits"""
    fourcc = cv2.VideoWriter_fourcc(*'mp4v')
    out = cv2.VideoWriter(filename, fourcc, FPS, (WIDTH, HEIGHT))
    
    total_frames = duration * FPS
    print(f"\nCreating scrolling credits ({duration}s)...")
    
    credits_text = [
        "",
        "   LED MATRIX",
        "    CLASSICS",
        "",
        "",
        "   DIRECTED BY",
        "    YOU",
        "",
        "",
        "  POWERED BY",
        "   ESP8266",
        "",
        "",
        "  STREAMING",
        "   SYSTEM",
        "",
        "",
        "   CREATED",
        "   WITH AI",
        "",
        "",
        "    ENJOY!",
        "",
        ""
    ]
    
    for frame_num in range(total_frames):
        frame = np.zeros((HEIGHT, WIDTH, 3), dtype=np.uint8)
        
        # Starfield background
        np.random.seed(42)
        for _ in range(20):
            sx = np.random.randint(0, WIDTH)
            sy = (np.random.randint(0, HEIGHT * 3) - frame_num) % HEIGHT
            brightness = np.random.randint(100, 255)
            frame[sy, sx] = (brightness, brightness, brightness)
        
        # Scrolling text
        scroll_pos = HEIGHT + frame_num // 2
        
        for i, line in enumerate(credits_text):
            y_pos = scroll_pos - i * 6
            if 0 <= y_pos < HEIGHT:
                # Draw each character
                for j, char in enumerate(line):
                    x_pos = j * 3
                    if 0 <= x_pos < WIDTH and char != ' ':
                        # Simple block text
                        cv2.rectangle(frame, (x_pos, y_pos), (x_pos + 2, y_pos + 4), 
                                    (100, 200, 255), -1)
        
        out.write(frame)
        if frame_num % 60 == 0:
            print(f"  {int(100 * frame_num / total_frames)}%")
    
    out.release()
    print(f"✓ {filename}")

if __name__ == "__main__":
    print("=" * 60)
    print("Creating Classic Demo Videos")
    print("=" * 60)
    
    create_classic_tunnel()
    create_classic_plasma()
    create_classic_rotozoom()
    create_scrolling_credits()
    
    print("\n" + "=" * 60)
    print("✨ All classic demos generated!")
    print("=" * 60)
    print("\nStream them with:")
    print("  python3 video_streamer.py tunnel_demo.mp4 --brightness 0.3 --loop")
    print("  python3 video_streamer.py plasma_demo.mp4 --brightness 0.3 --loop")
    print("  python3 video_streamer.py rotozoom_demo.mp4 --brightness 0.3 --loop")
    print("  python3 video_streamer.py credits_demo.mp4 --brightness 0.3")
    print("=" * 60)

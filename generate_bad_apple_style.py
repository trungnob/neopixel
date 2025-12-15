#!/usr/bin/env python3
"""
Create a Bad Apple-style high-contrast silhouette animation
"""
import cv2
import numpy as np
import math

WIDTH = 32
HEIGHT = 32
FPS = 24
DURATION = 30  # 30 seconds

def create_bad_apple_style():
    """Create a Bad Apple-style silhouette animation"""
    fourcc = cv2.VideoWriter_fourcc(*'mp4v')
    out = cv2.VideoWriter('bad_apple_style.mp4', fourcc, FPS, (WIDTH, HEIGHT))
    
    total_frames = DURATION * FPS
    
    print(f"Creating Bad Apple-style animation ({DURATION}s @ {FPS} FPS)...")
    
    for frame_num in range(total_frames):
        # Create high-contrast black and white frame
        frame = np.zeros((HEIGHT, WIDTH, 3), dtype=np.uint8)
        
        # Time in seconds
        t = frame_num / FPS
        
        # Different scenes/patterns
        scene = int(t / 5) % 6
        
        if scene == 0:  # Expanding circles
            num_circles = 5
            for i in range(num_circles):
                phase = (t * 2 + i * 0.5) % 3
                if phase < 2:
                    radius = int(phase * 10)
                    center_x = WIDTH // 2 + int(8 * math.sin(i))
                    center_y = HEIGHT // 2 + int(8 * math.cos(i))
                    cv2.circle(frame, (center_x, center_y), radius, (255, 255, 255), 1)
        
        elif scene == 1:  # Rotating shapes
            angle = t * 60
            for i in range(4):
                a = angle + i * 90
                x1 = WIDTH // 2 + int(12 * math.cos(math.radians(a)))
                y1 = HEIGHT // 2 + int(12 * math.sin(math.radians(a)))
                x2 = WIDTH // 2 + int(6 * math.cos(math.radians(a + 90)))
                y2 = HEIGHT // 2 + int(6 * math.sin(math.radians(a + 90)))
                cv2.line(frame, (x1, y1), (x2, y2), (255, 255, 255), 1)
        
        elif scene == 2:  # Wave pattern
            for x in range(WIDTH):
                y = int(HEIGHT // 2 + 10 * math.sin(x * 0.3 + t * 3))
                if 0 <= y < HEIGHT:
                    cv2.circle(frame, (x, y), 2, (255, 255, 255), -1)
        
        elif scene == 3:  # Tunnel effect
            for r in range(5, 20, 3):
                radius = int(r + 5 * math.sin(t * 2))
                cv2.circle(frame, (WIDTH // 2, HEIGHT // 2), radius, (255, 255, 255), 1)
        
        elif scene == 4:  # Falling objects
            num_objects = 8
            for i in range(num_objects):
                x = (i * 4 + int(t * 5)) % WIDTH
                y = int((t * 10 + i * 5) % HEIGHT)
                cv2.rectangle(frame, (x-2, y-2), (x+2, y+2), (255, 255, 255), -1)
        
        else:  # Spiral
            points = []
            for i in range(50):
                angle = i * 20 + t * 100
                radius = i * 0.4
                x = WIDTH // 2 + int(radius * math.cos(math.radians(angle)))
                y = HEIGHT // 2 + int(radius * math.sin(math.radians(angle)))
                if 0 <= x < WIDTH and 0 <= y < HEIGHT:
                    points.append((x, y))
            
            for i in range(len(points) - 1):
                cv2.line(frame, points[i], points[i + 1], (255, 255, 255), 1)
        
        # Add some noise for texture
        if frame_num % 2 == 0:
            noise = np.random.rand(HEIGHT, WIDTH) < 0.02
            frame[noise] = 255
        
        out.write(frame)
        
        # Progress indicator
        if frame_num % 60 == 0:
            print(f"  Progress: {int(100 * frame_num / total_frames)}%")
    
    out.release()
    print(f"✓ Generated: bad_apple_style.mp4")
    print(f"\nStream it with:")
    print(f"  python3 video_streamer.py bad_apple_style.mp4 --brightness 0.3 --loop")

if __name__ == "__main__":
    create_bad_apple_style()

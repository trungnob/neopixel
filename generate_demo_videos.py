#!/usr/bin/env python3
"""
Generate demo videos for 32x32 LED matrix
"""
import cv2
import numpy as np
import math

WIDTH = 32
HEIGHT = 32
FPS = 20

def create_color_test_video(filename="color_test.mp4", duration=10):
    """Create a color test pattern video"""
    fourcc = cv2.VideoWriter_fourcc(*'mp4v')
    out = cv2.VideoWriter(filename, fourcc, FPS, (WIDTH, HEIGHT))
    
    total_frames = duration * FPS
    
    for frame_num in range(total_frames):
        # Create frame
        frame = np.zeros((HEIGHT, WIDTH, 3), dtype=np.uint8)
        
        # Cycling through different test patterns
        pattern = (frame_num // (FPS * 2)) % 5
        
        if pattern == 0:  # Rainbow gradient
            for y in range(HEIGHT):
                for x in range(WIDTH):
                    hue = (x * 180 / WIDTH + frame_num * 2) % 180
                    hsv = np.uint8([[[hue, 255, 255]]])
                    rgb = cv2.cvtColor(hsv, cv2.COLOR_HSV2BGR)
                    frame[y, x] = rgb[0, 0]
        
        elif pattern == 1:  # Checkerboard
            checker_size = 4
            for y in range(HEIGHT):
                for x in range(WIDTH):
                    if ((x // checker_size) + (y // checker_size)) % 2:
                        frame[y, x] = (255, 255, 255)
        
        elif pattern == 2:  # Color bars
            bar_width = WIDTH // 7
            colors = [(255, 255, 255), (255, 255, 0), (0, 255, 255),
                     (0, 255, 0), (255, 0, 255), (255, 0, 0), (0, 0, 255)]
            for i, color in enumerate(colors):
                cv2.rectangle(frame, (i * bar_width, 0), ((i + 1) * bar_width, HEIGHT), color, -1)
        
        elif pattern == 3:  # Gradient
            for y in range(HEIGHT):
                value = int(255 * y / HEIGHT)
                frame[y, :] = (value, value, value)
        
        else:  # Solid colors cycling
            colors = [(255, 0, 0), (0, 255, 0), (0, 0, 255), (255, 255, 0), (255, 0, 255), (0, 255, 255)]
            color_idx = (frame_num // 10) % len(colors)
            frame[:, :] = colors[color_idx]
        
        out.write(frame)
    
    out.release()
    print(f"✓ Generated: {filename} ({duration}s, {FPS} FPS)")

def create_bouncing_ball_video(filename="bouncing_ball.mp4", duration=15):
    """Create a physics-based bouncing ball animation"""
    fourcc = cv2.VideoWriter_fourcc(*'mp4v')
    out = cv2.VideoWriter(filename, fourcc, FPS, (WIDTH, HEIGHT))
    
    # Ball physics
    x, y = WIDTH // 2, 5
    vx, vy = 2.5, 0
    gravity = 0.5
    damping = 0.85
    radius = 3
    
    total_frames = duration * FPS
    
    for frame_num in range(total_frames):
        # Create frame with gradient background
        frame = np.zeros((HEIGHT, WIDTH, 3), dtype=np.uint8)
        for i in range(HEIGHT):
            value = int(30 + 20 * i / HEIGHT)
            frame[i, :] = (value, value // 2, value // 3)
        
        # Update physics
        vy += gravity
        x += vx
        y += vy
        
        # Bounce off walls
        if x - radius < 0 or x + radius >= WIDTH:
            vx = -vx * damping
            x = max(radius, min(WIDTH - radius - 1, x))
        
        if y + radius >= HEIGHT:
            vy = -vy * damping
            y = HEIGHT - radius - 1
            if abs(vy) < 1:
                vy = -15  # Reset to top
        
        # Draw ball with glow
        cv2.circle(frame, (int(x), int(y)), radius + 2, (100, 100, 255), 1)
        cv2.circle(frame, (int(x), int(y)), radius, (255, 200, 100), -1)
        
        # Add trail
        trail_len = 5
        for i in range(trail_len):
            trail_x = int(x - vx * i * 0.3)
            trail_y = int(y - vy * i * 0.3)
            if 0 <= trail_x < WIDTH and 0 <= trail_y < HEIGHT:
                alpha = 1 - i / trail_len
                cv2.circle(frame, (trail_x, trail_y), radius - 1, 
                          (int(255 * alpha), int(200 * alpha), int(100 * alpha)), -1)
        
        out.write(frame)
    
    out.release()
    print(f"✓ Generated: {filename} ({duration}s, {FPS} FPS)")

def create_waveform_video(filename="waveform.mp4", duration=20):
    """Create mesmerizing waveform visualization"""
    fourcc = cv2.VideoWriter_fourcc(*'mp4v')
    out = cv2.VideoWriter(filename, fourcc, FPS, (WIDTH, HEIGHT))
    
    total_frames = duration * FPS
    
    for frame_num in range(total_frames):
        frame = np.zeros((HEIGHT, WIDTH, 3), dtype=np.uint8)
        
        # Multiple sine waves
        for x in range(WIDTH):
            # Combine multiple frequencies
            y1 = HEIGHT / 2 + 8 * math.sin(x * 0.3 + frame_num * 0.1)
            y2 = HEIGHT / 2 + 6 * math.sin(x * 0.5 - frame_num * 0.15)
            y3 = HEIGHT / 2 + 4 * math.sin(x * 0.2 + frame_num * 0.2)
            
            # Draw each wave
            for y in range(HEIGHT):
                dist1 = abs(y - y1)
                dist2 = abs(y - y2)
                dist3 = abs(y - y3)
                
                # Color based on proximity to waves
                r = int(255 * max(0, 1 - dist1 / 3))
                g = int(255 * max(0, 1 - dist2 / 3))
                b = int(255 * max(0, 1 - dist3 / 3))
                
                if r > 0 or g > 0 or b > 0:
                    frame[y, x] = (b, g, r)  # BGR format
        
        out.write(frame)
    
    out.release()
    print(f"✓ Generated: {filename} ({duration}s, {FPS} FPS)")

def create_starfield_video(filename="starfield.mp4", duration=20):
    """Create a moving starfield like a spaceship view"""
    fourcc = cv2.VideoWriter_fourcc(*'mp4v')
    out = cv2.VideoWriter(filename, fourcc, FPS, (WIDTH, HEIGHT))
    
    # Initialize stars
    num_stars = 100
    stars = []
    for _ in range(num_stars):
        stars.append({
            'x': np.random.rand() * WIDTH - WIDTH / 2,
            'y': np.random.rand() * HEIGHT - HEIGHT / 2,
            'z': np.random.rand() * WIDTH
        })
    
    total_frames = duration * FPS
    
    for frame_num in range(total_frames):
        frame = np.zeros((HEIGHT, WIDTH, 3), dtype=np.uint8)
        
        # Update and draw stars
        for star in stars:
            # Move star towards camera
            star['z'] -= 0.5
            
            # Reset if too close
            if star['z'] <= 0:
                star['z'] = WIDTH
                star['x'] = np.random.rand() * WIDTH - WIDTH / 2
                star['y'] = np.random.rand() * HEIGHT - HEIGHT / 2
            
            # Project to 2D
            k = 128 / star['z']
            x = int(star['x'] * k + WIDTH / 2)
            y = int(star['y'] * k + HEIGHT / 2)
            
            # Draw star if on screen
            if 0 <= x < WIDTH and 0 <= y < HEIGHT:
                # Size and brightness based on depth
                brightness = int(255 * (1 - star['z'] / WIDTH))
                size = max(1, int(3 * (1 - star['z'] / WIDTH)))
                
                cv2.circle(frame, (x, y), size // 2, 
                          (brightness, brightness, brightness), -1)
        
        out.write(frame)
    
    out.release()
    print(f"✓ Generated: {filename} ({duration}s, {FPS} FPS)")

if __name__ == "__main__":
    print("Generating demo videos for 32x32 LED matrix...\n")
    
    create_color_test_video()
    create_bouncing_ball_video()
    create_waveform_video()
    create_starfield_video()
    
    print(f"\n✨ Generated 4 demo videos!")
    print(f"\nTo stream any of them:")
    print(f"  python3 video_streamer.py <filename>.mp4 --brightness 0.3 --loop")
    print(f"\nAvailable videos:")
    print(f"  - color_test.mp4      (color test patterns)")
    print(f"  - bouncing_ball.mp4   (physics bouncing ball)")
    print(f"  - waveform.mp4        (mesmerizing waveforms)")
    print(f"  - starfield.mp4       (moving starfield)")
    print(f"\n--- How to get 'Bad Apple' video ---")
    print(f"Bad Apple is a famous demo video perfect for LED matrices!")
    print(f"Download it with:")
    print(f"  wget https://archive.org/download/bad-apple-32x32/bad_apple_32x32.mp4")
    print(f"Or search YouTube for 'Bad Apple 32x32' and download with yt-dlp:")
    print(f"  pip3 install --user yt-dlp")
    print(f"  yt-dlp -f 'worstvideo[height<=32]' 'https://www.youtube.com/watch?v=...'")

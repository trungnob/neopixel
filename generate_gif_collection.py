#!/usr/bin/env python3
"""
Generate a collection of fun animated GIFs for 32x32 LED matrix
"""
from PIL import Image, ImageDraw
import math
import random

WIDTH = 32
HEIGHT = 32

def save_gif(frames, filename, duration=50):
    """Save frames as animated GIF"""
    frames[0].save(
        filename,
        save_all=True,
        append_images=frames[1:],
        duration=duration,
        loop=0
    )
    print(f"✓ Generated: {filename} ({len(frames)} frames)")

def create_spinning_earth():
    """Spinning earth with stars"""
    frames = []
    for frame in range(24):
        img = Image.new('RGB', (WIDTH, HEIGHT), color=(0, 0, 20))
        draw = ImageDraw.Draw(img)
        
        # Stars
        random.seed(42)
        for _ in range(30):
            x, y = random.randint(0, WIDTH-1), random.randint(0, HEIGHT-1)
            brightness = random.randint(100, 255)
            draw.point((x, y), fill=(brightness, brightness, brightness))
        
        # Earth
        center_x, center_y = WIDTH // 2, HEIGHT // 2
        radius = 10
        
        # Draw sphere with shading
        for y in range(-radius, radius+1):
            for x in range(-radius, radius+1):
                if x*x + y*y <= radius*radius:
                    # Rotating texture
                    angle = (frame * 15 + x * 10) % 360
                    if 90 < angle < 270:
                        color = (0, 50, 200)  # Ocean
                    else:
                        color = (0, 150, 50)  # Land
                    
                    # Add shading
                    dist = math.sqrt(x*x + y*y) / radius
                    brightness = 1.0 - dist * 0.5
                    color = tuple(int(c * brightness) for c in color)
                    
                    px, py = center_x + x, center_y + y
                    if 0 <= px < WIDTH and 0 <= py < HEIGHT:
                        draw.point((px, py), fill=color)
        
        frames.append(img)
    
    save_gif(frames, "earth_spin.gif", 80)

def create_fire():
    """Animated fire effect"""
    frames = []
    for frame in range(20):
        img = Image.new('RGB', (WIDTH, HEIGHT), color='black')
        draw = ImageDraw.Draw(img)
        
        # Fire from bottom
        for y in range(HEIGHT):
            for x in range(WIDTH):
                # More fire at bottom
                fire_intensity = (HEIGHT - y) / HEIGHT
                fire_intensity += 0.3 * math.sin(x * 0.5 + frame * 0.5)
                fire_intensity = max(0, min(1, fire_intensity))
                
                # Add randomness
                random.seed(x * HEIGHT + y + frame * 100)
                fire_intensity *= random.uniform(0.7, 1.3)
                fire_intensity = max(0, min(1, fire_intensity))
                
                # Color gradient: black -> red -> orange -> yellow -> white
                if fire_intensity < 0.3:
                    r, g, b = 0, 0, 0
                elif fire_intensity < 0.5:
                    t = (fire_intensity - 0.3) / 0.2
                    r = int(255 * t)
                    g, b = 0, 0
                elif fire_intensity < 0.7:
                    r = 255
                    t = (fire_intensity - 0.5) / 0.2
                    g = int(165 * t)
                    b = 0
                elif fire_intensity < 0.9:
                    r, g = 255, 165
                    t = (fire_intensity - 0.7) / 0.2
                    b = int(255 * t)
                else:
                    t = (fire_intensity - 0.9) / 0.1
                    r = g = b = int(165 + 90 * t)
                
                draw.point((x, y), fill=(r, g, b))
        
        frames.append(img)
    
    save_gif(frames, "fire.gif", 60)

def create_pacman():
    """Pac-Man chomping and eating dots"""
    frames = []
    for frame in range(24):
        img = Image.new('RGB', (WIDTH, HEIGHT), color=(0, 0, 50))
        draw = ImageDraw.Draw(img)
        
        # Pac-Man position
        pacman_x = (frame * 2) % (WIDTH + 20) - 10
        pacman_y = HEIGHT // 2
        
        # Mouth opening/closing
        mouth_angle = 30 + 20 * abs(math.sin(frame * math.pi / 6))
        
        # Draw dots ahead of Pac-Man
        for i in range(4):
            dot_x = pacman_x + 15 + i * 8
            if 0 <= dot_x < WIDTH:
                draw.ellipse([dot_x-1, pacman_y-1, dot_x+1, pacman_y+1], fill=(255, 200, 100))
        
        # Draw Pac-Man
        if 0 <= pacman_x < WIDTH:
            bbox = [pacman_x-6, pacman_y-6, pacman_x+6, pacman_y+6]
            draw.pieslice(bbox, start=mouth_angle, end=360-mouth_angle, fill=(255, 255, 0))
        
        frames.append(img)
    
    save_gif(frames, "pacman.gif", 80)

def create_heart_beat():
    """Beating heart"""
    frames = []
    for frame in range(16):
        img = Image.new('RGB', (WIDTH, HEIGHT), color=(20, 0, 10))
        draw = ImageDraw.Draw(img)
        
        # Scale pulse
        scale = 1.0 + 0.3 * math.sin(frame * 2 * math.pi / 16)
        
        center_x, center_y = WIDTH // 2, HEIGHT // 2 + 2
        
        # Heart shape (two circles and a triangle)
        size = int(8 * scale)
        
        # Left circle
        draw.ellipse([center_x - size - 2, center_y - size, 
                     center_x + 2, center_y + size], fill=(255, 0, 100))
        # Right circle
        draw.ellipse([center_x - 2, center_y - size, 
                     center_x + size + 2, center_y + size], fill=(255, 0, 100))
        # Bottom triangle
        points = [
            (center_x - size - 2, center_y),
            (center_x + size + 2, center_y),
            (center_x, center_y + size + 4)
        ]
        draw.polygon(points, fill=(255, 0, 100))
        
        # Glow effect
        glow = int(100 * scale)
        draw.ellipse([center_x - size - 4, center_y - size - 2,
                     center_x + size + 4, center_y + size + 6], 
                     outline=(255, glow, glow))
        
        frames.append(img)
    
    save_gif(frames, "heart_beat.gif", 70)

def create_matrix_rain():
    """Matrix digital rain effect"""
    frames = []
    columns = WIDTH
    drops = [random.randint(0, HEIGHT) for _ in range(columns)]
    
    for frame in range(30):
        img = Image.new('RGB', (WIDTH, HEIGHT), color='black')
        draw = ImageDraw.Draw(img)
        
        for x in range(columns):
            # Update drop position
            drops[x] = (drops[x] + 1) % (HEIGHT + 10)
            
            # Draw trail
            for i in range(10):
                y = drops[x] - i
                if 0 <= y < HEIGHT:
                    brightness = int(255 * (10 - i) / 10)
                    if i == 0:
                        color = (200, 255, 200)  # Bright head
                    else:
                        color = (0, brightness, 0)
                    draw.point((x, y), fill=color)
        
        frames.append(img)
    
    save_gif(frames, "matrix_rain.gif", 50)

def create_rainbow_wave():
    """Rainbow wave animation"""
    frames = []
    for frame in range(30):
        img = Image.new('RGB', (WIDTH, HEIGHT), color='black')
        draw = ImageDraw.Draw(img)
        
        for x in range(WIDTH):
            # Wave equation
            y_offset = int(5 * math.sin(x * 0.3 + frame * 0.3))
            
            for y in range(HEIGHT):
                # Rainbow based on x position and time
                hue = (x * 360 / WIDTH + frame * 12) % 360
                h = hue / 60
                c = 255
                x_val = int(255 * (1 - abs(h % 2 - 1)))
                
                if h < 1:
                    r, g, b = c, x_val, 0
                elif h < 2:
                    r, g, b = x_val, c, 0
                elif h < 3:
                    r, g, b = 0, c, x_val
                elif h < 4:
                    r, g, b = 0, x_val, c
                elif h < 5:
                    r, g, b = x_val, 0, c
                else:
                    r, g, b = c, 0, x_val
                
                # Wave shape
                dist = abs(y - (HEIGHT // 2 + y_offset))
                if dist < 8:
                    brightness = 1.0 - dist / 8
                    color = (int(r * brightness), int(g * brightness), int(b * brightness))
                    draw.point((x, y), fill=color)
        
        frames.append(img)
    
    save_gif(frames, "rainbow_wave.gif", 50)

def create_loading_spinner():
    """Cool loading spinner"""
    frames = []
    for frame in range(24):
        img = Image.new('RGB', (WIDTH, HEIGHT), color=(10, 10, 20))
        draw = ImageDraw.Draw(img)
        
        center_x, center_y = WIDTH // 2, HEIGHT // 2
        
        # Rotating dots
        for i in range(8):
            angle = (frame * 15 + i * 45) % 360
            radius = 10
            x = center_x + int(radius * math.cos(math.radians(angle)))
            y = center_y + int(radius * math.sin(math.radians(angle)))
            
            # Color based on position
            brightness = int(128 + 127 * math.cos(math.radians(angle + frame * 15)))
            color = (0, brightness, 255)
            
            # Draw dot with trail
            for j in range(3):
                size = 3 - j
                trail_x = center_x + int(radius * math.cos(math.radians(angle - j * 15)))
                trail_y = center_y + int(radius * math.sin(math.radians(angle - j * 15)))
                alpha = 1.0 - j * 0.3
                c = tuple(int(v * alpha) for v in color)
                if 0 <= trail_x < WIDTH and 0 <= trail_y < HEIGHT:
                    draw.ellipse([trail_x-size//2, trail_y-size//2, 
                                 trail_x+size//2, trail_y+size//2], fill=c)
        
        frames.append(img)
    
    save_gif(frames, "loading_spinner.gif", 40)

def create_nyan_cat():
    """Nyan Cat style rainbow trail"""
    frames = []
    for frame in range(30):
        img = Image.new('RGB', (WIDTH, HEIGHT), color=(30, 30, 60))
        draw = ImageDraw.Draw(img)
        
        # Cat position
        cat_x = WIDTH // 2 + int(5 * math.sin(frame * 0.3))
        cat_y = HEIGHT // 2 + int(3 * math.cos(frame * 0.2))
        
        # Rainbow trail
        for x in range(cat_x):
            for i in range(6):
                y = cat_y - 3 + i
                if 0 <= y < HEIGHT:
                    scroll_offset = (x + frame * 2) % 6
                    hue = (scroll_offset * 60) % 360
                    h = hue / 60
                    if h < 1:
                        r, g, b = 255, int(255 * h), 0
                    elif h < 2:
                        r, g, b = int(255 * (2-h)), 255, 0
                    elif h < 3:
                        r, g, b = 0, 255, int(255 * (h-2))
                    elif h < 4:
                        r, g, b = 0, int(255 * (4-h)), 255
                    elif h < 5:
                        r, g, b = int(255 * (h-4)), 0, 255
                    else:
                        r, g, b = 255, 0, int(255 * (6-h))
                    draw.point((x, y), fill=(r, g, b))
        
        # Simple cat
        # Body
        draw.rectangle([cat_x-3, cat_y-2, cat_x+3, cat_y+2], fill=(150, 150, 150))
        # Head
        draw.rectangle([cat_x+2, cat_y-3, cat_x+5, cat_y+1], fill=(180, 180, 180))
        # Ears
        draw.point((cat_x+2, cat_y-3), fill=(200, 100, 150))
        draw.point((cat_x+5, cat_y-3), fill=(200, 100, 150))
        # Face
        draw.point((cat_x+3, cat_y-1), fill=(0, 0, 0))
        draw.point((cat_x+4, cat_y-1), fill=(0, 0, 0))
        
        frames.append(img)
    
    save_gif(frames, "nyan_cat.gif", 60)

if __name__ == "__main__":
    print("Generating GIF collection for 32x32 LED matrix...\n")
    
    create_spinning_earth()
    create_fire()
    create_pacman()
    create_heart_beat()
    create_matrix_rain()
    create_rainbow_wave()
    create_loading_spinner()
    create_nyan_cat()
    
    print(f"\n✨ Generated 8 animated GIFs!")
    print(f"\nTo stream any of them:")
    print(f"  python3 streamer.py <filename>.gif --brightness 0.3")
    print(f"\nAvailable GIFs:")
    print(f"  - earth_spin.gif      (spinning earth with stars)")
    print(f"  - fire.gif            (realistic fire effect)")
    print(f"  - pacman.gif          (Pac-Man chomping)")
    print(f"  - heart_beat.gif      (beating heart)")
    print(f"  - matrix_rain.gif     (Matrix digital rain)")
    print(f"  - rainbow_wave.gif    (colorful wave)")
    print(f"  - loading_spinner.gif (rotating loader)")
    print(f"  - nyan_cat.gif        (Nyan Cat with rainbow)")

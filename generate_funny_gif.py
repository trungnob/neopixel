#!/usr/bin/env python3
"""
Generate a funny animated GIF for 32x32 LED matrix
"""
from PIL import Image, ImageDraw, ImageFont
import math

# Canvas size
WIDTH = 32
HEIGHT = 32
FRAMES = 20  # Number of frames for animation

def create_bouncing_face():
    """Create a funny bouncing face animation"""
    frames = []
    
    for frame in range(FRAMES):
        # Create new frame
        img = Image.new('RGB', (WIDTH, HEIGHT), color='black')
        draw = ImageDraw.Draw(img)
        
        # Rainbow background that shifts
        hue_shift = (frame * 18) % 360  # Shift hue each frame
        for y in range(HEIGHT):
            hue = (hue_shift + y * 360 / HEIGHT) % 360
            # Convert HSV to RGB (simplified)
            h = hue / 60
            x = 255 * (1 - abs(h % 2 - 1))
            if h < 1:
                r, g, b = 255, int(x), 0
            elif h < 2:
                r, g, b = int(x), 255, 0
            elif h < 3:
                r, g, b = 0, 255, int(x)
            elif h < 4:
                r, g, b = 0, int(x), 255
            elif h < 5:
                r, g, b = int(x), 0, 255
            else:
                r, g, b = 255, 0, int(x)
            
            draw.line([(0, y), (WIDTH-1, y)], fill=(r, g, b))
        
        # Bouncing animation with sine wave
        bounce = int(5 * math.sin(frame * 2 * math.pi / FRAMES))
        center_y = HEIGHT // 2 + bounce
        center_x = WIDTH // 2
        
        # Wobble side to side
        wobble = int(3 * math.sin(frame * 4 * math.pi / FRAMES))
        center_x += wobble
        
        # Draw face (big yellow circle)
        face_radius = 10
        face_bbox = [
            center_x - face_radius, center_y - face_radius,
            center_x + face_radius, center_y + face_radius
        ]
        draw.ellipse(face_bbox, fill=(255, 220, 0), outline=(255, 180, 0), width=1)
        
        # Draw cool sunglasses
        glass_y = center_y - 2
        # Left glass
        draw.rectangle([center_x - 7, glass_y - 3, center_x - 2, glass_y + 1], fill=(50, 50, 50))
        # Right glass
        draw.rectangle([center_x + 2, glass_y - 3, center_x + 7, glass_y + 1], fill=(50, 50, 50))
        # Bridge
        draw.rectangle([center_x - 2, glass_y - 2, center_x + 2, glass_y - 1], fill=(30, 30, 30))
        
        # Draw big smile (changes size with bounce)
        smile_size = 6 + abs(bounce) // 2
        smile_bbox = [
            center_x - smile_size, center_y,
            center_x + smile_size, center_y + smile_size
        ]
        draw.arc(smile_bbox, start=0, end=180, fill=(0, 0, 0), width=2)
        
        # Add some sparkles that move around
        for i in range(3):
            sparkle_angle = (frame * 30 + i * 120) % 360
            sparkle_rad = 15
            sx = center_x + int(sparkle_rad * math.cos(math.radians(sparkle_angle)))
            sy = center_y + int(sparkle_rad * math.sin(math.radians(sparkle_angle)))
            
            # Draw sparkle as a small cross
            if 0 <= sx < WIDTH and 0 <= sy < HEIGHT:
                sparkle_brightness = int(128 + 127 * math.sin(frame * 2 * math.pi / FRAMES))
                color = (255, 255, sparkle_brightness)
                draw.point((sx, sy), fill=color)
                if sx > 0:
                    draw.point((sx-1, sy), fill=color)
                if sx < WIDTH-1:
                    draw.point((sx+1, sy), fill=color)
                if sy > 0:
                    draw.point((sx, sy-1), fill=color)
                if sy < HEIGHT-1:
                    draw.point((sx, sy+1), fill=color)
        
        frames.append(img)
    
    return frames

if __name__ == "__main__":
    print("Generating funny bouncing face animation...")
    frames = create_bouncing_face()
    
    # Save as animated GIF
    output_path = "funny_dance.gif"
    frames[0].save(
        output_path,
        save_all=True,
        append_images=frames[1:],
        duration=50,  # 50ms per frame = 20 FPS
        loop=0  # Loop forever
    )
    
    print(f"✓ Saved animated GIF to: {output_path}")
    print(f"  Resolution: {WIDTH}x{HEIGHT}")
    print(f"  Frames: {len(frames)}")
    print(f"  Duration: {50}ms per frame")
    print(f"\nTo stream it, run:")
    print(f"  python3 streamer.py {output_path} --brightness 0.3")

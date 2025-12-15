from PIL import Image, ImageDraw, ImageFont
import random

width = 32
height = 32
frames = []

# Matrix Rain Effect
columns = [0] * width
drops = [0] * width

# Generate 60 frames (approx 4 seconds at 15fps)
for f in range(60):
    img = Image.new('RGB', (width, height), (0, 0, 0))
    pixels = img.load()
    
    # Update drops
    for x in range(width):
        if drops[x] > 0:
            # Draw the trail
            head_y = drops[x]
            
            # Draw head (bright white/green)
            if head_y < height:
                pixels[x, head_y] = (200, 255, 200)
            
            # Draw trail (fading green)
            for i in range(1, 8):
                y = head_y - i
                if 0 <= y < height:
                    brightness = 255 - (i * 30)
                    if brightness < 0: brightness = 0
                    pixels[x, y] = (0, brightness, 0)
            
            # Move drop down
            drops[x] += 1
            
            # Reset if off screen
            if drops[x] > height + 10:
                drops[x] = 0
                columns[x] = 0 # Ready for new drop
        
        else:
            # Randomly start a new drop
            if random.random() < 0.05:
                drops[x] = 1

    frames.append(img)

# Save as GIF
# Duration 100ms = 10fps
frames[0].save('web/matrix_slow.gif', save_all=True, append_images=frames[1:], optimize=False, duration=100, loop=0)
print("Generated web/matrix_slow.gif")

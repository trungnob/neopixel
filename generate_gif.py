from PIL import Image, ImageDraw
import math

width = 32
height = 32
frames = []

# Create a bouncing ball animation
# 30 frames
for i in range(30):
    img = Image.new('RGB', (width, height), (0, 0, 0))
    draw = ImageDraw.Draw(img)
    
    # Calculate position
    # y moves up and down
    y = int(16 + 12 * math.sin(i * 0.2))
    # x moves constantly
    x = i % 32
    
    # Draw ball
    draw.ellipse((x-2, y-2, x+2, y+2), fill=(255, 0, 0))
    
    # Draw a scanner bar at the top
    bar_x = int(16 + 14 * math.sin(i * 0.5))
    draw.rectangle((bar_x-2, 0, bar_x+2, 2), fill=(0, 0, 255))
    
    frames.append(img)

# Save as GIF
# Duration 33ms = ~30fps
frames[0].save('web/test_anim.gif', save_all=True, append_images=frames[1:], optimize=False, duration=33, loop=0)
print("Generated web/test_anim.gif")

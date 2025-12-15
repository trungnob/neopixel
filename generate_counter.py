from PIL import Image, ImageDraw, ImageFont
import math

width = 32
height = 32
frames = []

# Create a counter animation
# 0 to 99
for i in range(100):
    img = Image.new('RGB', (width, height), (0, 0, 0))
    draw = ImageDraw.Draw(img)
    
    # Draw number
    # Using default font which might be small, but let's try
    # To make it visible, we might need a better font or just draw simple shapes
    # For now, let's just draw the text centered
    text = str(i)
    
    # Simple centering logic (approximate for default font)
    # Default font is usually ~6x10 pixels
    text_width = len(text) * 6
    x = (width - text_width) // 2
    y = (height - 10) // 2
    
    draw.text((x, y), text, fill=(0, 255, 0))
    
    # Add a progress bar at bottom
    draw.rectangle((0, 30, int(width * (i/100)), 31), fill=(255, 0, 0))
    
    frames.append(img)

# Save as GIF
# Duration 66ms = ~15fps
frames[0].save('web/counter_15fps.gif', save_all=True, append_images=frames[1:], optimize=False, duration=66, loop=0)
print("Generated web/counter_15fps.gif")

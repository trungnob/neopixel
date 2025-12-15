#!/usr/bin/env python3
"""
Test pattern to visualize XY mapping and find gaps
"""
from PIL import Image, ImageDraw

GRID_WIDTH = 32
GRID_HEIGHT = 32
PANELS_WIDE = 1

def XY(x, y):
    """Same XY function as in live_visualizer.py"""
    if x < 0 or x >= GRID_WIDTH or y < 0 or y >= GRID_HEIGHT:
        return -1
    
    panelCol = x // 32
    panelRow = y // 8
    localX = x % 32
    localY = y % 8
    
    if panelRow % 2 == 1:
        localX = 31 - localX
        localY = 7 - localY
    
    if panelRow % 2 == 0:
        panelIndex = (panelRow * PANELS_WIDE) + panelCol
    else:
        panelIndex = (panelRow * PANELS_WIDE) + (PANELS_WIDE - 1 - panelCol)
    
    panelOffset = panelIndex * 256
    
    if localX % 2 == 0:
        return panelOffset + (localX * 8 + localY)
    else:
        return panelOffset + (localX * 8 + (7 - localY))

def test_xy_mapping():
    """Create a visual representation of XML mapping"""
    
    # Create image showing LED index for each (x,y)
    img = Image.new('RGB', (GRID_WIDTH * 10, GRID_HEIGHT * 10), 'white')
    draw = ImageDraw.Draw(img)
    
    print("XY Mapping Test")
    print("=" * 60)
    print(f"Grid: {GRID_WIDTH}x{GRID_HEIGHT}")
    print(f"Total LEDs: {GRID_WIDTH * GRID_HEIGHT}")
    print()
    
    # Check which LED indices are used
    used_indices = set()
    xy_to_led = {}
    
    for y in range(GRID_HEIGHT):
        for x in range(GRID_WIDTH):
            led_idx = XY(x, y)
            xy_to_led[(x, y)] = led_idx
            if led_idx >= 0:
                used_indices.add(led_idx)
    
    # Check for gaps
    max_idx = max(used_indices) if used_indices else 0
    expected = set(range(max_idx + 1))
    missing = expected - used_indices
    
    print(f"Max LED index: {max_idx}")
    print(f"Unique indices used: {len(used_indices)}")
    
    if missing:
        print(f"\n⚠️ MISSING indices: {sorted(missing)[:20]}...")
    else:
        print(f"\n✓ No gaps - all indices 0-{max_idx} are used")
    
    # Visualize the spectrum analyzer pattern
    print("\n" + "=" * 60)
    print("Simulating spectrum analyzer bars...")
    print("=" * 60)
    
    # Simulate 32 frequency bins with varying heights
    test_heights = [24, 28, 20, 16, 30, 25, 22, 18, 
                    26, 31, 27, 23, 19, 15, 21, 29,
                    24, 20, 28, 16, 25, 22, 30, 18,
                    26, 21, 27, 23, 19, 31, 17, 29]
    
    # Create visualization
    vis = Image.new('RGB', (GRID_WIDTH, GRID_HEIGHT), 'black')
    
    for x in range(GRID_WIDTH):
        height = test_heights[x]
        for y in range(GRID_HEIGHT):
            actual_y = GRID_HEIGHT - 1 - y  # Flip for bottom-up
            led_idx = XY(x, actual_y)
            
            if y < height:
                # Rainbow color
                hue_pos = x / GRID_WIDTH
                if hue_pos < 0.33:
                    r, g, b = 255, int(hue_pos * 3 * 255), 0
                elif hue_pos < 0.66:
                    r, g, b = int((0.66 - hue_pos) * 3 * 255), 255, 0
                else:
                    r, g, b = 0, 255, int((hue_pos - 0.66) * 3 * 255)
                
                if led_idx >= 0:
                    vis.putpixel((x, GRID_HEIGHT - 1 - y), (r, g, b))
    
    # Scale up for visibility
    vis_large = vis.resize((GRID_WIDTH * 10, GRID_HEIGHT * 10), Image.Resampling.NEAREST)
    vis_large.save('spectrum_test.png')
    print(f"\n✓ Saved spectrum_test.png")
    
    # Also create a grid showing panel boundaries
    panel_vis = Image.new('RGB', (GRID_WIDTH * 10, GRID_HEIGHT * 10), 'black')
    draw = ImageDraw.Draw(panel_vis)
    
    for y in range(GRID_HEIGHT):
        for x in range(GRID_WIDTH):
            led_idx = XY(x, y)
            panelRow = y // 8
            
            # Color by panel
            colors = [(255, 100, 100), (100, 255, 100), (100, 100, 255), (255, 255, 100)]
            color = colors[panelRow % 4]
            
            px = x * 10
            py = y * 10
            draw.rectangle([px+1, py+1, px+8, py+8], fill=color)
    
    # Draw panel boundaries
    for i in range(5):
        y_pos = i * 8 * 10
        draw.line([(0, y_pos), (GRID_WIDTH * 10, y_pos)], fill='white', width=2)
    
    panel_vis.save('panel_layout.png')
    print(f"✓ Saved panel_layout.png")
    
    # Print the actual LED buffer layout for first few columns
    print("\n" + "=" * 60)
    print("LED indices for first 4 columns (spectrum bars):")
    print("=" * 60)
    for x in range(4):
        print(f"\nColumn {x} (frequency bin {x}):")
        indices = []
        for y in range(GRID_HEIGHT):
            actual_y = GRID_HEIGHT - 1 - y
            led_idx = XY(x, actual_y)
            indices.append(led_idx)
        print(f"  LEDs (bottom to top): {indices}")

if __name__ == "__main__":
    test_xy_mapping()

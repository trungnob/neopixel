import socket
import time
import sys
import argparse
from PIL import Image, ImageSequence

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
    # x is 0-31, y is 0-31
    # Panels are 8x32 (8 rows, 32 cols) ?? No, wait.
    # The C++ code says:
    # int panelCol = x / 32;
    # int panelRow = y / 8;
    # This implies panels are 32 wide and 8 high.
    
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
    # panelsWide is 1.
    # panelRow 0 -> index 0
    # panelRow 1 -> index 0 (if width 1, serpentine doesn't matter much for index, but does for rotation)
    
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

def stream_image(image_path, speed=0.1, brightness=1.0):
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    
    try:
        img = Image.open(image_path)
    except Exception as e:
        print(f"Error opening image: {e}")
        return

    print(f"Streaming {image_path} to {ESP_IP}:{UDP_PORT} with brightness {brightness}...")
    
    try:
        while True:
            for frame in ImageSequence.Iterator(img):
                # Resize to grid
                frame = frame.convert("RGB")
                frame = frame.resize((GRID_WIDTH, GRID_HEIGHT), Image.Resampling.LANCZOS)
                
                # Prepare buffer
                # Initialize with zeros (black)
                pixels = [0] * (MAX_LEDS * 3)
                
                width, height = frame.size
                for y in range(height):
                    for x in range(width):
                        r, g, b = frame.getpixel((x, y))
                        
                        # Apply brightness
                        r = int(r * brightness)
                        g = int(g * brightness)
                        b = int(b * brightness)
                        
                        led_idx = XY(x, y)
                        if led_idx != -1 and led_idx < MAX_LEDS:
                            # Try RGB Order
                            pixels[led_idx * 3] = r
                            pixels[led_idx * 3 + 1] = g
                            pixels[led_idx * 3 + 2] = b
                
                # Send packet
                data = bytes(pixels)
                sock.sendto(data, (ESP_IP, UDP_PORT))
                
                # Delay for animation
                if getattr(img, 'is_animated', False):
                    delay = frame.info.get('duration', 100) / 1000.0
                    if delay < 0.02: delay = 0.02 # Min delay
                    time.sleep(delay)
                else:
                    time.sleep(speed) # Static image refresh rate
                    
    except KeyboardInterrupt:
        print("\nStopping stream.")
    finally:
        sock.close()

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Stream images/GIFs to ESP8266 LED Matrix")
    parser.add_argument("file", help="Path to image or GIF file")
    parser.add_argument("--ip", default=ESP_IP, help="ESP8266 IP Address")
    parser.add_argument("--speed", type=float, default=0.1, help="Refresh rate for static images")
    parser.add_argument("--brightness", type=float, default=1.0, help="Brightness multiplier (0.0 - 1.0)")
    
    args = parser.parse_args()
    
    ESP_IP = args.ip
    stream_image(args.file, args.speed, args.brightness)

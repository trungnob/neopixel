# LED Pattern Designer

A cross-platform web application for designing custom LED patterns for your 9×144 LED grid.

## Features

- 🎨 **Touch/Mouse Drawing** - Works on desktop and mobile
- 🌈 **Color Picker** - Choose any color
- 🖌️ **Drawing Tools** - Draw, Erase, Fill All, Clear
- 📱 **Responsive** - Auto-scales to fit any screen
- 🔄 **Live Upload** - Send patterns directly to ESP8266
- ⚡ **Efficient** - Sparse data format (only sends non-black pixels)

## Quick Start

### 1. Start the Server

On your computer, run:

```bash
cd /home/trungnob/neopixel/web
./start-with-logging.sh
```

This will show:
```
Desktop: http://localhost:8080/pattern-designer.html
Phone:   http://192.168.1.XXX:8080/pattern-designer.html
```

### 2. Open on Your Phone

1. Make sure your phone is on the **same WiFi network**
2. Open browser on your phone
3. Navigate to: `http://<your-computer-ip>:8080/pattern-designer.html`

### 3. Design Your Pattern

1. **Select a Color** - Tap the color picker
2. **Choose Tool**:
   - ✏️ **Draw** - Paint individual LEDs
   - 🧹 **Erase** - Remove LEDs
   - 🎨 **Fill All** - Fill entire grid with current color
   - 🗑️ **Clear** - Reset to black
3. **Draw** - Touch/drag to paint the grid

### 4. Upload to ESP8266

1. Set **ESP8266 IP** (default: 192.168.1.129)
2. Adjust **Scroll Speed** if needed (20-200ms)
3. Click **🚀 Upload to ESP8266**
4. Pattern displays on your LED strips!

## Grid Layout

The canvas shows a **144×9 grid** matching your physical LED strips:
- **144 columns** = 144 LEDs per strip (1 meter)
- **9 rows** = 9 strips (45cm total height)
- **Zigzag wiring** = automatically handled

## Technical Details

### Architecture

```
Phone Browser (Drawing)
    ↓
Pattern Designer (Computer:8080)
    ↓ HTTP POST
ESP8266 (192.168.1.129:80)
    ↓
LED Strips
```

### Pattern Data

- Each pattern = 1296 LEDs × 3 bytes (RGB) = ~3.9KB
- Sent as JSON array: `[[r,g,b], [r,g,b], ...]`
- ESP8266 stores in RAM and displays immediately
- Pattern mode: **122** (Custom Pattern)

### Files

- `pattern-designer.html` - Web app (standalone, no dependencies)
- `start-designer.sh` - Server startup script
- `README.md` - This file

## Tips

- **Adjust Cell Size** - Use the slider if grid is too small/large
- **Landscape Mode** - Rotate phone for better view of 144-wide grid
- **Save Patterns** - Take screenshots to save your designs
- **Color Codes** - Use specific hex codes for precise colors

## Troubleshooting

**Can't connect from phone?**
- Check both devices on same WiFi
- Check firewall not blocking port 8080
- Try: `http://$(hostname -I | awk '{print $1}'):8080/pattern-designer.html`

**Upload fails?**
- Check ESP8266 IP address is correct
- Make sure ESP8266 is powered on and connected
- Check ESP8266 serial output for errors

**Pattern looks wrong?**
- Grid matches physical layout with zigzag wiring
- Colors may appear different on LEDs vs screen

## Future Ideas

- Save/load patterns to file
- Animation frame sequencing
- Scrolling pattern mode
- Pattern library
- Brightness control
- Mirror/rotate tools

## Development

To modify the designer:
1. Edit `pattern-designer.html`
2. Refresh browser (no build needed)
3. Test drawing and upload
4. All in one HTML file - no dependencies!

Enjoy creating patterns! 🎉

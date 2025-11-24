# LED Strip Panel - Magnetic Modular System

3D-printed TV-style LED panel with magnetic click-together modules for 9×144 WS2812B LED grid.

## 📐 Design Concept

**TV Panel Layout:**
- LED strips lay **FLAT** on front face (all LEDs face forward like TV pixels)
- 9 horizontal rows × 144 LEDs per row = 1,296 total LEDs
- 50mm vertical spacing between rows (like pixel pitch)
- Magnetic modular assembly - no glue, no screws between modules

**Final Dimensions:**
- Width: 1000mm (1 meter)
- Height: 450mm (9 rows × 50mm spacing)
- Depth: 20mm

## 🧩 Module System

**Each module:**
- Size: 250mm (W) × 50mm (H) × 20mm (D)
- Holds 36 LEDs (144 LEDs / 4 modules)
- Shallow channel on front face for LED strip
- 4 embedded neodymium magnets for connections

**Three module types:**
- **Left** (9× needed): Has wall mounting hole on left side
- **Middle** (18× needed): Connects modules horizontally
- **Right** (9× needed): Has wall mounting hole on right side

**Total: 36 modules**

## 🔧 Files

| File | Description |
|------|-------------|
| `led_strip_holder.scad` | OpenSCAD source (fully parametric) |
| `module_left.stl` | Left end module - print 9× |
| `module_middle.stl` | Center connector - print 18× |
| `module_right.stl` | Right end module - print 9× |

## 📦 Materials Needed

**3D Printing:**
- ~600-800g filament (PLA or PETG)
- Recommend: Black for best LED contrast

**Magnets (REQUIRED):**
- **144× Neodymium magnets**
- Size: **10mm diameter × 3mm thick**
- Grade: **N52** (strongest common grade)
- Amazon search: **"neodymium magnets 10mm x 3mm N52"**
- Cost: ~$15-20 per 100 (buy 2 packs)

**LED Strips:**
- 9× WS2812B strips, 144 LEDs each, ~1000mm long
- Width: 10-12mm

**Optional:**
- 36× M3 screws for wall mounting
- Frosted acrylic sheets for diffusion (1000×50×2mm per row)

## 🖨️ Printing Instructions

**Print Settings:**
```
Layer Height:    0.2mm
Infill:          15-20%
Supports:        None needed
Orientation:     As exported (front face down)
Material:        PLA, PETG, or ABS
Nozzle:          0.4mm
```

**Print Queue:**
```
Test print first (verify fit):
  1× module_left.stl
  1× module_middle.stl
  1× module_right.stl

After successful test:
  8× more module_left.stl
  17× more module_middle.stl
  8× more module_right.stl
```

**Estimated Time:**
- Per module: ~1.5-2 hours
- Total: 54-72 hours
- Tip: Print in batches overnight!

## 🔨 Assembly Instructions

### Step 1: Install Magnets

**CRITICAL: Get polarity correct!**

1. **Mark a reference magnet:**
   - Take one magnet
   - Mark one face with permanent marker
   - This is your "north" reference

2. **Magnet positions per module:**
   - LEFT/RIGHT sides: 2 magnets each (top + bottom positions)
   - TOP/BOTTOM surfaces: 2 magnets each (¼ and ¾ positions)
   - Total: 4 magnets per module

3. **Polarity rules:**
   - All RIGHT-side magnets: Same orientation (e.g., marked side OUT)
   - All LEFT-side magnets: Opposite orientation (marked side IN)
   - All TOP magnets: Same orientation (e.g., marked side UP)
   - All BOTTOM magnets: Opposite orientation (marked side DOWN)

4. **Installation:**
   - Magnets press-fit into 10.2mm holes
   - Should be snug - if loose, add tiny dab of super glue
   - **Test before gluing all!** Connect two modules to verify attraction

### Step 2: Build Horizontal Rows

For each of the 9 rows:

1. Take 4 modules: 1× left + 2× middle + 1× right
2. Bring modules close - magnets will **SNAP** together horizontally
3. Result: 1-meter long row holder
4. Repeat for all 9 rows

### Step 3: Install LED Strips

For each row:

1. Lay LED strip in the shallow front channel
2. Strip should sit flush with front surface
3. Route wiring through back channel
4. Use zigzag pattern:
   - Row 0: Left → Right (data out on right)
   - Row 1: Right → Left (data out on left)
   - Row 2: Left → Right
   - Continue alternating...

### Step 4: Stack Rows Vertically

1. Start with bottom row (Row 0)
2. Place Row 1 on top - magnets will attract and align
3. Continue stacking all 9 rows
4. Connect power/data between rows on the back

### Step 5: Mounting

**Wall Mount (Recommended):**
- Use M3 screws through mounting holes on left/right modules
- Each row has 2 mounting points (18 total for full panel)
- Mount into studs or use appropriate wall anchors

**Desktop/Table:**
- Lay horizontally on flat surface
- Optional: Add rubber feet or adhesive strips

## 🎨 Features

**Magnetic Connections:**
- Tool-free assembly - just snap together
- Easy disassembly for maintenance or reconfiguration
- Strong hold (10mm N52 magnets)
- Self-aligning design

**LED Management:**
- Shallow front channel holds LED strip flat
- Back channel for wire routing
- Organized zigzag wiring path
- Easy LED strip replacement

**Lightweight Design:**
- Hollow center cavity reduces weight
- ~20-25g per module (depends on infill)
- Total weight: ~900g for 36 modules

## 📷 Viewing in OpenSCAD

```bash
openscad led_strip_holder.scad
```

**Controls:**
- Press **F5** for fast preview
- Press **F6** for full render
- Mouse drag to rotate
- Shift+drag to pan
- Scroll to zoom

**View options** (edit lines 37-39):
```scad
module_type = "middle";      // "left", "middle", or "right"
show_assembly = true;        // true = full panel, false = single module
show_led_strips = true;      // true = show LEDs, false = holders only
```

## 🔧 Customization

Edit `led_strip_holder.scad` parameters:

```scad
// Change module size
strip_length = 250;        // Try 200mm for 5 modules instead of 4

// Adjust spacing
module_height = 50;        // Vertical spacing between LED rows

// LED channel fit
led_channel_depth = 5;     // Increase if strip sits too high
led_channel_width = 13;    // Adjust for your strip width

// Magnet size (if using different magnets)
magnet_dia = 10.2;         // Hole diameter
magnet_thickness = 3.2;    // Magnet depth
```

After changes:
1. Save file
2. Re-export STLs: `openscad -D "module_type=\"middle\"" -D "show_assembly=false" -o module_middle.stl led_strip_holder.scad`

## ✅ Pre-Print Checklist

1. **Print test modules** (1 of each type)
2. **Install magnets** in test modules
3. **Test connections:**
   - Do modules snap together horizontally?
   - Do modules stack vertically?
   - Is magnetic force sufficient?
4. **Test LED fit:**
   - Does your LED strip fit in the channel?
   - Does it lay flat on front surface?
   - Can you route wires through back?
5. **Adjust if needed:**
   - Too tight/loose: Adjust `led_channel_width`
   - Weak magnets: Try stronger N52 grade or larger size
   - Doesn't snap: Check magnet polarity!

## 🐛 Troubleshooting

**Modules repel instead of attract:**
- Magnet polarity is reversed
- Remove magnet (carefully!) and flip orientation
- Verify all magnets follow the polarity rules

**Magnets fall out:**
- Holes may be too large (printer calibration)
- Add small dab of super glue
- Or print at slightly higher extrusion multiplier

**LED strip doesn't fit:**
- Measure your actual strip width
- Adjust `led_channel_width` parameter
- Re-export and test

**Warping during print:**
- Increase bed temperature
- Use brim (5-10mm)
- Ensure bed is level

**Weak magnetic hold:**
- Verify magnets are N52 grade (strongest)
- Consider 12mm × 3mm magnets for more force
- Check that magnets are fully seated in holes

## 💡 Tips

- **Label modules** during assembly (Row 1-9, Position 1-4)
- **Print spares** (2-3 extra middle modules)
- **Test power supply** before full assembly (9 strips = ~70A peak!)
- **Use frosted acrylic** diffuser for professional look
- **Take photos** during assembly for reference

## 📝 Version History

- **v2.0** (2025-11-23): Complete redesign - flat LED mounting, TV panel style
- v1.0 (2025-11-23): Initial vertical holder design (deprecated)

## 📄 License

Open source - modify and share freely!

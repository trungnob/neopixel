# Single LED Strip Module - Minimal Design

Compact, standalone module for WS2812B LED strips. Perfect for flexible layouts you can move around!

## 📐 Specifications

**Minimal Dimensions:**
- **250mm (W) × 50mm (H) × 15mm (D)** ← Reduced depth!
- Wall thickness: 1.5mm (thinner, lighter)
- Weight: ~15-18g per module
- 5cm vertical spacing between strips

**What's Different:**
- ✅ Slimmer: 15mm deep (was 20mm)
- ✅ Lighter: 1.5mm walls (was 2mm)
- ✅ Compact: Minimal material, just enough for LED strip
- ✅ Flexible: Easy to rearrange modules
- ✅ Same magnets: 4× 10mm × 3mm per module

## 🔧 Files

- `single_module.scad` - Minimal module design (open & edit this)
- `single_module.stl` - Ready to print

## 🎨 Customization

Edit `single_module.scad` at the top:

```scad
// Want different length? Change this:
strip_segment_length = 250;   // Try: 200, 300, 333, 500

// Want tighter/wider spacing? Change this:
module_height = 50;           // Try: 30, 40, 60

// Want even slimmer? Change this:
module_depth = 15;            // Try: 12, 18, 20

// Your LED strip wider? Change this:
led_channel_width = 13;       // Measure your strip + add 1mm
```

### Examples:

**Short modules (5 per row):**
```scad
strip_segment_length = 200;  // 5× 200mm = 1000mm
```

**Triple-length modules (3 per row):**
```scad
strip_segment_length = 333;  // 3× 333mm ≈ 1000mm
```

**Tighter spacing (smaller panel):**
```scad
module_height = 30;          // 9 rows × 30mm = 270mm tall
```

**Even slimmer (ultra-minimal):**
```scad
module_depth = 12;           // Just 12mm deep!
wall_thickness = 1.2;        // Thinner walls
```

## 🖨️ Print Settings

```
Layer Height:    0.2mm
Infill:          15%
Supports:        None
Material:        PLA or PETG
Time:            ~1-1.5 hours
```

## 📦 What You Need

**Per module:**
- 4× neodymium magnets (10mm dia × 3mm thick, N52 grade)
- Amazon: "neodymium magnets 10mm x 3mm N52"

**For full 9×4 grid (36 modules):**
- 144 magnets total
- ~600g filament

## 🔨 Assembly

1. **Print modules** (as many as you need)
2. **Install magnets** with correct polarity:
   - Right side: Both same orientation
   - Left side: Opposite orientation
   - Top: Both same orientation
   - Bottom: Opposite orientation
3. **Snap together** horizontally (side-by-side)
4. **Stack vertically** (rows on top of each other)
5. **Insert LED strips** in front channels
6. **Rearrange anytime!** Magnets let you reconfigure

## 💡 Use Cases

**Flexible Grid:**
- Print 36 modules, arrange in 9×4 grid
- Move modules around to create patterns
- Easy to add/remove sections

**Different Patterns:**
- Staggered layout
- Custom spacing
- Non-uniform arrangements
- Add modules later as needed

**Testing:**
- Print 1 module to test
- Verify LED strip fit
- Test magnet strength
- Then print more

## 🎯 Tips

- **Print a test first** - verify fit before mass production
- **Label modules** if making different sizes
- **Use same filament** for consistent dimensions
- **Store extras** - modules are cheap, print spares
- **Mix lengths** - combine 200mm + 250mm modules

## ⚡ Export STL

```bash
# Export current design
openscad -o my_module.stl single_module.scad

# Export custom size via command line
openscad -D "strip_segment_length=200" -o module_200mm.stl single_module.scad
```

## 📝 Quick Reference

| Module Length | LEDs per Module | Modules per Row | Total Modules (9 rows) |
|--------------|----------------|----------------|----------------------|
| 200mm | 29 | 5 | 45 |
| 250mm | 36 | 4 | 36 |
| 333mm | 48 | 3 | 27 |
| 500mm | 72 | 2 | 18 |

Choose based on your printer size and desired flexibility!

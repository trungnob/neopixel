# 8×32 LED Panel Housing

3D printable housing for flexible 8×32 LED matrix panels (320mm × 80mm).

![Panel Reference](/home/trungnob/.gemini/antigravity/brain/7824e115-abd6-4747-9da1-586bbd83acb8/uploaded_image_1764638009166.png)

## 📐 Panel Specifications

- **Size**: 8 rows × 32 columns (256 LEDs total)
- **Dimensions**: 320mm × 80mm (~12.6" × 3.15")
- **Thickness**: ~1.5mm flexible PCB
- **Connectors**: Multiple JST connectors along panel

## 🎯 Housing Features

- **Thin profile**: Only 8mm deep
- **Front LED channel**: 2mm shallow groove holds panel flat
- **Wire routing**: Back channels and exit holes for connectors
- **Mounting**: 4× M3 holes at corners for hanging/attaching
- **Weight reduction**: Hollow cavity reduces material use

## 📁 Files

### Full Housing (requires large printer)
- `8x32_panel_housing.scad` - Single piece design (324mm long)
- Requires printer with ≥ 324mm × 84mm build area

### Split Housing (for smaller printers)
- `8x32_housing_split.scad` - Two-piece design (162mm each)
- For printers with < 324mm beds
- Includes alignment pins for easy assembly

## 🖨️ Print Settings

```
Layer Height:    0.2mm
Infill:          20%
Supports:        None
Orientation:     Face down (wire channels facing up)
Material:        PLA or PETG
```

**Full housing**: ~3-4 hours  
**Split housing**: ~2 hours per half

## 🔨 Assembly

### Option 1: Full Housing
1. Print `8x32_panel_housing.scad`
2. Insert LED panel from front
3. Route wires through back holes
4. Mount using corner M3 holes

### Option 2: Split Housing
1. Print both halves from `8x32_housing_split.scad`
2. Insert LED panel into left half
3. Align right half using pins
4. Join with CA glue or epoxy
5. Optional: Add screw through split for strength
6. Route wires and mount

## ⚙️ Customization

Edit the SCAD files to adjust:

```scad
// Panel fit
panel_length = 320;          // Your panel length
panel_width = 80;            // Your panel width

// Wire routing
wire_hole_diameter = 6;      // Connector size
wire_hole_positions = [...]; // Add/move holes

// Housing depth
housing_depth = 8;           // Make deeper if needed

// Mounting holes
mounting_hole_dia = 4;       // M3 = 4mm, M4 = 5mm
```

## 💡 Tips

**If wires don't fit through holes:**
- Increase `wire_hole_diameter` to 8mm or 10mm
- Add more holes at connector positions

**If panel is loose:**
- Add thin foam tape or silicone dots
- Increase `led_channel_depth` slightly

**For wall mounting:**
- Print with 100% infill near mounting holes
- Use M3 × 10mm screws into wall anchors

**For desk stand:**
- Add triangular supports to back
- Use rubber feet to prevent sliding

## 📦 Materials Needed

- PLA or PETG filament (~50g)
- LED panel (8×32, 320mm × 80mm)
- Optional: 4× M3 screws for mounting
- Optional: CA glue (for split version)
- Optional: Foam tape for panel security

## 🎨 Variants You Can Make

1. **Deeper housing** - increase `housing_depth` for electronics
2. **Diffuser mount** - add front lip for acrylic diffuser
3. **Stackable** - add alignment features for multiple panels
4. **Angled mount** - modify for tilted wall display

## 🚀 Quick Start

```bash
# Export full housing
cd /home/trungnob/neopixel/mechanical
openscad -o 8x32_housing.stl 8x32_panel_housing.scad

# Or export split (left half)
openscad -D "is_left_half=true" -o 8x32_left.stl 8x32_housing_split.scad

# Export split (right half)  
openscad -D "is_left_half=false" -o 8x32_right.stl 8x32_housing_split.scad
```

## 📏 Dimensions Reference

| Component | Measurement |
|-----------|-------------|
| Panel Length | 320mm (12.59in) |
| Panel Width | 80mm (3.15in) |
| Panel Thickness | ~1.5mm |
| Housing Length | 324mm |
| Housing Width | 84mm |
| Housing Depth | 8mm |
| Weight (printed) | ~40-50g |

Perfect for LED matrix displays, digital signs, or decorative lighting!

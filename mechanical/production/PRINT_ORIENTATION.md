# Print Orientation Guide - Critical for Magnet Holes!

## ❌ BAD Orientation (Causes Oval Magnet Holes)

**DO NOT print on the wide face (250mm × 25mm):**
```
        ┌──────────────────────────────┐
        │   250mm × 25mm face DOWN     │ ← Build plate
        └──────────────────────────────┘
              |||||||||||||||||
           (15mm standing up)
```

**Why this is BAD:**
- ❌ Horizontal magnet holes print as OVALS (not circles!)
- ❌ Layers run through magnet holes → poor dimensional accuracy
- ❌ LED channel becomes overhang → drooping/sagging
- ❌ Weak layer adhesion in critical areas

**Result:** Magnets don't fit properly, poor finish

---

## ✅ BEST Orientation (Perfect Circular Holes)

**Print on the narrow face (250mm × 15mm face DOWN):**

```
     LED channel faces UP ↑
     ═══════════════════════
     ║                     ║
     ║   Module body       ║  ← 25mm tall
     ║                     ║
     ═══════════════════════
     └──────────────────────┘
       250mm × 15mm face DOWN ← Build plate
```

**Why this is PERFECT:**
- ✅ ALL magnet holes perpendicular to build plate → **PERFECT CIRCLES**
- ✅ Layers run parallel to magnets → accurate dimensions
- ✅ LED channel opens upward → no overhangs
- ✅ Strong layer adhesion along length (250mm)
- ✅ No supports needed
- ✅ Best surface finish on LED channel

**Result:** Magnets fit perfectly, strong print, clean finish

---

## Bambu Lab Studio Setup

### Step 1: Import STL
- Load `single_module.stl` into Bambu Studio

### Step 2: Auto-Orient (DO NOT USE!)
- Bambu Studio auto-orient may choose wrong face
- **Skip auto-orient** or undo if it rotates incorrectly

### Step 3: Correct Orientation
1. Select the part
2. Click **"Rotate"** tool (or press R)
3. Orient so:
   - **Front or back face DOWN** (the 250mm × 15mm rectangle)
   - LED channel opening faces **UP**
   - Module is 25mm tall when standing

### Step 4: Visual Check
Look at the magnet holes in preview:
- ✅ All 4 side holes should be **vertical cylinders**
- ✅ Top/bottom holes should **point straight down** into build plate
- ✅ If holes look tilted or horizontal → **WRONG ORIENTATION**

### Step 5: Print Settings
```
Layer Height:       0.2mm (or 0.16mm for better magnet holes)
First Layer:        0.2mm
Infill:            15-20%
Perimeters:        3-4 walls (important for magnet retention!)
Top/Bottom:        4 layers
Support:           NONE needed
Brim:              Optional (5mm helps with bed adhesion)
```

**Advanced: For best magnet hole accuracy:**
- Use **0.16mm layer height** (makes holes rounder)
- Calibrate **flow rate** (prevents elephant's foot)
- Enable **arc fitting** if available (smoother circles)

---

## How to Verify Orientation

**Before slicing, check these:**

| Feature | Correct | Wrong |
|---------|---------|-------|
| LED channel | Opens upward ↑ | Sideways or down |
| Module length | Flat on bed (250mm) | Vertical (250mm tall) |
| Side magnet holes | Vertical cylinders | Horizontal (bad!) |
| Top/bottom holes | Into build plate ⊥ | Sideways |
| Stability | Wide base (250mm×15mm) | Narrow (15mm×25mm) |

**In Bambu Studio Layer Preview:**
- Scrub through layers
- Magnet holes should appear as **perfect circles** in every layer
- If oval/elongated → **rotate and re-slice!**

---

## Troubleshooting

### Problem: Magnet holes are oval/elongated
**Cause:** Wrong print orientation
**Fix:** Rotate 90° so holes are perpendicular to bed

### Problem: Magnets too loose
**Cause:** Over-extrusion or wrong hole size
**Fix:**
- Calibrate flow (should be ~95-98%)
- Check hole diameter (should be 10.2mm for 10mm magnets)
- Print small test piece first

### Problem: Magnets too tight
**Cause:** Under-extrusion or filament shrinkage
**Fix:**
- Increase hole diameter to 10.3-10.4mm in scad file
- Sand holes gently with 10mm drill bit (by hand)
- Use PLA instead of PETG (less shrinkage)

### Problem: Layer lines visible in magnet holes
**Cause:** Layer height too thick
**Fix:** Use 0.16mm or 0.12mm layer height for smoother finish

---

## Quick Reference

**Correct orientation in one sentence:**
> Print with **front or back face down** (LED channel facing UP), so all magnet holes are **perpendicular to the build plate**.

**Visual check:**
- Stand at eye level with printer
- Look at the side of the model
- You should see LED channel as a horizontal groove facing up
- Magnet holes point into the bed like wells

---

## Multiple Prints Per Plate

**Bambu X1 Carbon (256×256mm bed):**
- Fits **1 module** easily (250mm long)
- Leave 5-10mm margin from edges
- Enable brim for better adhesion

**For batch printing:**
- Print modules one at a time for best quality
- OR arrange 2 modules diagonally if careful (tight fit!)
- Consistency matters more than speed for magnet holes

---

## Final Checklist Before Print

- [ ] Model oriented: LED channel UP, magnet holes vertical
- [ ] Layer preview shows circular holes (not ovals)
- [ ] No supports generated
- [ ] Print settings: 0.2mm layers, 15% infill, 3+ walls
- [ ] Bed clean and leveled
- [ ] Using quality filament (PLA recommended)
- [ ] Print time: ~1-1.5 hours per module

**Good orientation = perfect magnet fit = strong modular system!**

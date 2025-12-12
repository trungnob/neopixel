# Print Orientation - SUPER SIMPLE GUIDE

## The Module Has 3 Dimensions:
- **LENGTH**: 250mm (long side)
- **HEIGHT**: 25mm (short side 1)
- **DEPTH**: 15mm (short side 2)

## Which Face Touches the Build Plate?

Your module has 6 faces. Only ONE is correct!

---

## ❌ BAD - DO NOT DO THIS

**If you lay it FLAT like this (wide side down):**

```
     Looking from above:
     ┌────────────────────────┐
     │                        │  ← 250mm long
     │    FLAT ON BED         │
     │    (25mm wide)         │
     └────────────────────────┘

     Only 15mm tall standing up
```

**This is WRONG because:**
- The side magnet holes become HORIZONTAL
- Horizontal holes = oval shapes (BAD!)
- Your magnet won't fit properly

---

## ✅ GOOD - DO THIS

**Stand it up on the NARROW edge:**

```
     Looking from the side:

     ╔════════════════════════╗  ← LED channel on top
     ║                        ║
     ║      25mm tall         ║
     ║                        ║
     ╚════════════════════════╝  ← 15mm face on bed
     ──────────────────────────
          BUILD PLATE

     250mm goes left to right
```

**This is CORRECT because:**
- Side magnet holes go straight DOWN (vertical)
- Vertical holes = perfect circles ✓
- Magnets fit perfectly!

---

## In Bambu Studio - Step by Step

### When you import the STL:

1. **It might load in wrong orientation** - that's OK, we'll fix it

2. **Look at your screen** - you should see the module from an angle

3. **Goal: The module should look like a LONG SKINNY RECTANGLE standing up**

### How to rotate:

1. Click the model
2. Press **R** (rotate tool)
3. Rotate until you see this view:

```
From the front, you should see:

    ┌──────────────────────────┐  ← Top (LED channel here)
    │                          │
    │       25mm tall          │
    │                          │
    └──────────────────────────┘  ← Bottom (on bed)

         250mm wide
```

4. **From the side** (rotate your view), you should see:

```
    ┌─────┐  ← 15mm thick (this edge touches bed)
    │     │
    │ 25  │  ← Module standing 25mm tall
    │ mm  │
    └─────┘
```

---

## The Simple Test

**Before you slice, check these 4 magnet holes on the SIDES:**

### ✅ CORRECT:
```
Side view of module:

    │         │
    │    ●    │  ← Top magnet hole (going INTO the module)
    │         │
    │  body   │
    │         │
    │    ●    │  ← Bottom magnet hole (going INTO the module)
    │         │
    ─────────────  Build plate
```
The holes should look like **wells** going down into the part!

### ❌ WRONG:
```
Side view of module:

    ──────○──────────○──────  ← Magnet holes laying sideways (BAD!)
    │                      │
    │                      │
    ─────────────────────────  Build plate
```
If the holes look like they're laying down sideways = WRONG!

---

## Visual Check in Bambu Studio Preview

### After slicing, check the layer preview:

1. Click **Preview** tab
2. Use the slider to go through layers
3. Look at the magnet holes in the layers

**What you should see:**

✅ **CORRECT:** Magnet holes appear as **CIRCLES** (○) in every layer

❌ **WRONG:** Magnet holes appear as **OVALS** or **ELONGATED** (⬭) shapes

---

## Still Confused? Use This Rule:

> **The LED channel (the groove) should open UPWARD toward the sky**
>
> **The narrow 15mm face should touch the bed**

If you follow this rule, the orientation will be perfect!

---

## What You See in Bambu Studio

**Correct orientation looks like:**
- A long bar (250mm) going left-to-right across the build plate
- Standing up 25mm tall
- The front face shows a horizontal groove (LED channel)
- Magnet holes are on the left/right ends, pointing inward

**Wrong orientation looks like:**
- A wide flat panel laying down
- Only 15mm tall
- Magnet holes visible on top surface
- Looks very flat and wide

---

## One More Way to Think About It

Imagine the module is a **tray** or **gutter**:
- The LED channel is the **inside of the tray**
- You want the tray opening facing **UP** (to catch rain)
- The tray sits on its narrow edge

That's the correct orientation!

---

## Final Check

Before clicking "Print":

- [ ] Module is standing up (25mm tall)
- [ ] LED channel groove faces up
- [ ] Narrow 15mm face is on the build plate
- [ ] When you look at the side, magnet holes go down like wells
- [ ] Layer preview shows circular holes (not ovals)

If all checks pass → **PRINT IT!** ✓

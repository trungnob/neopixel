/*
 * Single LED Strip Module - Minimal Design
 * Compact holder for one section of WS2812B LED strip
 *
 * Features:
 * - Minimal size - just enough to hold the strip
 * - 4 magnet holes for modular connections
 * - Lightweight and movable
 */

// ========== CONFIGURATION ==========

// LED Strip dimensions
strip_segment_length = 250;  // Length of this module
strip_width = 12;            // WS2812B strip width

// Module dimensions (MINIMIZED)
module_width = strip_segment_length;
module_height = 25;          // 2.5cm vertical spacing (compact!)
module_depth = 15;           // Reduced from 20mm to 15mm
wall_thickness = 1.5;        // Reduced from 2mm to 1.5mm

// LED channel (on front)
led_channel_depth = 4;       // Shallow groove (reduced from 5mm)
led_channel_width = 14;      // Increased to 14mm for better fit

// Wire channel (on back)
wire_channel_width = 8;      // Narrow channel for wires
wire_channel_depth = 5;      // Back routing space

// Magnets (user's magnets: 10mm dia × 1.54mm thick)
magnet_dia = 10.2;           // 10mm + 0.2mm tolerance
magnet_thickness = 1.8;      // 1.54mm + tolerance (thin!)
magnet_inset = 8;            // Distance from edges (adjusted for 25mm height)

// ========== SINGLE MODULE ==========

module minimal_strip_holder() {
    difference() {
        // Main body - thin shell
        cube([module_width, module_depth, module_height]);

        // Front LED channel (OPEN ENDS - strip passes through)
        translate([-0.1, -0.1, module_height/2 - led_channel_width/2])
            cube([module_width + 0.2,  // Goes all the way through left to right
                  led_channel_depth + 0.1,
                  led_channel_width]);

        // Back wire channel (OPEN ENDS - wires pass through)
        translate([-0.1, module_depth - wire_channel_depth, module_height/2 - wire_channel_width/2])
            cube([module_width + 0.2,  // Goes all the way through left to right
                  wire_channel_depth + 0.1,
                  wire_channel_width]);

        // Center cavity (weight reduction)
        translate([wall_thickness + 4,
                  led_channel_depth + wall_thickness,
                  wall_thickness + 3])
            cube([module_width - 2*wall_thickness - 8,
                  module_depth - led_channel_depth - wire_channel_depth - 2*wall_thickness,
                  module_height - 2*wall_thickness - 6]);

        // HORIZONTAL magnets (left/right)
        // Right side
        translate([module_width - magnet_thickness/2, module_depth/2, magnet_inset])
            rotate([0, 90, 0])
            cylinder(h=magnet_thickness + 0.1, d=magnet_dia, $fn=32);
        translate([module_width - magnet_thickness/2, module_depth/2, module_height - magnet_inset])
            rotate([0, 90, 0])
            cylinder(h=magnet_thickness + 0.1, d=magnet_dia, $fn=32);

        // Left side
        translate([-0.1, module_depth/2, magnet_inset])
            rotate([0, 90, 0])
            cylinder(h=magnet_thickness + 0.1, d=magnet_dia, $fn=32);
        translate([-0.1, module_depth/2, module_height - magnet_inset])
            rotate([0, 90, 0])
            cylinder(h=magnet_thickness + 0.1, d=magnet_dia, $fn=32);

        // VERTICAL magnets (top/bottom)
        // Top
        translate([module_width/4, module_depth/2, module_height - magnet_thickness/2])
            cylinder(h=magnet_thickness + 0.1, d=magnet_dia, $fn=32);
        translate([3*module_width/4, module_depth/2, module_height - magnet_thickness/2])
            cylinder(h=magnet_thickness + 0.1, d=magnet_dia, $fn=32);

        // Bottom
        translate([module_width/4, module_depth/2, -0.1])
            cylinder(h=magnet_thickness + 0.1, d=magnet_dia, $fn=32);
        translate([3*module_width/4, module_depth/2, -0.1])
            cylinder(h=magnet_thickness + 0.1, d=magnet_dia, $fn=32);
    }
}

// LED strip visualization (optional)
module led_strip_visual() {
    color("white", 0.9)
    translate([wall_thickness, led_channel_depth - 3, module_height/2 - strip_width/2])
    rotate([90, 0, 0])
    cube([module_width - 2*wall_thickness, strip_width, 3]);

    // LEDs
    led_spacing = module_width / 36;  // 36 LEDs per module
    for (i = [0:35]) {
        color("gold", 0.8)
        translate([i * led_spacing + led_spacing/2,
                  1,
                  module_height/2])
            rotate([90, 0, 0])
            cylinder(h=2, d=4, $fn=6);
    }
}

// ========== RENDER ==========

// Show module
minimal_strip_holder();

// Show LED strip (uncomment to visualize)
// led_strip_visual();

// ========== EXPORT NOTES ==========
/*
 * DIMENSIONS:
 * - Size: 250mm (W) × 50mm (H) × 15mm (D)
 * - Weight: ~15-18g per module (at 15% infill)
 * - Wall thickness: 1.5mm
 *
 * MAGNETS:
 * - 4× 10mm × 3mm neodymium magnets per module
 * - Amazon: "neodymium magnets 10mm x 3mm N52"
 *
 * PRINTING:
 * - Layer height: 0.2mm
 * - Infill: 15%
 * - No supports needed
 * - Print time: ~1-1.5 hours per module
 *
 * CUSTOMIZATION:
 * - Adjust module_width for different lengths (e.g., 200mm, 333mm)
 * - Change module_height for different spacing (e.g., 30mm, 40mm, 60mm)
 * - Modify led_channel_width if your strip is wider/narrower
 *
 * EXPORT STL:
 * openscad -o module.stl single_module.scad
 */

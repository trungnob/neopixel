/*
 * Compact LED Strip Module - 25mm Height
 * Three types: END, MIDDLE, or ALL (all have magnets both sides)
 */

// ========== CONFIGURATION ==========

// LED Strip dimensions
strip_segment_length = 250;  // Length of this module
strip_width = 12;            // WS2812B strip width

// Module dimensions (COMPACT!)
module_width = strip_segment_length;
module_height = 25;          // Only 2.5cm tall!
module_depth = 15;           // Thin profile
wall_thickness = 1.5;

// LED channel (on front)
led_channel_depth = 4;
led_channel_width = 13;

// Wire channel (on back)
wire_channel_width = 8;
wire_channel_depth = 5;

// Magnets (adjusted for user's magnets: 10mm dia × 1.54mm thick)
magnet_dia = 10.2;           // 10mm + 0.2mm tolerance
magnet_thickness = 1.7;      // 1.54mm + tolerance (thin magnets!)
magnet_inset = 8;

// Module type - CHANGE THIS!
module_type = "middle";      // "end_left", "middle", "end_right", or "all"
show_led_strip = true;       // Show LED visualization

// ========== MODULE BUILDER ==========

module compact_holder(type="middle") {
    difference() {
        // Main body
        cube([module_width, module_depth, module_height]);

        // Front LED channel (OPEN ENDS)
        translate([-0.1, -0.1, module_height/2 - led_channel_width/2])
            cube([module_width + 0.2, led_channel_depth + 0.1, led_channel_width]);

        // Back wire channel (OPEN ENDS)
        translate([-0.1, module_depth - wire_channel_depth, module_height/2 - wire_channel_width/2])
            cube([module_width + 0.2, wire_channel_depth + 0.1, wire_channel_width]);

        // Center cavity (weight reduction)
        translate([wall_thickness + 4, led_channel_depth + wall_thickness, wall_thickness + 2])
            cube([module_width - 2*wall_thickness - 8,
                  module_depth - led_channel_depth - wire_channel_depth - 2*wall_thickness,
                  module_height - 2*wall_thickness - 4]);

        // HORIZONTAL magnets (left/right sides)
        // RIGHT side magnets
        if (type == "middle" || type == "end_left" || type == "all") {
            translate([module_width - magnet_thickness/2, module_depth/2, magnet_inset])
                rotate([0, 90, 0])
                cylinder(h=magnet_thickness + 0.1, d=magnet_dia, $fn=32);
            translate([module_width - magnet_thickness/2, module_depth/2, module_height - magnet_inset])
                rotate([0, 90, 0])
                cylinder(h=magnet_thickness + 0.1, d=magnet_dia, $fn=32);
        }

        // LEFT side magnets
        if (type == "middle" || type == "end_right" || type == "all") {
            translate([-0.1, module_depth/2, magnet_inset])
                rotate([0, 90, 0])
                cylinder(h=magnet_thickness + 0.1, d=magnet_dia, $fn=32);
            translate([-0.1, module_depth/2, module_height - magnet_inset])
                rotate([0, 90, 0])
                cylinder(h=magnet_thickness + 0.1, d=magnet_dia, $fn=32);
        }

        // VERTICAL magnets (top/bottom) - ALL types have these
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

// LED strip visualization
module led_strip_visual() {
    color("white", 0.9)
    translate([0, led_channel_depth - 3, module_height/2 - strip_width/2])
    rotate([90, 0, 0])
    cube([module_width, strip_width, 3]);

    // LEDs
    led_spacing = module_width / 36;
    for (i = [0:35]) {
        color("gold", 0.8)
        translate([i * led_spacing + led_spacing/2, 1, module_height/2])
            rotate([90, 0, 0])
            cylinder(h=2, d=4, $fn=6);
    }
}

// ========== RENDER ==========

compact_holder(module_type);

if (show_led_strip) {
    led_strip_visual();
}

// ========== NOTES ==========
/*
 * COMPACT DESIGN - 25mm tall!
 *
 * Dimensions: 250mm (L) × 25mm (H) × 15mm (D)
 * Weight: ~12-15g per module
 *
 * MODULE TYPES:
 * - "end_left":   Magnets on RIGHT side only (left end of row)
 * - "middle":     Magnets on BOTH sides (middle connectors)
 * - "end_right":  Magnets on LEFT side only (right end of row)
 * - "all":        Magnets on BOTH sides (maximum flexibility)
 *
 * RECOMMENDED: Use "all" type for maximum flexibility!
 * This lets you rearrange modules anywhere.
 *
 * FOR 9 ROWS × 4 MODULES:
 * - Option A (specific ends): 9× end_left, 18× middle, 9× end_right
 * - Option B (all flexible):  36× all (recommended!)
 *
 * EXPORT:
 * openscad -D 'module_type="middle"' -D 'show_led_strip=false' -o middle.stl module_compact.scad
 * openscad -D 'module_type="all"' -D 'show_led_strip=false' -o module_25mm.stl module_compact.scad
 */

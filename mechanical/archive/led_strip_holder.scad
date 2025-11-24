/*
 * LED Strip Holder Module - TV Panel Style
 * For 9×144 LED Grid (WS2812B strips)
 *
 * Design Concept:
 * - LED strips lay FLAT on front face (like TV pixels)
 * - 50mm = spacing between strips (vertical pitch)
 * - Magnetic modular system for easy assembly
 * - Each module = 250mm section of one row
 */

// ========== CONFIGURATION ==========

// LED Strip dimensions
strip_length = 250;        // Module width (1000mm / 4 modules)
strip_width = 12;          // WS2812B strip width (~10-12mm)
led_strip_thickness = 3;   // LED strip thickness

// Module dimensions
module_height = 50;        // Vertical spacing (row pitch)
module_depth = 20;         // Front to back depth
wall_thickness = 2;        // Wall thickness

// LED mounting channel (on front face)
led_channel_depth = 5;     // Shallow groove for LED strip
led_channel_width = 13;    // Slightly wider than strip

// Magnet mounts
magnet_dia = 10.2;         // 10mm neodymium + 0.2mm tolerance
magnet_thickness = 3.2;    // 3mm + 0.2mm tolerance
magnet_inset = 15;         // Distance from edges

// Mounting holes (for wall mount)
mount_hole_dia = 3.5;      // M3 screw clearance

// Module type to render
module_type = "middle";    // "left", "middle", "right"
show_assembly = true;      // Show full 9-strip assembly
show_led_strips = true;    // Show LED strips in visualization

// ========== MODULES ==========

// LED strip visualization
module led_strip() {
    color("white", 0.9)
    translate([0, 0, -led_strip_thickness])
        cube([strip_length, strip_width, led_strip_thickness]);

    // Individual LEDs
    led_spacing = strip_length / 36;  // 36 LEDs per 250mm module
    for (i = [0:35]) {
        color("gold", 0.8)
        translate([i * led_spacing + led_spacing/2, strip_width/2, -1])
            cylinder(h=2, d=5, $fn=6);
    }
}

// Main module holder
module strip_holder(type="middle") {
    difference() {
        // Main body - solid frame
        cube([strip_length, module_depth, module_height]);

        // LED channel on FRONT face (shallow groove)
        translate([wall_thickness, -0.1, module_height/2 - led_channel_width/2])
            cube([strip_length - 2*wall_thickness,
                  led_channel_depth + 0.1,
                  led_channel_width]);

        // Wire routing channel on BACK
        translate([wall_thickness, module_depth - 6, module_height/2 - 4])
            cube([strip_length - 2*wall_thickness, 6.1, 8]);

        // Weight reduction hollow (center cavity)
        translate([wall_thickness + 5, led_channel_depth + wall_thickness, wall_thickness])
            cube([strip_length - 2*wall_thickness - 10,
                  module_depth - led_channel_depth - 2*wall_thickness - 6,
                  module_height - 2*wall_thickness]);

        // HORIZONTAL magnets (left/right connections)
        if (type == "left" || type == "middle") {
            // Right side magnets
            translate([strip_length - magnet_thickness/2, module_depth/2, magnet_inset])
                rotate([0, 90, 0])
                cylinder(h=magnet_thickness + 0.1, d=magnet_dia, $fn=32);
            translate([strip_length - magnet_thickness/2, module_depth/2, module_height - magnet_inset])
                rotate([0, 90, 0])
                cylinder(h=magnet_thickness + 0.1, d=magnet_dia, $fn=32);
        }

        if (type == "middle" || type == "right") {
            // Left side magnets
            translate([-0.1, module_depth/2, magnet_inset])
                rotate([0, 90, 0])
                cylinder(h=magnet_thickness + 0.1, d=magnet_dia, $fn=32);
            translate([-0.1, module_depth/2, module_height - magnet_inset])
                rotate([0, 90, 0])
                cylinder(h=magnet_thickness + 0.1, d=magnet_dia, $fn=32);
        }

        // VERTICAL magnets (top/bottom stacking)
        // Top magnets
        translate([strip_length/4, module_depth/2, module_height - magnet_thickness/2])
            cylinder(h=magnet_thickness + 0.1, d=magnet_dia, $fn=32);
        translate([3*strip_length/4, module_depth/2, module_height - magnet_thickness/2])
            cylinder(h=magnet_thickness + 0.1, d=magnet_dia, $fn=32);

        // Bottom magnets
        translate([strip_length/4, module_depth/2, -0.1])
            cylinder(h=magnet_thickness + 0.1, d=magnet_dia, $fn=32);
        translate([3*strip_length/4, module_depth/2, -0.1])
            cylinder(h=magnet_thickness + 0.1, d=magnet_dia, $fn=32);

        // Wall mounting holes
        if (type == "left") {
            translate([magnet_inset, module_depth/2, module_height/2])
                rotate([90, 0, 0])
                cylinder(h=module_depth + 0.2, d=mount_hole_dia, $fn=32, center=true);
        }
        if (type == "right") {
            translate([strip_length - magnet_inset, module_depth/2, module_height/2])
                rotate([90, 0, 0])
                cylinder(h=module_depth + 0.2, d=mount_hole_dia, $fn=32, center=true);
        }
    }
}

// Single row assembly (4 modules)
module row_assembly() {
    // 4 modules: left + middle + middle + right
    colors = ["lightblue", "lightgreen", "lightyellow", "lightcoral"];
    types = ["left", "middle", "middle", "right"];

    for (i = [0:3]) {
        color(colors[i])
        translate([i * strip_length, 0, 0])
            strip_holder(types[i]);

        // Show LED strip if enabled
        if (show_led_strips) {
            translate([i * strip_length + wall_thickness,
                      led_channel_depth - led_strip_thickness - 0.5,
                      module_height/2 - strip_width/2])
                rotate([90, 0, 0])
                led_strip();
        }
    }
}

// Full 9-row assembly
module full_assembly() {
    for (i = [0:8]) {
        translate([0, 0, i * module_height])
            row_assembly();
    }
}

// ========== RENDER ==========

if (show_assembly) {
    full_assembly();
} else {
    strip_holder(module_type);
}

// ========== NOTES ==========
/*
 * MAGNET INSTALLATION:
 * - Each module: 4 magnets (2 horizontal + 2 vertical)
 * - Total: 144 magnets for 36 modules
 * - Size: 10mm diameter × 3mm thick, N52 grade
 * - Amazon: "neodymium magnets 10mm x 3mm N52"
 *
 * ASSEMBLY:
 * 1. Install magnets with correct polarity
 * 2. Connect 4 modules horizontally (left→middle→middle→right)
 * 3. Lay LED strip in front channel
 * 4. Stack 9 rows vertically
 * 5. Connect power/data cables on back
 *
 * PRINTING:
 * - Print 9× left, 18× middle, 9× right
 * - No supports needed
 * - 0.2mm layer height, 15-20% infill
 */

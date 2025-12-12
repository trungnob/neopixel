/*
 * 8x32 LED Panel Housing
 * Compact housing for flexible 8x32 LED matrix panel
 *
 * Features:
 * - Front channel to hold LED panel flush
 * - 3 wire holes through LED panel surface (for connectors to pass through)
 * - 4× M3 mounting holes at corners
 * - Thin 8mm profile for space efficiency
 */

// ========== CONFIGURATION ==========

// LED Panel dimensions (from photo: 12.59in x 3.15in ≈ 320mm x 80mm)
panel_length = 320;      // 12.59 inches
panel_width = 80;        // 3.15 inches  
panel_thickness = 1.5;   // Flexible PCB thickness

// Housing dimensions
housing_length = panel_length + 4;  // 324mm (add small margin)
housing_width = panel_width + 4;    // 84mm
housing_depth = 8;                   // Shallow profile
wall_thickness = 2;

// Front LED channel (shallow groove to hold panel)
led_channel_depth = 2;      // Just enough to hold panel flat
led_channel_width = panel_width;
led_channel_length = panel_length;

// Wire routing
wire_hole_diameter = 6;     // For connector pass-through
wire_channel_width = 10;    // Channel width for wires
wire_channel_depth = 3;     // Back routing depth

// Mounting holes (for hanging/attaching)
mounting_hole_dia = 4;      // M3 screw holes
mounting_hole_inset = 10;   // Distance from edges

// ========== MAIN HOUSING ==========

module led_panel_housing() {
    difference() {
        // Main body
        cube([housing_length, housing_width, housing_depth]);
        
        // ===== FRONT: LED panel channel (shallow) =====
        translate([wall_thickness, wall_thickness, housing_depth - led_channel_depth])
            cube([led_channel_length, 
                  led_channel_width, 
                  led_channel_depth + 0.1]);  // Goes to top surface
        
        // ===== WIRE HOLES through LED panel surface (green area) =====
        // 3 holes in the bottom of LED channel so connectors can pass through
        
        // Left hole (for left connector)
        translate([housing_length * 0.25, housing_width/2, -0.1])
            cylinder(h=housing_depth - led_channel_depth + 0.2, d=wire_hole_diameter, $fn=32);
        
        // Center hole (for middle connector)
        translate([housing_length * 0.50, housing_width/2, -0.1])
            cylinder(h=housing_depth - led_channel_depth + 0.2, d=wire_hole_diameter, $fn=32);
        
        // Right hole (for right connector)
        translate([housing_length * 0.75, housing_width/2, -0.1])
            cylinder(h=housing_depth - led_channel_depth + 0.2, d=wire_hole_diameter, $fn=32);
        
        // ===== Weight reduction cavity (back side) =====
        translate([wall_thickness + 15,
                  wall_thickness + 15,
                  -0.1])
            cube([housing_length - 2*wall_thickness - 30,
                  housing_width - 2*wall_thickness - 30,
                  wall_thickness + 0.1]);
        
        // ===== Mounting holes (corners for hanging) =====
        // Top left
        translate([mounting_hole_inset, mounting_hole_inset, -0.1])
            cylinder(h=housing_depth + 0.2, d=mounting_hole_dia, $fn=24);
        
        // Top right
        translate([housing_length - mounting_hole_inset, mounting_hole_inset, -0.1])
            cylinder(h=housing_depth + 0.2, d=mounting_hole_dia, $fn=24);
        
        // Bottom left
        translate([mounting_hole_inset, housing_width - mounting_hole_inset, -0.1])
            cylinder(h=housing_depth + 0.2, d=mounting_hole_dia, $fn=24);
        
        // Bottom right
        translate([housing_length - mounting_hole_inset, housing_width - mounting_hole_inset, -0.1])
            cylinder(h=housing_depth + 0.2, d=mounting_hole_dia, $fn=24);
    }
}

// ========== LED PANEL VISUALIZATION (optional) ==========
module led_panel_visual() {
    color("darkgreen", 0.8)
    translate([wall_thickness, 
              wall_thickness, 
              housing_depth - led_channel_depth + 0.5])
    cube([panel_length, panel_width, 0.5]);
    
    // LED positions (8 rows x 32 columns)
    led_spacing_x = panel_length / 32;
    led_spacing_y = panel_width / 8;
    
    for (row = [0:7]) {
        for (col = [0:31]) {
            color("gold", 0.6)
            translate([wall_thickness + col * led_spacing_x + led_spacing_x/2,
                      wall_thickness + row * led_spacing_y + led_spacing_y/2,
                      housing_depth - 0.5])
                cylinder(h=1, d=3, $fn=6);
        }
    }
}

// ========== RENDER ==========

// Main housing
led_panel_housing();

// LED panel visualization (uncomment to see LED layout)
// led_panel_visual();

// ========== EXPORT NOTES ==========
/*
 * DIMENSIONS:
 * - Housing: 324mm (L) × 84mm (W) × 8mm (D)
 * - Panel fit: 320mm × 80mm × 1.5mm
 * - Weight: ~40-50g (at 20% infill)
 *
 * FEATURES:
 * - Front LED channel: 2mm deep to hold panel
 * - 3× wire holes in LED panel surface (connectors pass through to back)
 * - 4× M3 mounting holes at corners  
 * - Thin profile (8mm total depth)
 *
 * PRINTING:
 * - Layer height: 0.2mm
 * - Infill: 20%
 * - Supports: None needed (print face-down)
 * - Material: PLA or PETG
 * - Print time: ~3-4 hours
 * - NOTE: 324mm length exceeds many printer beds!
 *
 * SPLITTING FOR SMALLER PRINTERS:
 * If your printer bed is < 324mm, you can:
 * 1. Split into 2× 162mm sections
 * 2. Add alignment pins/holes at the split
 * 3. Join with glue or screws after printing
 *
 * ASSEMBLY:
 * 1. Place LED panel in front channel (LEDs facing up)
 * 2. Thread wires/connectors through the 3 holes from top
 * 3. Wires exit from the back/bottom
 * 4. Mount using corner M3 holes (screws or wall anchors)
 *
 * CUSTOMIZATION:
 * - Adjust wire_hole_diameter for your connectors
 * - Add more wire holes if needed
 * - Modify housing_depth for deeper cavity
 * - Add edge lips/clips if panel needs securing
 *
 * EXPORT STL:
 * openscad -o 8x32_housing.stl 8x32_panel_housing.scad
 */

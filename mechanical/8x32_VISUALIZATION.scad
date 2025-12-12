/*
 * 8x32 LED Panel Housing - VISUALIZATION MODE
 * This version shows a cutaway so you can see the wire holes clearly
 */

// Include all the same parameters
panel_length = 320;
panel_width = 80;
panel_thickness = 1.5;
housing_length = panel_length + 4;  // 324mm
housing_width = panel_width + 4;    // 84mm
housing_depth = 8;
wall_thickness = 2;
led_channel_depth = 2;
led_channel_width = panel_width;
led_channel_length = panel_length;
wire_hole_diameter = 6;
mounting_hole_dia = 4;
mounting_hole_inset = 10;

// CUTAWAY VIEW - shows internal structure
module cutaway_housing() {
    difference() {
        // Original housing
        difference() {
            cube([housing_length, housing_width, housing_depth]);
            
            // LED panel channel (front/top)
            translate([wall_thickness, wall_thickness, housing_depth - led_channel_depth])
                cube([led_channel_length, led_channel_width, led_channel_depth + 0.1]);
            
            // ===== 3 WIRE HOLES IN BACK WALL =====
            // Left hole
            translate([housing_length * 0.25, -0.1, housing_depth/2])
                rotate([-90, 0, 0])
                cylinder(h=wall_thickness + 0.2, d=wire_hole_diameter, $fn=32);
            
            // Center hole
            translate([housing_length * 0.50, -0.1, housing_depth/2])
                rotate([-90, 0, 0])
                cylinder(h=wall_thickness + 0.2, d=wire_hole_diameter, $fn=32);
            
            // Right hole
            translate([housing_length * 0.75, -0.1, housing_depth/2])
                rotate([-90, 0, 0])
                cylinder(h=wall_thickness + 0.2, d=wire_hole_diameter, $fn=32);
            
            // Weight reduction
            translate([wall_thickness + 15, wall_thickness + 15, -0.1])
                cube([housing_length - 2*wall_thickness - 30,
                      housing_width - 2*wall_thickness - 30,
                      wall_thickness + 0.1]);
            
            // Mounting holes
            translate([mounting_hole_inset, mounting_hole_inset, -0.1])
                cylinder(h=housing_depth + 0.2, d=mounting_hole_dia, $fn=24);
            translate([housing_length - mounting_hole_inset, mounting_hole_inset, -0.1])
                cylinder(h=housing_depth + 0.2, d=mounting_hole_dia, $fn=24);
            translate([mounting_hole_inset, housing_width - mounting_hole_inset, -0.1])
                cylinder(h=housing_depth + 0.2, d=mounting_hole_dia, $fn=24);
            translate([housing_length - mounting_hole_inset, housing_width - mounting_hole_inset, -0.1])
                cylinder(h=housing_depth + 0.2, d=mounting_hole_dia, $fn=24);
        }
        
        // CUT AWAY HALF to show interior
        translate([-1, housing_width/2, -1])
            cube([housing_length + 2, housing_width/2 + 2, housing_depth + 2]);
    }
}

// Show wire hole positions with colored markers
module wire_hole_markers() {
    // Left hole marker (RED)
    color("red", 0.8)
    translate([housing_length * 0.25, 0, housing_depth/2])
        rotate([-90, 0, 0])
        cylinder(h=5, d=wire_hole_diameter + 2, $fn=32);
    
    // Center hole marker (GREEN)
    color("green", 0.8)
    translate([housing_length * 0.50, 0, housing_depth/2])
        rotate([-90, 0, 0])
        cylinder(h=5, d=wire_hole_diameter + 2, $fn=32);
    
    // Right hole marker (BLUE)
    color("blue", 0.8)
    translate([housing_length * 0.75, 0, housing_depth/2])
        rotate([-90, 0, 0])
        cylinder(h=5, d=wire_hole_diameter + 2, $fn=32);
}

// RENDER
cutaway_housing();
wire_hole_markers();

// Add text labels
color("white")
translate([housing_length * 0.25 - 10, -10, housing_depth/2])
    text("LEFT", size=8);

color("white")
translate([housing_length * 0.50 - 15, -10, housing_depth/2])
    text("CENTER", size=8);

color("white")
translate([housing_length * 0.75 - 12, -10, housing_depth/2])
    text("RIGHT", size=8);

/*
 * VIEWING INSTRUCTIONS:
 * 
 * In OpenSCAD:
 * 1. Press F5 to preview
 * 2. Use mouse to rotate view
 * 3. Look at the BACK EDGE (narrow 84mm edge at y=0)
 * 4. You'll see 3 colored cylinders showing wire holes:
 *    - RED = Left hole (at 25%)
 *    - GREEN = Center hole (at 50%)
 *    - BLUE = Right hole (at 75%)
 * 
 * The cutaway shows the interior so you can see:
 * - Front LED channel (top surface)
 * - 3 wire holes going through back wall
 * - Mounting holes at corners
 */

/*
 * 8x32 LED Panel Housing - SPLIT VERSION
 * For printers with < 324mm build volume
 * Splits 324mm housing into 2× 162mm pieces with alignment
 */

// Include or copy the base design parameters
panel_length = 320;
panel_width = 80.6;
panel_thickness = 1.5;
housing_length = panel_length + 4;
housing_width = panel_width + 4;
housing_depth = 20; // Increased from 16mm to 20mm for more wire space
wall_thickness = 2; // Restored to 2mm (was 7mm, causing missing wall)
led_channel_depth = 2;
led_channel_width = panel_width;
led_channel_length = panel_length;
// ADJUST THIS PARAMETER to change the square hole size
wire_hole_size = 20; 
wire_channel_width = 14; // Increased from 10mm to 14mm
wire_channel_depth = 11; // Deeper wire channels
mounting_hole_dia = 4;
mounting_hole_inset = 10;

// Split parameters
split_position = housing_length / 2;  // Split in middle
alignment_pin_dia = 4;
alignment_pin_depth = 4;
alignment_pin_tolerance = 0.2;

module half_housing(is_left_half = true) {
    difference() {
        union() {
            // Half of the main housing
            intersection() {
                // Full housing (matching main design)
                difference() {
                    cube([housing_length, housing_width, housing_depth]);
                    
                    // LED channel
                    translate([wall_thickness, wall_thickness, housing_depth - led_channel_depth])
                        cube([led_channel_length, led_channel_width, led_channel_depth + 0.1]);
                    
                    // ===== WIRE HOLES through LED panel surface =====
                    // Y position: 2cm (20mm) from bottom edge
                    wire_y_pos = 20;
                    
                    // Left hole - 4cm (40mm) from left edge
                    if (is_left_half) {
                        translate([40, wire_y_pos, -0.1])
                            translate([-wire_hole_size/2, -wire_hole_size/2, 0]) // Center the square
                            cube([wire_hole_size, wire_hole_size, housing_depth - led_channel_depth + 0.2]);
                    }
                    // Right hole - 4cm (40mm) from right edge
                    else {
                        translate([housing_length - 40, wire_y_pos, -0.1])
                            translate([-wire_hole_size/2, -wire_hole_size/2, 0])
                            cube([wire_hole_size, wire_hole_size, housing_depth - led_channel_depth + 0.2]);
                    }

                    // Center hole - 16cm (approx middle). Using exact center (162mm) to align with split.
                    // User requested to keep this at 15mm while others are 20mm
                    translate([housing_length / 2, wire_y_pos, -0.1])
                        translate([-15/2, -15/2, 0])
                        cube([15, 15, housing_depth - led_channel_depth + 0.2]);
                    
                    // Weight reduction
                    // Increased depth to leave only 5mm of material at the front
                    // Reduced border width: Offset 5mm (was 15mm) -> 7mm border
                    // SPLIT into two parts to create a solid rib at the center (to hold alignment pins)
                    
                    // Part 1 (Left side)
                    translate([wall_thickness + 5, wall_thickness + 5, -0.1])
                        cube([split_position - (wall_thickness + 5) - 5, // Stop 5mm before center
                              housing_width - 2*wall_thickness - 10,
                              housing_depth - 5]);
                              
                    // Part 2 (Right side)
                    translate([split_position + 5, wall_thickness + 5, -0.1]) // Start 5mm after center
                        cube([housing_length - split_position - 5 - (wall_thickness + 5),
                              housing_width - 2*wall_thickness - 10,
                              housing_depth - 5]);
                    
                    // ===== SIDE WIRE CHANNELS (N/S/E/W) =====
                    // Allow wires to pass between panels through the walls
                    // Extended to 25mm to cut through the solid border into the hollow center
                    // Moved to Z=2 to make them "holes" instead of "slots" (Solid bottom border)
                    
                    // North/South Channels (Top/Bottom walls)
                    // Removed center channel (housing_length/2) as requested
                    for (x_pos = [40, housing_length - 40]) {
                         // Bottom wall (South)
                         translate([x_pos - wire_channel_width/2, -0.1, 2])
                            cube([wire_channel_width, 25, wire_channel_depth]);
                         
                         // Top wall (North)
                         translate([x_pos - wire_channel_width/2, housing_width - 25, 2])
                            cube([wire_channel_width, 25.1, wire_channel_depth]);
                    }

                    // East/West Channels (Left/Right walls)
                    // Left wall (West)
                    translate([-0.1, housing_width/2 - wire_channel_width/2, 2])
                        cube([25, wire_channel_width, wire_channel_depth]);

                    // Right wall (East)
                    translate([housing_length - 25, housing_width/2 - wire_channel_width/2, 2])
                        cube([25.1, wire_channel_width, wire_channel_depth]);


                    // Mounting holes
                    if (is_left_half) {
                        translate([mounting_hole_inset, mounting_hole_inset, -0.1])
                            cylinder(h=housing_depth + 0.2, d=mounting_hole_dia, $fn=24);
                        translate([mounting_hole_inset, housing_width - mounting_hole_inset, -0.1])
                            cylinder(h=housing_depth + 0.2, d=mounting_hole_dia, $fn=24);
                    } else {
                        translate([housing_length - mounting_hole_inset, mounting_hole_inset, -0.1])
                            cylinder(h=housing_depth + 0.2, d=mounting_hole_dia, $fn=24);
                        translate([housing_length - mounting_hole_inset, housing_width - mounting_hole_inset, -0.1])
                            cylinder(h=housing_depth + 0.2, d=mounting_hole_dia, $fn=24);
                    }
                }
                
                // Cut to half
                if (is_left_half) {
                    cube([split_position + 1, housing_width, housing_depth]);
                } else {
                    translate([split_position - 1, 0, 0])
                        cube([housing_length - split_position + 1, housing_width, housing_depth]);
                }
            }
            
            // Add alignment pins (on left half)
            if (is_left_half) {
                // Pins stick out from the split face
                // Reverted to center (housing_depth/2) now that we have a solid rib
                translate([split_position, 42, housing_depth/2])
                    rotate([0, 90, 0])
                    cylinder(h=alignment_pin_depth, d=alignment_pin_dia, $fn=24);
                
                translate([split_position, 70, housing_depth/2])
                    rotate([0, 90, 0])
                    cylinder(h=alignment_pin_depth, d=alignment_pin_dia, $fn=24);
            }
        }
        
        // Add alignment holes (on right half)
        if (!is_left_half) {
            translate([split_position - 1, 42, housing_depth/2])
                rotate([0, 90, 0])
                cylinder(h=alignment_pin_depth + 1, 
                        d=alignment_pin_dia + alignment_pin_tolerance, 
                        $fn=24);
            
            translate([split_position - 1, 70, housing_depth/2])
                rotate([0, 90, 0])
                cylinder(h=alignment_pin_depth + 1, 
                        d=alignment_pin_dia + alignment_pin_tolerance, 
                        $fn=24);
        }
    }
}

// ========== RENDER ==========

// ========== RENDER ==========

// Render Mode:
// 0 = Left Half (for printing)
// 1 = Right Half (for printing)
// 2 = Full Assembly (for preview)
render_mode = 2; 

if (render_mode == 0) {
    half_housing(is_left_half = true);
} else if (render_mode == 1) {
    half_housing(is_left_half = false);
} else if (render_mode == 3) {
    // Stacked Batch (3x Left Half with 2mm gap) - Matches 'housing_all.stl'
    for (i = [0:2]) {
        translate([0, 0, i * 22]) // 20mm height + 2mm gap
        half_housing(is_left_half = true);
    }
} else {
    // Full Assembly (Default)
    // Render both halves in place to show the complete panel
    half_housing(is_left_half = true);
    half_housing(is_left_half = false);
}

/*
 * PRINTING INSTRUCTIONS:
 * 
 * METHOD 1: Edit this file
 * - For LEFT half: Set render_left = true; (line 119)
 * - For RIGHT half: Set render_left = false; (line 119)
 * - Press F5 in OpenSCAD, then export STL
 * 
 * METHOD 2: Command line override
 * - Left:  openscad -D 'render_left=true' -o 8x32_left.stl 8x32_housing_split.scad
 * - Right: openscad -D 'render_left=false' -o 8x32_right.stl 8x32_housing_split.scad
 * 
 * ASSEMBLY:
 * 1. Print both pieces
 * 2. Align using pins and join with CA glue
 * 3. Optional: Add M3 screw through split for extra strength
 */

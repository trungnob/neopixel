use <single_module.scad>

// GOOD - Side view showing vertical magnet holes
translate([-150, 0, 0]) {
    minimal_strip_holder();
    
    // Build plate
    translate([0, -5, -3])
        color("green", 0.7)
        cube([250, 25, 2]);
        
    // Label
    translate([125, 7.5, 30])
        color("green")
        text("✓ CIRCLES", size=10, halign="center");
}

// BAD - Side view showing horizontal magnet holes
translate([150, 0, 0]) {
    rotate([90, 0, 0])
        translate([0, 0, -25])
        minimal_strip_holder();
    
    // Build plate
    translate([0, -30, -3])
        color("red", 0.7)
        cube([250, 30, 2]);
        
    // Label
    translate([125, 0, 30])
        color("red")
        text("✗ OVALS", size=10, halign="center");
}

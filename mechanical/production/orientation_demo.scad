// Import the module
use <single_module.scad>

// Show GOOD orientation
translate([0, 0, 0]) {
    rotate([0, 0, 0]) {
        minimal_strip_holder();
    }
    
    // Add text
    translate([125, -5, -10])
        color("green")
        text("✓ CORRECT", size=8, halign="center");
    
    // Show build plate
    translate([0, -20, -2])
        color("lightblue", 0.3)
        cube([250, 60, 1]);
}

// Show BAD orientation
translate([0, 100, 0]) {
    rotate([0, 90, 0]) {  // Rotated wrong!
        minimal_strip_holder();
    }
    
    // Add text
    translate([12, 80, -10])
        color("red")
        text("✗ WRONG", size=8, halign="center");
    
    // Show build plate
    translate([-10, 80, -2])
        color("lightblue", 0.3)
        cube([40, 60, 1]);
}

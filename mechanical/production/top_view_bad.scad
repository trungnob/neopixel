use <single_module.scad>

// BAD orientation - wide 25mm face on bed (rotated 90 degrees)
rotate([90, 0, 0])
    translate([0, 0, -25])
    minimal_strip_holder();

// Build plate
translate([0, -30, -3])
    color("red", 0.5)
    cube([250, 30, 2]);

use <single_module.scad>

// GOOD orientation - narrow 15mm face on bed
minimal_strip_holder();

// Build plate
translate([0, -5, -3])
    color("blue", 0.5)
    cube([250, 25, 2]);

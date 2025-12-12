use <single_module.scad>

// GOOD orientation - narrow side down
minimal_strip_holder();

// Show build plate underneath
translate([0, -5, -2])
    color("lightblue", 0.5)
    cube([250, 25, 1]);

// Arrow showing "this face down"
translate([125, 7.5, -8])
    color("green")
    cylinder(h=5, r=2, $fn=3);

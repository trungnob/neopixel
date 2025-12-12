use <single_module.scad>

// BAD orientation - wide side down (rotated 90°)
rotate([90, 0, 0])
    minimal_strip_holder();

// Show build plate underneath
translate([0, -30, -2])
    color("red", 0.5)
    cube([250, 30, 1]);

// X mark showing "WRONG"
translate([125, 0, -10]) {
    color("red") {
        rotate([0, 0, 45]) cube([5, 50, 1], center=true);
        rotate([0, 0, -45]) cube([5, 50, 1], center=true);
    }
}

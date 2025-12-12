#ifndef PATTERN_LIST_H
#define PATTERN_LIST_H

#include <Arduino.h>

struct PatternMetadata {
  uint8_t id;
  const char *name;
  const char *category;
};

// Master list of all patterns
const PatternMetadata patterns[] = {
    // Animation Patterns
    {125, "Digital Clock", "Animation Patterns"},
    {102, "2D Fire Rising", "Animation Patterns"},
    {109, "Plasma 2D", "Animation Patterns"},
    {110, "Matrix Rain 2D", "Animation Patterns"},
    {111, "Game of Life", "Animation Patterns"},
    {113, "Aurora 2D", "Animation Patterns"},
    {114, "Lava Lamp 2D", "Animation Patterns"},
    {115, "2D Ripple", "Animation Patterns"},
    {116, "Starfield 3D", "Animation Patterns"},
    {117, "Side Fire", "Animation Patterns"},
    {118, "Scrolling Rainbow", "Animation Patterns"},
    {121, "Test Card", "Animation Patterns"},
    {123, "Fireflies", "Animation Patterns"},
    {126, "Enumerate Panels", "Animation Patterns"},

    // Best for 32x32 (4x1 Panels)
    {127, "Hypnotic Rings", "Best for 32x32 (4x1 Panels)"},
    {128, "Noise Lava", "Best for 32x32 (4x1 Panels)"},
    {129, "Diagonal Plasma", "Best for 32x32 (4x1 Panels)"},

    // Awesome Extras
    {130, "Spiral Galaxy", "Awesome Extras"},
    {131, "Shooting Stars", "Awesome Extras"},
    {132, "Fireworks", "Awesome Extras"},
    {133, "DNA Helix", "Awesome Extras"},
    {134, "Radar", "Awesome Extras"},
    {135, "Snake AI", "Awesome Extras"},
    {136, "Spectrum", "Awesome Extras"},
    {137, "3D Sine", "Awesome Extras"},
    {138, "Confetti", "Awesome Extras"},
    {139, "Breathing", "Awesome Extras"},

    // Gravity Patterns
    {140, "Bouncing Balls (Old)", "Gravity Patterns"},
    {141, "Particle Fountain (Old)", "Gravity Patterns"},
    {142, "Gravity Rain (Old)", "Gravity Patterns"},

    // 32x32 Extras
    {143, "3D Tunnel", "32x32 Extras"},
    {144, "Kaleidoscope", "32x32 Extras"},
    {145, "Lissajous", "32x32 Extras"},
    {146, "Clouds", "32x32 Extras"},
    {147, "Tartan", "32x32 Extras"},
    {148, "Polar Waves", "32x32 Extras"},
    {149, "Swirl", "32x32 Extras"},
    {150, "Zoom", "32x32 Extras"},
    {151, "Liquid", "32x32 Extras"},
    {152, "Rorschach", "32x32 Extras"},

    // 8x32 Extras
    {153, "Cylon / Scanner", "8x32 Extras"},
    {154, "Pacman", "8x32 Extras"},
    {155, "Windy Rain", "8x32 Extras"},
    {156, "Traffic", "8x32 Extras"},
    {157, "Ping Pong", "8x32 Extras"},
    {158, "Horizontal Plasma", "8x32 Extras"},
    {159, "Equalizer", "8x32 Extras"},
    {160, "Knight Rider", "8x32 Extras"},
    {161, "Police Strobes", "8x32 Extras"},
    {162, "Horizontal DNA", "8x32 Extras"},

    // 32x32 Wow Patterns
    {163, "Metaballs", "32x32 Wow Patterns"},
    {164, "Julia Set", "32x32 Wow Patterns"},
    {165, "Voronoi", "32x32 Wow Patterns"},
    {166, "Star Warp", "32x32 Wow Patterns"},
    {167, "Perlin Fire", "32x32 Wow Patterns"},
    {168, "Water Caustics", "32x32 Wow Patterns"},
    {169, "Hypnotic Squares", "32x32 Wow Patterns"},
    {170, "Spiral Illusion", "32x32 Wow Patterns"},
    {171, "Glitch Art", "32x32 Wow Patterns"},

    // Unified Gravity Patterns (Top-Down)
    {173, "Bouncing Balls", "Gravity (Top-Down)"},
    {174, "Fountain", "Gravity (Top-Down)"},
    {175, "Gravity Rain", "Gravity (Top-Down)"},
    {176, "Gravity Sand", "Gravity (Top-Down)"},
    {177, "Gravity Snow", "Gravity (Top-Down)"}};

const int numPatterns = sizeof(patterns) / sizeof(patterns[0]);

#endif // PATTERN_LIST_H

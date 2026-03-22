#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include "fbsplash.h"
#include "svg_parser.h"
#include "svg_renderer.h"
#include "dt_rotation.h"

/*
 * SVG path data for rendering the logo
 * Contains the path data for each component of the logo
 * Index meaning:
 * 0: "A" in logo
 * 1: "U" in logo
 * 2: "R" in logo
 * 3: "K" in logo
 * 4: "N" in logo
 * 5: "I" in logo
 * 6: "X" in logo
 */
const char *svg_paths[] = {
    "M 0 359.628 L 75 140.106 L 150 359.628 Z M 47 255 L 103 255 L 75 175 Z M 30 340 L 55 275 L 95 275 L 120 340 Z",
    "M 166.036 140.106 L 166.036 280 C 166.036 340 220 363.886 276.734 363.886 C 333 363.886 387.424 340 387.424 280 L 387.424 140.106 L 358.154 140.106 L 358.154 280 C 358.154 320 320 337.676 276.734 337.676 C 233 337.676 195.306 320 195.306 280 L 195.306 140.106 Z",
    "M 414 140.106 L 414 359.628 L 441.138 359.628 L 441.138 270.09 L 461.626 270.09 L 529.612 359.628 L 564.077 359.628 L 491.701 267.695 C 526.29 262.506 547.445 239.357 547.445 204.499 C 547.445 164.719 519.902 140.106 476.269 140.106 L 414 140.106 Z M 441.138 161.925 L 468.944 161.925 C 500.613 161.925 519.368 178.024 519.368 204.499 C 519.368 230.975 500.613 247.073 468.944 247.073 L 441.138 247.073 L 441.138 161.925 Z",
    "M 661.357 359.628 L 661.357 262.108 L 762.073 359.628 L 799.853 359.628 L 685.438 247.872 L 788.145 140.106 L 754.353 140.106 L 661.357 237.894 L 661.357 140.106 L 634.217 140.106 L 634.217 359.628 L 661.357 359.628 Z",
    "M 821.944 140.106 L 821.944 359.628 L 849.084 359.628 L 849.084 186.006 L 960.038 359.628 L 987.178 359.628 L 987.178 140.106 L 960.038 140.106 L 960.038 313.861 L 849.084 140.106 L 821.944 140.106 Z",
    "M 1034.814 140.106 L 1034.814 359.627 L 1061.955 359.627 L 1061.955 140.106 L 1034.814 140.106 Z",
    "M 1116.9 359.628 L 1183.554 264.768 L 1250.208 359.628 L 1284 359.628 L 1200.45 241.219 L 1270.96 140.106 L 1238.369 140.106 L 1183.554 218.469 L 1128.74 140.106 L 1096.141 140.106 L 1166.658 241.219 L 1083.109 359.628 L 1116.9 359.628 Z"
};

/* Color definitions for each path component
 * First 4 paths are red (brand color)
 * Last 3 paths are gray (secondary color)
 */
const char *svg_colors[] = {
    "rgb(85,85,255)",  // Blue
    "rgb(85,85,255)",  // Blue
    "rgb(255,85,85)",  // Red
    "rgb(255,85,85)",  // Red
    "rgb(85,85,85)",   // Gray
    "rgb(85,85,85)",   // Gray
    "rgb(85,85,85)"    // Gray
};

#define NUM_PATHS (sizeof(svg_paths) / sizeof(svg_paths[0]))

/*
 * Main program entry point
 */
int main(void) {
    const char *fb_device = "/dev/fb0";

    // Get rotation from device tree
    int rotation = get_display_rotation();

    // Check framebuffer device accessibility
    if (access(fb_device, R_OK | W_OK) != 0) {
        fprintf(stderr, "Cannot access %s: %s\n", fb_device, strerror(errno));
        return 1;
    }

    // Initialize framebuffer
    Framebuffer *fb = fb_init(fb_device);
    if (!fb) {
        fprintf(stderr, "Failed to initialize framebuffer\n");
        return 1;
    }

    // Calculate display parameters
    DisplayInfo *display_info = calculate_display_info(fb);
    if (!display_info) {
        fprintf(stderr, "Failed to calculate display information\n");
        fb_cleanup(fb);
        return 1;
    }

    // Clear screen to black
    for (uint32_t y = 0; y < fb->vinfo.yres; y++) {
        for (uint32_t x = 0; x < fb->vinfo.xres; x++) {
            set_pixel(fb, x, y, 0x00000000);
        }
    }

    // Process and render each path component
    for (size_t i = 0; i < NUM_PATHS; i++) {
        SVGPath *svg = parse_svg_path(svg_paths[i], svg_colors[i]);
        if (!svg) {
            fprintf(stderr, "Failed to parse SVG path %zu\n", i);
            continue;
        }

        // Apply rotation from device tree if specified
        if (rotation)
            rotate_svg_path(svg, rotation);

        // Render the path
        render_svg_path(fb, svg, display_info);
        free_svg_path(svg);
    }

    // Flush changes to the framebuffer
    fb_flush(fb);

    // Clean up
    free(display_info);
    fb_cleanup(fb);

    return 0;
}

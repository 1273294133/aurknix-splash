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
    "M 143.1 315.3 L 157.6 359.6 L 204.0 359.6 L 128.0 140.0 L 78.0 140.0 L 0.0 359.6 L 46.1 359.6 L 60.8 315.3 Z M 130.7 277.7 L 73.5 277.7 L 102.1 191.8 Z",
    "M 341.0 140.0 L 341.0 288.8 C 341.0 315.3 327.4 328.0 299.1 328.0 C 270.7 328.0 257.2 315.3 257.2 288.8 L 257.2 140.0 L 212.0 140.0 L 212.0 288.8 C 212.0 313.5 218.6 331.0 233.4 344.3 C 249.3 358.7 272.6 366.6 299.1 366.6 C 325.6 366.6 348.8 358.7 364.8 344.3 C 379.5 331.0 386.2 313.5 386.2 288.8 L 386.2 140.0 Z",
    "M 406.217 140.106 L 406.217 359.628 L 433.357 359.628 L 433.357 270.090 L 453.845 270.090 L 521.831 359.628 L 556.296 359.628 L 483.920 267.695 C 518.509 262.506 539.664 239.357 539.664 204.499 C 539.664 164.719 512.121 140.106 468.488 140.106 L 406.217 140.106 Z M 433.357 161.925 L 461.163 161.925 C 492.832 161.925 511.587 178.024 511.587 204.499 C 511.587 230.975 492.832 247.073 461.163 247.073 L 433.357 247.073 L 433.357 161.925 Z",
    "M 603.359 359.628 L 603.359 262.108 L 704.075 359.628 L 741.855 359.628 L 627.440 247.872 L 730.147 140.106 L 696.355 140.106 L 603.359 237.894 L 603.359 140.106 L 576.219 140.106 L 576.219 359.628 L 603.359 359.628 Z",
    "M 762.219 140.106 L 762.219 359.628 L 789.359 359.628 L 789.359 186.006 L 900.313 359.628 L 927.453 359.628 L 927.453 140.106 L 900.313 140.106 L 900.313 313.861 L 789.359 140.106 L 762.219 140.106 Z",
    "M 947.219 140.106 L 947.219 359.627 L 974.360 359.627 L 974.360 140.106 L 947.219 140.106 Z",
    "M 1033.010 359.628 L 1099.664 264.768 L 1166.318 359.628 L 1200.110 359.628 L 1116.560 241.219 L 1187.070 140.106 L 1154.479 140.106 L 1099.664 218.469 L 1044.850 140.106 L 1012.251 140.106 L 1082.768 241.219 L 999.219 359.628 L 1033.010 359.628 Z"
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


#include "cpu.h"
#include <math.h>
#include <stddef.h>

// Keep integer values within a specified range
static int clamp_int(int value, int min, int max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

// Keep RGB values between 0 and 255
static unsigned char clamp_byte(float value) {
    return (unsigned char)clamp_int(
        (int)lroundf(value), 0, 255
    );
}

// ============================================
// 1. CPU BRIGHTNESS ADJUSTMENT
// ============================================
void cpu_brightness(const unsigned char *input,
                    unsigned char *output,
                    int width, int height,
                    int brightness) {

    if (!input || !output ||
        width <= 0 || height <= 0)
        return;

    size_t total = (size_t)width * height * 3;

    for (size_t i = 0; i < total; i++) {
        float value = (float)input[i] + brightness;
        output[i] = clamp_byte(value);
    }
}

// ============================================
// 2. CPU NEAREST-NEIGHBOR RESIZING
// ============================================
void cpu_resize(const unsigned char *input,
                unsigned char *output,
                int src_width, int src_height,
                int dst_width, int dst_height) {

    if (!input || !output ||
        src_width <= 0 || src_height <= 0 ||
        dst_width <= 0 || dst_height <= 0)
        return;

    for (int y = 0; y < dst_height; y++) {
        for (int x = 0; x < dst_width; x++) {

            // Map output coordinates to input image
            float sx = (x + 0.5f) * src_width /
                       dst_width - 0.5f;

            float sy = (y + 0.5f) * src_height /
                       dst_height - 0.5f;

            // Round to nearest source pixel
            int src_x = clamp_int(
                (int)lroundf(sx), 0, src_width - 1
            );

            int src_y = clamp_int(
                (int)lroundf(sy), 0, src_height - 1
            );

            // Copy RGB channels
            for (int c = 0; c < 3; c++) {
                size_t src_index =
                    ((size_t)src_y * src_width +
                     src_x) * 3 + c;

                size_t dst_index =
                    ((size_t)y * dst_width +
                     x) * 3 + c;

                output[dst_index] = input[src_index];
            }
        }
    }
}

// ============================================
// 3. CPU 3x3 BOX BLUR
// ============================================
void cpu_blur(const unsigned char *input,
              unsigned char *output,
              int width, int height) {

    if (!input || !output ||
        width <= 0 || height <= 0)
        return;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            for (int c = 0; c < 3; c++) {

                float sum = 0.0f;

                // Visit all 9 neighboring positions
                for (int dy = -1; dy <= 1; dy++) {
                    for (int dx = -1; dx <= 1; dx++) {

                        // Clamp coordinates at borders
                        int nx = clamp_int(
                            x + dx, 0, width - 1
                        );

                        int ny = clamp_int(
                            y + dy, 0, height - 1
                        );

                        size_t index =
                            ((size_t)ny * width +
                             nx) * 3 + c;

                        sum += (float)input[index];
                    }
                }

                // Average 9 values and round
                size_t output_index =
                    ((size_t)y * width + x) * 3 + c;

                output[output_index] =
                    clamp_byte(sum / 9.0f);
            }
        }
    }
}

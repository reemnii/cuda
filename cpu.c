#include "cpu.h"

static unsigned char clamp_byte(int value) {
    if (value < 0) return 0;
    if (value > 255) return 255;
    return (unsigned char)value;
}

void cpu_brightness(const unsigned char *input, unsigned char *output,
                    int width, int height, int brightness) {
    if (!input || !output || width <= 0 || height <= 0) return;
    size_t count = (size_t)width * (size_t)height * 3u;
    for (size_t i = 0; i < count; ++i) {
        output[i] = clamp_byte((int)input[i] + brightness);
    }
}

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

            int src_x = (int)((long long)x * src_width / dst_width);
            int src_y = (int)((long long)y * src_height / dst_height);

            int src_index = (src_y * src_width + src_x) * 3;
            int dst_index = (y * dst_width + x) * 3;

            for (int c = 0; c < 3; c++) {
                output[dst_index + c] = input[src_index + c];
            }
        }
    }
}

void cpu_blur(const unsigned char *input,
              unsigned char *output,
              int width, int height) {

    if (!input || !output || width <= 0 || height <= 0)
        return;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            for (int c = 0; c < 3; c++) {

                int sum = 0;
                int count = 0;

                for (int dy = -1; dy <= 1; dy++) {
                    for (int dx = -1; dx <= 1; dx++) {

                        int nx = x + dx;
                        int ny = y + dy;

                        if (nx >= 0 && nx < width &&
                            ny >= 0 && ny < height) {

                            int index =
                                (ny * width + nx) * 3 + c;

                            sum += input[index];
                            count++;
                        }
                    }
                }

                int output_index =
                    (y * width + x) * 3 + c;

                output[output_index] =
                    (unsigned char)(sum / count);
            }
        }
    }
}

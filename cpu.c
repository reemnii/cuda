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
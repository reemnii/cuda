
#include <stdio.h>
#include <string.h>
#include "image_io.h"

int main(void) {
    unsigned char pixels[12] = {
        255, 0, 0,
        0, 255, 0,
        0, 0, 255,
        255, 255, 0
    };

    Image original = {2, 2, pixels};
    Image loaded = {0};

    if (!save_ppm("test_output.ppm", &original)) {
        printf("FAIL: Could not save PPM image.\n");
        return 1;
    }

    if (!load_ppm("test_output.ppm", &loaded)) {
        printf("FAIL: Could not load PPM image.\n");
        return 1;
    }

    int passed =
        loaded.width == original.width &&
        loaded.height == original.height &&
        memcmp(loaded.data, pixels, sizeof(pixels)) == 0;

    free_image(&loaded);

    if (passed) {
        printf("PASS: PPM image loading and saving match.\n");
        return 0;
    }

    printf("FAIL: Loaded image does not match original.\n");
    return 1;
}
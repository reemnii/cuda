
#include <stdio.h>
#include <string.h>
#include "cpu.h"

int main(void) {
    // Original image: 2x2 RGB pixels
    unsigned char input[12] = {
        255, 0, 0,     // Red
        0, 255, 0,     // Green
        0, 0, 255,     // Blue
        255, 255, 0    // Yellow
    };

    // Expected image after resizing to 4x4
    unsigned char expected[48] = {
        255,0,0, 255,0,0, 0,255,0, 0,255,0,
        255,0,0, 255,0,0, 0,255,0, 0,255,0,
        0,0,255, 0,0,255, 255,255,0, 255,255,0,
        0,0,255, 0,0,255, 255,255,0, 255,255,0
    };

    unsigned char output[48] = {0};

    cpu_resize(input, output, 2, 2, 4, 4);

    if (memcmp(output, expected, sizeof(expected)) == 0) {
        printf("PASS: CPU resize matches expected pixels.\n");
        return 0;
    }

    printf("FAIL: CPU resize output is incorrect.\n");

    for (int i = 0; i < 48; i++) {
        if (output[i] != expected[i]) {
            printf("Index %d: expected %u, got %u\n",
                   i, expected[i], output[i]);
        }
    }

    return 1;
}

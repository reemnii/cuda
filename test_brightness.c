
#include <stdio.h>
#include <string.h>
#include "cpu.h"

int main(void) {
    // 2x2 RGB image = 4 pixels = 12 values
    unsigned char input[12] = {
        50, 100, 150,
        200, 250, 255,
        0, 10, 20,
        120, 130, 140
    };

    unsigned char expected[12] = {
        60, 110, 160,
        210, 255, 255,
        10, 20, 30,
        130, 140, 150
    };

    unsigned char output[12] = {0};

    cpu_brightness(input, output, 2, 2, 10);

    if (memcmp(output, expected, sizeof(expected)) == 0) {
        printf("PASS: CPU brightness matches expected pixels.\n");
        return 0;
    }

    printf("FAIL: CPU brightness output is incorrect.\n");

    for (int i = 0; i < 12; i++) {
        if (output[i] != expected[i]) {
            printf("Index %d: expected %u, got %u\n",
                   i, expected[i], output[i]);
        }
    }

    return 1;
}

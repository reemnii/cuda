
#include <stdio.h>
#include <string.h>
#include "cpu.h"

int main(void) {
    // 3x3 RGB image
    unsigned char input[27] = {
        10,10,10, 20,20,20, 30,30,30,
        40,40,40, 50,50,50, 60,60,60,
        70,70,70, 80,80,80, 90,90,90
    };

    // Expected results using clamped borders
    // and nearest-integer rounding
    unsigned char expected[27] = {
        23,23,23, 30,30,30, 37,37,37,
        43,43,43, 50,50,50, 57,57,57,
        63,63,63, 70,70,70, 77,77,77
    };

    unsigned char output[27] = {0};

    cpu_blur(input, output, 3, 3);

    if (memcmp(output, expected, sizeof(expected)) == 0) {
        printf("PASS: CPU blur matches expected pixels.\n");
        return 0;
    }

    printf("FAIL: CPU blur output is incorrect.\n");

    for (int i = 0; i < 27; i++) {
        if (output[i] != expected[i]) {
            printf("Index %d: expected %u, got %u\n",
                   i, expected[i], output[i]);
        }
    }

    return 1;
}

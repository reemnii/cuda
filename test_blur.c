
#include <stdio.h>
#include <string.h>
#include "cpu.h"

int main(void) {
    // 3x3 RGB image, each pixel has equal R, G, B
    unsigned char input[27] = {
        10,10,10, 20,20,20, 30,30,30,
        40,40,40, 50,50,50, 60,60,60,
        70,70,70, 80,80,80, 90,90,90
    };

    // Expected result using valid-neighbor averaging
    unsigned char expected[27] = {
        30,30,30, 35,35,35, 40,40,40,
        45,45,45, 50,50,50, 55,55,55,
        60,60,60, 65,65,65, 70,70,70
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


#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <errno.h>
#include <limits.h>

#include "cpu.h"
#include "image_io.h"

#define RUNS 5

static double now_seconds(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

static int parse_int(const char *s, int *value) {
    char *end;
    errno = 0;
    long n = strtol(s, &end, 10);

    if (errno || s == end || *end != '\0' ||
        n < 1 || n > INT_MAX)
        return 0;

    *value = (int)n;
    return 1;
}

static int compare_double(const void *a, const void *b) {
    double x = *(const double *)a;
    double y = *(const double *)b;
    return (x > y) - (x < y);
}

int main(int argc, char *argv[]) {
    if (argc != 5) {
        fprintf(stderr,
            "Usage: %s input.ppm output.ppm width height\n",
            argv[0]);
        return 1;
    }

    int width, height;

    if (!parse_int(argv[3], &width) ||
        !parse_int(argv[4], &height) ||
        (size_t)width > SIZE_MAX / (size_t)height / 3) {
        fprintf(stderr, "Invalid output dimensions.\n");
        return 1;
    }

    size_t bytes = (size_t)width * height * 3;
    double times[RUNS];

    for (int run = 0; run < RUNS; run++) {
        Image input = {0};
        Image output = {0};

        unsigned char *resized = NULL;
        unsigned char *brightened = NULL;

        // Full application timing starts before loading.
        double start = now_seconds();

        if (!load_ppm(argv[1], &input)) {
            fprintf(stderr, "Failed to load image.\n");
            return 1;
        }

        resized = malloc(bytes);
        brightened = malloc(bytes);
        output.data = malloc(bytes);

        if (!resized || !brightened || !output.data) {
            fprintf(stderr, "Memory allocation failed.\n");
            free(resized);
            free(brightened);
            free_image(&input);
            free_image(&output);
            return 1;
        }

        output.width = width;
        output.height = height;

        // 1. Nearest-neighbor resize
        cpu_resize(input.data, resized,
                   input.width, input.height,
                   width, height);

        // 2. Brightness adjustment (+10)
        cpu_brightness(resized, brightened,
                       width, height, 10);

        // 3. 3x3 box blur
        cpu_blur(brightened, output.data,
                 width, height);

        // Save the processed image.
        if (!save_ppm(argv[2], &output)) {
            fprintf(stderr, "Failed to save image.\n");
            free(resized);
            free(brightened);
            free_image(&input);
            free_image(&output);
            return 1;
        }

        free(resized);
        free(brightened);
        free_image(&input);
        free_image(&output);

        double end = now_seconds();
        times[run] = end - start;

        printf("Run %d: %.9f seconds\n",
               run + 1, times[run]);
    }

    qsort(times, RUNS, sizeof(double), compare_double);

    double median = times[RUNS / 2];
    double megapixels = (double)width * height / 1e6;

    printf("\n=== CPU Benchmark Results ===\n");
    printf("Output resolution: %dx%d\n", width, height);
    printf("Median time: %.9f seconds\n", median);
    printf("Throughput: %.3f outputs/s\n", 1.0 / median);
    printf("Output throughput: %.3f MP/s\n",
           megapixels / median);
    printf("Input: %s\n", argv[1]);
    printf("Output: %s\n", argv[2]);

    return 0;
}

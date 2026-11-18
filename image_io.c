
#include "image_io.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>
#include <string.h>

static int read_token(FILE *file, char *buffer, size_t capacity) {
    int ch;
    size_t i = 0;

    do {
        ch = fgetc(file);
        if (ch == '#') {
            while (ch != '\n' && ch != EOF)
                ch = fgetc(file);
        }
    } while (ch != EOF && isspace((unsigned char)ch));

    if (ch == EOF)
        return 0;

    while (ch != EOF && !isspace((unsigned char)ch)) {
        if (i + 1 >= capacity)
            return 0;
        buffer[i++] = (char)ch;
        ch = fgetc(file);
    }

    buffer[i] = '\0';
    return i > 0;
}

static int read_int(FILE *file, int *value) {
    char token[64];
    char *end;
    long result;

    if (!read_token(file, token, sizeof(token)))
        return 0;

    result = strtol(token, &end, 10);
    if (*end != '\0' || result <= 0 || result > 2147483647L)
        return 0;

    *value = (int)result;
    return 1;
}

int load_ppm(const char *filename, Image *image) {
    if (!filename || !image)
        return 0;

    image->width = 0;
    image->height = 0;
    image->data = NULL;

    FILE *file = fopen(filename, "rb");
    if (!file)
        return 0;

    char magic[16];
    int width, height, maxval;

    if (!read_token(file, magic, sizeof(magic)) ||
        strcmp(magic, "P6") != 0 ||
        !read_int(file, &width) ||
        !read_int(file, &height) ||
        !read_int(file, &maxval) ||
        maxval != 255) {
        fclose(file);
        return 0;
    }

    if ((size_t)width > SIZE_MAX / (size_t)height / 3) {
        fclose(file);
        return 0;
    }

    size_t bytes = (size_t)width * height * 3;
    unsigned char *data = malloc(bytes);

    if (!data) {
        fclose(file);
        return 0;
    }

    if (fread(data, 1, bytes, file) != bytes) {
        free(data);
        fclose(file);
        return 0;
    }

    fclose(file);

    image->width = width;
    image->height = height;
    image->data = data;
    return 1;
}

int save_ppm(const char *filename, const Image *image) {
    if (!filename || !image || !image->data ||
        image->width <= 0 || image->height <= 0)
        return 0;

    if ((size_t)image->width >
        SIZE_MAX / (size_t)image->height / 3)
        return 0;

    FILE *file = fopen(filename, "wb");
    if (!file)
        return 0;

    size_t bytes = (size_t)image->width * image->height * 3;

    int ok = fprintf(file, "P6\n%d %d\n255\n",
                     image->width, image->height) > 0;

    if (ok)
        ok = fwrite(image->data, 1, bytes, file) == bytes;

    if (fclose(file) != 0)
        ok = 0;

    return ok;
}

void free_image(Image *image) {
    if (!image)
        return;

    free(image->data);
    image->data = NULL;
    image->width = 0;
    image->height = 0;
}

#ifndef IMAGE_IO_H
#define IMAGE_IO_H

typedef struct {
    int width;
    int height;
    unsigned char *data;
} Image;

int load_ppm(const char *filename, Image *image);
int save_ppm(const char *filename, const Image *image);
void free_image(Image *image);

#endif


#ifndef CPU_H
#define CPU_H

// Sequential CPU brightness adjustment
void cpu_brightness(const unsigned char *input,
                    unsigned char *output,
                    int width, int height,
                    int brightness);

// Sequential CPU nearest-neighbor resizing
void cpu_resize(const unsigned char *input,
                unsigned char *output,
                int src_width, int src_height,
                int dst_width, int dst_height);

// Sequential CPU 3x3 box blur
void cpu_blur(const unsigned char *input,
              unsigned char *output,
              int width, int height);

#endif

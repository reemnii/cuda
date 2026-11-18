
#ifndef CPU_H
#define CPU_H

#include <stddef.h>

void cpu_brightness(const unsigned char *input,
                    unsigned char *output,
                    int width, int height,
                    int brightness);


void cpu_resize(const unsigned char *input,
                unsigned char *output,
                int src_width, int src_height,
                int dst_width, int dst_height);

#endif

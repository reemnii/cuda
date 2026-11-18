#include "studio.h"
#include <string.h>
STUDIO_API const char *studio_implementation_language(void) { return "C17"; }
STUDIO_API int studio_brightness(const unsigned char *input, unsigned char *output, size_t count,
                                 int offset) {
    if (!input || !output || count > 30000000 || offset < -255 || offset > 255)
        return 1;
    for (size_t i = 0; i < count; ++i) {
        int value = input[i] + offset;
        output[i] = (unsigned char)(value < 0 ? 0 : value > 255 ? 255 : value);
    }
    return 0;
}
STUDIO_API int studio_process_rgb_v2(const unsigned char *input, unsigned char *output, int width,
                                     int height, const StudioOptions *o, size_t capacity, int *ow,
                                     int *oh) {
    if (!input || !output || !o || !ow || !oh)
        return 1;
    int w, h, status = studio_target_dimensions(width, height, o, &w, &h);
    if (status)
        return status;
    if (capacity < (size_t)w * h * 3)
        return 4;
    StudioImage image = {0};
    status = studio_image_process(input, width, height, o, 1, &image);
    if (status)
        return status;
    memcpy(output, image.pixels, (size_t)w * h * 3);
    *ow = w;
    *oh = h;
    studio_image_free(&image);
    return 0;
}
STUDIO_API int studio_process_rgb(const unsigned char *input, unsigned char *output, int width,
                                  int height, const StudioOptions *o, int *ow, int *oh) {
    int w, h, status = studio_target_dimensions(width, height, o, &w, &h);
    if (status)
        return status;
    return studio_process_rgb_v2(input, output, width, height, o, (size_t)w * h * 3, ow, oh);
}

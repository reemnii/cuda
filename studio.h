#ifndef STUDIO_H
#define STUDIO_H
#include <stddef.h>
#include <stdint.h>
#ifdef _WIN32
#define STUDIO_API __declspec(dllexport)
#else
#define STUDIO_API __attribute__((visibility("default")))
#endif
#ifdef __cplusplus
extern "C" {
#endif
/* All fields and exported layouts are stable across the C/Python boundary. */
typedef struct StudioOptions {
    int resize_width, resize_height, preserve_aspect, interpolation;
    int brightness, saturation, red, green, blue, intensity, filters[5];
} StudioOptions;
typedef struct StudioImage {
    int width, height;
    unsigned char *pixels;
} StudioImage;

StudioOptions studio_options_default(void);
int studio_target_dimensions(int,int,const StudioOptions*,int*,int*);
int studio_image_process(const unsigned char*,int,int,const StudioOptions*,int,StudioImage*);
void studio_image_free(StudioImage*);
double studio_clock_ms(void);
STUDIO_API const char *studio_implementation_language(void);
STUDIO_API int studio_brightness(const unsigned char*,unsigned char*,size_t,int);
STUDIO_API int studio_process_rgb_v2(const unsigned char*,unsigned char*,int,int,const StudioOptions*,size_t,int*,int*);
#ifdef __cplusplus
}
#else
_Static_assert(sizeof(StudioOptions)==60,"Options layout");
#endif
#endif

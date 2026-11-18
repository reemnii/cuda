/* ISO C17 pixel implementation. No C++ runtime or C++ source is linked. */
#include "studio.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <time.h>
#endif
static int clamp_int(int x, int lo, int hi) { return x < lo ? lo : x > hi ? hi : x; }
static float clamp_float(float x, float lo, float hi) { return x < lo ? lo : x > hi ? hi : x; }
static unsigned char clamp_byte(float value) {
    return (unsigned char)clamp_int((int)lroundf(value), 0, 255);
}
StudioOptions studio_options_default(void) {
    StudioOptions o = {0};
    o.preserve_aspect = 1;
    o.interpolation = 1;
    o.saturation = 100;
    o.intensity = 100;
    return o;
}
double studio_clock_ms(void) {
#ifdef _WIN32
    LARGE_INTEGER tick, frequency;
    QueryPerformanceCounter(&tick);
    QueryPerformanceFrequency(&frequency);
    return (double)tick.QuadPart * 1000.0 / (double)frequency.QuadPart;
#else
    struct timespec tick;
    clock_gettime(CLOCK_MONOTONIC, &tick);
    return tick.tv_sec * 1000.0 + tick.tv_nsec / 1e6;
#endif
}
int studio_target_dimensions(int width, int height, const StudioOptions *o, int *ow, int *oh) {
    if (!o || !ow || !oh || width < 1 || height < 1 || width > 10000 || height > 10000 ||
        (int64_t)width * height > 10000000)
        return 2;
    int nw = o->resize_width, nh = o->resize_height;
    if (nw < 0 || nh < 0 || nw > 10000 || nh > 10000 || o->preserve_aspect < 0 ||
        o->preserve_aspect > 1)
        return 2;
    if (!nw && !nh) {
        *ow = width;
        *oh = height;
        return 0;
    }
    if (o->preserve_aspect) {
        double scale = !nw   ? (double)nh / height
                       : !nh ? (double)nw / width
                             : fmin((double)nw / width, (double)nh / height);
        nw = clamp_int((int)lround(width * scale), 1, 100000000);
        nh = clamp_int((int)lround(height * scale), 1, 100000000);
    }
    if (nw < 1 || nh < 1 || (int64_t)nw * nh > 10000000)
        return 2;
    *ow = nw;
    *oh = nh;
    return 0;
}
static int valid(const StudioOptions *o, int workers) {
    if (!o || workers < 1 || workers > 32 || o->brightness < -255 || o->brightness > 255 ||
        o->saturation < 0 || o->saturation > 200 || o->intensity < 0 || o->intensity > 200 ||
        o->red < -255 || o->red > 255 || o->green < -255 || o->green > 255 || o->blue < -255 ||
        o->blue > 255 || o->interpolation < 0 || o->interpolation > 1)
        return 0;
    for (int i = 0; i < 5; ++i)
        if (o->filters[i] < 0 || o->filters[i] > 5)
            return 0;
    return 1;
}
static void resize_rgb(const unsigned char *src, int w, int h, StudioImage *out, int mode,
                       int workers) {
    int nw = out->width, nh = out->height;
    (void)workers;
    for (int y = 0; y < nh; ++y)
        for (int x = 0; x < nw; ++x) {
            float sx = (x + .5f) * w / nw - .5f, sy = (y + .5f) * h / nh - .5f;
            if (mode == 0) {
                int ix = clamp_int((int)lroundf(sx), 0, w - 1),
                    iy = clamp_int((int)lroundf(sy), 0, h - 1);
                for (int c = 0; c < 3; ++c)
                    out->pixels[(y * nw + x) * 3 + c] = src[(iy * w + ix) * 3 + c];
            } else {
                int xb = (int)floorf(sx), yb = (int)floorf(sy), x0 = clamp_int(xb, 0, w - 1),
                    x1 = clamp_int(xb + 1, 0, w - 1);
                int y0 = clamp_int(yb, 0, h - 1), y1 = clamp_int(yb + 1, 0, h - 1);
                float fx = clamp_float(sx - xb, 0.f, 1.f), fy = clamp_float(sy - yb, 0.f, 1.f);
                for (int c = 0; c < 3; ++c) {
                    float a =
                        src[(y0 * w + x0) * 3 + c] * (1 - fx) + src[(y0 * w + x1) * 3 + c] * fx;
                    float b =
                        src[(y1 * w + x0) * 3 + c] * (1 - fx) + src[(y1 * w + x1) * 3 + c] * fx;
                    out->pixels[(y * nw + x) * 3 + c] = clamp_byte(a * (1 - fy) + b * fy);
                }
            }
        }
}
static int filter_rgb(StudioImage *image, int code, int workers) {
    const float gaussian[] = {1, 2, 1, 2, 4, 2, 1, 2, 1},
                sharpen[] = {0, -1, 0, -1, 5, -1, 0, -1, 0};
    const float denoise[] = {1, 1, 1, 1, 1, 1, 1, 1, 1},
                edge[] = {-1, -1, -1, -1, 8, -1, -1, -1, -1};
    int w = image->width, h = image->height;
    (void)workers;
    if (code == 3) {
        for (int p = 0; p < w * h; ++p) {
            size_t i = (size_t)p * 3;
            unsigned char g = clamp_byte(.2126f * image->pixels[i] + .7152f * image->pixels[i + 1] +
                                         .0722f * image->pixels[i + 2]);
            image->pixels[i] = image->pixels[i + 1] = image->pixels[i + 2] = g;
        }
        return 0;
    }
    const float *kernel = code == 1 ? gaussian : code == 2 ? sharpen : code == 4 ? denoise : edge;
    float divisor = code == 1 ? 16.f : code == 4 ? 9.f : 1.f, bias = code == 5 ? 128.f : 0.f;
    size_t bytes = (size_t)w * h * 3;
    unsigned char *source = malloc(bytes);
    if (!source)
        return 5;
    memcpy(source, image->pixels, bytes);
    for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x)
            for (int c = 0; c < 3; ++c) {
                float sum = 0;
                for (int ky = -1; ky <= 1; ++ky)
                    for (int kx = -1; kx <= 1; ++kx) {
                        int px = clamp_int(x + kx, 0, w - 1), py = clamp_int(y + ky, 0, h - 1);
                        sum += source[(py * w + px) * 3 + c] * kernel[(ky + 1) * 3 + kx + 1];
                    }
                image->pixels[(y * w + x) * 3 + c] = clamp_byte(sum / divisor + bias);
            }
    free(source);
    return 0;
}
void studio_image_free(StudioImage *image) {
    if (image) {
        free(image->pixels);
        image->pixels = NULL;
        image->width = image->height = 0;
    }
}
int studio_image_process(const unsigned char *input, int width, int height, const StudioOptions *o,
                         int workers, StudioImage *result) {
    if (!result)
        return 1;
    memset(result, 0, sizeof(*result));
    if (!input || !valid(o, workers))
        return 2;
    int nw, nh, status = studio_target_dimensions(width, height, o, &nw, &nh);
    if (status)
        return status;
    StudioImage image = {nw, nh, malloc((size_t)nw * nh * 3)};
    if (!image.pixels)
        return 5;
    if (nw == width && nh == height)
        memcpy(image.pixels, input, (size_t)nw * nh * 3);
    else
        resize_rgb(input, width, height, &image, o->interpolation, workers);
    if (o->brightness || o->intensity != 100 || o->saturation != 100 || o->red || o->green ||
        o->blue) {
        for (int p = 0; p < nw * nh; ++p) {
            size_t i = (size_t)p * 3;
            float r = image.pixels[i], g = image.pixels[i + 1], b = image.pixels[i + 2];
            r = clamp_float((r - 128) * o->intensity / 100.f + 128 + o->brightness, 0.f, 255.f);
            g = clamp_float((g - 128) * o->intensity / 100.f + 128 + o->brightness, 0.f, 255.f);
            b = clamp_float((b - 128) * o->intensity / 100.f + 128 + o->brightness, 0.f, 255.f);
            float l = .2126f * r + .7152f * g + .0722f * b;
            image.pixels[i] = clamp_byte(l + (r - l) * o->saturation / 100.f + o->red);
            image.pixels[i + 1] = clamp_byte(l + (g - l) * o->saturation / 100.f + o->green);
            image.pixels[i + 2] = clamp_byte(l + (b - l) * o->saturation / 100.f + o->blue);
        }
    }
    for (int i = 0; i < 5 && o->filters[i]; ++i)
        if ((status = filter_rgb(&image, o->filters[i], workers))) {
            studio_image_free(&image);
            return status;
        }
    *result = image;
    return 0;
}

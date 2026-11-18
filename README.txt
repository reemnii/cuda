SEQUENTIAL C17 REFERENCE - students implement their own parallel versions.
image.c contains the serial pixel algorithms used by Studio. api.c is the buffer interface.
No completed Pthreads/OpenMP/CUDA/MPI implementation is supplied in this pack.

Build on Linux/WSL with an existing C compiler:
  cc -std=c17 -D_POSIX_C_SOURCE=200809L -O2 -ffp-contract=off image.c api.c reference.c -lm -o reference
Run:
  ./reference input.ppm output.ppm 10
Last argument: brightness (-255 to 255). Edit the recipe in reference.c to test resize/filters.
The runner accepts binary RGB PPM (P6). Convert PNG/JPEG using Pillow or your image library.
Studio's interface uses Pillow for PNG/JPEG/WebP codecs and the same C pixel algorithms.

Read studio_image_process first, then resize_rgb and filter_rgb. Images use packed RGB bytes.
Order: resize -> color -> ordered filters. Clamped edges; nearest-integer rounding; RGB 0-255.
Filters: 1 Gaussian, 2 sharpen, 3 grayscale, 4 box blur (denoise), 5 edge.
Measure the same inputs/recipe/timing scope in serial and parallel. Check outputs first.
Native app timings exclude codecs; a full-program comparison needs load-to-save times for BOTH versions.
Cite reused code/datasets in your report. You must implement and explain the assigned parallel API.

C++ alternative (same sequential algorithms):
  c++ -std=c++17 -O2 -ffp-contract=off image.cpp api.cpp reference.cpp -o reference
The course accepts C or C++. Projects 2/3 and the GPU kernels must be implemented by the group; use reference code for understanding/checking and cite reuse.

PROJECT-SPECIFIC RUNNER: GPU-based image processing
Create a folder named outputs, then run: reference outputs input.ppm [input2.ppm ...]
This program processes sources/variants sequentially and measures the full load-to-save interval. Edit the recipe to match your app experiment.

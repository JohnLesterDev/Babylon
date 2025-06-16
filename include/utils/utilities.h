#ifndef UTILITIES_H
#define UTILITIES_H

#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdint.h>


extern double total_time_elapsed;

#define VOID_PROFILED(printed, printer, printer_level, func, ...) \
  do { \
    uint64_t start = SDL_GetPerformanceCounter(); \
    func(__VA_ARGS__); \
    uint64_t end = SDL_GetPerformanceCounter(); \
    uint64_t freq = SDL_GetPerformanceFrequency(); \
    double duration = ((double)(end - start) / freq) * 1000.0; \
    total_time_elapsed += duration; \
    if (printed) printer(printer_level, __FILE__, "Function %s took %.3f ms to execute\n", #func, duration); \
  } while (0)

#define TYPE_PROFILED(printed, printer, printer_level, func, ...) \
({ \
  uint64_t start = SDL_GetPerformanceCounter(); \
  __typeof__(func(__VA_ARGS__)) result = func(__VA_ARGS__); \
  uint64_t end = SDL_GetPerformanceCounter(); \
  uint64_t freq = SDL_GetPerformanceFrequency(); \
  double duration = ((double)(end - start) / freq) * 1000.0; \
  total_time_elapsed += duration; \
  if (printed) printer(printer_level, __FILE__, "Function %s took %.3f ms to execute\n", #func, duration); \
  result; \
})

#define TOTAL_PROFILED(printer, printer_level, format) printer(printer_level, __FILE__, format, total_time_elapsed)\

char *argv_join(char **argv,const char *sep, int argc, int start);
char *path_join(const char *a, const char *b);

bool makedir(const char *path);
// bool makedirs(const char *root_path, ...);

#endif
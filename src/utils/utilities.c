#include <utils/utilities.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>
#include <SDL2/SDL.h>

#ifdef _WIN32
#include <windows.h>

#define MKDIR(path) CreateDirectory(path, NULL)
#define PATH_SEP '\\'
#else
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>

#define MKDIR(path) mkdir(path, 0755)
#define PATH_SEP '/'
#endif


double total_time_elapsed = 0.0;


/**
 * @brief Joins an array of strings into one string using a specified separator.
 *
 * @param argv The array of strings to join.
 * @param sep The separator to use.
 * @param argc The length of argv.
 * @param start The starting index of argv to start joining from.
 * @return The joined string. Must be carefully freed.
 */
char *argv_join(char **argv,const char *sep, int argc, int start) {
  if (argc == 0 || start < 0 || start >= argc) {
    char *empty = malloc(1);
    if (!empty) return NULL;
    empty[0] = '\0';
    return empty;
  }

  size_t sum_len = 0;
  size_t sep_len = strlen(sep);

  for (int i = start; i < argc; i++) {
    sum_len += strlen(argv[i]);

    if (i < argc - 1) {
      sum_len += sep_len;
    }
  }

  char *joined = calloc(sum_len + 1, sizeof(char));
  if (!joined) return NULL;

  char *dest = joined;
  for (int i = start; i < argc; i++) {
    size_t len = strlen(argv[i]);
    memcpy(dest, argv[i], len);
    dest += len;

    if (i < argc - 1) {
      memcpy(dest, sep, sep_len);
      dest += sep_len;
    }
  }

  return joined;
}

/**
 * @brief Join two paths together with the appropriate path separator.
 *
 * @param a The first path.
 * @param b The second path.
 * @return The joined path. Must be carefully freed.
 *
 * If either @a or @b is NULL, an error message is printed to stderr and NULL is returned.
 *
 * The join is performed by appending @b to @a with a path separator in between.
 * If @a ends with a path separator, no extra separator is added.
 * If @b starts with a path separator, that separator is ignored.
 *
 * The resulting string is allocated with malloc and must be freed when no longer needed.
 */
char *path_join(const char *a, const char *b) {
  if (a == NULL || b == NULL) {
      fprintf(stderr, "path_join error: Input path cannot be NULL.\n");
      return NULL;
  }

  size_t a_len = strlen(a);
  size_t b_len = strlen(b);

  bool add_sep = false;
  if (a_len > 0 && b_len > 0) {
      bool a_ends_with_sep = (a[a_len - 1] == PATH_SEP);
      bool b_starts_with_sep = (b[0] == PATH_SEP);
      if (!a_ends_with_sep && !b_starts_with_sep) {
          add_sep = true;
      }
  }

  size_t effective_b_len = b_len;
  size_t b_offset = 0;
  if (a_len > 0 && b_len > 0 && a[a_len - 1] == PATH_SEP && b[0] == PATH_SEP) {
      b_offset = 1;
      effective_b_len--;
  }

  size_t total_len = a_len + (add_sep ? 1 : 0) + effective_b_len + 1;
  char *result = malloc(total_len);
  if (!result) {
      perror("Failed to allocate memory for path_join");
      return NULL;
  }

  char *p = result;
  memcpy(p, a, a_len);
  p += a_len;

  if (add_sep) {
      *p = PATH_SEP;
      p++;
  }

  memcpy(p, b + b_offset, effective_b_len);
  p += effective_b_len;

  *p = '\0';

  return result;
}


bool makedir(const char *path) {
  if (MKDIR(path) == 0) {
    return true;
  } else {
    #ifdef _WIN32
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
      return false;
    }
    #else
    if (errno == EEXIST) {
      return false;
    }
    #endif
    return false;
  }
}

// bool makedirs(const char *root_path, ...) {

// }
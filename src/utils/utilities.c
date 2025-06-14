#include <utils/utilities.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>



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

char *path_join(const char *a, const char *b, const char *sep) {
  if (a == NULL || b == NULL) {
      fprintf(stderr, "path_join error: Input path cannot be NULL.\n");
      return NULL;
  }

  size_t a_len = strlen(a);
  size_t b_len = strlen(b);

  // Determine if a separator is needed
  // 'add_sep' will be true if 'a' does NOT end with a sep AND 'b' does NOT start with a sep
  bool add_sep = false;
  if (a_len > 0 && b_len > 0) { // Only consider adding a separator if both parts are non-empty
      bool a_ends_with_sep = (a[a_len - 1] == sep);
      bool b_starts_with_sep = (b[0] == sep);

      if (!a_ends_with_sep && !b_starts_with_sep) {
          add_sep = true; // Neither has a separator, so we need to add one
      }
      // If one of them has it, or both have it, we handle it by not adding an extra one
      // and potentially skipping 'b's leading separator if both have one.
  }
  
  // Calculate effective length of 'b' that will be copied
  size_t effective_b_len = b_len;
  size_t b_offset = 0;

  if (a_len > 0 && b_len > 0 && a[a_len - 1] == sep && b[0] == sep) {
      // Case: "dir1/" + "/dir2" -> "dir1/dir2" (remove leading slash of b)
      b_offset = 1;
      effective_b_len--;
  }

  // Calculate total length needed:
  // length of 'a' + (1 if add_sep is true) + effective length of 'b' + null terminator
  size_t total_len = a_len + (add_sep ? 1 : 0) + effective_b_len + 1;

  char *result = malloc(total_len);
  if (!result) {
      perror("Failed to allocate memory for path_join");
      return NULL;
  }

  char *p = result;

  // Copy 'a'
  memcpy(p, a, a_len);
  p += a_len;

  // Insert separator if needed
  if (add_sep) {
      *p = sep;
      p++;
  }

  // Copy 'b' with offset
  memcpy(p, b + b_offset, effective_b_len);
  p += effective_b_len;

  // Null-terminate
  *p = '\0';

  return result;
}
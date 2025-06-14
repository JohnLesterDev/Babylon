#define _POSIX_C_SOURCE 200809L

#include <engine/logger.h> // Your header first

// Standard C library headers
#include <time.h>   // For time(), localtime(), strftime()
#include <stdio.h>  // For FILE, fopen(), fclose(), fprintf(), stderr, stdout
#include <stdlib.h> // For malloc(), free(), exit() (if needed)
#include <stdarg.h> // For va_list, va_start, va_end, va_copy, vsnprintf
#include <string.h> // For strdup(), strlen()
#include <pthread.h> // For pthread_mutex functions

// Define reasonable buffer sizes for safety
#define MAX_LOG_MESSAGE_SIZE 1024
#define MAX_TIME_STRING_SIZE 64
#define MAX_FULL_LOG_LINE_SIZE (MAX_TIME_STRING_SIZE + 64 + MAX_LOG_MESSAGE_SIZE)

#define DEFAULT_LEVEL LOGGER_LEVEL_INFO
static const char *const DEFAULT_FORMAT_STR = "%s - [%s:%s]: %s";


static FILE *g_logfile_ptr = NULL;
static Logger_Level g_log_level = DEFAULT_LEVEL;
static char *g_log_format = NULL;
static pthread_mutex_t g_log_mutex = PTHREAD_MUTEX_INITIALIZER;
static int g_is_initialized = 0; // Use bool for clarity (true/false)


const char *log_level_name(Logger_Level level) {
  switch (level) {
    case LOGGER_LEVEL_DEBUG:
      return "DEBUG";
    case LOGGER_LEVEL_INFO:
      return "INFO";
    case LOGGER_LEVEL_WARN:
      return "WARN";
    case LOGGER_LEVEL_ERROR:
      return "ERROR";
    default:
      return "UNKNOWN";
  }
}

static char *format_string_from_var_args(const char *format, va_list args) {
  va_list args_copy;
  va_copy(args_copy, args);

  int needed_len = vsnprintf(NULL, 0, format, args_copy) + 1;
  va_end(args_copy);

  char *buffer = malloc(needed_len);
  if (buffer == NULL) {
    fprintf(stderr, "LOGGER ERROR: Failed to allocate memory for formatted message.\n");
    return NULL;
  }

  vsnprintf(buffer, needed_len, format, args);

  return buffer;
}

// This static function performs the essential one-time default setup
static void internal_logger_lazy_init(void) {
  // We need a lock around this critical section for true thread-safety during lazy init
  // (even though the mutex itself is static initialized, its first use must be protected)
  pthread_mutex_lock(&g_log_mutex); 

  if (g_is_initialized) {
    pthread_mutex_unlock(&g_log_mutex); // Unlock before returning
    return;
  }

  g_log_level = DEFAULT_LEVEL;
    
  g_log_format = strdup(DEFAULT_FORMAT_STR);
  if (g_log_format == NULL) {
      fprintf(stderr, "LOGGER CRITICAL: Failed to allocate default log format during lazy init. Using static literal fallback.\n");
      g_log_format = (char*)DEFAULT_FORMAT_STR; // Cast to char* to remove const for assignment
  }

  g_logfile_ptr = stdout; 
  
  g_is_initialized = 1; // Mark as initialized
  fprintf(stdout, "LOGGER INFO: Logger core lazily initialized (default to stdout).\n");

  pthread_mutex_unlock(&g_log_mutex); // Unlock after successful init
}


void Logger_Init(const char* filename, Logger_Level level, const char* format) {
  // Ensure the core lazy initialization has happened first.
  // This will only run once and set g_is_initialized to true.
  internal_logger_lazy_init();

  // Now, this function will primarily *re-configure* the logger.
  // We need a separate lock here because internal_logger_lazy_init has its own lock.
  // However, since internal_logger_lazy_init only acquires/releases its lock once,
  // we can treat this as the main configuration lock.
  pthread_mutex_lock(&g_log_mutex);

  // If Logger_Init is called multiple times for reconfiguration, log a warning.
  // (We've already established g_is_initialized is true by internal_logger_lazy_init())
  if (g_is_initialized && g_logfile_ptr != NULL) { // Check for proper initialization state
      fprintf(stdout, "LOGGER INFO: Logger re-configuring with new settings.\n");
  } else {
      // This branch should ideally not be hit if internal_logger_lazy_init works.
      // It handles an odd state where g_is_initialized is true but something is wrong.
      fprintf(stderr, "LOGGER WARNING: Logger_Init called in an unexpected state. Proceeding with configuration.\n");
  }


  g_log_level = level; // Update level

  // Manage g_log_format (strdup for dynamic ownership)
  // Free existing format ONLY if it's dynamic (not the DEFAULT_FORMAT_STR literal)
  if (g_log_format != NULL && g_log_format != DEFAULT_FORMAT_STR) {
      free(g_log_format);
      g_log_format = NULL; // Prevent dangling pointer
  }

  if (format != NULL) {
    g_log_format = strdup(format);
    if (g_log_format == NULL) {
        fprintf(stderr, "LOGGER ERROR: Failed to duplicate log format string. Reverting to default.\n");
        g_log_format = strdup(DEFAULT_FORMAT_STR); // Fallback to default
        if (g_log_format == NULL) {
             fprintf(stderr, "LOGGER CRITICAL: Failed to set any log format. Aborting.\n");
             pthread_mutex_unlock(&g_log_mutex);
             return;
        }
    }
  } else {
    // If format is NULL, revert to default format string
    g_log_format = strdup(DEFAULT_FORMAT_STR);
    if (g_log_format == NULL) {
        fprintf(stderr, "LOGGER CRITICAL: Failed to revert to default log format. Aborting.\n");
        pthread_mutex_unlock(&g_log_mutex);
        return;
    }
  }


  if (filename != NULL) {
    // Close existing file IF it's not stdout/stderr before opening a new one
    if (g_logfile_ptr != NULL && g_logfile_ptr != stdout && g_logfile_ptr != stderr) {
      fclose(g_logfile_ptr);
    }
    g_logfile_ptr = fopen(filename, "a");

    if (g_logfile_ptr == NULL) {
      fprintf(stderr, "LOGGER ERROR: Failed to open log file: %s. Defaulting to stdout.\n", filename);
      g_logfile_ptr = stdout;
    } else {
      fprintf(stdout, "LOGGER INFO: Log file opened: %s\n", filename);
    }
  } else {
    // If filename is NULL, ensure we're using stdout (close file if previously open)
    if (g_logfile_ptr != NULL && g_logfile_ptr != stdout && g_logfile_ptr != stderr) {
        fclose(g_logfile_ptr);
    }
    g_logfile_ptr = stdout;
  }

  // g_is_initialized remains true from internal_logger_lazy_init
  pthread_mutex_unlock(&g_log_mutex);
}

void Logger_SetLevel(Logger_Level level) {
  // Ensure lazy init first
  internal_logger_lazy_init();

  pthread_mutex_lock(&g_log_mutex);

  if (!g_is_initialized) { // Redundant check, but safe
      fprintf(stderr, "LOGGER ERROR: Logger not initialized. Cannot set level.\n");
      pthread_mutex_unlock(&g_log_mutex);
      return;
  }

  if (level < LOGGER_LEVEL_DEBUG || level > LOGGER_LEVEL_ERROR) {
    fprintf(stderr, "LOGGER ERROR: Invalid log level provided: %d. Level not changed.\n", level);
    pthread_mutex_unlock(&g_log_mutex);
    return;
  }

  fprintf(g_logfile_ptr, "LOGGER INFO: Log level set from %s to %s\n", log_level_name(g_log_level), log_level_name(level));
  g_log_level = level;

  pthread_mutex_unlock(&g_log_mutex);
}

void Logger_SetFormat(const char* format) {
  // Ensure lazy init first
  internal_logger_lazy_init();

  pthread_mutex_lock(&g_log_mutex);

  if (!g_is_initialized) { // Redundant check, but safe
      fprintf(stderr, "LOGGER ERROR: Logger not initialized. Cannot set format.\n");
      pthread_mutex_unlock(&g_log_mutex);
      return;
  }

  if (g_log_format != NULL && g_log_format != DEFAULT_FORMAT_STR) {
      free(g_log_format);
      g_log_format = NULL; // Prevent dangling pointer
  }

  if (format != NULL) {
    g_log_format = strdup(format);
    if (g_log_format == NULL) {
        fprintf(stderr, "LOGGER ERROR: Failed to duplicate new log format string. Keeping old format.\n");
        g_log_format = strdup(DEFAULT_FORMAT_STR);
        if (g_log_format == NULL) {
             fprintf(stderr, "LOGGER CRITICAL: Failed to set any log format. Aborting.\n");
             pthread_mutex_unlock(&g_log_mutex);
             return;
        }
    }
  } else {
    g_log_format = strdup(DEFAULT_FORMAT_STR);
    if (g_log_format == NULL) {
        fprintf(stderr, "LOGGER CRITICAL: Failed to revert to default log format. Aborting.\n");
        pthread_mutex_unlock(&g_log_mutex);
        return;
    }
  }

  fprintf(g_logfile_ptr, "LOGGER INFO: Log format set to: %s\n", g_log_format);

  pthread_mutex_unlock(&g_log_mutex);
}


void Logger_Destroy(void) {
  pthread_mutex_lock(&g_log_mutex);

  if (!g_is_initialized) {
      pthread_mutex_unlock(&g_log_mutex);
      return;
  }

  if (g_logfile_ptr != NULL && g_logfile_ptr != stdout && g_logfile_ptr != stderr) {
      fprintf(g_logfile_ptr, "LOGGER INFO: Logger shutting down...\n");
      fflush(g_logfile_ptr);
      fclose(g_logfile_ptr);
  }
  g_logfile_ptr = NULL;

  if (g_log_format != NULL && g_log_format != DEFAULT_FORMAT_STR) {
      free(g_log_format);
  }
  g_log_format = NULL;

  pthread_mutex_unlock(&g_log_mutex);
  pthread_mutex_destroy(&g_log_mutex); // Only destroy the mutex ONCE here!

  g_is_initialized = 0; // Reset initialization flag
}

void Logger_RootLog(Logger_Level level, const char* file, const char* format, ...) {
  // Ensure core lazy initialization has happened first.
  internal_logger_lazy_init();

  // Check against global log level FIRST, outside of lock for performance
  if (level < g_log_level) {
    return;
  }

  // Only lock if we're actually going to log something
  pthread_mutex_lock(&g_log_mutex);

  // Critical check for initialization after lock (should always be true after internal_logger_lazy_init)
  if (!g_is_initialized || g_logfile_ptr == NULL) {
    fprintf(stderr, "LOGGER ERROR: Logger not ready to log (initialization state issue).\n");
    pthread_mutex_unlock(&g_log_mutex);
    return;
  }

  va_list args;
  va_start(args, format);

  char *formatted_user_message = format_string_from_var_args(format, args);
  va_end(args);

  if (formatted_user_message == NULL) {
    pthread_mutex_unlock(&g_log_mutex);
    return;
  }

  time_t t = time(NULL);
  struct tm *tm = localtime(&t);
  char time_str[MAX_TIME_STRING_SIZE];
  strftime(time_str, sizeof(time_str), "%H:%M:%S", tm);

  const char *level_name = log_level_name(level);

  char final_log_line[MAX_FULL_LOG_LINE_SIZE];
  int printed_len = snprintf(final_log_line, sizeof(final_log_line),
                             g_log_format,
                             time_str,
                             level_name,
                             file,
                             formatted_user_message
                            );

  if ((size_t)printed_len >= sizeof(final_log_line)) {
      fprintf(stderr, "LOGGER WARNING: Log line truncated. Increase MAX_FULL_LOG_LINE_SIZE.\n");
  }

  fprintf(g_logfile_ptr, "%s", final_log_line);
  fflush(g_logfile_ptr);

  free(formatted_user_message);
  formatted_user_message = NULL;

  pthread_mutex_unlock(&g_log_mutex);
  // REMOVED: pthread_mutex_destroy(&g_log_mutex); // THIS WAS THE PROBLEM!
  // REMOVED: g_is_initialized = 0; // THIS WAS ALSO A PROBLEM!
}
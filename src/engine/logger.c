#define _POSIX_C_SOURCE 200809L

#include <engine/logger.h> 

#include <time.h>   
#include <stdio.h>  
#include <stdlib.h> 
#include <stdarg.h> 
#include <string.h> 
#include <stdbool.h>
#include <pthread.h> 

#define MAX_LOG_MESSAGE_SIZE 1024
#define MAX_TIME_STRING_SIZE 64
#define MAX_FULL_LOG_LINE_SIZE (MAX_TIME_STRING_SIZE + 64 + MAX_LOG_MESSAGE_SIZE)

#define DEFAULT_LEVEL LOGGER_LEVEL_INFO
static const char *const DEFAULT_FORMAT_STR = "%s - [%s:%s]: %s";


static FILE *g_console_ptr = NULL;
static FILE *g_logfile_ptr = NULL;
static Logger_Level g_log_level = DEFAULT_LEVEL;
static char *g_log_format = NULL;
static pthread_mutex_t g_log_mutex = PTHREAD_MUTEX_INITIALIZER;
static bool g_is_initialized = false;

static void internal_logger_lazy_init(void);

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

bool Logger_IsFullyInitialized(void) {
  return (g_is_initialized && g_console_ptr != NULL && g_logfile_ptr != NULL);
}

static void internal_logger_lazy_init(void) {
  pthread_mutex_lock(&g_log_mutex); 

  if (g_is_initialized) {
    pthread_mutex_unlock(&g_log_mutex); 
    return;
  }

  g_log_level = DEFAULT_LEVEL;
    
  g_log_format = strdup(DEFAULT_FORMAT_STR);
  if (g_log_format == NULL) {
      fprintf(stderr, "LOGGER CRITICAL: Failed to allocate default log format during lazy init. Using static literal fallback.\n");
      g_log_format = (char*)DEFAULT_FORMAT_STR; 
  }

  g_console_ptr = stdout;
  g_logfile_ptr = NULL;  

  g_is_initialized = true; 
  fprintf(stdout, "LOGGER INFO: Logger core lazily initialized (default to stdout).\n");

  pthread_mutex_unlock(&g_log_mutex);
}


void Logger_Init(const FILE *stream, const char* filename, Logger_Level level, const char* format) {
  internal_logger_lazy_init(); 
  pthread_mutex_lock(&g_log_mutex);

  if (g_is_initialized && g_console_ptr != NULL) {
      fprintf(g_console_ptr, "LOGGER INFO: Logger re-configuring with new settings.\n");
  } else {
      fprintf(stderr, "LOGGER WARNING: Logger_Init called in an unexpected state. Proceeding with configuration.\n");
  }

  if (level < LOGGER_LEVEL_DEBUG || level > LOGGER_LEVEL_ERROR) {
    fprintf(stderr, "LOGGER ERROR: Invalid log level provided: %d. Reverting to default.\n", level);
    g_log_level = DEFAULT_LEVEL; 
  } else {
    g_log_level = level; 
  }

  if (stream != NULL && (stream == stdout || stream == stderr)) {
    g_console_ptr = (FILE *)stream;
  } else {
    g_console_ptr = stdout;
    if (stream != NULL) { 
        fprintf(stderr, "LOGGER WARNING: Invalid console stream provided. Defaulting to stdout.\n");
    }
  }


  if (g_log_format != NULL && g_log_format != DEFAULT_FORMAT_STR) {
      free(g_log_format);
  }
  g_log_format = NULL; 

  if (format != NULL) {
    g_log_format = strdup(format);
    if (g_log_format == NULL) {
        fprintf(stderr, "LOGGER ERROR: Failed to duplicate provided log format. Reverting to default.\n");

        g_log_format = strdup(DEFAULT_FORMAT_STR);
        if (g_log_format == NULL) {
             fprintf(stderr, "LOGGER CRITICAL: Failed to allocate default log format. Using static literal fallback.\n");
             g_log_format = (char*)DEFAULT_FORMAT_STR;
        }
    }
  } else {
    g_log_format = strdup(DEFAULT_FORMAT_STR);
    if (g_log_format == NULL) {
        fprintf(stderr, "LOGGER CRITICAL: Failed to allocate default log format. Using static literal fallback.\n");
        g_log_format = (char*)DEFAULT_FORMAT_STR;
    }
  }


  if (g_logfile_ptr != NULL && g_logfile_ptr != stdout && g_logfile_ptr != stderr) {
      fclose(g_logfile_ptr);
  }
  g_logfile_ptr = NULL; 

  if (filename != NULL) {
    g_logfile_ptr = fopen(filename, "a");
    
    if (g_logfile_ptr == NULL) {
      fprintf(stderr, "LOGGER ERROR: Failed to open log file: %s. File logging is disabled.\n", filename);

    } else {
      if (g_console_ptr != NULL) {
          fprintf(g_console_ptr, "LOGGER INFO: Log file opened: %s\n", filename);
      }
    }
  } else {
    g_logfile_ptr = NULL; 
  }

  pthread_mutex_unlock(&g_log_mutex);
}

void Logger_SetLevel(Logger_Level level) {
  internal_logger_lazy_init();
  pthread_mutex_lock(&g_log_mutex);

  if (level < LOGGER_LEVEL_DEBUG || level > LOGGER_LEVEL_ERROR) {
    fprintf(stderr, "LOGGER ERROR: Invalid log level provided: %d. Level not changed.\n", level);
    pthread_mutex_unlock(&g_log_mutex);
    return;
  }

  if (g_console_ptr != NULL) {
      fprintf(g_console_ptr, "LOGGER INFO: Log level set from %s to %s\n", log_level_name(g_log_level), log_level_name(level));
  } else {
      fprintf(stderr, "LOGGER INFO: Log level set from %s to %s\n", log_level_name(g_log_level), log_level_name(level));
  }
  g_log_level = level;

  pthread_mutex_unlock(&g_log_mutex);
}

void Logger_SetFormat(const char* format) {
  internal_logger_lazy_init();
  pthread_mutex_lock(&g_log_mutex);

  if (g_log_format != NULL && g_log_format != DEFAULT_FORMAT_STR) {
      free(g_log_format);
  }
  g_log_format = NULL; 

  if (format != NULL) {
    g_log_format = strdup(format);
    if (g_log_format == NULL) {
        fprintf(stderr, "LOGGER ERROR: Failed to duplicate new log format string. Reverting to default.\n");
        g_log_format = strdup(DEFAULT_FORMAT_STR);
        if (g_log_format == NULL) {
             fprintf(stderr, "LOGGER CRITICAL: Failed to set any log format. Using static literal fallback.\n");
             g_log_format = (char*)DEFAULT_FORMAT_STR;
        }
    }
  } else {
    g_log_format = strdup(DEFAULT_FORMAT_STR);
    if (g_log_format == NULL) {
        fprintf(stderr, "LOGGER CRITICAL: Failed to revert to default log format. Using static literal fallback.\n");
        g_log_format = (char*)DEFAULT_FORMAT_STR; 
    }
  }

  if (g_console_ptr != NULL) {
      fprintf(g_console_ptr, "LOGGER INFO: Log format set to: %s\n", g_log_format);
  } else {
      fprintf(stderr, "LOGGER INFO: Log format set to: %s\n", g_log_format);
  }

  pthread_mutex_unlock(&g_log_mutex);
}


void Logger_Destroy(void) {
  pthread_mutex_lock(&g_log_mutex);

  if (!g_is_initialized) {
      pthread_mutex_unlock(&g_log_mutex);
      return;
  }

  if (g_logfile_ptr != NULL && g_logfile_ptr != stdout && g_logfile_ptr != stderr) {
      fprintf(g_logfile_ptr, "LOGGER INFO: Logger shutting down file log...\n");
      fflush(g_logfile_ptr);
      fclose(g_logfile_ptr);
  }
  g_logfile_ptr = NULL; 

  if (g_console_ptr != NULL) {
      fprintf(g_console_ptr, "LOGGER INFO: Logger shutting down console stream...\n");
      fflush(g_console_ptr);
  }
  g_console_ptr = NULL;


  if (g_log_format != NULL && g_log_format != DEFAULT_FORMAT_STR) {
      free(g_log_format);
  }
  g_log_format = NULL;

  pthread_mutex_unlock(&g_log_mutex);
  pthread_mutex_destroy(&g_log_mutex); 

  g_is_initialized = false;
}

void Logger_RootLog(Logger_Level level, const char* file, const char* format, ...) {
  internal_logger_lazy_init();

  if (level < g_log_level) {
    return;
  }

  pthread_mutex_lock(&g_log_mutex);

  if (!g_is_initialized || (g_console_ptr == NULL && g_logfile_ptr == NULL)) {
    fprintf(stderr, "LOGGER ERROR: Logger not ready to log (no active output streams).\n");
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

  if (g_console_ptr != NULL) {
    fprintf(g_console_ptr, "%s", final_log_line);
    fflush(g_console_ptr);
  }

  if (g_logfile_ptr != NULL) {
    fprintf(g_logfile_ptr, "%s", final_log_line);
    fflush(g_logfile_ptr);
  }

  free(formatted_user_message);
  formatted_user_message = NULL;

  pthread_mutex_unlock(&g_log_mutex);
}
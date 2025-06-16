#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>

typedef enum {
  LOGGER_LEVEL_DEBUG,
  LOGGER_LEVEL_INFO,
  LOGGER_LEVEL_WARN,
  LOGGER_LEVEL_ERROR
} Logger_Level;

// Logger initialization, setting level, setting format, and destruction
// Added 'format' to Init, changed 'level' to be optional as a value, not a pointer.
void Logger_Init(const FILE *stream, const char* filename, Logger_Level level, const char* format);
void Logger_SetLevel(Logger_Level level);
void Logger_SetFormat(const char* format); // New function, needs a char* not const char* potentially
void Logger_Destroy(void);

bool Logger_IsFullyInitialized(void);

// Internal helper for getting level name (optional to expose)
const char *log_level_name(Logger_Level level);

// Internal core log function that takes the formatted message (char*)
// This will be called by Logger_RootLog
void Logger_RootLog_Core(Logger_Level level, const char* file, const char* final_message);

// The primary variadic log function, which prepares the message
void Logger_RootLog(Logger_Level level, const char* file, const char* format, ...);

// Public facing macros that inject __FILE__ and variadic arguments
#define LOGGER_DEBUG(format, ...) Logger_RootLog(LOGGER_LEVEL_DEBUG, __FILE__, format, ##__VA_ARGS__)
#define LOGGER_INFO(format, ...) Logger_RootLog(LOGGER_LEVEL_INFO, __FILE__, format, ##__VA_ARGS__)
#define LOGGER_WARN(format, ...) Logger_RootLog(LOGGER_LEVEL_WARN, __FILE__, format, ##__VA_ARGS__)
#define LOGGER_ERROR(format, ...) Logger_RootLog(LOGGER_LEVEL_ERROR, __FILE__, format, ##__VA_ARGS__)

#endif
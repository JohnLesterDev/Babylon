#include <engine/constants.h>
#include <utils/utilities.h>
#include <engine/logger.h>
#include <game/game.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define COMMAND(name, return_type, ...) return_type name(int argc, char **argv, ##__VA_ARGS__)


COMMAND(CMD_Help, bool) {
  bool command_hit = false;

  const char *help_text =
  "Usage: babylon [options]\n"
  "Description: A game engine written by JohnLesterDev\n"
  "\n"
  "Options:\n"
  "  -h, --help     Display this help message\n"
  "  -v, --version  Display the version information\n"
  "\n"
  "Written by JohnLesterDev, and built for x86_64-pc-linux-gnu\n"
  ;

  for (int i = 0; i < argc; i++) {
    if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
      printf("%s", help_text);
      command_hit = true;
      break;
    }
  }

  return command_hit;
}

COMMAND(CMD_Version, bool) {
  bool command_hit = false;

  const char *version_text = 
  GAME_NAME " v" GAME_VERSION 
  "\nWritten by: " GAME_AUTHOR "\n"
  ;
  
  for (int i = 0; i < argc; i++) {
    if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0) {
      printf("%s", version_text);
      command_hit = true;
      break;
    }
  }

  return command_hit;
}


int main(int argc, char **argv) {
  VOID_PROFILED(false, Logger_RootLog, LOGGER_LEVEL_INFO, Constants_InitPaths);

  if (!TYPE_PROFILED((_Bool)true, Logger_RootLog, LOGGER_LEVEL_INFO, Logger_IsFullyInitialized)) {
    VOID_PROFILED(false, Logger_RootLog, LOGGER_LEVEL_INFO, Logger_Init, stdout, path_join(GAME_ROOT_PATH, ".log"), LOGGER_LEVEL_INFO, NULL);
  }

  if (CMD_Help(argc, argv) || CMD_Version(argc, argv)) return 0;
  
  Game *game = NULL;

  if (!TYPE_PROFILED((_Bool)true, Logger_RootLog, LOGGER_LEVEL_INFO, Game_Init, &game)) {
    LOGGER_ERROR("Failed to initialize game\n");
    return -1;
  }

  Game_Run(game);
  Game_Destroy(game);
  Constants_DestroyPaths();
  Logger_Destroy();

  return 0;
}

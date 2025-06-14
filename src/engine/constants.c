#include <engine/constants.h>
#include <utils/utilities.h>
#include <engine/logger.h>

#include <SDL2/SDL.h>

char *GAME_ROOT_PATH = NULL;
char *GAME_CONFIG_PATH = NULL;

void Constants_InitPaths(void) {
  // IMPORTANT: Ensure SDL is initialized before calling SDL_GetPrefPath!
  // Example: if (SDL_Init(0) < 0) { /* handle error */ }

  // 1. Get the root preference path
  GAME_ROOT_PATH = SDL_GetPrefPath(GAME_AUTHOR, GAME_NAME);
  if (GAME_ROOT_PATH == NULL) {
      fprintf(stderr, "ERROR: Failed to get game root path: %s\n", SDL_GetError());
      // Handle error: perhaps exit, or default to current directory
      return; 
  }

  // 2. Join the root path with "config"
  // Use path_join correctly with two arguments
  GAME_CONFIG_PATH = path_join(GAME_ROOT_PATH, "config", "/");
  if (GAME_CONFIG_PATH == NULL) {
      perror("ERROR: Failed to join game config path");
      // Handle error: Free GAME_ROOT_PATH if GAME_CONFIG_PATH fails, then exit or fallback
      SDL_free(GAME_ROOT_PATH); // Clean up the first allocated path
      GAME_ROOT_PATH = NULL;
      return;
  }

  fprintf(stdout, "INFO: Game Root Path: %s\n", GAME_ROOT_PATH);
  fprintf(stdout, "INFO: Game Config Path: %s\n", GAME_CONFIG_PATH);
}

void Constants_DestroyPaths(void) {
  // Free memory allocated by SDL_GetPrefPath
  if (GAME_ROOT_PATH != NULL) {
      SDL_free(GAME_ROOT_PATH); // Use SDL_free for paths allocated by SDL
      GAME_ROOT_PATH = NULL;
  }

  // Free memory allocated by path_join
  if (GAME_CONFIG_PATH != NULL) {
      free(GAME_CONFIG_PATH); // Use standard free for paths allocated by your path_join
      GAME_CONFIG_PATH = NULL;
  }

  fprintf(stdout, "INFO: Game paths deallocated.\n");
}

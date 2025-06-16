#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <SDL2/SDL.h>

#define GAME_NAME "Babylon"
#define GAME_VERSION "0.1.1"
#define GAME_AUTHOR "JohnLesterDev"

extern char *GAME_ROOT_PATH;
extern char *GAME_CONFIG_PATH;

void Constants_InitPaths(void);
void Constants_DestroyPaths(void);

#endif
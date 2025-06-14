// #include "game/game.h"
#include <stdio.h>
#include <stdbool.h>

#define SANITIZE()
#define COMMAND(name) void name(int argc, char **argv)

COMMAND(CMD_Help) {

}


int main(int argc, char **argv) {
  for (int i = 0; i < argc; i++) {
    printf("%s\n", argv[i]);
  }
  // Game *game = NULL;

  // if (!Game_Init(&game)) {
  //     fprintf(stderr, "Failed to initialize game\n");
  //     return -1;
  // }

  // Game_Run(game);
  // Game_Destroy(game);

  return 0;
}

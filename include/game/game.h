#ifndef GAME_H
#define GAME_H

typedef struct Game Game;

Game* Game_Init(Game** game);
void Game_Destroy(Game* game);
void Game_Run(Game* game);
void Game_Test(void);

#endif
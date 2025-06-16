#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <SDL2/SDL.h>

#include <game/game.h>
#include <engine/logger.h>
#include <utils/utilities.h>


struct Game {
    bool running;
    SDL_Window* window;
    SDL_Renderer* renderer;
};

static Game* Game_Create() {
    Game* game = malloc(sizeof(Game));
    if (!game) return NULL;

    game->running = true;
    game->window = NULL;
    game->renderer = NULL;

    return game;
}

Game* Game_Init(Game** game) {
  LOGGER_INFO("Initializing game...\n");
    if (*game == NULL) {
        *game = Game_Create();
        if (!*game) return NULL;
    }

    if (SDL_Init(
        SDL_INIT_VIDEO |
        SDL_INIT_TIMER |
        SDL_INIT_EVENTS
        ) != 0) {
        LOGGER_ERROR("SDL_Init Error: %s\n", SDL_GetError());
        return NULL;
    }

    if ((*game)->window == NULL) {
        (*game)->window = SDL_CreateWindow("SDL2 Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_SHOWN);
        if (!(*game)->window) {
            LOGGER_ERROR("SDL_CreateWindow Error: %s\n", SDL_GetError());
            SDL_Quit();
            return NULL;
        }
    }

    if ((*game)->renderer == NULL) {
        (*game)->renderer = SDL_CreateRenderer((*game)->window, -1, 0);
        if (!(*game)->renderer) {
            LOGGER_ERROR("SDL_CreateRenderer Error: %s\n", SDL_GetError());
            SDL_DestroyWindow((*game)->window);
            SDL_Quit();
            return NULL;
        }
    }
    
    LOGGER_INFO("Game initialized.\n");
    return *game;
}

void Game_Destroy(Game* game) {
    LOGGER_INFO("Destroying game...\n");
    if (!game) return;

    if (game->renderer) SDL_DestroyRenderer(game->renderer);
    if (game->window) SDL_DestroyWindow(game->window);

    free(game);
    SDL_Quit();
}

void Game_Run(Game* game) {
    TOTAL_PROFILED(Logger_RootLog, LOGGER_LEVEL_INFO, "ALL took %.3f ms. Starting game loop...\n");
    SDL_Event event;

    while (game->running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) game->running = 0;
        }

        SDL_SetRenderDrawColor(game->renderer, 0, 0, 0, 255);
        SDL_RenderClear(game->renderer);
        SDL_RenderPresent(game->renderer);
    }
}

void Game_Test(void) {
  
}
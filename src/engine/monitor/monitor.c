#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <engine/monitor/monitor.h>

struct Monitor_Info {
    int id;
    char* name;
    SDL_Rect bounds;
    int has_bounds;
};

Monitor_Info* Monitor_GetAllMonitors(int* count_out) {
    if (SDL_WasInit(SDL_INIT_VIDEO) == 0) {
        if (SDL_InitSubSystem(SDL_INIT_VIDEO) != 0) {
            fprintf(stderr, "Failed to initialize SDL video: %s\n", SDL_GetError());
            *count_out = 0;
            return NULL;
        }
    }

    int monitor_count = SDL_GetNumVideoDisplays();
    if (monitor_count < 0) {
        fprintf(stderr, "No monitors found: %s\n", SDL_GetError());
        *count_out = 0;
        return NULL;
    }

    Monitor_Info* monitors = calloc(monitor_count, sizeof(Monitor_Info));
    if (!monitors) {
        fprintf(stderr, "Failed to allocate memory for monitors\n");
        *count_out = 0;
        return NULL;
    }

    for (int i = 0; i < monitor_count; i++) {
        const char*  name = SDL_GetDisplayName(i);
        
        monitors[i].id = i;

        if (name) {
            monitors[i].name = strdup(name);
        } else {
            char fallback_name[32];
            snprintf(fallback_name, sizeof(fallback_name), "Monitor %d", i);
            monitors[i].name = strdup(fallback_name);
        }

        if (SDL_GetDisplayBounds(i, &monitors[i].bounds) == 0) {
            monitors[i].has_bounds = 1;
        } else {
            monitors[i].has_bounds = 0;
        }
    }

    *count_out = monitor_count;
    return monitors;
}

void Monitor_FreeMonitors(Monitor_Info* monitors, int count) {
    if (!monitors) return;

    for (int i = 0; i < count; i++) {
        free(monitors[i].name);
    }
    free(monitors);
}
#ifndef MONITOR_H
#define MONITOR_H

#include <SDL2/SDL.h>


typedef struct Monitor_Info Monitor_Info;

Monitor_Info* Monitor_GetAllMonitors(int* count_out);
void Monitor_FreeMonitors(Monitor_Info* monitors, int count);

#endif
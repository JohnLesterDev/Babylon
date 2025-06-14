#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <engine/config/config.h>

Config_Map* Config_CreateNewMap(void);
void Config_DestroyMap(Config_Map* config_map);

void Config_AddMapValue(Config_Map* config_map, const char* key, Config config);
void Config_UpdateMapValue(Config_Map* config_map, const char* key, Config config);
Config* Config_GetMapValue(Config_Map* config_map, const char* key);

void Config_UpdateConfigValue(Config* config, Config_Value value);

Config_Map* Config_LoadConfigFile(const char* path);
void Config_WriteConfigFile(Config_Map* config_map, const char* path);

#endif
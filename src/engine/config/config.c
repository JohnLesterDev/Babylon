#include <stdbool.h>

#include <engine/config/config.h>

enum Config_Type {
    CONFIG_TYPE_BOOL,
    CONFIG_TYPE_INT,
    CONFIG_TYPE_FLOAT,
    CONFIG_TYPE_STRING
};

union Config_Value {
    bool bool_value;
    int int_value;
    float float_value;
    char* string_value;
};

struct Config {
    Config_Type type;
    Config_Value value;
};

struct Config_Map {
    char* key;
    Config config;
};


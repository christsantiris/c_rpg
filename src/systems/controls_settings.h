#ifndef CONTROLS_SETTINGS_H
#define CONTROLS_SETTINGS_H

#include "../game/controls.h"

#define CONTROLS_SETTINGS_PATH "saves/controls.json"

// Failed reads leave the supplied bindings unchanged.
int controls_settings_load(int bindings[CONTROL_COUNT], const char *path);
int controls_settings_save(const int bindings[CONTROL_COUNT], const char *path);

#endif

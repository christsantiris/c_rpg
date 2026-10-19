#include "controls_settings.h"
#include "cJSON.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

int controls_settings_load(int bindings[CONTROL_COUNT], const char *path) {
    FILE *file = fopen(path, "r");
    if (!file) {
        return 0;
    }
    char text[4096];
    size_t length = fread(text, 1, sizeof(text) - 1, file);
    int readable = !ferror(file) && feof(file);
    fclose(file);
    if (!readable) {
        return 0;
    }
    text[length] = '\0';
    cJSON *root = cJSON_ParseWithOpts(text, NULL, 1);
    int saved[CONTROL_COUNT];
    int valid = cJSON_IsArray(root) && cJSON_GetArraySize(root) == CONTROL_COUNT;
    for (int i = 0; valid && i < CONTROL_COUNT; i++) {
        cJSON *key = cJSON_GetArrayItem(root, i);
        valid = cJSON_IsNumber(key) && key->valuedouble == key->valueint;
        saved[i] = valid ? key->valueint : 0;
    }
    if (valid && controls_valid(saved)) {
        memcpy(bindings, saved, sizeof(saved));
    } else {
        valid = 0;
    }
    cJSON_Delete(root);
    return valid;
}

int controls_settings_save(const int bindings[CONTROL_COUNT], const char *path) {
    if (!controls_valid(bindings)) {
        return 0;
    }
    char temporary[512];
    int length = snprintf(temporary, sizeof(temporary), "%s.tmp", path);
    if (length < 0 || length >= (int)sizeof(temporary)) {
        return 0;
    }
    if (strcmp(path, CONTROLS_SETTINGS_PATH) == 0) {
        mkdir("saves", 0755);
    }
    cJSON *root = cJSON_CreateIntArray(bindings, CONTROL_COUNT);
    char *text = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    if (!text) {
        return 0;
    }
    FILE *file = fopen(temporary, "w");
    if (!file) {
        free(text);
        return 0;
    }
    int written = fputs(text, file) >= 0;
    int closed = fclose(file) == 0;
    free(text);
    if (!written || !closed || rename(temporary, path) != 0) {
        remove(temporary);
        return 0;
    }
    return 1;
}

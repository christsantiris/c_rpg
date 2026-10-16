#include "highscore.h"
#include "../external/cJSON.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

void highscore_load(HighScoreTable *t) {
    highscore_load_from(t, HIGHSCORE_PATH);
}

void highscore_load_from(HighScoreTable *t, const char *path) {
    t->count = 0;
    t->crown_of_unbound = 0;
    FILE *f = fopen(path, "r");
    if (!f) {
        return;
    }
    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    rewind(f);
    char *buf = malloc(len + 1);
    fread(buf, 1, len, f);
    buf[len] = '\0';
    fclose(f);

    cJSON *root = cJSON_Parse(buf);
    free(buf);
    if (!root) {
        return;
    }
    cJSON *trophy = cJSON_GetObjectItem(root, "crown_of_unbound");
    t->crown_of_unbound = cJSON_IsNumber(trophy) && trophy->valueint == 1;
    cJSON *entries = cJSON_GetObjectItem(root, "entries");
    t->count = cJSON_GetArraySize(entries);
    if (t->count > MAX_HIGH_SCORES) {
        t->count = MAX_HIGH_SCORES;
    }
    for (int i = 0; i < t->count; i++) {
        cJSON *e = cJSON_GetArrayItem(entries, i);
        t->entries[i] = (HighScoreEntry){0};
        strncpy(t->entries[i].name,
            cJSON_GetObjectItem(e, "name")->valuestring, 20);
        t->entries[i].score = cJSON_GetObjectItem(e, "score")->valueint;
        t->entries[i].level = cJSON_GetObjectItem(e, "level")->valueint;
        cJSON *crown = cJSON_GetObjectItem(e, "crown_of_unbound");
        t->entries[i].crown_of_unbound = cJSON_IsNumber(crown) && crown->valueint == 1;
    }
    cJSON_Delete(root);
}

void highscore_save(const HighScoreTable *t) {
    highscore_save_to(t, HIGHSCORE_PATH);
}

void highscore_save_to(const HighScoreTable *t, const char *path) {
    mkdir("saves", 0755);
    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "crown_of_unbound", t->crown_of_unbound);
    cJSON *entries = cJSON_CreateArray();
    for (int i = 0; i < t->count; i++) {
        cJSON *e = cJSON_CreateObject();
        cJSON_AddStringToObject(e, "name",  t->entries[i].name);
        cJSON_AddNumberToObject(e, "score", t->entries[i].score);
        cJSON_AddNumberToObject(e, "level", t->entries[i].level);
        cJSON_AddNumberToObject(e, "crown_of_unbound", t->entries[i].crown_of_unbound);
        cJSON_AddItemToArray(entries, e);
    }
    cJSON_AddItemToObject(root, "entries", entries);
    char *json = cJSON_Print(root);
    cJSON_Delete(root);
    FILE *f = fopen(path, "w");
    if (!f) {
        free(json);
        return;
    }
    fputs(json, f);
    fclose(f);
    free(json);
}

int highscore_qualifies(const HighScoreTable *t, int score) {
    if (t->count < MAX_HIGH_SCORES) {
        return 1;
    }
    return score > t->entries[t->count - 1].score;
}

void highscore_insert(HighScoreTable *t, const char *name, int score, int level, int crown) {
    if (crown) {
        t->crown_of_unbound = 1;
    }
    if (!highscore_qualifies(t, score)) {
        return;
    }
    int pos = t->count;
    for (int i = 0; i < t->count; i++) {
        if (score > t->entries[i].score) {
            pos = i;
            break;
        }
    }
    int new_count = t->count < MAX_HIGH_SCORES ? t->count + 1 : MAX_HIGH_SCORES;
    for (int i = new_count - 1; i > pos; i--) {
        t->entries[i] = t->entries[i - 1];
    }
    strncpy(t->entries[pos].name, name, 20);
    t->entries[pos].name[20] = '\0';
    t->entries[pos].score = score;
    t->entries[pos].level = level;
    t->entries[pos].crown_of_unbound = crown != 0;
    t->count = new_count;
}

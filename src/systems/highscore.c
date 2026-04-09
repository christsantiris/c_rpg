#include "highscore.h"
#include "../external/cJSON.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

void highscore_load(HighScoreTable *t) {
    t->count = 0;
    FILE *f = fopen(HIGHSCORE_PATH, "r");
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
    cJSON *entries = cJSON_GetObjectItem(root, "entries");
    t->count = cJSON_GetArraySize(entries);
    if (t->count > MAX_HIGH_SCORES) {
        t->count = MAX_HIGH_SCORES;
    }
    for (int i = 0; i < t->count; i++) {
        cJSON *e = cJSON_GetArrayItem(entries, i);
        strncpy(t->entries[i].name,
            cJSON_GetObjectItem(e, "name")->valuestring, 20);
        t->entries[i].score = cJSON_GetObjectItem(e, "score")->valueint;
        t->entries[i].level = cJSON_GetObjectItem(e, "level")->valueint;
    }
    cJSON_Delete(root);
}

void highscore_save(const HighScoreTable *t) {
    mkdir("saves", 0755);
    cJSON *root = cJSON_CreateObject();
    cJSON *entries = cJSON_CreateArray();
    for (int i = 0; i < t->count; i++) {
        cJSON *e = cJSON_CreateObject();
        cJSON_AddStringToObject(e, "name",  t->entries[i].name);
        cJSON_AddNumberToObject(e, "score", t->entries[i].score);
        cJSON_AddNumberToObject(e, "level", t->entries[i].level);
        cJSON_AddItemToArray(entries, e);
    }
    cJSON_AddItemToObject(root, "entries", entries);
    char *json = cJSON_Print(root);
    cJSON_Delete(root);
    FILE *f = fopen(HIGHSCORE_PATH, "w");
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

void highscore_insert(HighScoreTable *t, const char *name, int score, int level) {
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
    t->entries[pos].score = score;
    t->entries[pos].level = level;
    t->count = new_count;
}
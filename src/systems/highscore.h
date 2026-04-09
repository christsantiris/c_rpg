#ifndef HIGHSCORE_HEADER_H
#define HIGHSCORE_HEADER_H

#define MAX_HIGH_SCORES 10
#define HIGHSCORE_PATH "saves/highscores.json"

typedef struct {
    char name[21];
    int score;
    int level;
} HighScoreEntry;

typedef struct {
    HighScoreEntry entries[MAX_HIGH_SCORES];
    int count;
} HighScoreTable;

void highscore_load(HighScoreTable *t);
void highscore_save(const HighScoreTable *t);
int highscore_qualifies(const HighScoreTable *t, int score);
void highscore_insert(HighScoreTable *t, const char *name, int score, int level);

#endif
#ifndef HIGHSCORE_HEADER_H
#define HIGHSCORE_HEADER_H

#define MAX_HIGH_SCORES 10
#define HIGHSCORE_PATH "saves/highscores.json"

typedef struct {
    char name[21];
    int score;
    int level;
    int crown_of_unbound;
} HighScoreEntry;

typedef struct {
    HighScoreEntry entries[MAX_HIGH_SCORES];
    int count;
    int crown_of_unbound; // Permanent trophy history, including unranked victories.
} HighScoreTable;

void highscore_load(HighScoreTable *t);
void highscore_save(const HighScoreTable *t);
int highscore_qualifies(const HighScoreTable *t, int score);
void highscore_insert(HighScoreTable *t, const char *name, int score, int level, int crown);
void highscore_load_from(HighScoreTable *t, const char *path);
void highscore_save_to(const HighScoreTable *t, const char *path);

#endif

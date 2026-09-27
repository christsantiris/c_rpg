#ifndef MUSIC_HEADER_H
#define MUSIC_HEADER_H

void music_init(void);
void music_update(int screen, int in_town, int in_town2);
void music_free(void);
void music_toggle(void);
int  music_enabled(void);

#endif

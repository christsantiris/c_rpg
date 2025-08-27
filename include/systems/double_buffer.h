#ifndef DOUBLE_BUFFER_H
#define DOUBLE_BUFFER_H

#include "../core/types.h"
#include <ncurses.h>

// Double buffer functions
void init_double_buffer(DoubleBuffer *db);
void cleanup_double_buffer(DoubleBuffer *db);
void resize_double_buffer(DoubleBuffer *db);
void begin_draw(DoubleBuffer *db);
void end_draw_and_present(DoubleBuffer *db);
void mark_buffer_resize_needed(DoubleBuffer *db);

#endif
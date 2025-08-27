#include "../../include/systems/double_buffer.h"
#include "../../include/core/types.h"
#include <ncurses.h>
#include <stdlib.h>

// Initialize the double buffer system
void init_double_buffer(DoubleBuffer *db) {
    // Get screen dimensions
    getmaxyx(stdscr, db->buffer_height, db->buffer_width);
    
    // Create off-screen window (back buffer)
    db->back_buffer = newwin(db->buffer_height, db->buffer_width, 0, 0);
    
    if (!db->back_buffer) {
        endwin();
        printf("Error: Could not create back buffer window\n");
        exit(1);
    }
    
    // Enable colors and keypad for back buffer
    if (has_colors()) {
        // Colors are already initialized in main ncurses init
        // The back buffer will use the same color pairs
    }
    
    keypad(db->back_buffer, TRUE);
    
    // Front buffer is the standard screen
    db->front_buffer = stdscr;
    db->needs_resize = 0;
    
    printf("Double buffer initialized: %dx%d\n", db->buffer_width, db->buffer_height);
}

// Clean up double buffer resources
void cleanup_double_buffer(DoubleBuffer *db) {
    if (db->back_buffer) {
        delwin(db->back_buffer);
        db->back_buffer = NULL;
    }
}

// Handle terminal resize for double buffer
void resize_double_buffer(DoubleBuffer *db) {
    // Clean up old buffer
    if (db->back_buffer) {
        delwin(db->back_buffer);
    }
    
    // Get new screen dimensions
    getmaxyx(stdscr, db->buffer_height, db->buffer_width);
    
    // Create new back buffer with new size
    db->back_buffer = newwin(db->buffer_height, db->buffer_width, 0, 0);
    
    if (!db->back_buffer) {
        endwin();
        printf("Error: Could not resize back buffer window\n");
        exit(1);
    }
    
    keypad(db->back_buffer, TRUE);
    db->needs_resize = 0;
}

// Start drawing to the back buffer
void begin_draw(DoubleBuffer *db) {
    // Check if we need to resize
    if (db->needs_resize) {
        resize_double_buffer(db);
    }
    
    // Clear the back buffer
    wclear(db->back_buffer);
    
    // All subsequent drawing commands will go to back buffer
    // (We'll handle this by passing the back_buffer window to drawing functions)
}

// Finish drawing and display the complete frame
void end_draw_and_present(DoubleBuffer *db) {
    // Refresh the back buffer (prepares it for copying)
    wrefresh(db->back_buffer);
    
    // Copy back buffer to front buffer (this is the magic!)
    copywin(db->back_buffer,     // source window
            db->front_buffer,    // destination window  
            0, 0,                // source start coordinates
            0, 0,                // destination start coordinates
            db->buffer_height - 1, db->buffer_width - 1,  // copy area
            0);                  // overlay flag (0 = overwrite)
    
    // Make the front buffer visible
    refresh();
}

// Mark that buffer needs resizing (called from resize handler)
void mark_buffer_resize_needed(DoubleBuffer *db) {
    db->needs_resize = 1;
}
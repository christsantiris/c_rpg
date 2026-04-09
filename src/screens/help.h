#ifndef HELP_HEADER_H
#define HELP_HEADER_H

typedef enum {
    HELP_NONE = 0,
    HELP_CLOSED
} HelpResult;

HelpResult help_handle_key(int scancode);

#endif
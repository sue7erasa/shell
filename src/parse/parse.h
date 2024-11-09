#ifndef SHELL_PARSE_H
#define SHELL_PARSE_H

#include <stddef.h>
#include <stdbool.h>

#include "../errors.h"

//struct parse_info;
struct parse_info {
    size_t count;
    bool has_inpipe;
    bool has_outpipe;
    bool is_background;
    struct command* commands;

};

enum command_type {
    CT_ARG,
    CT_INPIPE,
    CT_OUTPIPE,
    CT_BACKGROUND,
};

struct command {
    enum command_type type;
    ptrdiff_t len;
    const char* text;
    struct command* next;
};

struct error init_parse_info(struct parse_info** res);
void free_parse_info(struct parse_info** info);

struct error parse(const char* line, struct parse_info* info);

#endif //SHELL_PARSE_H

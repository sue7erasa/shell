#include "parse.h"

#include <stdint.h>
#include <malloc.h>

//enum command_type {
//    CT_ARG,
//    CT_INPIPE,
//    CT_OUTPIPE,
//    CT_BACKGROUND,
//};

//struct command {
//    enum command_type type;
//    ptrdiff_t len;
//    const char* text;
//    struct command* next;
//};

//struct parse_info {
//    size_t count;
//    bool has_inpipe;
//    bool has_outpipe;
//    bool is_background;
//    struct command* commands;
//
//};

static void parse_info_add_command(const char* start, ptrdiff_t len, struct parse_info* info);


struct error parse(const char* line, struct parse_info* info) {
    const char* c = line;
    const char* word_start = line;

    if(*c == '\0') return get_error(P_ZERO_LEN);

    while(1) {
        if(*c == ' ') {
            parse_info_add_command(word_start, (c) - word_start, info);
            while(*c == ' ') {
                word_start = c + 1;
                c++;
            }
        }
        else if(*c == '\0') {
            parse_info_add_command(word_start, (c) - word_start, info);
            break;
        }
        else
            c++;
    }

    return get_error(NO_ERROR);
}

enum command_type determine_command_type(const char* start) {
    if(start[0] == '&') return CT_BACKGROUND;
    else if(start[0] == '<') return CT_INPIPE;
    else if(start[0] == '>') return CT_OUTPIPE;
    else return CT_ARG;
}

static void parse_info_add_command(const char* start, ptrdiff_t  len, struct parse_info* info) {
    struct command* cmd = nullptr;
    cmd = (struct command*) malloc(sizeof(struct command));
    if(!cmd) return; // Todo error

    cmd->next = nullptr;
    cmd->len = len;
    cmd->text = start;
    cmd->type = determine_command_type(start);

    if(cmd->type == CT_OUTPIPE) info->has_outpipe = true;
    else if(cmd->type == CT_INPIPE) info->has_inpipe = true;
    else if(cmd->type == CT_BACKGROUND) info->is_background = true;

    info->count += 1;

    if(!info->commands) {
        info->commands = cmd;
    }
    else {
        struct command* c = info->commands;
        while(c->next) {
            c = c->next;
        }
        c->next = cmd;
    }
}

struct error init_parse_info(struct parse_info** res) {
    *res = malloc(sizeof(struct parse_info));
    if(!*res) return ERROR(PI_ALLOC_FAIL);

    (*res)->count = 0;
    (*res)->has_inpipe = false;
    (*res)->has_outpipe = false;
    (*res)->is_background = false;
    (*res)->commands = nullptr;

    return SUCCESS;
}

void free_commands(struct command* cmd) {
    if(cmd->next) free_commands(cmd->next);
    free(cmd);
}

void free_parse_info_commands(struct parse_info* info) {
    if(!info) return;

    free_commands(info->commands);
    info->commands = nullptr;
}

void free_parse_info(struct parse_info** info) {
    if(!(*info)) return;

    if((*info)->count == 0) {
        free(*info);
        return;
    }

    free_commands((*info)->commands);

    (*info)->count = 0;
    (*info)->is_background = false;
    (*info)->has_inpipe = false;
    (*info)->has_outpipe = false;


    free(*info);

    *info = nullptr;
}

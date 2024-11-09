#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include "builtin_cmds.h"

struct cmd {
    const char* cmd_str;
    struct error (* handler)( char** args );
};

static struct error cd_handler(char** args);

#define COMMANDS_COUNT 1

static const struct cmd commands[COMMANDS_COUNT] = {
        [0] = { "cd", cd_handler },
};


struct error execute_builtin_command(char** args) {
    for(size_t i = 0; i < COMMANDS_COUNT; i++) {
        if(strcmp(args[0], commands[i].cmd_str) == 0) {
            return commands[i].handler(args);
        }
    }
    return ERROR(CMDP_COMMAND_MISSING);
}

static struct error cd_handler(char** args) {
    if(args[1] == nullptr) return SUCCESS;

    constexpr size_t buffer_size = 64;
    char buffer[buffer_size];
    if(!getcwd(buffer, buffer_size)) {
        ERROR_WITH_ERRNO(CMDP_BUFFER_OVERFLOW);
        return ERROR(CMDP_BUFFER_OVERFLOW);
    }
    const size_t actual_wd_len = strlen(buffer) + 1;

    if(actual_wd_len <= 1) return SUCCESS;

    if(strcmp(args[1], "..") == 0) {
        for(size_t i = actual_wd_len; i--;) {
            char symbol = buffer[i];
            buffer[i] = '\0';
            if(symbol == '/') {
                break;
            }
        }
    }else {
        if( strlen(args[1]) > actual_wd_len ) return ERROR(CMDP_BUFFER_OVERFLOW);
        if(buffer[actual_wd_len - 1] != '/' && args[1][0] != '/') strcat(buffer, "/");
        strcat(buffer, args[1]);
    }

    errno = 0;
    if(chdir(buffer) != 0) {
        ERROR_WITH_ERRNO(CMDP_EXEC_FAIL);
        return ERROR(CMDP_EXEC_FAIL);
    }

    return SUCCESS;
}
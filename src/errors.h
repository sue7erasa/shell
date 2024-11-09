#ifndef SHELL_ERRORS_H
#define SHELL_ERRORS_H

#include <inttypes.h>

#define ERROR(code) get_error(code)
#define ERROR_WITH_ERRNO(code) perror(get_error(code).message)
#define SUCCESS get_error(NO_ERROR)

typedef uint8_t error_code;

enum error_type : error_code {
    NO_ERROR,
    RL_BUFFER_OVERFLOW,
    RL_ALLOC_FAIL,
    RL_ZERO_LEN,

    PI_ALLOC_FAIL,

    P_ZERO_LEN,

    H_ALLOC_FAIL,
    H_REALLOC_FAIL,

    CMDP_FORK_FAIL,
    CMDP_EXEC_FAIL,
    CMDP_COMMAND_NULL,
    CMDP_COMMAND_MISSING,
    CMDP_BUFFER_OVERFLOW,

    PASSWD_GET_FAIL,
    PASSWD_BUFFER_OVERFLOW,


    ERRORS_COUNT,
};

struct error {
    error_code code;
    bool is_fatal;
    const char* message;
};

static const struct error null_error = { NO_ERROR, false, nullptr };

struct error get_error(error_code code);
void handle_error(struct error);

#endif //SHELL_ERRORS_H

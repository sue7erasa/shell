#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include "errors.h"


static const struct error codes[ERRORS_COUNT] = {
        [RL_BUFFER_OVERFLOW] = {RL_BUFFER_OVERFLOW, true, "READ LINE BUFFER OVERFLOW" },
        [RL_ALLOC_FAIL] = {RL_ALLOC_FAIL, true, "READ LINE ALLOC FAIL" },
        [RL_ZERO_LEN] = {RL_ZERO_LEN, false, "READ LINE ZERO LEN" },
        [PI_ALLOC_FAIL] = {PI_ALLOC_FAIL, true, "PARSE INFO ALLOC FAIL" },
        [P_ZERO_LEN] = {P_ZERO_LEN, false, "PARSE ZERO LEN" },
        [H_ALLOC_FAIL] = { H_ALLOC_FAIL, true, "HISTORY ALLOC FAIL" },
        [H_REALLOC_FAIL] = { H_REALLOC_FAIL, true, "HISTORY REALLOC FAIL" },
        [CMDP_EXEC_FAIL] = { CMDP_EXEC_FAIL, false, "COMMAND EXECUTION FAIL" },
        [CMDP_FORK_FAIL] = { CMDP_FORK_FAIL, true, "COMMAND FORK FAIL" },
        [CMDP_COMMAND_NULL] = { CMDP_COMMAND_NULL, true, "COMMAND IS NULL" },
        [CMDP_COMMAND_MISSING] = { CMDP_COMMAND_MISSING, false, "BUILTIN COMMAND IS MISSING" },
        [CMDP_BUFFER_OVERFLOW] = { CMDP_BUFFER_OVERFLOW, true, "BUILTIN COMMAND BUFFER OVERFLOW" },
        [PASSWD_GET_FAIL] = { PASSWD_GET_FAIL, true, "PASSWD GET FAIL" },
        [PASSWD_BUFFER_OVERFLOW] = { PASSWD_BUFFER_OVERFLOW, true, "PASSWD BUFFER OVERFLOW" },
};

struct error get_error(error_code code) {
    if(!code) return null_error;
    else return codes[code];
}

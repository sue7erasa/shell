#ifndef SHELL_BUILTIN_CMDS_H
#define SHELL_BUILTIN_CMDS_H

#include "errors.h"

struct error execute_builtin_command(char** args);

#endif //SHELL_BUILTIN_CMDS_H

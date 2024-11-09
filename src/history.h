#ifndef SHELL_HISTORY_H
#define SHELL_HISTORY_H

#include <stddef.h>

void history_debug_print();

void history_free();
size_t history_count();
const char* history_get_next();
const char* history_get_prev();
struct error history_add(const char* const line);

#endif //SHELL_HISTORY_H


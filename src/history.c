#include <malloc.h>
#include <memory.h>
#include "history.h"
#include "errors.h"

struct history_entry {
    const char* const line;
    struct history_entry* next;
    struct history_entry* prev;
};

static struct history_entry* history = nullptr;
static struct history_entry* last_entry = nullptr;
static struct history_entry* iterator = nullptr;
static size_t count = 0;

size_t history_count() { return count; }

void history_debug_print() {
    if(!history) return;

    printf("history (%zu):\n", count);
    for(struct history_entry* it = history; it->next; it = it->next) {
        printf("\t%s\n", it->line);
    }
}

void history_free() {
    if(!history) return;

    struct history_entry* entry = history;
    while(entry) {
        struct history_entry* next = nullptr;
        if(entry->next) next = entry->next;
        free(entry);
        entry = next;
    }
    history = nullptr;
    last_entry = nullptr;
    count = 0;
}

static bool first_prev = true;
const char* history_get_prev() {
    if(iterator->prev && !first_prev)
        iterator = iterator->prev;

    first_prev = false;

    return iterator->line;
}

const char* history_get_next() {
    if(iterator->next)
        iterator = iterator->next;

    return iterator->line;
}

struct error history_add(const char* const line) {
    struct history_entry* new = nullptr;

    struct history_entry* last = history;
    if(last) {
        while (last->next) {
            last = last->next;
        }
    }

    new = malloc(sizeof(struct history_entry));
    if(!new) return ERROR(H_ALLOC_FAIL);

    struct history_entry init_entry = { line, nullptr, nullptr };
    memcpy(new, &init_entry, sizeof(struct history_entry));

    if(last) {
        new->prev = last;
        last->next = new;
    } else {
        history = new;
    }

    last_entry = new;
    iterator = last_entry;
    first_prev = true;

    count++;

    return SUCCESS;
}

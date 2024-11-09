#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // for gatcwd(), getuid(), execvp()
#include <errno.h>
#include <sys/types.h>
#include <pwd.h> // for getpwuid()
#include <sys/wait.h>
#include <termios.h>

#include "builtin_cmds.h"
#include "history.h"
#include "parse/parse.h"

#define DEFAULT_LINE_SIZE 127
#define CWD_BUFFER_SIZE 255
#define USERNAME_BUFFER_SIZE 32
#define PROMPT_BUFFER_SIZE (CWD_BUFFER_SIZE + USERNAME_BUFFER_SIZE)

#define MAINF_FREE_AND_CONTINUE() \
free_parse_info(&info);           \
continue;

#define MAINF_FREE_AND_EXIT() history_free(); \
free_parse_info(&info);                       \
history_free();                               \
exit(EXIT_FAILURE);

#define MAINF_HANDLE_ERROR(err)                                         \
if(error.code != NO_ERROR) {                                            \
    if(errno) perror(error.message);                                    \
    else printf("ERROR [%" PRIx8 "]: %s\n", error.code, error.message); \
    errno = 0;                                                          \
    if( error.is_fatal ) {                                              \
    MAINF_FREE_AND_EXIT();                                              \
    } else { MAINF_FREE_AND_CONTINUE() }                                \
}

static struct error readline(const char* prompt, char** line);
static struct error process_command(const struct parse_info* info);
static void trim_line(char* line, size_t buffer_size);
static struct error get_username(char* buffer, size_t buffer_size);
static void get_cwd(char* buffer, size_t buffer_size);

int main(void) {
    char username_buffer[USERNAME_BUFFER_SIZE];

    struct parse_info* info = nullptr;

    char* line = nullptr;

    struct error username_get_err = get_username(username_buffer, USERNAME_BUFFER_SIZE);
    if(username_get_err.code != NO_ERROR) {
        perror(username_get_err.message);
        exit(EXIT_FAILURE);
    }

    while (1) {
        errno = 0;
        struct error error = null_error;

        char cwd_buffer[CWD_BUFFER_SIZE] ;
        char prompt_buffer[PROMPT_BUFFER_SIZE] = {'\0'};
        get_cwd(cwd_buffer, CWD_BUFFER_SIZE);

        strcat(prompt_buffer, username_buffer);
        strcat(prompt_buffer, cwd_buffer);

        error = readline(prompt_buffer, &line);
        MAINF_HANDLE_ERROR(error)


        error = init_parse_info(&info);
        MAINF_HANDLE_ERROR(error)

        trim_line(line, DEFAULT_LINE_SIZE);

        error = parse(line, info);
        MAINF_HANDLE_ERROR(error)

        history_add(line);
        //history_debug_print();

        error = process_command(info);
        MAINF_HANDLE_ERROR(error)

        free_parse_info(&info);
    }

    history_free();

    return EXIT_FAILURE;
}

void free_command_args(char** args) {
    size_t freed = 0;
    while(args[freed] != nullptr) {
        free(args[freed]);
        freed++;
    }
    free(args);
}

static struct error process_command(const struct parse_info* info) {
    if(!info->commands) return ERROR(CMDP_COMMAND_NULL);

    char* command = malloc(sizeof(char) * (info->commands->len + 1));
    memcpy(command, info->commands->text, info->commands->len);
    command[info->commands->len] = '\0';

    char** args = nullptr;
    if(info->count > 1) {
        size_t loaded = 0;
        args = malloc( sizeof(char*) * info->count + 1 ); // + 1 for last null element (nullptr)
        args[loaded++] = command;

        const struct command* cmd = info->commands->next;
        while (cmd) {
            args[loaded] = malloc( sizeof(char) * (cmd->len + 1) );
            memcpy(args[loaded], cmd->text, cmd->len);
            args[loaded][cmd->len] = '\0';

            loaded++;
            cmd = cmd->next;
        }
        args[info->count] = nullptr;
    } else {
        args = malloc( sizeof(char*) * 2 ); // command and additional null element
        args[0] = command;
        args[1] = nullptr;
    }

    errno = 0;
    pid_t pid = fork();
    if(pid == -1) {
        free_command_args(args);
        return ERROR(CMDP_FORK_FAIL);
    }

    struct error builtin_command_error = null_error;
    int external_command_error = 0;
    if(pid == 0) {
        //errno = 0;
        if(execvp(args[0], args) == -1) {
            exit(errno);
        }
        exit(EXIT_SUCCESS);
    }else {
        wait(&external_command_error);
        external_command_error = WEXITSTATUS(external_command_error);
        if(external_command_error != EXIT_SUCCESS) {
            builtin_command_error = execute_builtin_command(args);
        }
    }

    free_command_args(args);

    if(builtin_command_error.code == NO_ERROR) {
        return SUCCESS;
    }
    else{
        if(builtin_command_error.code == CMDP_COMMAND_MISSING) {
            errno = external_command_error;
            perror("");
            errno = 0;
            return ERROR(CMDP_EXEC_FAIL);
        }
        return ERROR(builtin_command_error.code);
    }
}

static void move_buffer_content_to(size_t dist_index, size_t content_start, size_t content_len, char* buffer, size_t buffer_size) {
    if( (dist_index + content_len) >= buffer_size
        || dist_index == content_start
        || content_len == 0
        || !buffer ) return;

    if(dist_index < content_start) {
        for (size_t ci = content_start, di = dist_index; ci < (content_start + content_len); ++ci, ++di) {
            buffer[di] = buffer[ci];
            buffer[ci] = ' ';
        }
    }else {
        for (size_t ci = content_len, di = dist_index + (content_len - 1); ci--; --di) {
            buffer[di] = buffer[ci];
        }
    }
}

static size_t trim_start(char* line, size_t content_len, size_t line_max_size) {
    if(line[0] != ' ') return 0;

    size_t spaces_count = 0;
    for(size_t it = 0; it <= content_len; ++it) {
        if(spaces_count == content_len) return 0;

        if(line[it] == ' ') ++spaces_count;
        else if (line[it] != ' ' && spaces_count) {
            move_buffer_content_to(0, it, content_len, line, line_max_size);
            break;
        }
    }

    return spaces_count;
}

static size_t trim_end(char* line, size_t content_len) {
    size_t trimmed = 0;
    for(size_t it = content_len ; it--;) {
        if(line[it] != ' ') break;
        else { line[it] = '\0'; ++trimmed; };
    }
    return trimmed;
}

static void trim_line(char* line, size_t buffer_size) {
    size_t content_len = strlen(line);
    content_len -= trim_start(line, content_len, buffer_size);
    trim_end(line, content_len);
}

void raw_mode(bool enable) {
    struct termios raw = {};
    tcgetattr(STDIN_FILENO, &raw);

    if(enable) raw.c_lflag &= ~(ICANON | ECHO);
    else raw.c_lflag |= (ICANON | ECHO);

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}


enum arrow_keys {
    ARROW_UP = 65,
    ARROW_DOWN = 66,
    ARROW_RIGHT = 67,
    ARROW_LEFT = 68,
};

#define ERASE_CHAR() putchar('\b'); putchar(' '); putchar('\b');

static struct error readline(const char* prompt, char** line) {
    printf("%s ", prompt);

    raw_mode(true);

    char line_buffer[DEFAULT_LINE_SIZE] = {};

    int c = 0;
    size_t count = 0;
    for(; (c = getchar()) != EOF && c != '\n'; /*++count*/) {
        if(count >= DEFAULT_LINE_SIZE) return get_error(RL_BUFFER_OVERFLOW);

        if (c == 8 || c == 127) {
            if(count == 0) continue;
            ERASE_CHAR()
            count--;
        }
        else if(c == 27 && getchar() == 91) {
            if(history_count()) {
                int arrow_key = getchar();
                if (arrow_key == ARROW_UP) {
                    strcpy(line_buffer, history_get_prev());
                } else if (arrow_key == ARROW_DOWN) {
                    strcpy(line_buffer, history_get_next());
                }

                size_t new_count = strlen(line_buffer);
                size_t to_clear = count;
                while (to_clear--) {
                    ERASE_CHAR()
                }
                count = new_count;
                printf("%s", line_buffer);
            }
        }
        else {
            line_buffer[count] = (char)c;
            putchar(c);
            count++;
        }
    }
    putchar('\n');
    if(count == DEFAULT_LINE_SIZE - 1) {
        return ERROR(RL_BUFFER_OVERFLOW);
    }

    raw_mode(false);

    line_buffer[count] = '\0';
    size_t line_size = count + 1;

    *line = malloc(sizeof(char) * line_size);
    if(!*line) return ERROR(RL_ALLOC_FAIL);
    memcpy(*line, line_buffer, sizeof(char) * line_size);

    if(count == 0)
        return ERROR(RL_ZERO_LEN);
    else
        return SUCCESS;
}

static struct error get_username(char* buffer, size_t buffer_size) {
    struct passwd* passwd = nullptr;

    if( !(passwd = getpwuid( getuid() )) ) return ERROR(PASSWD_GET_FAIL);

    if(strlen(passwd->pw_name) >= buffer_size) return ERROR(PASSWD_BUFFER_OVERFLOW);

    strcpy(buffer, passwd->pw_name);
    strcat(buffer, ":");

    return SUCCESS;
}
static void get_cwd(char* buffer, size_t buffer_size) { // Todo error handling
    if(!getcwd(buffer, CWD_BUFFER_SIZE)) {
        if(errno == ERANGE) {
            printf("cwd len exceeds prompt buffer size\n");
        }
        exit(EXIT_FAILURE);
    }

    if(strlen(buffer) >= buffer_size)
        exit(EXIT_FAILURE);

    strcat(buffer, "$");
}

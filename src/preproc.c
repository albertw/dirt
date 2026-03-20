/*
 * preproc.c - Simple preprocessor for MUD zone files
 *
 * This preprocessor handles the subset of C preprocessor features used by
 * the zone files: #include, #define, and #undef.
 *
 * Key features:
 * - Preserves multiline strings exactly as-is
 * - Simple macro substitution (identifier -> value)
 * - Include file processing
 * - Comment stripping (C and C++ style)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>

#define MAX_MACROS 256
#define MAX_MACRO_NAME 64
#define MAX_MACRO_VALUE 256
#define MAX_PATH 512
#define BUFFER_SIZE 4096

/* Macro definition structure */
typedef struct {
    char name[MAX_MACRO_NAME];
    char value[MAX_MACRO_VALUE];
} Macro;

/* Global macro table */
static Macro macros[MAX_MACROS];
static int num_macros = 0;

/* Forward declarations */
static void add_macro(const char *name, const char *value);
static void remove_macro(const char *name);
static const char *lookup_macro(const char *name);
static void process_file(const char *filename, const char *include_path, FILE *out);
static char *find_include_file(const char *incname, const char *include_path, const char *current_dir);
static void strip_comments_and_substitute(char *line, FILE *out);
static int is_identifier_char(int c);

/*
 * Add a macro to the macro table
 */
static void add_macro(const char *name, const char *value) {
    if (num_macros >= MAX_MACROS) {
        fprintf(stderr, "Warning: Maximum number of macros reached\n");
        return;
    }

    /* Check if macro already exists */
    for (int i = 0; i < num_macros; i++) {
        if (strcmp(macros[i].name, name) == 0) {
            strncpy(macros[i].value, value, MAX_MACRO_VALUE - 1);
            macros[i].value[MAX_MACRO_VALUE - 1] = '\0';
            return;
        }
    }

    /* Add new macro */
    strncpy(macros[num_macros].name, name, MAX_MACRO_NAME - 1);
    macros[num_macros].name[MAX_MACRO_NAME - 1] = '\0';
    strncpy(macros[num_macros].value, value, MAX_MACRO_VALUE - 1);
    macros[num_macros].value[MAX_MACRO_VALUE - 1] = '\0';
    num_macros++;
}

/*
 * Remove a macro from the macro table
 */
static void remove_macro(const char *name) {
    for (int i = 0; i < num_macros; i++) {
        if (strcmp(macros[i].name, name) == 0) {
            /* Shift remaining macros down */
            for (int j = i; j < num_macros - 1; j++) {
                macros[j] = macros[j + 1];
            }
            num_macros--;
            return;
        }
    }
}

/*
 * Look up a macro by name
 */
static const char *lookup_macro(const char *name) {
    for (int i = 0; i < num_macros; i++) {
        if (strcmp(macros[i].name, name) == 0) {
            return macros[i].value;
        }
    }
    return NULL;
}

/*
 * Check if a character is valid in an identifier
 */
static int is_identifier_char(int c) {
    return isalnum(c) || c == '_';
}

/*
 * Substitute macros in a line and output to file
 */
static void substitute_macros(const char *line, FILE *out) {
    const char *p = line;

    while (*p) {
        /* Check for potential macro start (identifier character) */
        if (isalpha(*p) || *p == '_') {
            char name[MAX_MACRO_NAME];
            int len = 0;

            /* Extract the identifier */
            while (is_identifier_char(*p) && len < MAX_MACRO_NAME - 1) {
                name[len++] = *p++;
            }
            name[len] = '\0';

            /* Check if it's a macro */
            const char *value = lookup_macro(name);
            if (value) {
                fputs(value, out);
            } else {
                fputs(name, out);
            }
        } else {
            fputc(*p, out);
            p++;
        }
    }
}

/*
 * Strip comments from a line (both C and C++ style)
 * and perform macro substitution
 */
static void strip_comments_and_substitute(char *line, FILE *out) {
    char *p = line;
    int in_string = 0;
    int in_char = 0;
    char string_char = 0;
    char output[BUFFER_SIZE];
    int out_idx = 0;

    while (*p && out_idx < BUFFER_SIZE - 1) {
        /* Track if we're inside a string literal or character constant */
        if (!in_string && !in_char && (*p == '"' || *p == '\'')) {
            if (*p == '"') {
                in_string = 1;
                string_char = '"';
            } else {
                in_char = 1;
                string_char = '\'';
            }
            output[out_idx++] = *p++;
            continue;
        }

        if (in_string && *p == string_char) {
            /* Check for escaped quote */
            if (p > line && *(p-1) != '\\') {
                in_string = 0;
            }
            output[out_idx++] = *p++;
            continue;
        }

        if (in_char && *p == string_char) {
            /* Check for escaped quote */
            if (p > line && *(p-1) != '\\') {
                in_char = 0;
            }
            output[out_idx++] = *p++;
            continue;
        }

        /* Inside string/char, preserve everything */
        if (in_string || in_char) {
            output[out_idx++] = *p++;
            continue;
        }

        /* Check for C++ style comment */
        if (*p == '/' && *(p + 1) == '/') {
            break; /* Skip rest of line */
        }

        /* Check for C style comment start */
        if (*p == '/' && *(p + 1) == '*') {
            p += 2;
            /* Skip until comment end */
            while (*p && !(*p == '*' && *(p + 1) == '/')) {
                if (*p == '\n') {
                    /* Preserve newlines in multi-line comments */
                    output[out_idx++] = '\n';
                }
                p++;
            }
            if (*p) {
                p += 2; /* Skip closing */
            }
            continue;
        }

        output[out_idx++] = *p++;
    }

    output[out_idx] = '\0';

    /* Now substitute macros in the processed line */
    substitute_macros(output, out);
}

/*
 * Find an include file
 * Searches in include_path first, then current directory
 */
static char *find_include_file(const char *incname, const char *include_path, const char *current_dir) {
    static char path[MAX_PATH];
    FILE *test;

    /* First try include_path */
    if (include_path) {
        snprintf(path, MAX_PATH, "%s/%s", include_path, incname);
        test = fopen(path, "r");
        if (test) {
            fclose(test);
            return path;
        }
    }

    /* Then try current directory */
    if (current_dir) {
        snprintf(path, MAX_PATH, "%s/%s", current_dir, incname);
        test = fopen(path, "r");
        if (test) {
            fclose(test);
            return path;
        }
    }

    /* Finally try just the filename */
    test = fopen(incname, "r");
    if (test) {
        fclose(test);
        strncpy(path, incname, MAX_PATH - 1);
        path[MAX_PATH - 1] = '\0';
        return path;
    }

    return NULL;
}

/*
 * Process a single file
 */
static void process_file(const char *filename, const char *include_path, FILE *out) {
    FILE *in;
    char line[BUFFER_SIZE];
    char *last_slash;
    char current_dir[MAX_PATH];
    char *include_file;

    /* Store current directory for relative includes */
    strncpy(current_dir, filename, MAX_PATH - 1);
    current_dir[MAX_PATH - 1] = '\0';
    last_slash = strrchr(current_dir, '/');
    if (last_slash) {
        *last_slash = '\0';
    } else {
        strcpy(current_dir, ".");
    }

    in = fopen(filename, "r");
    if (!in) {
        fprintf(stderr, "Error: Cannot open file '%s'\n", filename);
        return;
    }

    while (fgets(line, BUFFER_SIZE, in)) {
        /* Skip leading whitespace */
        char *p = line;
        while (isspace(*p)) p++;

        /* Check for preprocessor directives */
        if (*p == '#') {
            p++; /* Skip '#' */

            /* Skip whitespace after # */
            while (isspace(*p)) p++;

            /* #include directive */
            if (strncmp(p, "include", 7) == 0 && isspace(p[7])) {
                p += 7;
                while (isspace(*p)) p++;

                /* Get the filename (can be in quotes or angle brackets) */
                char incname[MAX_PATH];
                char *start = strchr(p, '"');
                char *end;

                if (start) {
                    start++; /* Skip opening quote */
                    end = strchr(start, '"');
                } else {
                    start = strchr(p, '<');
                    if (start) {
                        start++; /* Skip opening bracket */
                        end = strchr(start, '>');
                    } else {
                        fprintf(stderr, "Warning: Malformed #include directive\n");
                        continue;
                    }
                }

                if (end) {
                    *end = '\0';
                    strncpy(incname, start, MAX_PATH - 1);
                    incname[MAX_PATH - 1] = '\0';

                    include_file = find_include_file(incname, include_path, current_dir);
                    if (include_file) {
                        process_file(include_file, include_path, out);
                    } else {
                        fprintf(stderr, "Warning: Cannot find include file '%s'\n", incname);
                    }
                }
                continue;
            }

            /* #define directive */
            if (strncmp(p, "define", 6) == 0 && isspace(p[6])) {
                p += 6;
                while (isspace(*p)) p++;

                /* Extract macro name */
                char name[MAX_MACRO_NAME];
                int idx = 0;
                while (is_identifier_char(*p) && idx < MAX_MACRO_NAME - 1) {
                    name[idx++] = *p++;
                }
                name[idx] = '\0';

                if (idx > 0) {
                    /* Skip whitespace after name */
                    while (isspace(*p)) p++;

                    /* Extract value (rest of line, up to comment or newline) */
                    char value[MAX_MACRO_VALUE];
                    idx = 0;
                    while (*p && *p != '\n' && idx < MAX_MACRO_VALUE - 1) {
                        /* Check for comment start BEFORE adding this character */
                        if (*p == '/' && *(p + 1) == '*') {
                            break; /* C-style comment, stop here */
                        }
                        if (*p == '/' && *(p + 1) == '/') {
                            break; /* C++ style comment, stop here */
                        }
                        value[idx++] = *p++;
                    }
                    /* Trim trailing whitespace to match cpp behavior */
                    while (idx > 0 && isspace(value[idx - 1])) {
                        idx--;
                    }
                    value[idx] = '\0';

                    add_macro(name, value);
                }
                continue;
            }

            /* #undef directive */
            if (strncmp(p, "undef", 5) == 0 && isspace(p[5])) {
                p += 5;
                while (isspace(*p)) p++;

                char name[MAX_MACRO_NAME];
                int idx = 0;
                while (is_identifier_char(*p) && idx < MAX_MACRO_NAME - 1) {
                    name[idx++] = *p++;
                }
                name[idx] = '\0';

                if (idx > 0) {
                    remove_macro(name);
                }
                continue;
            }
        }

        /* Not a preprocessor directive - process and output */
        strip_comments_and_substitute(line, out);
    }

    fclose(in);
}

/*
 * preproc_open - Open and preprocess a file
 *
 * This function creates a pipe and forks a child process to do the
 * preprocessing. The parent process returns a FILE* that can be read
 * from to get the preprocessed output.
 */
FILE *preproc_open(const char *filename, const char *include_path) {
    int pipefd[2];
    pid_t pid;

    if (pipe(pipefd) == -1) {
        perror("pipe");
        return NULL;
    }

    pid = fork();
    if (pid == -1) {
        perror("fork");
        close(pipefd[0]);
        close(pipefd[1]);
        return NULL;
    }

    if (pid == 0) {
        /* Child process - do the preprocessing */
        close(pipefd[0]); /* Close read end */

        /* Redirect stdout to the pipe */
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);

        /* Process the file and write to stdout */
        process_file(filename, include_path, stdout);

        exit(0);
    } else {
        /* Parent process - return a FILE* for reading */
        close(pipefd[1]); /* Close write end */
        return fdopen(pipefd[0], "r");
    }
}

/*
 * preproc_close - Close a preprocessed file stream
 */
int preproc_close(FILE *fp) {
    return pclose(fp);
}

#ifndef OUNG_H
#define OUNG_H

enum token_t {
    ZERO,
    INPUT,
    PRINT_NUM,
    BYTE,
    ADD,
    SUB,
    MUL,
    DIV,
    VAR,
    PRINT,
    OR,
    AND,
    SEP,
    SLEEP,
    END,
    ONE,
    ERROR
};

struct variable_t {
    char* identifier;
    char value;
    struct variable_t* prev;
    struct variable_t* next;
};

int oung_input(enum token_t cmd[256], unsigned int limit);
int oung_init_var(enum token_t cmd[256], unsigned int limit);
int oung_eval(enum token_t cmd[256], unsigned int limit, enum token_t mode);
int oung_print(enum token_t cmd[256], unsigned int limit, enum token_t mode);
void traverse_forward();
void traverse_backward();

#endif

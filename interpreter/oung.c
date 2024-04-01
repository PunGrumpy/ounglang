#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "oung.h"

#define GOOD 1
#define BAD 0
#define STMT_LIMIT 256
#define LINE_LIMIT 256

struct variable_t* var_list_head = NULL;
struct variable_t* var_list_tail = NULL;

enum token_t convert_oung_to_inst(const char oung[4]);
int execute(enum token_t cmd[STMT_LIMIT], unsigned int limit);
int allocate_var(char* identifier, unsigned int id_len, char value);
int is_id_valid(const char* identifier);
struct variable_t* get_variable(const char* identifier);
int deallocate_var(char* identifier);

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Proper command usage: oung [oung_source_name]\n");
        return 1;
    }

    // --help or -h
    if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
        printf("Oung Interpreter\n"
               "Usage: oung [oung_source_name]\n"
               "Proper source file extension: *.oung\n");
        return 0;
    }

    const char* file_name = argv[1];
    size_t file_name_len = strlen(argv[1]);

    if (strcmp(&file_name[file_name_len - 5], ".oung")) {
        printf("Proper source file extension: *.oung\n");
        return 1;
    }

    FILE* source = fopen(file_name, "r");
    if (source == NULL) {
        printf("The file %s does not exist!\n", file_name);
        return 1;
    }

    // scan each line
    char line[LINE_LIMIT];
    enum token_t cmd[STMT_LIMIT];
    unsigned int cmd_pos = 0;

    while (fgets(line, LINE_LIMIT, source) != NULL) {
        // remove new lines
        line[strcspn(line, "\n")] = '\0';

        char* token = strtok(line, " ");
        while (token != NULL) {
            enum token_t inst = convert_oung_to_inst(token);

            if (inst == END) {
                execute(cmd, cmd_pos);
                cmd_pos = 0;
            } else {
                cmd[cmd_pos++] = inst;
            }

            token = strtok(NULL, " ");
        }
    }

    fclose(source);
    return 0;
}

enum token_t convert_oung_to_inst(const char oung[4]) {
    const char* oungs[16] = {
        "oung", "ounG", "ouNg", "ouNG",
        "oUng", "oUnG", "oUNg", "oUNG",
        "Oung", "OunG", "OuNg", "OuNG",
        "OUng", "OUnG", "OUNg", "OUNG"
    };

    // Change to trie search
    return ((oung[0] == 'O') * 8) +
           ((oung[1] == 'U') * 4) +
           ((oung[2] == 'N') * 2) +
           ((oung[3] == 'G') * 1);
}

int execute(enum token_t cmd[STMT_LIMIT], unsigned int limit) {
    if (limit <= 0)
        return 0; // Empty command

    int status_code = 1; // Default status code

    switch (cmd[0]) {
        case INPUT:
            status_code = oung_input(&cmd[1], limit - 1);
            break;
        case BYTE:
            status_code = oung_init_var(&cmd[1], limit - 1);
            break;
        case ADD:
        case SUB:
        case MUL:
        case DIV:
        case OR:
        case AND:
            status_code = oung_eval(&cmd[1], limit - 1, cmd[0]);
            break;
        case PRINT:
        case PRINT_NUM:
            status_code = oung_print(&cmd[1], limit - 1, cmd[0]);
            break;
        default:
            status_code = 0; // Invalid command
    }

    return status_code;
}

int allocate_var(char* identifier, unsigned int id_len, char value) {
    if (!is_id_valid(identifier)) {
        return 0; // Invalid identifier
    }

    struct variable_t* new_var = (struct variable_t*)malloc(sizeof(struct variable_t));
    if (new_var == NULL) {
        return 0; // Memory allocation failed
    }

    new_var->identifier = (char*)malloc(sizeof(char) * id_len);
    if (new_var->identifier == NULL) {
        free(new_var);
        return 0; // Memory allocation failed
    }

    strncpy(new_var->identifier, identifier, id_len);

    new_var->value = value;

    new_var->prev = var_list_tail;
    new_var->next = NULL;

    // initialize the list
    if (var_list_head == NULL) {
        var_list_head = new_var;
    } else {
        var_list_tail->next = new_var;
    }

    // move the tail pointer
    var_list_tail = new_var;

    return 1; // Variable allocated successfully
}

int is_id_valid(const char* identifier) {
    struct variable_t* cur = var_list_head;
    while (cur != NULL) {
        if (strcmp(cur->identifier, identifier) == 0) {
            return 0; // Identifier already exists
        }
        cur = cur->next;
    }
    return 1; // Identifier is valid
}

struct variable_t* get_variable(const char* identifier) {
    struct variable_t* cur = var_list_head;
    while (cur != NULL) {
        if (strcmp(cur->identifier, identifier) == 0) {
            return cur;
        }
        cur = cur->next;
    }
    return NULL; // Variable not found
}

int oung_input(enum token_t cmd[STMT_LIMIT], unsigned int limit) {
    if (cmd[0] != VAR) {
        return 0; // Invalid input command
    }

    int id_len = 0;
    for (int i = 1; cmd[i] != VAR && i < limit; i += 8) {
        id_len++;
    }

    char id[id_len + 1];
    id[id_len] = '\0';

    for (int i = 0; i < id_len; i++) {
        for (int j = 0; j < 8; j++) {
            enum token_t cur_token = cmd[i * 8 + j + 1];
            id[i] <<= 1;
            if (cur_token == ONE) {
                id[i] |= 1;
            } else if (cur_token != ZERO) {
                return 0; // Invalid token
            }
        }
    }

    char input;
    scanf(" %c", &input);

    return allocate_var(id, id_len, input);
}

int oung_init_var(enum token_t cmd[STMT_LIMIT], unsigned int limit) {
    if (cmd[0] != VAR) {
        return 0; // Invalid command
    }

    int id_len = 0;
    for (int i = 1; cmd[i] != VAR && i < limit; i += 8) {
        id_len++;
    }

    char id[id_len + 1];
    id[id_len] = '\0';

    for (int i = 0; i < id_len; i++) {
        for (int j = 0; j < 8; j++) {
            enum token_t cur_token = cmd[i * 8 + j + 1];
            id[i] <<= 1;
            if (cur_token == ONE) {
                id[i] |= 1;
            } else if (cur_token != ZERO) {
                return 0; // Invalid token
            }
        }
    }

    char value = 0;
    for (int i = id_len * 8 + 2; i < limit; i++) {
        value <<= 1;
        if (cmd[i] == ONE) {
            value |= 1;
        } else if (cmd[i] != ZERO) {
            return 0; // Invalid token
        }
    }

    return allocate_var(id, id_len, value);
}

int oung_eval(enum token_t cmd[STMT_LIMIT], unsigned int limit, enum token_t mode) {
    if (cmd[0] != VAR) {
        return 0; // Invalid command
    }

    int id_len = 0;
    for (int i = 1; cmd[i] != VAR && i < limit; i += 8) {
        id_len++;
    }

    char l_operand[id_len + 1];
    l_operand[id_len] = '\0';

    for (int i = 0; i < id_len; i++) {
        for (int j = 0; j < 8; j++) {
            enum token_t cur_token = cmd[i * 8 + j + 1];
            l_operand[i] <<= 1;
            if (cur_token == ONE) {
                l_operand[i] |= 1;
            } else if (cur_token != ZERO) {
                return 0; // Invalid token
            }
        }
    }

    char r_value = 0;

    if (cmd[id_len * 8 + 2] != SEP) {
        return 0; // Invalid token
    } else if (cmd[id_len * 8 + 3] == VAR) {
        if (cmd[limit - 1] != VAR) {
            return 0; // Invalid token
        }

        int r_id_len = 0;
        for (int i = id_len * 8 + 4; cmd[i] != VAR && i < limit; i += 8) {
            r_id_len++;
        }

        char r_operand[r_id_len + 1];
        r_operand[r_id_len] = '\0';

        for (int i = 0; i < r_id_len; i++) {
            for (int j = 0; j < 8; j++) {
                enum token_t cur_token = cmd[(id_len * 8 + 4) + i * 8 + j];

                r_operand[i] <<= 1;

                if (cur_token == ONE) {
                    r_operand[i] |= 1;
                } else if (cur_token != ZERO) {
                    return 0; // Invalid token
                }
            }
        }

        r_value = get_variable(r_operand)->value;
        deallocate_var(r_operand);
    } else {
        for (int i = id_len * 8 + 3; i < limit; i++) {
            r_value <<= 1;

            if (cmd[i] == ONE) {
                r_value |= 1;
            } else if (cmd[i] != ZERO) {
                return 0; // Invalid token
            }
        }
    }

    struct variable_t* l_var = get_variable(l_operand);

    switch (mode) {
        case ADD:
            l_var->value += r_value;
            break;
        case SUB:
            l_var->value -= r_value;
            break;
        case MUL:
            l_var->value *= r_value;
            break;
        case DIV:
            if (r_value == 0) {
                return 0; // Division by zero error
            }
            l_var->value /= r_value;
            break;
        case AND:
            l_var->value &= r_value;
            break;
        case OR:
            l_var->value |= r_value;
            break;
        default:
            return 0; // Invalid mode
    }

    return 1; // Evaluation successful
}

int oung_print(enum token_t cmd[STMT_LIMIT], unsigned int limit, enum token_t mode) {
    if (cmd[0] == VAR) {
        int id_len = 0;
        for (int i = 1; cmd[i] != VAR && i < limit; i += 8) {
            id_len++;
        }

        char id[id_len + 1];
        id[id_len] = '\0';

        for (int i = 0; i < id_len; i++) {
            for (int j = 0; j < 8; j++) {
                enum token_t cur_token = cmd[i * 8 + j + 1];
                id[i] <<= 1;
                if (cur_token == ONE) {
                    id[i] |= 1;
                } else if (cur_token != ZERO) {
                    printf("Given identifier is not a completed byte!");
                    return 0; // Invalid token
                }
            }
        }

        struct variable_t* var = get_variable(id);
        if (var == NULL) {
            printf("Could not find the variable! "
                   "Maybe it was deleted by OVOT policy.");
            return 0; // Variable not found
        }

        if (mode == PRINT_NUM) {
            printf("%d\n", var->value);
        } else {
            printf("%c", var->value);
        }

        deallocate_var(var->identifier);
    } else if (cmd[0] == ONE || cmd[0] == ZERO) {
        char printed_char = 0;

        for (int i = 0; i < limit; i++) {
            printed_char <<= 1;

            if (cmd[i] == ONE) {
                printed_char |= 1;
            } else if (cmd[i] != ZERO) {
                return 0; // Invalid token
            }
        }

        if (mode == PRINT_NUM) {
            printf("%d\n", printed_char);
        } else {
            printf("%c", printed_char);
        }
    } else {
        return 0; // Invalid command
    }

    return 1; // Print successful
}

void traverse_forward() {
    struct variable_t* cur = var_list_head;
    while (cur != NULL) {
        printf("%s\n", cur->identifier);
        cur = cur->next;
    }
}

void traverse_backward() {
    struct variable_t* cur = var_list_tail;
    while (cur != NULL) {
        printf("%s\n", cur->identifier);
        cur = cur->prev;
    }
}

int deallocate_var(char* identifier) {
    struct variable_t* cur = var_list_head;
    while (cur != NULL) {
        if (strcmp(cur->identifier, identifier) == 0) {
            // case [X] <-> [] -> 
            if (cur == var_list_head) {
                var_list_head = cur->next;

                if (cur->next != NULL) {
                    cur->next->prev = NULL;
                }
            }

            // case -> [] <-> [X] ->
            else if (cur == var_list_tail) {
                var_list_tail = cur->prev;

                if (cur->prev != NULL) {
                    cur->prev->next = NULL;
                }
            }

            // case -> [] <-> [X] <-> [] ->
            else {
                cur->prev->next = cur->next;
                cur->next->prev = cur->prev;
            }

            free(cur->identifier);
            free(cur);
            break;
        }

        cur = cur->next;
    }

    return 1; // Deallocation successful
}

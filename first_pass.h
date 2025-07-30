#ifndef FIRST_PASS_H
#define FIRST_PASS_H

#define MAX_LABEL_LENGTH 31
#define MAX_SYMBOLS 100

typedef enum { CODE, DATA, EXTERN, ENTRY } SymbolType;

typedef struct {
    char name[MAX_LABEL_LENGTH];
    int address;
    SymbolType type;
} Symbol;

extern Symbol symbol_table[MAX_SYMBOLS];
extern int symbol_count;

void first_pass(const char *filename);
void print_symbol_table();

int count_data_words(const char *line);
int count_string_chars(const char *line);
int count_mat_words(const char *line);
int is_instruction(const char *line);
int count_instruction_words(const char *line);
int symbol_exists(const char *name);


#endif

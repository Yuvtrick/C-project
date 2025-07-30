#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "first_pass.h"
#include <stdlib.h>

#define MAX_LINE_LENGTH 81
#define IC_INIT_VALUE 100

Symbol symbol_table[MAX_SYMBOLS];
int symbol_count = 0;

int IC = IC_INIT_VALUE;
int DC = 0;

/* checks if a label name is valid */
int is_valid_label(const char *label)
{
    int i;
    if (!isalpha(label[0])) return 0;
    for (i = 1; label[i] != '\0'; i++)
        {
        if (!isalnum(label[i]) && label[i] != '_') return 0;
    }
    return 1;
}

/* add a label to the symbol table */
void add_symbol(const char *name, int address, SymbolType type) 
{
    if (symbol_count >= MAX_SYMBOLS)
        {
        printf("Error: Symbol table full\n");
        return;
    }

    strncpy(symbol_table[symbol_count].name, name, MAX_LABEL_LENGTH - 1);
    symbol_table[symbol_count].name[MAX_LABEL_LENGTH - 1] = '\0';
    symbol_table[symbol_count].address = address;
    symbol_table[symbol_count].type = type;
    symbol_count++;
}

/* looking for a label in a row */
int extract_label(const char *line, char *label_out)
{
    const char *colon;
    int len;

    colon = strchr(line, ':');
    if (colon != NULL) 
    {
        len = colon - line;
        if (len < MAX_LABEL_LENGTH)
            {
            strncpy(label_out, line, len);
            label_out[len] = '\0';
            return 1;
        }
    }
    return 0;
}

/* checks if a row of data (.data/.string) */
int is_data_instruction(const char *line)
{
    return strstr(line, ".data") != NULL || strstr(line, ".string") != NULL;
}

/* checks if an outside line */
int is_extern_instruction(const char *line)
{
    return strstr(line, ".extern") != NULL;
}

const char *instruction_names[] =
    {
    "mov", "cmp", "add", "sub", "lea",
    "clr", "not", "inc", "dec",
    "jmp", "bne", "jsr",
    "red", "prn",
    "rts", "stop"
};

int is_instruction(const char *line)
{
    int i;
    char instr[10];
    sscanf(line, "%s", instr);
    for (i = 0; i < sizeof(instruction_names) / sizeof(instruction_names[0]); i++)
        {
        if (strcmp(instr, instruction_names[i]) == 0)
            return 1;
    }
    return 0;
}


int count_data_words(const char *line)
{
    int count = 0;
    const char *ptr = strstr(line, ".data");
    if (!ptr) return 0;

    ptr += strlen(".data");
    while (*ptr)
        {
        while (isspace(*ptr) || *ptr == ',') ptr++;
        if (*ptr == '\0') break;
        if (isdigit(*ptr) || *ptr == '-' || *ptr == '+')
            {
            count++;
            while (*ptr && (*ptr == '-' || *ptr == '+' || isdigit(*ptr))) ptr++;
        } else
            {
            break;
        }
    }
    return count;
}

int count_string_chars(const char *line)
{
    const char *start = strchr(line, '"');
    const char *end;
    int count = 0;

    if (!start) return 0;
    start++;
    end = strchr(start, '"');
    if (!end) return 0;

    while (start < end) 
    {
        count++;
        start++;
    }
    return count + 1;
}

int count_mat_words(const char *line)
{
    int rows = 0, cols = 0;
    const char *ptr = strstr(line, ".mat");
    if (!ptr) return 0;

    ptr += strlen(".mat");
    while (*ptr && *ptr != '[') ptr++;
    if (*ptr != '[') return 0;
    ptr++;
    rows = atoi(ptr);

    while (*ptr && *ptr != ']') ptr++;
    while (*ptr && *ptr != '[') ptr++;
    if (*ptr != '[') return 0;
    ptr++;
    cols = atoi(ptr);

    return rows * cols;
}

int count_instruction_words(const char *line)
{
    char command[10], operand1[31], operand2[31];
    int operand_count = 0;

    if (sscanf(line, "%s %30[^,], %30s", command, operand1, operand2) == 3)
        operand_count = 2;
    else if (sscanf(line, "%s %30s", command, operand1) == 2)
        operand_count = 1;
    else
        operand_count = 0;

    return 1 + operand_count;
}

int symbol_exists(const char *name) 
{
    int i;
    for (i = 0; i < symbol_count; i++) {
        if (strcmp(symbol_table[i].name, name) == 0) {
            return 1;
        }
    }
    return 0;
}

/* returns the first actual word in a line — ignoring labels */
char *get_first_real_word(char *line)
{
    static char word[MAX_LINE_LENGTH];
    char *copy = strdup(line);
    char *token;

    token = strtok(copy, " \t");
    if (token == NULL) {
        free(copy);
        return NULL;
    }

    if (strchr(token, ':') != NULL)
        {
        token = strtok(NULL, " \t");
    }

    if (token != NULL)
        {
        strncpy(word, token, MAX_LINE_LENGTH);
        word[MAX_LINE_LENGTH - 1] = '\0';
    } else {
        word[0] = '\0';
    }

    free(copy);
    return word;
}


/* first pass */
void first_pass(const char *filename)
{
    FILE *file;
    char input_filename[100];
    char line[MAX_LINE_LENGTH];
    char label[MAX_LABEL_LENGTH];
    char *trimmed;
    int has_label;
    int count = 0;
    char extern_label[MAX_LABEL_LENGTH];
    char *first_word;

    snprintf(input_filename, sizeof(input_filename), "%s.am", filename);
    file = fopen(input_filename, "r");

    if (!file)
        {
        printf("Error: Cannot open file %s\n", input_filename);
        return;
    }

    while (fgets(line, MAX_LINE_LENGTH, file))
    {
        trimmed = line;
        while (isspace(*trimmed)) trimmed++;
        if (trimmed[0] == '\0' || trimmed[0] == ';') continue;

        has_label = extract_label(trimmed, label);
        if (has_label) {
            if (!is_valid_label(label)) 
            {
                printf("Error: Invalid label '%s'\n", label);
                continue;
            }
            if (symbol_exists(label)) 
            {
                printf("Error: Duplicate label '%s'\n", label);
                continue;
            }
        }

        if (is_extern_instruction(trimmed))
        {
            sscanf(trimmed, ".extern %s", extern_label);
            add_symbol(extern_label, 0, EXTERN);
        }
        else if (strstr(trimmed, ".data")) 
        {
            count = count_data_words(trimmed);
            if (has_label) add_symbol(label, DC, DATA);
            DC += count;
        }
        else if (strstr(trimmed, ".string"))
        {
            count = count_string_chars(trimmed);
            if (has_label) add_symbol(label, DC, DATA);
            DC += count;
        }
        else if (strstr(trimmed, ".mat"))
        {
            count = count_mat_words(trimmed);
            if (has_label) add_symbol(label, DC, DATA);
            DC += count;
        }
        else
        {
            first_word = get_first_real_word(trimmed);
            if (first_word && is_instruction(first_word))
            {
                count = count_instruction_words(trimmed);
                if (has_label) add_symbol(label, IC, CODE);
                IC += count;
            }
        }
    }


    fclose(file);
}

/* Print symbol table */
void print_symbol_table()
{
    int i;
    printf("\nSymbol Table:\n");
    printf("%-15s %-10s %-5s\n", "Label", "Address", "Type");

    for (i = 0; i < symbol_count; i++)
        {
        printf("%-15s %-10d ", symbol_table[i].name, symbol_table[i].address);
        switch (symbol_table[i].type)
        {
            case CODE:
                printf("CODE\n");
                break;
            case DATA:
                printf("DATA\n");
                break;
            case EXTERN:
                printf("EXTERN\n");
                break;
            case ENTRY:
                printf("ENTRY\n");
                break;
        }
    }
}

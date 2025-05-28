#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_LINE_LENGTH 81
#define MAX_MACROS 50
#define MAX_MACRO_NAME 31
#define MAX_MACRO_INSIDE 1000

typedef struct {
    char name[MAX_MACRO_NAME];
    char inside[MAX_MACRO_INSIDE];
} Macro;

Macro macro_table[MAX_MACROS];
int macro_count = 0;

void add_macro(char *name, char *inside)
{
    if (macro_count >= MAX_MACROS)
    {
        printf("too many macros\n");
        return;
    }
    strncpy(macro_table[macro_count].name, name, MAX_MACRO_NAME);
    strncpy(macro_table[macro_count].inside, inside, MAX_MACRO_INSIDE);
    macro_count++;
}

char *get_macro(char *name)
{
    int i;
    for (i = 0; i < macro_count; i++)
    {
        if (strcmp(macro_table[i].name, name) == 0)
        {
            return macro_table[i].inside;
        }
    }
    return NULL;
}


void remove_newline(char *line)
{
    size_t len = strlen(line);
    if (len > 0 && line[len - 1] == '\n')
    {
        line[len - 1] = '\0';
    }
}

char *remove_whitespaces(char *str)
{
    while (*str == ' ' || *str == '\t') {
        str++;
    }
    return str;
}



char *system_words[] = {"mov", "cmp", "add", "sub", "not", "clr",
    "lea", "inc", "dec", "jmp", "bne", "red", "prn", "jsr",
    "rts", "stop", ".data", ".string", ".extern", ".entry",
    "mcro", "endmcro", NULL};

int is_system_word(const char *word)
{
    int i = 0;
    while (system_words[i] != NULL)
    {
        if (strcmp(system_words[i], word) == 0)
        {
            return 1;
        }
        i++;
    }
    return 0;
}

/* This pre-assembler gets the code and "clean" it which mean its removing unnecessary whitespaces/newlines
 and replacing the macros name with their inside information(the body of the macro)*/
void preassembler_file(char *filename)
{
    FILE *input;
    FILE *output;
    char input_filename[100];
    char output_filename[100];
    char line[MAX_LINE_LENGTH];
    char original_line[MAX_LINE_LENGTH];
    char line_copy[MAX_LINE_LENGTH];
    char macro_name[MAX_MACRO_NAME];
    char macro_inside[MAX_MACRO_INSIDE];
    char *first_word;
    char *name;
    char *macro_body;
    char *line_without_whitespace;
    bool flag_in_macro = false;


    macro_inside[0] = '\0';

    /* making the .as and .am files */
    snprintf(input_filename, sizeof(input_filename), "%s.as", filename);
    snprintf(output_filename, sizeof(output_filename), "%s.am", filename);

    input = fopen(input_filename, "r");
    if (!input)
    {
        printf("Cant open file %s\n", input_filename);
        return;
    }

    output = fopen(output_filename, "w");
    if (!output)
    {
        printf("Cant open file %s\n", output_filename);
        fclose(input);
        return;
    }

    /* starts checking the lines and words of the file */
    while (fgets(line, MAX_LINE_LENGTH, input))
    {
        strcpy(original_line, line);
        remove_newline(line);

        /* removing unnecessary lines */
        if (!flag_in_macro && (strlen(line) == 0 || line[0] == '\n' || line[0] == ';'))
        {
            continue;
        }

        line_without_whitespace = remove_whitespaces(line);
        strcpy(line_copy, line_without_whitespace);
        first_word = strtok(line_copy, " \t");

        if (first_word == NULL)
        {
            continue;
        }

        /* enter into the macro and getting its name */
        if (strcmp(first_word, "mcro") == 0)
        {
            flag_in_macro = true;
            name = strtok(NULL, " \t");
            if (name != NULL)
            {
                if (is_system_word(name))
                {
                    printf("macro name '%s' is a reserved word.\n", name);
                    exit(1);
                    continue;
                }
                strncpy(macro_name, name, MAX_MACRO_NAME);
                macro_inside[0] = '\0';
            }
            continue;
        }

        /* when the macro ends we add it into the table and save it */
        if (strcmp(first_word, "mcroend") == 0 && flag_in_macro == true)
        {
            add_macro(macro_name, macro_inside);
            flag_in_macro = false;
            continue;
        }

        /* add the relevant line into the macro */
        if (flag_in_macro == true)
        {
            strcat(macro_inside, original_line);
            continue;
        }

        macro_body = get_macro(first_word);

        /* add the relevant line(macro line or regular line) into the .am file */
        if (macro_body)
        {
            fputs(macro_body, output);
        }
        else
        {
            fputs(original_line, output);
        }
    }

    fclose(input);
    fclose(output);
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "preassembler.h"
#include "first_pass.h"
#include "second_pass.h"

int main(int argc, char *argv[]) {
int i;

    if (argc < 2) {
        printf("Usage: %s <source file name without extension>\n", argv[0]);
        return 1;
    }

    for (i = 1; i < argc; i++) {
        const char *filename = argv[i];

        printf("Processing file: %s\n", filename);

        /* step 1: pre-assembler – creates .am file */
        preassembler_file(filename);

        /* step 2: first pass – constructs a symbol table */
        first_pass(filename);
        print_symbol_table();

        /* step 3: second pass – translates to machine code and creates output files */
        //second_pass(filename);

        printf("Finished processing: %s\n\n", filename);
    }

    return 0;
}

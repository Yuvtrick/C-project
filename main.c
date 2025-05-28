#include <stdio.h>

void preassembler_file(const char *filename);

int main(int argc, char *argv[]) {
    preassembler_file(argv[1]);
    return 0;
}


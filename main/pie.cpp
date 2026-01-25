#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>

#include "compiler/scanner.h"
#include "compiler/parser.h"
#include "compiler/backend/print.h"
#include "compiler/backend/eval.h"

using namespace pie::compiler;

void printUsage(const char *prog)
{
    fprintf(stderr, "Usage: %s [options] <file.pie>\n", prog);
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  --print    Print the AST (don't execute)\n");
    fprintf(stderr, "  --help     Show this help message\n");
}

int main(int argc, char **argv)
{
    FILE *file = NULL;
    bool print_mode = false;
    const char *filename = nullptr;

    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--print") == 0) {
            print_mode = true;
        } else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            printUsage(argv[0]);
            return 0;
        } else if (argv[i][0] != '-') {
            filename = argv[i];
        } else {
            fprintf(stderr, "Unknown option: %s\n", argv[i]);
            printUsage(argv[0]);
            return 1;
        }
    }

    if (filename) {
        file = fopen(filename, "r");
        if (!file) {
            fprintf(stderr, "Failed to open file: %s\n", filename);
            return 1;
        }
    } else {
        fprintf(stderr, "No input file specified.\n");
        printUsage(argv[0]);
        return 1;
    }

    // Parse the source file
    Scanner scanner(file);
    Parser parser(scanner);

    int ret = parser.parse();

    fclose(file);

    if (ret != 0) {
        fprintf(stderr, "Failed to parse: %s\n", filename);
        return 2;
    }

    ModuleNode *module = parser.module;

    if (print_mode) {
        // Print mode: output the AST
        PrintVisitor printer;
        module->visit(&printer);
        std::cout << printer.output();
    } else {
        // Execution mode: run the program
        try {
            EvalVisitor interpreter;
            Value result = interpreter.run(module);

            // If main returned a value, use it as exit code
            if (result.type == Value::Type::Int) {
                return (int)result.int_val;
            }
        } catch (const std::exception &e) {
            fprintf(stderr, "Runtime error: %s\n", e.what());
            return 3;
        }
    }

    return 0;
}

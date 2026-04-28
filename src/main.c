#include "assembler.h"
#include "executor.h"
#include "parser.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

void print_usage(const char *prog) {
    printf("Uso:\n");
    printf("  %s assemble <entrada.asm> <saida.mem>\n", prog);
    printf("  %s run <entrada.mem> [--hex|--dec] [--step]\n", prog);
    printf("  %s asmrun <entrada.asm> [--hex|--dec] [--step]\n", prog);
    printf("  %s parse \"<expressao>\"\n", prog);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "assemble") == 0) {
        char error[256];
        if (argc < 4) {
            print_usage(argv[0]);
            return 1;
        }
        if (!assemble_file(argv[2], argv[3], error, sizeof(error))) {
            printf("Erro no assembler: %s\n", error);
            return 1;
        }
        printf("Assembly concluido com sucesso: %s\n", argv[3]);
        return 0;
    }

    if (strcmp(argv[1], "run") == 0 || strcmp(argv[1], "asmrun") == 0) {
        uint8_t memory[256];
        uint8_t origin = 0;
        int use_hex = 0;
        int step_mode = 0;
        int i;
        char error[256];

        if (argc < 3) {
            print_usage(argv[0]);
            return 1;
        }

        for (i = 3; i < argc; i++) {
            if (strcmp(argv[i], "--hex") == 0) {
                use_hex = 1;
            } else if (strcmp(argv[i], "--dec") == 0) {
                use_hex = 0;
            } else if (strcmp(argv[i], "--step") == 0) {
                step_mode = 1;
            }
        }

        if (strcmp(argv[1], "run") == 0) {
            if (!load_mem_file(argv[2], &origin, memory, error, sizeof(error))) {
                printf("Erro ao carregar arquivo .mem: %s\n", error);
                return 1;
            }
        } else {
            if (!assemble_to_memory(argv[2], &origin, memory, error, sizeof(error))) {
                printf("Erro no assembler: %s\n", error);
                return 1;
            }
        }

        run_cpu(memory, origin, step_mode, use_hex);
        return 0;
    }

    if (strcmp(argv[1], "parse") == 0) {
        int result;
        char error[256];

        if (argc < 3) {
            print_usage(argv[0]);
            return 1;
        }

        if (!evaluate_expression(argv[2], &result, error, sizeof(error))) {
            printf("Erro no parser: %s\n", error);
            return 1;
        }

        printf("Resultado = %d\n", result);
        return 0;
    }

    print_usage(argv[0]);
    return 1;
}

#include "executor.h"

#include <stdio.h>
#include <string.h>

uint8_t memoria_exec[256];
uint8_t AC = 0;
uint8_t PC = 0;
uint8_t IR = 0;
uint8_t MAR = 0;
uint8_t MDR = 0;
int flagN = 0;
int flagZ = 0;
unsigned long acessos_memoria = 0;
unsigned long instrucoes_executadas = 0;

void atualiza_flags(void) {
    flagZ = (AC == 0) ? 1 : 0;
    flagN = ((AC & 0x80u) != 0u) ? 1 : 0;
}

void imprimir_memoria_nao_zero(const uint8_t memoria[256], int usar_hex) {
    int i;
    for (i = 0; i < 256; i++) {
        if (memoria[i] != 0) {
            if (usar_hex) {
                printf("  [0x%02X] = 0x%02X\n", i, memoria[i]);
            } else {
                printf("  [%3d] = %d\n", i, memoria[i]);
            }
        }
    }
}

void imprimir_estado_cpu(int usar_hex) {
    if (usar_hex) {
        printf("AC=0x%02X PC=0x%02X IR=0x%02X MAR=0x%02X MDR=0x%02X N=%d Z=%d\n",
               AC, PC, IR, MAR, MDR, flagN, flagZ);
    } else {
        printf("AC=%d PC=%d IR=%d MAR=%d MDR=%d N=%d Z=%d\n",
               AC, PC, IR, MAR, MDR, flagN, flagZ);
    }
}

int executar_passo(void) {
    uint8_t op_addr;
    uint8_t operand;
    /* ciclo basico: busca instrucao no PC */
    MAR = PC;
    MDR = memoria_exec[MAR];
    IR = MDR;
    acessos_memoria++;
    instrucoes_executadas++;

    switch (IR) {
        case 0x00:
            PC += 1;
            break;
        case 0x10:
            op_addr = memoria_exec[(uint8_t)(PC + 1)];
            acessos_memoria++;
            MAR = op_addr;
            MDR = AC;
            memoria_exec[MAR] = MDR;
            acessos_memoria++;
            PC += 2;
            break;
        case 0x20:
            op_addr = memoria_exec[(uint8_t)(PC + 1)];
            acessos_memoria++;
            MAR = op_addr;
            MDR = memoria_exec[MAR];
            acessos_memoria++;
            AC = MDR;
            atualiza_flags();
            PC += 2;
            break;
        case 0x30:
            op_addr = memoria_exec[(uint8_t)(PC + 1)];
            acessos_memoria++;
            MAR = op_addr;
            MDR = memoria_exec[MAR];
            acessos_memoria++;
            AC = (uint8_t)(AC + MDR);
            atualiza_flags();
            PC += 2;
            break;
        case 0x40:
            op_addr = memoria_exec[(uint8_t)(PC + 1)];
            acessos_memoria++;
            MAR = op_addr;
            MDR = memoria_exec[MAR];
            acessos_memoria++;
            AC = (uint8_t)(AC | MDR);
            atualiza_flags();
            PC += 2;
            break;
        case 0x50:
            op_addr = memoria_exec[(uint8_t)(PC + 1)];
            acessos_memoria++;
            MAR = op_addr;
            MDR = memoria_exec[MAR];
            acessos_memoria++;
            AC = (uint8_t)(AC & MDR);
            atualiza_flags();
            PC += 2;
            break;
        case 0x70:
            AC = (uint8_t)(~AC);
            atualiza_flags();
            PC += 1;
            break;
        case 0x80:
            operand = memoria_exec[(uint8_t)(PC + 1)];
            acessos_memoria++;
            PC = operand;
            break;
        case 0x90:
            operand = memoria_exec[(uint8_t)(PC + 1)];
            acessos_memoria++;
            /* JN so desvia quando o acumulador ficou negativo */
            if (flagN) {
                PC = operand;
            } else {
                PC += 2;
            }
            break;
        case 0xA0:
            operand = memoria_exec[(uint8_t)(PC + 1)];
            acessos_memoria++;
            /* JZ so desvia quando AC == 0 */
            if (flagZ) {
                PC = operand;
            } else {
                PC += 2;
            }
            break;
        case 0xF0:
            PC += 1;
            return 0;
        default:
            printf("Opcode invalido 0x%02X em 0x%02X\n", IR, MAR);
            return 0;
    }
    return 1;
}

void run_cpu(uint8_t memory[256], uint8_t origin, int step_mode, int use_hex) {
    uint8_t before[256];
    char line[16];
    int running = 1;
    memcpy(before, memory, 256);
    memcpy(memoria_exec, memory, 256);

    AC = 0;
    PC = origin;
    IR = 0;
    MAR = 0;
    MDR = 0;
    flagN = 0;
    flagZ = 0;
    acessos_memoria = 0;
    instrucoes_executadas = 0;
    atualiza_flags();

    while (running) {
        if (step_mode) {
            /* no step, mostra estado antes de executar a instrucao */
            imprimir_estado_cpu(use_hex);
            printf("Pressione ENTER para executar a proxima instrucao...");
            fgets(line, sizeof(line), stdin);
        }
        running = executar_passo();
    }

    memcpy(memory, memoria_exec, 256);
    printf("\nMEMORIA ANTES DA EXECUCAO:\n");
    imprimir_memoria_nao_zero(before, use_hex);
    printf("\nRESULTADO:\n");
    imprimir_estado_cpu(use_hex);
    printf("Acessos a memoria = %lu\n", acessos_memoria);
    printf("Instrucoes executadas = %lu\n", instrucoes_executadas);
    printf("\nMEMORIA DEPOIS DA EXECUCAO:\n");
    imprimir_memoria_nao_zero(memory, use_hex);
}

#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <stddef.h>
#include <stdint.h>

int assemble_to_memory(const char *asm_path, uint8_t *origin, uint8_t memory[256], char *error_msg, size_t error_msg_sz);
int assemble_file(const char *asm_path, const char *mem_path, char *error_msg, size_t error_msg_sz);
int load_mem_file(const char *path, uint8_t *origin, uint8_t memory[256], char *error_msg, size_t error_msg_sz);

#endif

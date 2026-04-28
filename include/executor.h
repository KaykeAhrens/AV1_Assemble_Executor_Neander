#ifndef EXECUTOR_H
#define EXECUTOR_H

#include <stdint.h>

void run_cpu(uint8_t memory[256], uint8_t origin, int step_mode, int use_hex);

#endif

/* File: cpu.c */
/* CPU architecture and organization. */

#include <stdint.h>

#include "cpu.h"

struct cpu_context_t cpu_context;

int16_t ir0;
int16_t ir1;

uint16_t mar;

int16_t *operand[2];

int16_t ivtp;
int16_t ivtentry;


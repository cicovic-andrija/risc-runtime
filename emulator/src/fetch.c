/* File: fetch.h */
/* CPU fetch block. */

#include <stdlib.h>

#include "log.h"
#include "cpu.h"
#include "mem.h"
#include "constants.h"
#include "fetch.h"

void fetch(void)
{
    unsigned char *byte = NULL;

    /* read first instruction word */
    byte = mem + (uint16_t)cpu_context.reg[7];
    ++cpu_context.reg[7];
    ir0 = *byte << 8;
    byte = mem + (uint16_t)cpu_context.reg[7];
    ++cpu_context.reg[7];
    ir0 |= *byte;

    int long_instruction = 0;
    int reg0 = (ir0 >> 5) & 0x7;
    int reg1 = ir0 & 0x7;
    int am0 = (ir0 >> 8) & 0x3;
    int am1 = (ir0 >> 3) & 0x3;
    long_instruction |= (am0 == MEMDIR || am0 == REGINDDISP);
    long_instruction |= (am1 == MEMDIR || am1 == REGINDDISP);
    long_instruction |= (am0 == IMMED && reg0 != 0x7);
    long_instruction |= (am1 == IMMED && reg1 != 0x7);
    if (long_instruction)
    {
        /* read second instruction word */
        byte = mem + (uint16_t)cpu_context.reg[7];
        ++cpu_context.reg[7];
        ir1 = *byte;
        byte = mem + (uint16_t)cpu_context.reg[7];
        ++cpu_context.reg[7];
        ir1 |= *byte << 8;
    }
}


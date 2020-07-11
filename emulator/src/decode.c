/* File: decode.c */
/* CPU decode block. */

#include <stdlib.h>

#include "log.h"
#include "cpu.h"
#include "mem.h"
#include "constants.h"
#include "intr.h"
#include "decode.h"

int memory_dst;

void decode(void)
{
    memory_dst = 0;
    mar = (uint16_t) 0xffff;

    int reg_idx[2];
    int address_mode[2];
    reg_idx[0] = (ir0 >> 5) & 0x7;
    reg_idx[1] = ir0 & 0x7;
    address_mode[0] = (ir0 >> 8) & 0x3;
    address_mode[1] = (ir0 >> 3) & 0x3;
    int opcode = (ir0 >> 10) & 0x000f;

    int i;
    for (i = 0; i < 2; ++i)
    {
        switch (address_mode[i])
        {
        case IMMED:
            if (reg_idx[i] == 0x7)
            {
                /* if IMMED addressed operand's register field is 0x7, PSW is used */
                operand[i] = &cpu_context.psw;
            }
            else
            {
                operand[i] = &ir1;

                if (i == 0 && opcode != PUSH && opcode != IRET) /* illegal instruction */
                {
                    intr = 1;
                    ivtentry = 2;
                    return;
                }
            }
            break;
        case REGDIR:
            operand[i] = &cpu_context.reg[reg_idx[i]];
            break;
        case MEMDIR:
            mar = (uint16_t) ir1;
            operand[i] = (int16_t *)(mem + mar);
            if (i == 0) memory_dst = 1;
            break;
        case REGINDDISP:
            mar = (uint16_t)(cpu_context.reg[reg_idx[i]] + ir1);
            operand[i] = (int16_t *)(mem + mar);
            if (i == 0) memory_dst = 1;
            break;
        default:
            break;
        }
    }
}


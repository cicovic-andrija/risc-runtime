/* File: intr.c */
/* CPU interrupt block. */

#include "cpu.h"
#include "mem.h"
#include "exec.h"
#include "intr.h"

int intr;

void push(int16_t);
void pop(int16_t *);

void interrupt(void)
{
    if (!intr)
        return;

    if (PSW_TEST_FLAG(PSW_FLAG_I))
        return;

    intr = 0;

    push(cpu_context.reg[7]);
    push(cpu_context.psw);

    PSW_SET_FLAG(PSW_FLAG_I);

    ivtentry &= 0x7; /* look at the 3 least significant bits (8 entries in IVT) */
    uint16_t intr_routine_addr = ivtp + ivtentry * IVTENTRY_SIZE;
    cpu_context.reg[7] = (int16_t) *(mem + intr_routine_addr);
    cpu_context.reg[7] |= (int16_t) *(mem + intr_routine_addr + 1) << 8;

    if (cpu_context.reg[7] == 0) /* null pointer to interrupt routine */
    {
        pop(&cpu_context.psw);
        pop(&cpu_context.reg[7]);
    }
}


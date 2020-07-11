/* File: cpu.h */
/* CPU architecture and organization. */

#ifndef CPU_H
#define CPU_H

#include <stdint.h>

/* Instruction outline.
 *
 * BYTE 1 MEM[PC]                                  BYTE 2 MEM[PC + 1]
 * +-----------+--------+--------------------------+----------------------+---------------------+-----------------+
 * | condition | opcode | destination address mode | destination register | source address mode | source register |
 * +-----------+--------+--------------------------+----------------------+---------------------+-----------------+
 *  2 bits      4 bits   2 bits                     3 bits                 2 bits                3 bits
 *
 * BYTE 3 MEM[PC + 2]                              BYTE 4 MEM[PC + 3]
 * +-----------+--------+--------------------------+----------------------+---------------------+-----------------+
 * | address, offset or immediate data (low byte)  | address, offset or immediate data (high byte)                |
 * +-----------+--------+--------------------------+----------------------+---------------------+-----------------+
 *  8 bits                                          8 bits
 **/

/* PSW macro definitions. */
#define PSW_FLAG_Z 0x0001 /* zero */
#define PSW_FLAG_O 0x0002 /* overflow */
#define PSW_FLAG_C 0x0004 /* carry */
#define PSW_FLAG_N 0x0008 /* negative */
#define PSW_FLAG_H 0x0010 /* halt */
#define PSW_FLAG_T 0x2000 /* timer interrupt switch */
#define PSW_FLAG_I 0x8000 /* interrupt mask */

#define PSW_SET_FLAG(flag) ((cpu_context.psw) |= (flag))
#define PSW_CLEAR_FLAG(flag) ((cpu_context.psw) &= ~(flag))
#define PSW_TEST_FLAG(flag) ((cpu_context.psw) & (flag))

/* Input/output devices. */
#define OUTPUT_DEVICE_ADDRESS ((uint16_t) 0xfffe)
#define INPUT_DEVICE_ADDRESS ((uint16_t) 0xfffc)

/* Timer tick period in seconds. */
#define TIMER_PERIOD_IN_SEC 1

/* Default IVT entries. */
#define CPU_RESET_IVTENTRY 0
#define TIMER_TICK_IVTENTRY 1
#define ILLEGAL_INSTRUCTION_IVTENTRY 2
#define INPUT_DEVICE_IVTENTRY 3

#define ILLEGAL_INSTRUCTION ((intr) && (ivtentry == ILLEGAL_INSTRUCTION_IVTENTRY))

/* CPU context definition. */
/* General purpose registers and program status word. */
struct cpu_context_t {
    int16_t reg[8];
    int16_t psw;
};
extern struct cpu_context_t cpu_context;

/* Instruction registers. */
extern int16_t ir0;
extern int16_t ir1;

/* Memory address register. */
extern uint16_t mar;

/* Operand addresses. */
extern int16_t *operand[2];

/* Interrupt Vector Table Pointer. */
extern int16_t ivtp;
extern int16_t ivtentry;

#endif /* CPU_H */


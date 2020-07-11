/* File: control.c */
/* CPU control unit. */

#include <stdlib.h>

#include "log.h"
#include "obj_format.h"
#include "mem.h"
#include "cpu.h"
#include "fetch.h"
#include "decode.h"
#include "exec.h"
#include "intr.h"
#include "devices.h"
#include "control.h"

static SymbolTable symtab;

static ProgramHeaderTable prog_hdrtab;

/* Note: assumed bin is a valid executable file. No error checking performed. */
void load(FILE *bin)
{
    read_symtab(&symtab, bin);
    read_program_hdrtab(&prog_hdrtab, bin);
    ProgramHeaderNode *prog_hdrtab_node = NULL;
    for (prog_hdrtab_node = prog_hdrtab.first; prog_hdrtab_node; prog_hdrtab_node = prog_hdrtab_node->next)
    {
        SegmentRecord segment = prog_hdrtab_node->record;
        read_section(mem + segment.load_addr, segment.size, bin);
    }
}

void init_cpu(void)
{
    SymbolTableEntry *start = find_symbol(&symtab, "START");
    if (!start)
    {
        write_log(LOG_ERROR, "failed to find 'START'");
        exit(EXIT_FAILURE);
    }

    /* interrupt vector table starts at address 0 */
    ivtp = 0;

    /* generate interrupt at startup */
    intr = 1;
    ivtentry = CPU_RESET_IVTENTRY;

    cpu_context.psw = 0;
    PSW_SET_FLAG(PSW_FLAG_T);

    /* reg[7] used as PC */
    cpu_context.reg[7] = start->sym_val;

    /* reg[6] used as SP */
    cpu_context.reg[6] = (int16_t) 0xff7f;
}

void run(FILE *bin)
{
    load(bin);
    init_cpu();
    init_timer();

    while (!PSW_TEST_FLAG(PSW_FLAG_H))
    {
        fetch();
        decode();
        if (!ILLEGAL_INSTRUCTION)
        {
            execute();
            signal_output_device();
        }
        interrupt();
        poll_timer();
        poll_input_device();
    }
}


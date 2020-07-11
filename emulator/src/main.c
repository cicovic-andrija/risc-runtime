/* File: main.c */
/* System software project: emulator */

#include <stdio.h>
#include <stdlib.h>

#include "log.h"
#include "terminal.h"
#include "control.h"

int main(int argc, char *argv[])
{
    if (argc == 1)
    {
        printf("ETF - System software - Emulator v1.0\n"
               "Usage:\n\t%s [exec_file]\n", argv[0]);
        return EXIT_SUCCESS;
    }
    if (argc > 2)
    {
        fprintf(stderr, "%s allows at most one input file\n", argv[0]);
        return EXIT_FAILURE;
    }

    FILE *bin = fopen(argv[1], "rb");
    if (!bin)
    {
        fprintf(stderr, "error: failed to open file '%s'\n", argv[1]);
        return EXIT_FAILURE;
    }

    /* set logging policy */
    set_log_level(LOG_DEBUG);
    open_log("emu.log");
    atexit(close_log);
    write_log(LOG_NORMAL, "file: '%s'", argv[1]);

    /* set terminal settings */
    enable_raw_mode();
    atexit(disable_raw_mode);

    run(bin);

    return EXIT_SUCCESS;
}


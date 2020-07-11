/* File: cmdline.c */
/* Command line arguments parsing. */

#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Note: non-standard header, available on POSIX systems */
#include <getopt.h>

#include "util.h"
#include "cmdline.h"

extern char *src_filename;
extern char *out_filename;
extern char *out_txt_filename;
extern char *log_filename;

extern uint16_t sections_load_address;

void parse_cmdline(int argc, char *argv[]) {
    int c;
    long temp;

    opterr = 0;

    while ((c = getopt(argc, argv, "a:o:t:l:h")) != -1)
    {
        switch (c)
        {
        case 'a':
            if (!is_dec_number((*optarg == '+' || *optarg == '-') ? optarg + 1 : optarg))
            {
                fprintf(stderr, "Argument '%s' is not a valid address\n", optarg);
                exit(EXIT_FAILURE);
            }
            temp = strtol(optarg, NULL, 10);
            if (temp < 0 || temp > UINT16_MAX)
            {
                fprintf(stderr, "Address %s falls out of allowed range\n", optarg);
                exit(EXIT_FAILURE);
            }
            if (temp == UINT16_MAX)
            {
                fprintf(stderr, "Address %s is reserved\n", optarg);
                exit(EXIT_FAILURE);
            }
            sections_load_address = (uint16_t) temp;
            break;
        case 'o':
            out_filename = optarg;
            break;
        case 't':
            out_txt_filename = optarg;
            break;
        case 'l':
            log_filename = optarg;
            break;
        case 'h':
            printf("ETF - System software - Assembler v1.0\n"
                   "Usage:\n\t%s [-o output_file] [-t output_text_file] [-l log_file] "
                   "[-a load_address] [-h] source_file\n\n", argv[0]);
            printf("\t-o file\t-- specify relocatable object filename (default: a.o)\n"
                   "\t-t file\t-- specify text output filename\n"
                   "\t-l file\t-- specify log filename\n"
                   "\t-a     \t-- define load address for sections from the source file\n"
                   "\t-h     \t-- print this message and exit\n");
            exit(EXIT_SUCCESS);
            break;
        case '?':
            if (optopt == 'o' || optopt == 't' || optopt == 'l')
            {
                fprintf(stderr, "Option -%c requires an argument\n", optopt);
            }
            else if (isprint(optopt))
            {
                fprintf(stderr, "Unknown option '-%c'\n", optopt);
            }
            else
            {
                fprintf(stderr, "Unknown option character '\\x%x'\n", optopt);
            }
            exit(EXIT_FAILURE);
            break;
        default:
            abort();
            break;
        }
    }

    int index = optind;
    if (index == argc)
    {
        fprintf(stderr, "%s requires an input file\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    src_filename = argv[index];

    if (++index < argc)
    {
        fprintf(stderr, "%s allows at most one input file\n", argv[0]);
        exit(EXIT_FAILURE);
    }
}


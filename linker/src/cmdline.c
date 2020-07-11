/* File: cmdline.c */
/* Command line arguments parsing. */

#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Note: non-standard header, available on POSIX systems */
#include <getopt.h>

#include "cmdline.h"

extern char *out_filename;
extern char *out_txt_filename;
extern char *log_filename;
extern char *const *object_filenames;

extern int ignore_predefined_origin;

void parse_cmdline(int argc, char *argv[]) {
    int c;

    opterr = 0;

    while ((c = getopt(argc, argv, "o:t:l:h")) != -1)
    {
        switch (c)
        {
        case 'o':
            out_filename = optarg;
            break;
        case 't':
            out_txt_filename = optarg;
            break;
        case 'l':
            log_filename = optarg;
            break;
        case 'i':
            ignore_predefined_origin = 1;
            break;
        case 'h':
            printf("ETF - System software - Linker v1.0\n"
                    "Usage:\n\t%s [-o output_file] [-t output_text_file] [-l log_file] "
                    "[-i] [-h] object_file...\n\n", argv[0]);
            printf("\t-o file\t-- specify executable filename (default: a.out)\n"
                   "\t-t file\t-- specify text output filename\n"
                   "\t-l file\t-- specify log filename\n"
                   "\t-i     \t-- ignore section load addresses predefined in input files\n"
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
        fprintf(stderr, "%s requires at least one input file\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    object_filenames = argv + index;
}


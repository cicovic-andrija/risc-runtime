/* File: main.c */
/* System software project: assembler */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "log.h"
#include "cmdline.h"
#include "input.h"
#include "lexer.h"
#include "parser.h"
#include "codegen.h"

#include "tests.h"

char *src_filename = NULL;
char *out_filename = "a.o";
char *out_txt_filename = NULL;
char *log_filename = NULL;

uint16_t sections_load_address = (uint16_t) -1;

int main(int argc, char *argv[])
{
    parse_cmdline(argc, argv);
    clock_t t = clock();

    /* set logging policy */
    if (log_filename)
    {
        open_log(log_filename);
        set_log_level(LOG_DEBUG);
        atexit(close_log);
    }

    write_log(LOG_DEBUG, "reading source file '%s'...", src_filename);
    FileBuffer src_file = read_file(src_filename);
    if (!src_file.buffer)
    {
        fprintf(stderr, "error: failed to open file '%s'\n", src_filename);
        return EXIT_FAILURE;
    }

    write_log(LOG_DEBUG, "starting lexical analysis...");
    TokenizedFile tokenized_file = tokenize(src_file);

    free_filebuff(src_file);

    write_log(LOG_DEBUG, "starting syntax analysis...");
    ParsedFile parsed_file = parse(tokenized_file);

    free_tokenized_file(tokenized_file);

    write_log(LOG_DEBUG, "generating code...");
    generate_code(parsed_file, sections_load_address, out_filename, out_txt_filename);

    t = clock() - t;
    double elapsed = ((double) t) / CLOCKS_PER_SEC;
    write_log(LOG_NORMAL, "assembly finished in %.5fs", elapsed);

    return EXIT_SUCCESS;
}


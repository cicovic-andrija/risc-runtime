/* File: main.c */
/* System software project: linker */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "log.h"
#include "util.h"
#include "cmdline.h"
#include "link.h"

char *out_filename = "a.out";
char *out_txt_filename = NULL;
char *log_filename = NULL;

char *const *object_filenames = NULL;
FILE **obj_fp = NULL;

int ignore_predefined_origin = 0;

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

    /* open object files */
    int nfiles = (argv + argc) - object_filenames;
    obj_fp = (FILE **) calloc(nfiles, sizeof(FILE *));
    if (!obj_fp)
        memory_alloc_error("linker", "file pointer(s)", nfiles * sizeof(FILE *));
    int i;
    for (i = 0; i < nfiles; ++i)
    {
        obj_fp[i] = fopen(object_filenames[i], "rb");
        if (!obj_fp[i])
        {
            fprintf(stderr, "error: failed to open file '%s'\n", object_filenames[i]);
            return EXIT_FAILURE;
        }
    }

    /* Link */
    link_files(obj_fp, nfiles, out_filename, out_txt_filename);

    /* close object files */
    for (i = 0; i < nfiles; ++i)
        fclose(obj_fp[i]);
    free(obj_fp);

    t = clock() - t;
    double elapsed = ((double) t) / CLOCKS_PER_SEC;
    write_log(LOG_NORMAL, "linking finished in %.5fs", elapsed);

    return EXIT_SUCCESS;
}


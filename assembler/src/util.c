/* File: util.c */
/* Misc. useful functions. */

#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "log.h"
#include "util.h"

void memory_alloc_error(const char *where, const char *what, long num_bytes)
{
    write_log(LOG_ERROR, "%s: failed to allocate %ldB of memory for %s", where, num_bytes, what);
    fprintf(stderr, "memory allocation error\n");
    exit(EXIT_FAILURE);
}

int is_dec_number(const char *str)
{
    while (*str)
    {
        if (!isdigit(*str)) return 0;
        ++str;
    }
    return 1;
}

int starts_with(const char *str, const char *substr)
{
    if (!str || !substr) return -1; /* error */

    if (strlen(substr) > strlen(str)) return 0;

    while (*substr)
        if (*substr++ != *str++)
            return 0;

    return 1;
}


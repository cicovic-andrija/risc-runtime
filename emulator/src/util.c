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


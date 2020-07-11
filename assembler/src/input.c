/* File: input.c */
/* Input handling functionality. */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "log.h"
#include "input.h"

FileBuffer read_file(const char *filename)
{
    FileBuffer fbuff;
    fbuff.buffer = NULL;
    fbuff.buffsize = 0;

    if (!filename) return fbuff;

    FILE *fp = fopen(filename, "r");
    if (!fp) return fbuff;

    long fsize;
    fseek(fp, 0, SEEK_END);
    fsize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char *buffer = (char *) malloc(fsize + 1);
    if (!buffer)
    {
        write_log(LOG_ERROR, "failed to allocate %ldB of memory for file buffer", fsize);
        fclose(fp);
        return fbuff;
    }

    fread(buffer, fsize, sizeof(char), fp);
    buffer[fsize] = '\0';
    fclose(fp);

    fbuff.buffer = buffer;
    fbuff.buffsize = fsize;
    return fbuff;
}

void free_filebuff(FileBuffer filebuff)
{
    free(filebuff.buffer);
}


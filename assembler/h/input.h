/* File: input.h */
/* Input handling functionality. */

#ifndef INPUT_H
#define INPUT_H

typedef struct {
    char *buffer;
    long buffsize;
} FileBuffer;

/* Function read_file loads file contents in a buffer
 * and returns a pointer to the buffer, or NULL in case of error. */
FileBuffer read_file(const char *filename);

/* Function free_filebuff frees the memory allocated by the
 * FileBuffer object. */
void free_filebuff(FileBuffer filebuff);

#endif /* INPUT_H */


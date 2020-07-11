/* File: util.h */
/* Misc. useful functions. */

#ifndef UTIL_H
#define UTIL_H

/* Function memory_alloc_error logs the error, prints the message to
 * standard error stream and exits. */
void memory_alloc_error(const char *where, const char *what, long num_bytes);

#endif /* UTIL_H */


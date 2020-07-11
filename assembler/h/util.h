/* File: util.h */
/* Misc. useful functions. */

#ifndef UTIL_H
#define UTIL_H

/* Function is_dec_number returns a bool indicating if str is a string
 * representing a decimal number in format: dec_digit{dec_digit} */
/* Note: assumed no + or - sign in front of a number. */
int is_dec_number(const char *str);

/* Function memory_alloc_error logs the error, prints the message to
 * standard error stream and exits. */
void memory_alloc_error(const char *where, const char *what, long num_bytes);

/* Function starts_with checks whether str starts with substr. */
int starts_with(const char *str, const char *substr);

#endif /* UTIL_H */


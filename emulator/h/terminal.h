/* File: termina.h */
/* Terminal settings. */

#ifndef TERMINAL_H
#define TERMINAL_H

/* Function enable_raw_mode puts terminal into 'raw' mode
 * (no echoing, read/write byte-by-byte etc) */
void enable_raw_mode(void);

/* Function disable_raw_mode puts terminal in the original state
 * it was in before enable_raw_mode was called. */
void disable_raw_mode(void);

#endif /* TERMINAL_H */


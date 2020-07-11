/* File: intr.h */
/* CPU interrupt block. */

#ifndef INTR_H
#define INTR_H

#define IVTENTRY_SIZE 2

extern int intr;

/* Function interrup handles interrupt signals
 * by calling interrupt routines. */
void interrupt(void);

#endif /* INTR_H */

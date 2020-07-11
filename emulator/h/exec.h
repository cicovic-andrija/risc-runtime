/* File: exec.h */
/* CPU execute block. */

#ifndef EXEC_H
#define EXEC_H

/* Flag indicating if explicit write to memory was performed.
 * Push to stack isn't considered as explicit write to memory. */
int memory_write;

/* Function execute executes an instruction on decoded operands. */
void execute(void);

#endif /* EXEC_H */


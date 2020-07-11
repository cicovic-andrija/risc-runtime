/* File: global.h */
/* Global data declarations. */

#ifndef GLOBAL_H
#define GLOBAL_H

#include "asm_lang.h"

extern char *string_pool_global;
extern long string_pool_global_size;

extern const Instruction instruction_table[];
extern const int instruction_table_size;

extern const char *register_table[];
extern const int register_table_size;

extern const Directive directive_table[];
extern const int directive_table_size;

/* Function find_instruction fetches instruction information,
 * or NULL, if no matching instruction is found. */
const Instruction *find_instruction(const char *mnemonic);

/* Function find_directive fetches directive information,
 * or NULL, if no matching directive is found. */
const Directive *find_directive(const char *mnemonic);

/* Function find_register fetches register string,
 * or NULL, if no matching register is found. */
const char *find_register(const char *reg);

#endif /* GLOBAL_H */


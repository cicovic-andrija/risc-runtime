/* File: codegen.h */
/* First and second pass of the assembler. Code generation. */

#ifndef CODEGEN_H
#define CODEGEN_H

#include "asm_lang.h"

/* Function generate_code creates relocatable object file, or exits
 * in case of a semantic error. */
int generate_code(ParsedFile parsed_file, uint16_t secions_load_address,
        const char *out_filename, const char *out_txt_filename);

#endif /* CODEGEN_H */


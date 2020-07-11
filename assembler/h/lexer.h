/* File: lexer.h */
/* Lexical analysis functionality. */

#ifndef LEXER_H
#define LEXER_H

#include "asm_lang.h"
#include "input.h"

/* Function tokenize parses the given file and returns a list of tokens.
 * Some tokens are stored as a string in a token pool. */
TokenizedFile tokenize(FileBuffer file_buffer);

/* Function free_tokenized_file frees memory allocated for a file. */
void free_tokenized_file(TokenizedFile tf);

#endif /* LEXER_H */


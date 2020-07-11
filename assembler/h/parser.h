/* File: parser.h */
/* Syntax analysis functionality: Recursive descent parser */

#ifndef PARSER_H
#define PARSER_H

#define REG_UNUSED 0

#include "asm_lang.h"

/* Function analyze_syntax parses given token list for syntax errors
 * and returns the number of errors found. */
ParsedFile parse(TokenizedFile tf);

/* Function semantic_analysis asserts the given parsed file
 * for semantical errors, and exits if errors were found. */
void semantic_analysis(ParsedFile pf);

#endif /* PARSER_H */


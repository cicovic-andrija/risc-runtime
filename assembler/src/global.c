/* File: global.c */
/* Global data definitions. */

#include <ctype.h>
#include <string.h>

#include "global.h"

char *string_pool_global = 0L;

long string_pool_global_size = 0L;

/* Note: no mnemonic can be longer than RESERVED_WORD_MAXLEN */
const Instruction instruction_table[] = {
    {  "ADDEQ", 2, AF_DST, OP_ADD, COND_EQ }, {  "ADDNE", 2, AF_DST, OP_ADD, COND_NE },
    {  "ADDGT", 2, AF_DST, OP_ADD, COND_GT }, {  "ADDAL", 2, AF_DST, OP_ADD, COND_AL },
    {    "ADD", 2, AF_DST, OP_ADD, COND_AL },

    {  "SUBEQ", 2, AF_DST, OP_SUB, COND_EQ }, {  "SUBNE", 2, AF_DST, OP_SUB, COND_NE },
    {  "SUBGT", 2, AF_DST, OP_SUB, COND_GT }, {  "SUBAL", 2, AF_DST, OP_SUB, COND_AL },
    {    "SUB", 2, AF_DST, OP_SUB, COND_AL },

    {  "MULEQ", 2, AF_DST, OP_MUL, COND_EQ }, {  "MULNE", 2, AF_DST, OP_MUL, COND_NE },
    {  "MULGT", 2, AF_DST, OP_MUL, COND_GT }, {  "MULAL", 2, AF_DST, OP_MUL, COND_AL },
    {    "MUL", 2, AF_DST, OP_MUL, COND_AL },

    {  "DIVEQ", 2, AF_DST, OP_DIV, COND_EQ }, {  "DIVNE", 2, AF_DST, OP_DIV, COND_NE },
    {  "DIVGT", 2, AF_DST, OP_DIV, COND_GT }, {  "DIVAL", 2, AF_DST, OP_DIV, COND_AL },
    {    "DIV", 2, AF_DST, OP_DIV, COND_AL },

    {  "CMPEQ", 2, AF_DST, OP_CMP, COND_EQ }, {  "CMPNE", 2, AF_DST, OP_CMP, COND_NE },
    {  "CMPGT", 2, AF_DST, OP_CMP, COND_GT }, {  "CMPAL", 2, AF_DST, OP_CMP, COND_AL },
    {    "CMP", 2, AF_DST, OP_CMP, COND_AL },

    {  "ANDEQ", 2, AF_DST, OP_AND, COND_EQ }, {  "ANDNE", 2, AF_DST, OP_AND, COND_NE },
    {  "ANDGT", 2, AF_DST, OP_AND, COND_GT }, {  "ANDAL", 2, AF_DST, OP_AND, COND_AL },
    {    "AND", 2, AF_DST, OP_AND, COND_AL },

    {   "OREQ", 2, AF_DST, OP_OR, COND_EQ }, {   "ORNE", 2, AF_DST, OP_OR, COND_NE },
    {   "ORGT", 2, AF_DST, OP_OR, COND_GT }, {   "ORAL", 2, AF_DST, OP_OR, COND_AL },
    {     "OR", 2, AF_DST, OP_OR, COND_AL },

    {  "NOTEQ", 2, AF_DST, OP_NOT, COND_EQ }, {  "NOTNE", 2, AF_DST, OP_NOT, COND_NE },
    {  "NOTGT", 2, AF_DST, OP_NOT, COND_GT }, {  "NOTAL", 2, AF_DST, OP_NOT, COND_AL },
    {    "NOT", 2, AF_DST, OP_NOT, COND_AL },

    { "TESTEQ", 2, AF_DST, OP_TEST, COND_EQ }, { "TESTNE", 2, AF_DST, OP_TEST, COND_NE },
    { "TESTGT", 2, AF_DST, OP_TEST, COND_GT }, { "TESTAL", 2, AF_DST, OP_TEST, COND_AL },
    {   "TEST", 2, AF_DST, OP_TEST, COND_AL },

    { "PUSHEQ", 1, AF_SRC, OP_PUSH, COND_EQ }, { "PUSHNE", 1, AF_SRC, OP_PUSH, COND_NE },
    { "PUSHGT", 1, AF_SRC, OP_PUSH, COND_GT }, { "PUSHAL", 1, AF_SRC, OP_PUSH, COND_AL },
    {   "PUSH", 1, AF_SRC, OP_PUSH, COND_AL },

    {  "POPEQ", 1, AF_DST, OP_POP, COND_EQ }, {  "POPNE", 1, AF_DST, OP_POP, COND_NE },
    {  "POPGT", 1, AF_DST, OP_POP, COND_GT }, {  "POPAL", 1, AF_DST, OP_POP, COND_AL },
    {    "POP", 1, AF_DST, OP_POP, COND_AL },

    { "CALLEQ", 1, AF_DST, OP_CALL, COND_EQ }, { "CALLNE", 1, AF_DST, OP_CALL, COND_NE },
    { "CALLGT", 1, AF_DST, OP_CALL, COND_GT }, { "CALLAL", 1, AF_DST, OP_CALL, COND_AL },
    {   "CALL", 1, AF_DST, OP_CALL, COND_AL },

    { "IRETEQ", 0, AF_NON, OP_IRET, COND_EQ }, { "IRETNE", 0, AF_NON, OP_IRET, COND_NE },
    { "IRETGT", 0, AF_NON, OP_IRET, COND_GT }, { "IRETAL", 0, AF_NON, OP_IRET, COND_AL },
    {   "IRET", 0, AF_NON, OP_IRET, COND_AL },

    {  "MOVEQ", 2, AF_DST, OP_MOV, COND_EQ }, {  "MOVNE", 2, AF_DST, OP_MOV, COND_NE },
    {  "MOVGT", 2, AF_DST, OP_MOV, COND_GT }, {  "MOVAL", 2, AF_DST, OP_MOV, COND_AL },
    {    "MOV", 2, AF_DST, OP_MOV, COND_AL },

    {  "SHLEQ", 2, AF_DST, OP_SHL, COND_EQ }, {  "SHLNE", 2, AF_DST, OP_SHL, COND_NE },
    {  "SHLGT", 2, AF_DST, OP_SHL, COND_GT }, {  "SHLAL", 2, AF_DST, OP_SHL, COND_AL },
    {    "SHL", 2, AF_DST, OP_SHL, COND_AL },

    {  "SHREQ", 2, AF_DST, OP_SHR, COND_EQ }, {  "SHRNE", 2, AF_DST, OP_SHR, COND_NE },
    {  "SHRGT", 2, AF_DST, OP_SHR, COND_GT }, {  "SHRAL", 2, AF_DST, OP_SHR, COND_AL },
    {    "SHR", 2, AF_DST, OP_SHR, COND_AL },

    {  "JMPEQ", 1, AF_DST, OP_NONE, COND_EQ }, {  "JMPNE", 1, AF_DST, OP_NONE, COND_NE },
    {  "JMPGT", 1, AF_DST, OP_NONE, COND_GT }, {  "JMPAL", 1, AF_DST, OP_NONE, COND_AL },
    {    "JMP", 1, AF_DST, OP_NONE, COND_AL },

    {  "RETEQ", 0, AF_NON, OP_NONE, COND_EQ }, {  "RETNE", 0, AF_NON, OP_NONE, COND_NE },
    {  "RETGT", 0, AF_NON, OP_NONE, COND_GT }, {  "RETAL", 0, AF_NON, OP_NONE, COND_AL },
    {    "RET", 0, AF_NON, OP_NONE, COND_AL },

    { "HALTEQ", 0, AF_NON, OP_NONE, COND_EQ }, { "HALTNE", 0, AF_NON, OP_NONE, COND_NE },
    { "HALTGT", 0, AF_NON, OP_NONE, COND_GT }, { "HALTAL", 0, AF_NON, OP_NONE, COND_AL },
    {   "HALT", 0, AF_NON, OP_NONE, COND_AL },
};
const int instruction_table_size = sizeof(instruction_table) / sizeof(instruction_table[0]);

/* Note: no mnemonic can be longer than RESERVED_WORD_MAXLEN */
const char *register_table[] = {
     "R0",  "R1",  "R2",  "R3",
     "R4",  "R5",  "R6",  "R7",
     "R8",  "R9", "R10", "R11",
    "R12", "R13", "R14", "R15",
};
const int register_table_size = sizeof(register_table) / sizeof(register_table[0]);

/* Note: no mnemonic can be longer than RESERVED_WORD_MAXLEN */
const Directive directive_table[] = {
    { ".TEXT",   DIR_SECTION       },
    { ".DATA",   DIR_SECTION       },
    { ".RODATA", DIR_SECTION       },
    { ".BSS",    DIR_SECTION       },
    { ".GLOBAL", DIR_IMPORT_EXPORT },
    { ".END",    DIR_EOF           },
    { ".ALIGN",  DIR_ALIGN         },
    { ".SKIP",   DIR_SKIP          },
    { ".CHAR",   DIR_DATA_DEF      },
    { ".WORD",   DIR_DATA_DEF      },
    { ".LONG",   DIR_DATA_DEF      },
};
const int directive_table_size = sizeof(directive_table) / sizeof(directive_table[0]);

const Instruction *find_instruction(const char *mnemonic)
{
    char norm_mn[RESERVED_WORD_MAXLEN + 1]; /* "normalized" mnemonic - all upper letters */
    int i = 0;
    while (*mnemonic) norm_mn[i++] = toupper(*mnemonic++);
    norm_mn[i] = '\0';

    for (i = 0; i < instruction_table_size; ++i)
    {
        if (strcmp(norm_mn, instruction_table[i].mnemonic) == 0)
            return instruction_table + i;
    }

    return NULL;
}

const Directive *find_directive(const char *mnemonic)
{
    char norm_mn[RESERVED_WORD_MAXLEN + 1]; /* "normalized" mnemonic - all upper letters */
    int i = 0;
    while (*mnemonic) norm_mn[i++] = toupper(*mnemonic++);
    norm_mn[i] = '\0';

    for (i = 0; i < directive_table_size; ++i)
    {
        if (strcmp(norm_mn, directive_table[i].mnemonic) == 0)
            return directive_table + i;
    }

    return NULL;
}

const char *find_register(const char *reg)
{
    char norm_reg[RESERVED_WORD_MAXLEN + 1]; /* "normalized" mnemonic - all upper letters */
    int i = 0;
    while (*reg) norm_reg[i++] = toupper(*reg++);
    norm_reg[i] = '\0';

    for (i = 0; i < register_table_size; ++i)
    {
        if (strcmp(norm_reg, register_table[i]) == 0)
            return register_table[i];
    }

    return NULL;
}


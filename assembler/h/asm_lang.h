/* File: asm_lang.h */
/* Definitions of data types used to translate assembly language into object code. */

#ifndef ASM_LANG_H
#define ASM_LANG_H

#define RESERVED_WORD_MAXLEN 7

#define SYMBOL_MAXLEN 31

#include <stdint.h>

typedef enum {
    TT_UNDEFINED = 0,
    TT_EOF,
    TT_NEWLINE,
    TT_COMMENT,
    TT_SYMBOL,
    TT_INSTRUCTION,
    TT_DIRECTIVE,
    TT_REGISTER,
    TT_LITERAL,
    TT_COMMA,
    TT_COLON,
    TT_ASTERISK,
    TT_AMPERSAND,
    TT_DOLLAR,
    TT_LBRACKET,
    TT_RBRACKET,
    TT_PLUS,
    TT_MINUS,
} TokenType;

typedef struct {
    const char *tokstr;
    TokenType type;
} Token;

typedef struct tnode {
    Token token;
    struct tnode *next;
} TokenNode;

typedef TokenNode *TokenizedFile;

typedef enum {
    AM_IMMED = 0x0,
    AM_REGDIR = 0x1,
    AM_MEMDIR = 0x2,
    AM_REGINDDISP = 0x3,
} AddressingMode;

typedef enum {
    AF_NON = 0,
    AF_SRC,
    AF_DST,
} AddressFieldType;

typedef enum {
    COND_EQ = 0x0,
    COND_NE = 0x1,
    COND_GT = 0x2,
    COND_AL = 0x3,
} Condition;

typedef enum {
    DIR_NONE = 0,
    DIR_SECTION,
    DIR_IMPORT_EXPORT,
    DIR_EOF,
    DIR_DATA_DEF,
    DIR_ALIGN,
    DIR_SKIP,
} DirectiveType;

typedef enum {
    OP_NONE =  -1,
    OP_ADD  = 0x0,
    OP_SUB  = 0x1,
    OP_MUL  = 0x2,
    OP_DIV  = 0x3,
    OP_CMP  = 0x4,
    OP_AND  = 0x5,
    OP_OR   = 0x6,
    OP_NOT  = 0x7,
    OP_TEST = 0x8,
    OP_PUSH = 0x9,
    OP_POP  = 0xa,
    OP_CALL = 0xb,
    OP_IRET = 0xc,
    OP_MOV  = 0xd,
    OP_SHL  = 0xe,
    OP_SHR  = 0xf,
} Opcode;

typedef struct {
    const char *mnemonic;
    int nparam;
    AddressFieldType af1_type;
    Opcode opcode;
    Condition cond;
} Instruction;

typedef struct {
    const char *mnemonic;
    DirectiveType type;
} Directive;

typedef enum {
    PAR_NO_PARAM = 0,
    PAR_IMMED_LITERAL,
    PAR_IMMED_SYMVAL,
    PAR_MEMDIR_SYM,
    PAR_MEMDIR_LITERAL,
    PAR_REGDIR,
    PAR_REGIND_LITERAL,
    PAR_REGIND_SYM,
    PAR_PCREL,
} ParameterType;

typedef struct{
    ParameterType type;
    char is_long;
    char reg;
    union {
        const char *sym_name;
        int32_t num_val;
    };
} Parameter;

typedef struct pnode {
    Parameter param;
    struct pnode *next;
} ParameterNode;

typedef enum {
    CMD_NO_COMMAND = 0,
    CMD_INSTRUCTION,
    CMD_DIRECTIVE,
} CommandType;

typedef struct line {
    const char *label;
    CommandType type;
    union {
        const Instruction *instruction;
        const Directive *directive;
    };
    int nparam;
    ParameterNode *phead;
    int src_line;
    struct line *next;
} Line;

typedef Line *ParsedFile;

#endif // ASM_LANG_H


/* File: constants.h */
/* Constant and enum definitions used in various emulator modules. */

#ifndef CONSTANTS_H
#define CONSTANTS_H

enum {
    ADD  = 0x0,
    SUB  = 0x1,
    MUL  = 0x2,
    DIV  = 0x3,
    CMP  = 0x4,
    AND  = 0x5,
    OR   = 0x6,
    NOT  = 0x7,
    TEST = 0x8,
    PUSH = 0x9,
    POP  = 0xa,
    CALL = 0xb,
    IRET = 0xc,
    MOV  = 0xd,
    SHL  = 0xe,
    SHR  = 0xf,
};

enum {
    IMMED = 0x0,
    REGDIR = 0x1,
    MEMDIR = 0x2,
    REGINDDISP = 0x3
};

enum {
    EQ = 0x0,
    NE = 0x1,
    GT = 0x2,
    AL = 0x3,
};

#endif /* CONSTANTS_H */


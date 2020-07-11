/* File: exec.c */
/* CPU execute block. */

#include "log.h"
#include "cpu.h"
#include "mem.h"
#include "constants.h"
#include "decode.h"
#include "exec.h"

int memory_write;

int fast_test_carry16(int16_t a, int16_t b)
{
    if (a & (int16_t)0x8000 && b & (int16_t)0x8000)
        return 1;
    if (!(a & (int16_t)0x8000) && !(b & (int16_t)0x8000))
        return 0;
    return (a + b >= 0) ? 1 : 0;
}

int test_carry16(int16_t a, int16_t b)
{
    int carry = 0;
    int i;
    for (i = 0; i < 16; ++i)
        carry = ((a >> i) & 1) + ((b >> i) & 1) + carry > 1;
    return carry;
}

void update_zn(int16_t result)
{
    if (result == 0)
        PSW_SET_FLAG(PSW_FLAG_Z);
    else
        PSW_CLEAR_FLAG(PSW_FLAG_Z);

    if (result < 0)
        PSW_SET_FLAG(PSW_FLAG_N);
    else
        PSW_CLEAR_FLAG(PSW_FLAG_N);
}

void add(int16_t *dst, int16_t *src)
{
    if (test_carry16(*dst, *src))
        PSW_SET_FLAG(PSW_FLAG_C);
    else
        PSW_CLEAR_FLAG(PSW_FLAG_C);

    int16_t res = *dst + *src;
    if ((*dst < 0 && *src < 0 && !(res < 0)) || (*dst > 0 && *src > 0 && !(res > 0)))
        PSW_SET_FLAG(PSW_FLAG_O);
    else
        PSW_CLEAR_FLAG(PSW_FLAG_O);

    update_zn(res);

    *dst = res;
}

void sub(int16_t *dst, int16_t *src)
{
    if (test_carry16(*dst, -(*src)))
        PSW_SET_FLAG(PSW_FLAG_C);
    else
        PSW_CLEAR_FLAG(PSW_FLAG_C);

    int16_t res = *dst - *src;
    if (*src == INT16_MIN)
    {
        PSW_SET_FLAG(PSW_FLAG_O);
    }
    else
    {
        if ((*dst < 0 && -(*src) < 0 && !(res < 0)) || (*dst > 0 && -(*src) > 0 && !(res > 0)))
            PSW_SET_FLAG(PSW_FLAG_O);
        else
            PSW_CLEAR_FLAG(PSW_FLAG_O);
    }

    update_zn(res);

    *dst = res;
}

void cmp(int16_t a, int16_t b)
{

    if (test_carry16(a, -b))
        PSW_SET_FLAG(PSW_FLAG_C);
    else
        PSW_CLEAR_FLAG(PSW_FLAG_C);

    int16_t res = a - b;
    if (b == INT16_MIN)
    {
        PSW_SET_FLAG(PSW_FLAG_O);
    }
    else
    {
        if ((a < 0 && -b < 0 && !(res < 0)) || (a > 0 && -b > 0 && !(res > 0)))
            PSW_SET_FLAG(PSW_FLAG_O);
        else
            PSW_CLEAR_FLAG(PSW_FLAG_O);
    }

    update_zn(res);
}

void mul(int16_t *dst, int16_t *src)
{
    *dst = *dst * *src;
    update_zn(*dst);
}

void div(int16_t *dst, int16_t *src)
{
    *dst = *dst / *src;
    update_zn(*dst);
}

void and(int16_t *dst, int16_t *src)
{
    *dst = *dst & *src;
    update_zn(*dst);
}

void test(int16_t a, int16_t b)
{
    update_zn(a & b);
}

void or(int16_t *dst, int16_t *src)
{
    *dst = *dst | *src;
    update_zn(*dst);
}

void not(int16_t *dst)
{
    *dst = ~(*dst);
    update_zn(*dst);
}

void shl(int16_t *dst, uint16_t *src)
{
    if (*src == 0)
    {
        PSW_CLEAR_FLAG(PSW_FLAG_C);
        return;
    }

    if (*src > 16)
    {
        *dst = 0;
        PSW_CLEAR_FLAG(PSW_FLAG_C);
        return;
    }

    int carry = (*dst & (1 << (16 - *src))) > 0;
    if (carry)
        PSW_SET_FLAG(PSW_FLAG_C);
    else
        PSW_CLEAR_FLAG(PSW_FLAG_C);

    *dst = *dst << *src;
}

void shr(int16_t *dst, uint16_t *src)
{
    if (*src == 0)
    {
        PSW_CLEAR_FLAG(PSW_FLAG_C);
        return;
    }

    if (*src > 16)
    {
        if (*dst & 0x8000)
        {
            *dst = (int16_t) 0xffff;
            PSW_SET_FLAG(PSW_FLAG_C);
        }
        else
        {
            *dst = 0;
            PSW_CLEAR_FLAG(PSW_FLAG_C);
        }
        return;
    }

    int carry = (*dst & (1 << (*src - 1))) > 0;
    if (carry)
        PSW_SET_FLAG(PSW_FLAG_C);
    else
        PSW_CLEAR_FLAG(PSW_FLAG_C);

    *dst = *dst >> *src;
}

void push(int16_t src)
{
    char *byte = (char *) &src;
    --cpu_context.reg[6];
    *(mem + (uint16_t) cpu_context.reg[6]) = *(byte + 1);
    --cpu_context.reg[6];
    *(mem + (uint16_t) cpu_context.reg[6]) = *byte;
}

void pop(int16_t *dst)
{
    char *byte = (char *) dst;
    *byte = *(mem + (uint16_t) cpu_context.reg[6]);
    ++cpu_context.reg[6];
    *(byte + 1) = *(mem + (uint16_t) cpu_context.reg[6]);
    ++cpu_context.reg[6];
}

void call(int16_t address)
{
    push(cpu_context.reg[7]);
    cpu_context.reg[7] = address;
}

void iret(void)
{
    pop(&cpu_context.psw);
    pop(&cpu_context.reg[7]);
}

void mov(int16_t *dst, int16_t *src)
{
    *dst = *src;
    update_zn(*dst);
}

int test_condition(int cond)
{
    switch(cond)
    {
    case EQ:
        if (!PSW_TEST_FLAG(PSW_FLAG_Z))
            return 0;
        break;
    case NE:
        if (PSW_TEST_FLAG(PSW_FLAG_Z))
            return 0;
        break;
    case GT:
        if (PSW_TEST_FLAG(PSW_FLAG_Z) || PSW_TEST_FLAG(PSW_FLAG_N))
            return 0;
        break;
    case AL:
        return 1;
        break;
    }
    return 1;
}

void execute(void)
{
    memory_write = 0;

    int cond = (ir0 >> 14) & 0x3;
    if (test_condition(cond) == 0)
        return;

    int opcode = (ir0 >> 10) & 0xf;
    switch (opcode)
    {
    case ADD:
        add(operand[0], operand[1]);
        if (memory_dst) memory_write = 1;
        break;
    case SUB:
        sub(operand[0], operand[1]);
        if (memory_dst) memory_write = 1;
        break;
    case MUL:
        mul(operand[0], operand[1]);
        if (memory_dst) memory_write = 1;
        break;
    case DIV:
        div(operand[0], operand[1]);
        if (memory_dst) memory_write = 1;
        break;
    case SHL:
        shl(operand[0], (uint16_t *) operand[1]);
        if (memory_dst) memory_write = 1;
        break;
    case SHR:
        shr(operand[0], (uint16_t *) operand[1]);
        if (memory_dst) memory_write = 1;
        break;
    case AND:
        and(operand[0], operand[1]);
        if (memory_dst) memory_write = 1;
        break;
    case OR:
        or(operand[0], operand[1]);
        if (memory_dst) memory_write = 1;
        break;
    case NOT:
        not(operand[0]);
        if (memory_dst) memory_write = 1;
        break;
    case CMP:
        cmp(*operand[0], *operand[1]);
        break;
    case TEST:
        test(*operand[0], *operand[1]);
        break;
    case PUSH:
        push(*operand[0]);
        break;
    case POP:
        pop(operand[0]);
        if (memory_dst) memory_write = 1;
        break;
    case CALL:
        call(mar);
        break;
    case IRET:
        iret();
        break;
    case MOV:
        mov(operand[0], operand[1]);
        if (memory_dst) memory_write = 1;
        break;
    }
}


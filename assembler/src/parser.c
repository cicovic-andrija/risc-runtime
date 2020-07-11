/* File: parser.c */
/* Syntax analysis functionality: Recursive descent parser */

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

#include "log.h"
#include "global.h"
#include "util.h"
#include "parser.h"

static TokenNode *tokenptr = NULL;
static Token token = { NULL, TT_EOF };

static Line *lineptr = NULL;

static int line_num = 0;

void __attribute__((noreturn)) syntax_error(const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    fprintf(stderr, "error: on line %d: ", line_num);
    vfprintf(stderr, format, ap);
    fputc('\n', stderr);
    va_end(ap);
    exit(EXIT_FAILURE);
}

void nexttok(void)
{
    if (!tokenptr)
    {
        token.tokstr = NULL;
        token.type = TT_EOF;
    }
    else
    {
        token = tokenptr->token;
        tokenptr = tokenptr->next;
    }
}

int accept_token(TokenType t)
{
    if (token.type == t)
        return 1;
    return 0;
}

int expect_token(TokenType t, const char *errorm)
{
    if (accept_token(t))
        return 1;
    syntax_error("%s", errorm);
}

int32_t expect_literal(int sign)
{
    const char *literal = token.tokstr;
    nexttok();

    unsigned long val = strtoul(literal, NULL, 10);

    unsigned long max_val = (sign == 1) ? INT32_MAX : (unsigned long)INT32_MAX + 1;
    if (val > max_val)
        syntax_error("integer %s%s out of allowed range", (sign == 1) ? "+" : "-", literal);

    if (val == (unsigned long)INT32_MAX + 1) return INT32_MIN;

    return ((int32_t) val) * sign;
}

Parameter expect_parameter(void)
{
    Parameter param = { PAR_NO_PARAM, 1, REG_UNUSED, { NULL } };
    int sign = 1;

    switch(token.type)
    {
    case TT_PLUS: case TT_MINUS:
        if (token.type == TT_PLUS) sign = 1;
        else sign = -1;
        nexttok();
    case TT_LITERAL:
        param.type = PAR_IMMED_LITERAL;
        param.num_val = expect_literal(sign);
        break;
    case TT_REGISTER:
        param.type = PAR_REGDIR;
        param.is_long = 0;
        param.reg = atoi(token.tokstr + 1); // +1 to skip 'R'/'r' prefix
        nexttok();

        if (accept_token(TT_LBRACKET))
        {
            nexttok();

            param.is_long = 1;

            switch(token.type)
            {
            case TT_PLUS: case TT_MINUS:
                if (token.type == TT_PLUS) sign = 1;
                else sign = -1;
                nexttok();
            case TT_LITERAL:
                param.type = PAR_REGIND_LITERAL;
                param.num_val = expect_literal(sign);
                break;
            case TT_SYMBOL:
                param.type = PAR_REGIND_SYM;
                param.sym_name = token.tokstr;
                nexttok();
                break;
            default:
                syntax_error("offset expected after '['");
                break;
            }

            expect_token(TT_RBRACKET, "']' expected");
            nexttok();
        }
        break;
    case TT_SYMBOL:
        param.type = PAR_MEMDIR_SYM;
        param.sym_name = token.tokstr;
        nexttok();
        break;
    case TT_AMPERSAND:
        param.type = PAR_IMMED_SYMVAL;
        nexttok();
        expect_token(TT_SYMBOL, "symbol expected after '&'");
        param.sym_name = token.tokstr;
        nexttok();
        break;
    case TT_ASTERISK:
        param.type = PAR_MEMDIR_LITERAL;
        nexttok();
        if (accept_token(TT_PLUS))
            nexttok();
        else if (accept_token(TT_MINUS))
        {
            sign = -1;
            nexttok();
        }
        param.num_val = expect_literal(sign);
        break;
    case TT_DOLLAR:
        param.type = PAR_PCREL;
        nexttok();
        expect_token(TT_SYMBOL, "symbol expected after '$'");
        param.sym_name = token.tokstr;
        nexttok();
        break;
    default:
        syntax_error("argument expected");
        break;
    }

    return param;
}

void accept_param_list(void)
{
    ParameterNode *head = NULL;
    ParameterNode *tail = NULL;
    int nparam = 0;

    TokenType tt = token.type;

    if (tt == TT_PLUS || tt == TT_MINUS || tt == TT_LITERAL || tt == TT_AMPERSAND
            || tt == TT_SYMBOL || tt == TT_ASTERISK || tt == TT_REGISTER || tt == TT_DOLLAR)
    {
        head = tail = (ParameterNode *) calloc(1, sizeof(ParameterNode));
        if (!head)
            memory_alloc_error("parser", "ParameterNode", sizeof(ParameterNode));

        head->param = expect_parameter();
        ++nparam;
    }
    else
    {
        lineptr->nparam = 0;
        lineptr->phead = NULL;
        return;
    }

    while (accept_token(TT_COMMA))
    {
        nexttok();
        tail->next = calloc(1, sizeof(ParameterNode));
        if (!tail->next)
            memory_alloc_error("parser", "ParameterNode", sizeof(ParameterNode));

        tail = tail->next;
        tail->param = expect_parameter();
        ++nparam;
    }

    lineptr->nparam = nparam;
    lineptr->phead = head;
}

void accept_command(void)
{
    if (accept_token(TT_DIRECTIVE))
    {
        lineptr->type = CMD_DIRECTIVE;
        lineptr->directive = find_directive(token.tokstr);
        nexttok();
        accept_param_list();
    }
    else if (accept_token(TT_INSTRUCTION))
    {
        lineptr->type = CMD_INSTRUCTION;
        lineptr->instruction = find_instruction(token.tokstr);
        nexttok();
        accept_param_list();
    }
}

void accept_label(void)
{
    if (accept_token(TT_SYMBOL))
    {
        lineptr->label = token.tokstr;
        nexttok();
        expect_token(TT_COLON, "':' expected");
        nexttok();
    }
}

void accept_line(void)
{
    accept_label();

    accept_command();

    if (accept_token(TT_COMMENT))
        nexttok();

    expect_token(TT_NEWLINE, "invalid statement");

    nexttok();
}

Line *parse_module(void)
{
    Line *first_line = NULL;
    Line *last_line = NULL;

    nexttok();
    while (token.type != TT_EOF)
    {
        lineptr = (Line *) calloc(1, sizeof(Line));
        if (!lineptr) memory_alloc_error("parser", "Line", sizeof(Line));

        accept_line();

        if (lineptr->label || lineptr->type != CMD_NO_COMMAND)
        {
            lineptr->src_line = line_num;
            if (!first_line)
                first_line = lineptr;
            else
                last_line->next = lineptr;
            last_line = lineptr;
        }
        else
            free(lineptr);

        ++line_num;
    }

    return first_line;
}

/* Note: assumed a valid TokenizedFile is passed */
ParsedFile parse(TokenizedFile tf)
{
    tokenptr = tf;
    token.tokstr = NULL;
    token.type = TT_EOF;
    line_num = 1;
    lineptr = NULL;

    ParsedFile parsed_file = parse_module();

    return parsed_file;
}


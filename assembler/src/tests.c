/* File: tests.c */
/* Testing module. */

#include <stdlib.h>
#include <stdio.h>

#include "input.h"
#include "lexer.h"
#include "parser.h"
#include "tests.h"
#include "util.h"
#include <stdint.h>

void test_parser(const char *filename)
{
    FileBuffer fb = read_file(filename);
    if (!fb.buffer)
    {
        fprintf(stderr, "Couldn't open file '%s'\n", filename);
        exit(EXIT_FAILURE);
    }

    TokenizedFile tf = tokenize(fb);
    ParsedFile pf = parse(tf);
    Line *curr = (Line *) pf;
    while (curr)
    {
        putchar('{'); putchar('\n');

        printf("\tLabel: %s\n", curr->label ? curr->label : "---");

        if (curr->type != CMD_NO_COMMAND)
        {
            printf("\tMnemonic: %s\n",
                    (curr->type == CMD_INSTRUCTION) ? curr->instruction->mnemonic : curr->directive->mnemonic);
            printf("\tNum. parameters: %d\n", curr->nparam);
            printf("\tParameters:\n");
            printf("\t[\n");
            ParameterNode *pcurr = curr->phead;
            while (pcurr)
            {
                printf("\t\t{\n");

                printf("\t\t\tType: ");
                switch (pcurr->param.type)
                {
                    case PAR_IMMED_LITERAL:
                         printf("%s\n", "PAR_IMMED_LITERAL");
                         printf("\t\t\tNum. value: %d\n", pcurr->param.num_val);
                         break;
                    case PAR_IMMED_SYMVAL:
                         printf("%s\n", "PAR_IMMED_SYMVAL");
                         printf("\t\t\tSymbol: %s\n", pcurr->param.sym_name);
                         break;
                    case PAR_MEMDIR_SYM:
                         printf("%s\n", "PAR_MEMDIR_SYM");
                         printf("\t\t\tSymbol: %s\n", pcurr->param.sym_name);
                         break;
                    case PAR_MEMDIR_LITERAL:
                         printf("%s\n", "PAR_MEMDIR_LITERAL");
                         printf("\t\t\tNum. value: %d\n", pcurr->param.num_val);
                         break;
                    case PAR_REGDIR:
                         printf("%s\n", "PAR_REGDIR");
                         break;
                    case PAR_REGIND_LITERAL:
                         printf("%s\n", "PAR_REGIND_LITERAL");
                         printf("\t\t\tNum. value: %d\n", pcurr->param.num_val);
                         break;
                    case PAR_REGIND_SYM:
                         printf("%s\n", "PAR_REGIND_SYM");
                         printf("\t\t\tSymbol: %s\n", pcurr->param.sym_name);
                         break;
                    case PAR_PCREL:
                         printf("%s\n", "PAR_PCREL");
                         printf("\t\t\tSymbol: %s\n", pcurr->param.sym_name);
                         break;
                    default:
                         break;
                }

                printf("\t\t\tLength: %s\n", pcurr->param.is_long ? "Long" : "Short");
                if (pcurr->param.reg == -1)
                    printf("\t\t\tRegister: UNUSED\n");
                else
                    printf("\t\t\tRegister: %d\n", pcurr->param.reg);

                printf("\t\t},\n");
                pcurr = pcurr->next;
            }
            printf("\t]\n");
        }
        putchar('}'); putchar(','); putchar('\n');
        curr = curr->next;
    }
}

void test_tokenizer(const char *filename)
{

    FileBuffer fb = read_file(filename);
    if (!fb.buffer)
    {
        fprintf(stderr, "Couldn't open file '%s'\n", filename);
        exit(EXIT_FAILURE);
    }

    TokenizedFile tf = tokenize(fb);
    TokenNode *head = tf;
    while (head)
    {
        switch (head->token.type)
        {
        case TT_NEWLINE:
            printf("[TT_NEWLINE]\n");
            break;
        case TT_SYMBOL:
            printf("[TT_SYMBOL]       # %s\n", head->token.tokstr);
            break;
        case TT_INSTRUCTION:
            printf("[TT_INSTRUCTION]  # %s\n", head->token.tokstr);
            break;
        case TT_REGISTER:
            printf("[TT_REGISTER]     # %s\n", head->token.tokstr);
            break;
        case TT_DIRECTIVE:
            printf("[TT_DIRECTIVE]    # %s\n", head->token.tokstr);
            break;
        case TT_LITERAL:
            printf("[TT_LITERAL]      # %s\n", head->token.tokstr);
            break;
        case TT_COMMENT:
            printf("[TT_COMMENT]\n");
            break;
        case TT_COMMA:
            printf("[TT_COMMA]        # ,\n");
            break;
        case TT_COLON:
            printf("[TT_COLON]        # :\n");
            break;
        case TT_ASTERISK:
            printf("[TT_ASTERISK]     # *\n");
            break;
        case TT_DOLLAR:
            printf("[TT_DOLLAR]       # $\n");
            break;
        case TT_LBRACKET:
            printf("[TT_LBRACKET]     # [\n");
            break;
        case TT_RBRACKET:
            printf("[TT_RBRACKET]     # ]\n");
            break;
        case TT_PLUS:
            printf("[TT_PLUS]         # +\n");
            break;
        case TT_MINUS:
            printf("[TT_MINUS]        # -\n");
            break;
        default:
            printf("Test failed.\n");
            exit(EXIT_FAILURE);
            break;
        }

        head = head->next;
    }
}


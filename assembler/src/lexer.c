/* File: lexer.c */
/* Lexical analysis functionality. */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "log.h"
#include "global.h"
#include "util.h"
#include "lexer.h"

static char *src_cursor = NULL;
static char *pool_cursor = NULL;
static int line_num = 0;

Token parse_multichar_token()
{
    /* token string of current token starts at address pool_cursor inside a token pool */
    Token tok = { pool_cursor, TT_UNDEFINED };

    *pool_cursor++ = *src_cursor;

    /* if invalid starting character found */
    if (!isalnum(*src_cursor) && *src_cursor != '_' && *src_cursor != '.')
    {
        *pool_cursor++ = '\0';
        return tok;
    }

    while (isalnum(*(++src_cursor)) || *src_cursor == '_' || *src_cursor == '.')
    {
        *pool_cursor++ = *src_cursor;
    }
    /* token string of current token ends here in token pool */
    *pool_cursor++ = '\0';

    /* back away, src_cursor should be on the last character of token string */
    --src_cursor;

    if (isdigit(*tok.tokstr)) /* assume it's a numeric literal */
    {
        if (is_dec_number(tok.tokstr)) tok.type = TT_LITERAL;
    }
    else
    {
        if (strlen(tok.tokstr) > SYMBOL_MAXLEN) tok.type = TT_UNDEFINED;
        else if (strlen(tok.tokstr) > RESERVED_WORD_MAXLEN) tok.type = TT_SYMBOL;
        else if (find_instruction(tok.tokstr) != NULL) tok.type = TT_INSTRUCTION;
        else if (find_directive(tok.tokstr) != NULL) tok.type = TT_DIRECTIVE;
        else if (find_register(tok.tokstr) != NULL) tok.type = TT_REGISTER;
        else tok.type = TT_SYMBOL;
    }

    return tok;
}

Token next_token()
{
    Token tok = { NULL, TT_UNDEFINED };

    /* skip whitespace preceding a token */
    while (isspace(*src_cursor) && *src_cursor != '\n')
        ++src_cursor;

    switch (*src_cursor)
    {
    case '\0': tok.type = TT_EOF; break;
    case '\n': tok.type = TT_NEWLINE; ++line_num; break;
    case ':': tok.type = TT_COLON; break;
    case '&': tok.type = TT_AMPERSAND; break;
    case ',': tok.type = TT_COMMA; break;
    case '[': tok.type = TT_LBRACKET; break;
    case ']': tok.type = TT_RBRACKET; break;
    case '+': tok.type = TT_PLUS; break;
    case '-': tok.type = TT_MINUS; break;
    case '*': tok.type = TT_ASTERISK; break;
    case '$': tok.type = TT_DOLLAR; break;
    case ';':
        tok.type = TT_COMMENT;
        while (*(++src_cursor) != '\n' && *src_cursor != '\0')
        {
            /* skip every character to the end of line */
        }
        --src_cursor;
        break;
    default: /* multi-character token expected */
        tok = parse_multichar_token();
        break;
    }

    ++src_cursor;

    return tok;
}

TokenNode *assemble_token_list(void)
{
    TokenNode *head = NULL;
    TokenNode *tail = NULL;
    int nerrors = 0;

    while (1)
    {
        Token t = next_token();

        TokenNode *node = NULL;
        node = (TokenNode *) malloc(sizeof(TokenNode));
        if (!node)
            memory_alloc_error("tokenizer", "TokenNode", sizeof(TokenNode));

        /* if no next token found */
        if (t.type == TT_EOF)
        {
            if (!head)
            {
                fprintf(stderr, "input file empty\n");
                break;
            }

            if (tail->token.type != TT_NEWLINE) /* check if last line ends with newline char. */
            {
                fprintf(stdout, "warning: on line %d: last line not terminated, appended EOL byte(s)\n", line_num);
                write_log(LOG_NORMAL, "tokenizer: appended newline byte to line %d", line_num);
                t.type = TT_NEWLINE;
                node->token = t;
                node->next = NULL;
                tail = tail->next = node;
            }
            else free(node);

            break;
        }

        if (t.type == TT_UNDEFINED)
        {
            ++nerrors;
            fprintf(stderr, "error: on line %d: unrecognized token '%s'\n", line_num, t.tokstr);
        }

        /* insert at the end of the list */
        node->token = t;
        node->next = NULL;
        if (!head) head = node;
        else tail->next = node;
        tail = node;
    }

    if (nerrors > 0)
    {
        TokenNode *t = head;
        while (t)
        {
            head = head->next;
            free(t);
            t = head;
        }
        return NULL;
    }

    return head;
}

TokenizedFile tokenize(FileBuffer file_buffer)
{
    if (!file_buffer.buffer) return NULL;

    TokenizedFile tf = NULL;

    /* Since not all tokens are stored in the pool, and for every token that is stored in the pool there
     * should be a delimiter after it, the size of the pool is not greater than the size of the input file. */
    string_pool_global_size = file_buffer.buffsize;
    string_pool_global = (char *) malloc(file_buffer.buffsize);
    if (!string_pool_global)
        memory_alloc_error("tokenizer", "string pool", file_buffer.buffsize);

    src_cursor = file_buffer.buffer;
    pool_cursor = string_pool_global;
    line_num = 1;

    TokenNode *list_head = assemble_token_list();
    if (list_head != NULL)
    {
        long string_pool_size = pool_cursor - string_pool_global;
        if (string_pool_size > 0)
        {
            char *temp = (char *) realloc(string_pool_global, string_pool_size);
            if (!temp)
                write_log(LOG_NORMAL, "tokenizer: failed to shrink string pool memory block");
            else
            {
                string_pool_global = temp;
                string_pool_global_size = string_pool_size;
            }
        }
        else
        {
            free(string_pool_global);
            string_pool_global = NULL;
            string_pool_global_size = 0L;
        }

        tf = list_head;
    }
    else /* if the list is empty */
    {
        exit(EXIT_FAILURE);
    }

    src_cursor = NULL;
    pool_cursor = NULL;
    line_num = 0;

    return tf;
}

void free_tokenized_file(TokenizedFile tokfile)
{
    TokenNode *head = (TokenNode *)tokfile;
    while (head)
    {
        TokenNode *t = head;
        free(t);
        head = head->next;
    }
}


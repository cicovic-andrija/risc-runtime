/* File: codegen.c */
/* First and second pass of the assembler. Code generator. */

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "log.h"
#include "util.h"
#include "parser.h"
#include "asm_lang.h"
#include "obj_format.h"

static unsigned char *cursor;
static unsigned char obj_code[UINT16_MAX + 1]; /* 2^16 B */

static uint16_t location_counter;

static SymbolTable symtab;

static SymbolTableEntry *current_section;

static RelocationTable reltabs[MAX_NUM_SECTIONS_IN_MODULE];
static RelocationTable *current_reltab;

static SectionHeaderTable hdrtab;

static int eof;

void __attribute__((noreturn)) semantic_error(int line_num, const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    fprintf(stderr, "error: on line %d: ", line_num);
    vfprintf(stderr, format, ap);
    fputc('\n', stderr);
    va_end(ap);
    exit(EXIT_FAILURE);
}

void end_current_section(void)
{
    if (current_section)
    {
        int i = hdrtab.num_sections;
        hdrtab.section[i].idx = current_section->sym_ndx;
        hdrtab.section[i].size = location_counter;
        hdrtab.section[i].load_addr = (uint16_t) -1;
        ++hdrtab.num_sections;
    }
}

uint16_t relocation_patch(SymbolTableEntry *sym, uint16_t relocation_type, uint16_t offset)
{
    uint16_t patch = 0;

    switch (relocation_type)
    {
    case REL_TYPE_ABS16:
        if (sym->sym_bind == BIND_GLOBAL)
        {
            new_relocation_record(current_reltab, relocation_type, offset, sym->sym_num);
            patch = 0;
        }
        else /* BIND_LOCAL */
        {
            new_relocation_record(current_reltab, relocation_type, offset, sym->sym_ndx);
            patch = sym->sym_val;
        }
        break;
    case REL_TYPE_PCREL16:
        if (sym->sym_bind == BIND_GLOBAL)
        {
            new_relocation_record(current_reltab, relocation_type, offset, sym->sym_num);
            patch = (uint16_t)(-2);
        }
        else /* BIND_LOCAL */
        {
            new_relocation_record(current_reltab, relocation_type, offset, sym->sym_ndx);
            patch = (uint16_t)((int16_t)sym->sym_val - 2);
        }
        break;
    default:
        write_log(LOG_ERROR, "codegen: invalid relocation type found while generating relocation patch");
        break;
    }

    return patch;
}

void second_pass_new_section_handler(Line *line)
{
    end_current_section();
    location_counter = 0;
    current_section = find_symbol(&symtab, line->directive->mnemonic);
    if (current_reltab) ++current_reltab;
    else current_reltab = &reltabs[0];
    current_reltab->section_idx = current_section->sym_ndx;
    current_reltab->rel_cnt = 0;
}

void first_pass_new_section_handler(Line *line)
{
    if (line->nparam > 0)
        semantic_error(line->src_line, "directive '%s': invalid number of arguments", line->directive->mnemonic);

    if (find_symbol(&symtab, line->directive->mnemonic) != NULL)
        semantic_error(line->src_line, "multiple occurrence of section '%s'", line->directive->mnemonic);

    new_section(&symtab, line->directive->mnemonic);
    current_section = find_symbol(&symtab, line->directive->mnemonic);
    location_counter = 0;
}

void second_pass_import_export_handler(__attribute__((unused)) Line *line)
{

}

void first_pass_import_export_handler(Line *line)
{
    if (line->nparam == 0)
        semantic_error(line->src_line, "directive '%s': no arguments", line->directive->mnemonic);

    ParameterNode *p = NULL;
    for (p = line->phead; p; p = p->next)
    {
        if (p->param.type != PAR_MEMDIR_SYM)
            semantic_error(line->src_line, "directive '%s': invalid argument type", line->directive->mnemonic);

        SymbolTableEntry *sym = find_symbol(&symtab, p->param.sym_name);
        if (!sym)
            new_symbol(&symtab, 0, 0, BIND_GLOBAL, p->param.sym_name);
        else
            sym->sym_bind = BIND_GLOBAL;
    }
}

void second_pass_eof_handler(__attribute__((unused)) Line *line)
{
    /* void(line); // can be used instead of __attribute__((unused)) */
    eof = 1;
}

void first_pass_eof_handler(Line *line)
{
    if (line->nparam > 0)
        semantic_error(line->src_line, "directive '%s': invalid number of arguments", line->directive->mnemonic);

    second_pass_eof_handler(line);
}

void second_pass_data_def_handler(Line *line)
{
    int size = 0;
    if (strcmp(line->directive->mnemonic, ".CHAR") == 0)
        size = 1;
    else if (strcmp(line->directive->mnemonic, ".WORD") == 0)
        size = 2;
    else
        size = 4;

    int nparam = line->nparam;
    int i;
    if (nparam == 0)
    {
        int32_t val = 0;
        char *byte = (char *) &val;
        for (i = 0; i < size; ++i)
            *cursor++ = *(byte + i);
        location_counter += size;
        return;
    }

    ParameterNode *p;
    int32_t val;
    for (p = line->phead; p; p = p->next)
    {
        if (p->param.type == PAR_IMMED_LITERAL)
        {
            val = p->param.num_val;
        }
        else
        {
            /* parameter type is PAR_MEMDIR_SYM here, but in the context of data initialization,
             * the value of the symbol will be used to initialize data */
            SymbolTableEntry *e = find_symbol(&symtab, p->param.sym_name);
            if (!e)
                semantic_error(line->src_line, "undeclared symbol '%s'", p->param.sym_name);
            val = (int32_t) relocation_patch(e, REL_TYPE_ABS16, location_counter);
        }

        char *byte = (char *) &val;
        for (i = 0; i < size; ++i)
            *cursor++ = *(byte + i);

        location_counter += size;
    }
}

void first_pass_data_def_handler(Line *line)
{
    if (!current_section)
        semantic_error(line->src_line, "unknown section");

    int32_t low_limit, high_limit;
    int size = 0;

    if (strcmp(line->directive->mnemonic, ".CHAR") == 0)
        size = 1, high_limit = INT8_MAX, low_limit = INT8_MIN;
    else if (strcmp(line->directive->mnemonic, ".WORD") == 0)
        size = 2, high_limit = INT16_MAX, low_limit = INT16_MIN;
    else
        size = 4, high_limit = INT32_MAX, low_limit = INT32_MIN;

    ParameterNode *p = NULL;
    for (p = line->phead; p; p = p->next)
    {
        if (p->param.type != PAR_MEMDIR_SYM && p->param.type != PAR_IMMED_LITERAL)
            semantic_error(line->src_line, "directive '%s': invalid argument type", line->directive->mnemonic);
        if (size == 1 && p->param.type == PAR_MEMDIR_SYM)
            semantic_error(line->src_line, "directive '%s': initializer too wide", line->directive->mnemonic);
        if (p->param.type == PAR_IMMED_LITERAL && (p->param.num_val > high_limit || p->param.num_val < low_limit))
            semantic_error(line->src_line, "directive '%s': initializer too wide", line->directive->mnemonic);
    }

    int nparam = line->nparam;
    do
        location_counter += size;
    while (--nparam > 0);
}

void second_pass_align_handler(Line *line)
{
    int power = (uint16_t) line->phead->param.num_val;
    uint16_t mask = ~(~0U << power);
    while (location_counter & mask)
    {
        *cursor++ = 0;
        ++location_counter;
    }
}

void first_pass_align_handler(Line *line)
{
    if (!current_section)
        semantic_error(line->src_line, "unknown section");
    if (line->nparam != 1)
        semantic_error(line->src_line, "directive '%s': invalid number of arguments", line->directive->mnemonic);
    if (line->phead->param.type != PAR_IMMED_LITERAL)
        semantic_error(line->src_line, "directive '%s': invalid argument type", line->directive->mnemonic);

    int power = (uint16_t) line->phead->param.num_val;
    uint16_t mask = ~(~0U << power);
    while (location_counter & mask)
        ++location_counter;
}

void second_pass_skip_handler(Line *line)
{
    uint16_t nbytes = (uint16_t) line->phead->param.num_val;
    while (nbytes--)
    {
        *cursor++ = 0;
        ++location_counter;
    }
}

void first_pass_skip_handler(Line *line)
{
    if (!current_section)
        semantic_error(line->src_line, "unknown section");
    if (line->nparam != 1)
        semantic_error(line->src_line, "directive '%s': invalid number of arguments", line->directive->mnemonic);
    if (line->phead->param.type != PAR_IMMED_LITERAL)
        semantic_error(line->src_line, "directive '%s': invalid argument type", line->directive->mnemonic);

    location_counter += (uint16_t) line->phead->param.num_val;
}

void (*second_pass_directive_handler[])(Line *line) = {
    NULL,
    second_pass_new_section_handler,
    second_pass_import_export_handler,
    second_pass_eof_handler,
    second_pass_data_def_handler,
    second_pass_align_handler,
    second_pass_skip_handler,
};

void (*first_pass_directive_handler[])(Line *line) = {
    NULL,
    first_pass_new_section_handler,
    first_pass_import_export_handler,
    first_pass_eof_handler,
    first_pass_data_def_handler,
    first_pass_align_handler,
    first_pass_skip_handler,
};

uint16_t encode_parameter(Parameter param, uint16_t *first_word, uint16_t in_word_offset, int src_line)
{
    uint16_t second_word = 0;
    SymbolTableEntry *e = NULL;

    switch (param.type)
    {
    case PAR_IMMED_LITERAL:
        *first_word |= AM_IMMED << in_word_offset;
        second_word = (int16_t) param.num_val;
        break;
    case PAR_IMMED_SYMVAL:
        *first_word |= AM_IMMED << in_word_offset;
        e = find_symbol(&symtab, param.sym_name);
        if (!e)
            semantic_error(src_line, "undeclared symbol '%s'", param.sym_name);
        second_word = relocation_patch(e, REL_TYPE_ABS16, location_counter + INSTRUCTION_SIZE);
        break;
    case PAR_MEMDIR_SYM:
        *first_word |= AM_MEMDIR << in_word_offset;
        e = find_symbol(&symtab, param.sym_name);
        if (!e)
            semantic_error(src_line, "undeclared symbol '%s'", param.sym_name);
        second_word = relocation_patch(e, REL_TYPE_ABS16, location_counter + INSTRUCTION_SIZE);
        break;
    case PAR_MEMDIR_LITERAL:
        *first_word |= AM_MEMDIR << in_word_offset;
        second_word = (int16_t) param.num_val;
        break;
    case PAR_REGDIR:
        *first_word |= AM_REGDIR << in_word_offset;
        break;
    case PAR_REGIND_LITERAL:
        *first_word |= AM_REGINDDISP << in_word_offset;
        second_word = (int16_t) param.num_val;
        break;
    case PAR_REGIND_SYM:
        *first_word |= AM_REGINDDISP << in_word_offset;
        e = find_symbol(&symtab, param.sym_name);
        if (!e)
            semantic_error(src_line, "undeclared symbol '%s'", param.sym_name);
        second_word = relocation_patch(e, REL_TYPE_ABS16, location_counter + INSTRUCTION_SIZE);
        break;
    case PAR_PCREL:
        *first_word |= AM_REGINDDISP << in_word_offset;
        e = find_symbol(&symtab, param.sym_name);
        if (!e)
            semantic_error(src_line, "undeclared symbol '%s'", param.sym_name);
        if (e->sym_ndx == current_section->sym_ndx)
        {
            second_word = (uint16_t)((int16_t) e->sym_val - (int16_t) location_counter - INSTRUCTION_SIZE_LONG);
        }
        else
        {
            second_word = relocation_patch(e, REL_TYPE_PCREL16, location_counter + INSTRUCTION_SIZE);
        }
        break;
    default:
        write_log(LOG_ERROR, "codegen: invalid parameter type found while encoding parameter");
        break;
    }

    return second_word;
}

int instruction_size(Line *line)
{
    if (!line || line->type != CMD_INSTRUCTION) return 0; /* error */

    if (line->instruction->opcode != OP_NONE) /* regular instruction */
    {
        if (line->nparam > 0 && (line->phead->param.is_long || (line->phead->next && line->phead->next->param.is_long)))
            return INSTRUCTION_SIZE_LONG;

        return INSTRUCTION_SIZE;
    }
    else /* pseudoinstruction */
    {
        if (starts_with(line->instruction->mnemonic, "HALT"))
            return INSTRUCTION_SIZE_LONG;
        if (starts_with(line->instruction->mnemonic, "RET"))
            return INSTRUCTION_SIZE;
        if (starts_with(line->instruction->mnemonic, "JMP"))
        {
            ParameterType type;
            if (line->nparam == 0) return 0; /* error */
            type = line->phead->param.type;
            if (type == PAR_REGDIR)
                return INSTRUCTION_SIZE;
            if (type == PAR_MEMDIR_SYM || type == PAR_MEMDIR_LITERAL)
                return INSTRUCTION_SIZE_LONG;
            if (type == PAR_PCREL)
                return INSTRUCTION_SIZE_LONG;
            if (type == PAR_REGIND_LITERAL || type == PAR_REGIND_SYM)
                return INSTRUCTION_SIZE + INSTRUCTION_SIZE_LONG;
            return 0; /* error */
        }
        return 0; /* error */
    }
}

void write_instruction(uint16_t first_word, uint16_t second_word, int is_long)
{
    /* don't use little-endian ordering for first instruction word */
    char *byte = (char *) &first_word;
    *cursor++ = *(byte + 1);
    *cursor++ = *byte;
    if (is_long)
    {
        /* use little-endian ordering of bytes for second instruction word */
        byte = (char *) &second_word;
        *cursor++ = *byte;
        *cursor++ = *(byte + 1);
    }
}

void second_pass_instruction_handler(Line *line)
{
    uint16_t first_word = 0x0000;
    uint16_t second_word = 0x0000;

    if (line->instruction->opcode != OP_NONE) /* regular instruction */
    {
        first_word |= line->instruction->cond << 14;
        first_word |= line->instruction->opcode << 10;

        if (line->phead)
        {
            Parameter p1 = line->phead->param;
            first_word |= ((p1.type == PAR_PCREL) ? 0x7 : p1.reg) << 5; /* 0x7 == R7 (PC) */
            second_word = encode_parameter(p1, &first_word, 8, line->src_line);

            if (line->phead->next)
            {
                Parameter p2 = line->phead->next->param;
                first_word |= ((p2.type == PAR_PCREL) ? 0x7 : p2.reg); /* 0x7 == R7 (PC) */
                if (p2.is_long)
                    second_word = encode_parameter(p2, &first_word, 3, line->src_line);
                else
                    encode_parameter(p2, &first_word, 3, line->src_line);
            }
            else
            {
                /* if the first operand isn't used, use register direct addressing mode
                 * to indicate that operand doesn't require two extra bytes */
                first_word |= AM_REGDIR << 3;
            }
        }
        else
        {
            /* if the first and second operand aren't used, use register direct addressing mode
             * to indicate that operands don't require two extra bytes */
            first_word |= AM_REGDIR << 8;
            first_word |= AM_REGDIR << 3;
        }

        int is_long = (instruction_size(line) == INSTRUCTION_SIZE_LONG);
        write_instruction(first_word, second_word, is_long);
    }
    else /* pseudoinstruction */
    {
        SymbolTableEntry *e = NULL;

        if (starts_with(line->instruction->mnemonic, "RET"))
        {
            /* POP<cond> r7 */
            first_word |= line->instruction->cond << 14;
            first_word |= 0xa << 10;
            first_word |= AM_REGDIR << 8;
            first_word |= 0x7 << 5;
            first_word |= AM_REGDIR << 3;
            write_instruction(first_word, second_word, 0);
        }
        else if (starts_with(line->instruction->mnemonic, "HALT"))
        {
            /* OR<cond> psw, 0x0010 */
            first_word |= line->instruction->cond << 14;
            first_word |= (uint16_t) 0x18e0;
            second_word = (uint16_t) 0x0010;
            write_instruction(first_word, second_word, 1);
        }
        else if (starts_with(line->instruction->mnemonic, "JMP"))
        {
            first_word |= line->instruction->cond << 14;
            Parameter src = line->phead->param;
            int opcode = (src.type != PAR_PCREL) ? 0xd : 0x0;

            first_word |= opcode << 10;
            first_word |= AM_REGDIR << 8;
            first_word |= 0x7 << 5;

            switch (src.type)
            {
            default:
                break;
            case PAR_MEMDIR_SYM:
                /* mov<cond> r7, &sym */
                src.type = PAR_IMMED_SYMVAL;
                second_word = encode_parameter(src, &first_word, 3, line->src_line);
                write_instruction(first_word, second_word, 1);
                break;
            case PAR_MEMDIR_LITERAL:
                /* mov<cond> r7, *address */
                src.type = PAR_IMMED_LITERAL;
                second_word = encode_parameter(src, &first_word, 3, line->src_line);
                write_instruction(first_word, second_word, 1);
                break;
            case PAR_REGDIR:
                /* mov<cond> r7, r<n> */
                first_word |= AM_REGDIR << 3;
                first_word |= src.reg;
                write_instruction(first_word, second_word, 0);
                break;
            case PAR_PCREL:
                /* add<cond> r7, displacement */
                e = find_symbol(&symtab, src.sym_name);
                if (!e)
                    semantic_error(line->src_line, "undeclared symbol '%s'", src.sym_name);
                first_word |= AM_IMMED << 3;
                if (e->sym_ndx == current_section->sym_ndx)
                    second_word = (uint16_t)((int16_t) e->sym_val - (int16_t)location_counter - INSTRUCTION_SIZE_LONG);
                else
                    second_word = relocation_patch(e, REL_TYPE_PCREL16, location_counter + INSTRUCTION_SIZE);
                write_instruction(first_word, second_word, 1);
                break;
            }
        }
    }

    location_counter += instruction_size(line);
}

void first_pass_instruction_handler(Line *line)
{
    if (!current_section)
        semantic_error(line->src_line, "unknown section");
    if (line->nparam != line->instruction->nparam)
        semantic_error(line->src_line, "invalid number of arguments");
    if (line->nparam > 0)
    {
        ParameterNode *p1 = line->phead;
        if (line->instruction->af1_type == AF_DST &&
                (p1->param.type == PAR_IMMED_LITERAL || p1->param.type == PAR_IMMED_SYMVAL))
        {
            semantic_error(line->src_line, "invalid argument type for destination field");
        }
        if (p1->param.type == PAR_PCREL &&
             !(line->instruction->opcode == OP_CALL || starts_with(line->instruction->mnemonic, "JMP")))
        {
            semantic_error(line->src_line, "PC-relative addressing mode used in non-jump instruction");
        }
        if ((p1->param.type == PAR_REGIND_LITERAL || p1->param.type == PAR_REGIND_SYM) &&
             (line->instruction->opcode == OP_CALL || starts_with(line->instruction->mnemonic, "JMP")))
        {
            semantic_error(line->src_line, "regind addressing mode used in jump instruction");
        }
        if (line->nparam > 1)
        {
            ParameterNode *p2 = p1->next;
            if (p2->param.type == PAR_PCREL &&
                 !(line->instruction->opcode == OP_CALL || starts_with(line->instruction->mnemonic, "JMP")))
            {
                semantic_error(line->src_line, "PC-relative addressing mode used in non-jump instruction");
            }
            if (p1->param.is_long && p2->param.is_long)
                semantic_error(line->src_line, "more than one long addressing mode found");
        }
    }

    location_counter += instruction_size(line);
}

void label_handler(Line *line)
{
    if (!current_section)
        semantic_error(line->src_line, "unknown section");

    SymbolTableEntry *sym = find_symbol(&symtab, line->label);
    if (!sym)
        new_symbol(&symtab, current_section->sym_ndx, location_counter, BIND_LOCAL, line->label);
    else if (sym->sym_ndx != 0)
        semantic_error(line->src_line, "multiple definitions of symbol '%s'", line->label);
    else
    {
        sym->sym_ndx = current_section->sym_ndx;
        sym->sym_val = location_counter;
    }
}

void second_pass(ParsedFile parsed_file)
{
    location_counter = 0;
    current_section = NULL;
    eof = 0;

    Line *line = NULL;
    for (line = parsed_file; !eof && line; line = line->next)
    {
        if (line->type == CMD_INSTRUCTION)
        {
            second_pass_instruction_handler(line);
        }
        else if (line->type == CMD_DIRECTIVE)
        {
            second_pass_directive_handler[line->directive->type](line);
        }
    }

    end_current_section();
}

void first_pass(ParsedFile parsed_file)
{
    cursor = obj_code;
    location_counter = 0;
    symtab.first = NULL;
    symtab.last = NULL;
    symtab.sym_cnt = 0;
    current_section = NULL;
    current_reltab = NULL;
    hdrtab.num_sections = 0;
    eof = 0;

    Line *line = NULL;
    for (line = parsed_file; !eof && line; line = line->next)
    {
        if (line->label)
        {
            label_handler(line);
        }
        if (line->type == CMD_INSTRUCTION)
        {
            first_pass_instruction_handler(line);
        }
        else if (line->type == CMD_DIRECTIVE)
        {
            first_pass_directive_handler[line->directive->type](line);
        }
    }
}

int generate_code(ParsedFile parsed_file, uint16_t sections_load_address,
        const char *out_filename, const char *out_txt_filename)
{
    if (!parsed_file || !out_filename) return 1; /* error */

    /* generate code */
    first_pass(parsed_file);

    second_pass(parsed_file);

    /* calc. load addresses */
    unsigned i;
    if (sections_load_address != (uint16_t) -1)
    {
        uint16_t load_addr = sections_load_address;
        for (i = 0; i < hdrtab.num_sections; ++i)
        {
            hdrtab.section[i].load_addr = load_addr;

            if (((long)load_addr + hdrtab.section[i].size) > UINT16_MAX)
            {
                fprintf(stderr, "content too large to load at address %d\n", sections_load_address);
                exit(EXIT_FAILURE);
            }

            load_addr += hdrtab.section[i].size;
        }
    }

    /* open output files */
    FILE *out_fp = NULL;
    FILE *out_txt_fp = NULL;

    out_fp = fopen(out_filename, "wb");
    if (!out_fp)
    {
        fprintf(stderr, "error: failed to open output file '%s'\n", out_filename);
        exit(EXIT_FAILURE);
    }

    if (out_txt_filename && !(out_txt_fp = fopen(out_txt_filename, "w")))
    {
        fprintf(stderr, "error: failed to open output file '%s'\n", out_txt_filename);
        exit(EXIT_FAILURE);
    }

    /* write symbol table */
    write_symtab(&symtab, out_fp, out_txt_fp);

    /* write relocation record tables */
    write_reltabs(&reltabs[0], hdrtab.num_sections, &symtab, out_fp, out_txt_fp);

    /* write section header table */
    write_section_hdrtab(&hdrtab, out_fp, out_txt_fp);

    /* write section content */
    cursor = obj_code;
    uint16_t cursor_offset = 0;
    for (i = 0; i < hdrtab.num_sections; ++i)
    {
        const char *section_name = find_section(&symtab, hdrtab.section[i].idx)->sym_name;
        write_section(cursor + cursor_offset, hdrtab.section[i].size, out_fp, out_txt_fp, section_name);
        cursor_offset += hdrtab.section[i].size;
    }

    fclose(out_fp);
    if (out_txt_fp)
        fclose(out_txt_fp);
    return 0;
}


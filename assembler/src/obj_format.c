/* File: obj_format.c */
/* Data structures and operations used to encode object files. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "log.h"
#include "util.h"
#include "obj_format.h"

/* Symbol Table */

SymbolTableEntry *find_symbol(SymbolTable *symtab, const char *sym_name)
{
    if (!symtab || !sym_name) return NULL;

    SymbolTableNode *t = NULL;
    for (t = symtab->first; t; t = t->next)
    {
        if (strcmp(t->entry.sym_name, sym_name) == 0) return &t->entry;
    }

    return NULL;
}

SymbolTableEntry *find_symbol_with_num(SymbolTable *symtab, uint16_t sym_num)
{
    if (!symtab) return NULL;

    SymbolTableNode *t = NULL;
    for (t = symtab->first; t; t = t->next)
    {
        if (t->entry.sym_num == sym_num) return &t->entry;
    }

    return NULL;
}

SymbolTableEntry *find_section(SymbolTable *symtab, uint16_t section_idx)
{
    if (!symtab || section_idx == 0) return NULL;

    SymbolTableNode *t = NULL;
    for (t = symtab->first; t; t = t->next)
    {
        if (section_idx == t->entry.sym_ndx && t->entry.sym_type == TYPE_SECTION)
        {
            return &t->entry;
        }
    }

    return NULL;
}

void init_symtab(SymbolTable *symtab)
{
    symtab->first = (SymbolTableNode *) malloc(sizeof(SymbolTableNode));
    if (!symtab->first)
        memory_alloc_error("object input/output", "SymbolTableNode", sizeof(SymbolTableNode));

    symtab->first->entry.sym_num = 0;
    symtab->first->entry.sym_type = TYPE_UNDEF;
    symtab->first->entry.sym_ndx = 0;
    symtab->first->entry.sym_val = 0;
    symtab->first->entry.sym_bind = BIND_LOCAL;
    symtab->first->entry.sym_name[0] = '\0';

    symtab->first->next = NULL;
    symtab->last = symtab->first;
    symtab->sym_cnt = 1;
}

SymbolTableEntry *new_symbol(SymbolTable *symtab, uint16_t sym_ndx,
        uint16_t sym_val, uint16_t sym_bind, const char *sym_name)
{
    if (!symtab || !sym_name) return NULL; /* error */

    if (!symtab->first)
        init_symtab(symtab);

    SymbolTableNode *symtab_node = (SymbolTableNode *) malloc(sizeof(SymbolTableNode));
    if (!symtab_node)
        memory_alloc_error("object input/output", "SymbolTableNode", sizeof(SymbolTableNode));

    symtab_node->entry.sym_num = symtab->sym_cnt++;
    symtab_node->entry.sym_type = TYPE_SYMBOL;
    symtab_node->entry.sym_ndx = sym_ndx;
    symtab_node->entry.sym_val = sym_val;
    symtab_node->entry.sym_bind = sym_bind;
    strcpy(symtab_node->entry.sym_name, sym_name);

    symtab->last->next = symtab_node;
    symtab->last = symtab_node;
    symtab_node->next = NULL;

    return &symtab_node->entry;
}

SymbolTableEntry *new_section(SymbolTable *symtab, const char *section_name)
{
    if (!symtab || !section_name) return NULL; /* error */

    if (!symtab->first)
        init_symtab(symtab);

    SymbolTableNode *symtab_node = (SymbolTableNode *) malloc(sizeof(SymbolTableNode));
    if (!symtab_node)
        memory_alloc_error("object input/output", "SymbolTableNode", sizeof(SymbolTableNode));

    symtab_node->entry.sym_num = symtab->sym_cnt++;
    symtab_node->entry.sym_type = TYPE_SECTION;
    symtab_node->entry.sym_ndx = symtab_node->entry.sym_num;
    symtab_node->entry.sym_val = 0;
    symtab_node->entry.sym_bind = BIND_LOCAL;
    strcpy(symtab_node->entry.sym_name, section_name);

    symtab->last->next = symtab_node;
    symtab->last = symtab_node;
    symtab_node->next = NULL;

    return &symtab_node->entry;
}

int write_symtab(SymbolTable *symtab, FILE *fp, FILE *txt_fp)
{
    if (!symtab || !fp) return 1; /* error */

    fwrite(&symtab->sym_cnt, sizeof(uint32_t), 1, fp);

    SymbolTableNode *symtab_node = NULL;
    for (symtab_node = symtab->first; symtab_node; symtab_node = symtab_node->next)
        fwrite(&symtab_node->entry, sizeof(SymbolTableEntry), 1, fp);

    if (txt_fp)
    {
        fprintf(txt_fp, "### SYMBOL TABLE ###\n");
        fprintf(txt_fp, "+------------------------------------------------------------------------------+\n");
        fprintf(txt_fp, "|%7s|%32s|%9s|%9s|%8s|%8s|\n", "INDEX", "NAME", "TYPE", "SECTION", "VALUE", "BIND");
        fprintf(txt_fp, "+------------------------------------------------------------------------------+\n");
        for (symtab_node = symtab->first; symtab_node; symtab_node = symtab_node->next)
        {
            SymbolTableEntry e = symtab_node->entry;

            char type[8];
            if (e.sym_type == TYPE_UNDEF) strcpy(type, "UNDEF");
            if (e.sym_type == TYPE_SECTION) strcpy(type, "SECTION");
            if (e.sym_type == TYPE_SYMBOL) strcpy(type, "SYMBOL");

            char bind[8];
            if (e.sym_bind == BIND_LOCAL) strcpy(bind, "LOCAL");
            if (e.sym_bind == BIND_GLOBAL) strcpy(bind, "GLOBAL");

            fprintf(txt_fp, "|%#7x|%32s|%9s|%#9x|%#8x|%8s|\n",
                    e.sym_num, e.sym_name, type, e.sym_ndx, e.sym_val, bind);
        }
        fprintf(txt_fp, "+------------------------------------------------------------------------------+\n");
    }

    return 0;
}

int read_symtab(SymbolTable *symtab, FILE *fp)
{
    if (!symtab || !fp) return 1; /* error */

    symtab->first = symtab->last = NULL;
    symtab->sym_cnt = 0;

    /* read total symbol count */
    fread(&symtab->sym_cnt, sizeof(uint32_t), 1, fp);

    unsigned i;
    for (i = 0; i < symtab->sym_cnt; ++i)
    {
        SymbolTableNode *node = (SymbolTableNode *) malloc(sizeof(SymbolTableNode));
        if (!node)
            memory_alloc_error("object input/output", "SymbolTableNode", sizeof(SymbolTableNode));
        fread(&node->entry, sizeof(SymbolTableEntry), 1, fp);

        if (!symtab->first)
            symtab->first = node;
        else
            symtab->last->next = node;
        symtab->last = node;
        node->next = NULL;
    }

    return 0;
}

void free_symtab(SymbolTable *symtab)
{
    if (!symtab) return;

    while (symtab->first)
    {
        SymbolTableNode *temp = symtab->first;
        symtab->first = symtab->first->next;
        free(temp);
    }
    symtab->last = NULL;
    symtab->sym_cnt = 0;
}

/* Relocation Records Table */

int new_relocation_record(RelocationTable *reltab, uint16_t rel_type, uint16_t offset, uint16_t sym_num)
{
    if (!reltab) return 1; /* error */

    RelocationTableNode *reltab_node = (RelocationTableNode *) malloc(sizeof(RelocationTableNode));
    if (!reltab_node)
        memory_alloc_error("object input/output", "RelocationTableNode", sizeof(RelocationTableNode));

    reltab_node->record.rel_num = reltab->rel_cnt++;
    reltab_node->record.rel_type = rel_type;
    reltab_node->record.offset = offset;
    reltab_node->record.sym_num = sym_num;

    if (!reltab->first)
        reltab->first = reltab_node;
    else
        reltab->last->next = reltab_node;
    reltab->last = reltab_node;
    reltab_node->next = NULL;

    return 0;
}

int write_reltabs(RelocationTable *reltab, uint32_t ntabs, SymbolTable *symtab, FILE *fp, FILE *txt_fp)
{
    if (!reltab || !symtab || !fp) return 1; /* error */

    /* Write how many relocation tables there is */
    fwrite(&ntabs, sizeof(uint32_t), 1, fp);

    unsigned i;
    for (i = 0; i < ntabs; ++i)
    {
        /* For each table first write the number of relocation records and sectiond id */
        fwrite(&reltab[i].rel_cnt, sizeof(uint32_t), 1, fp);
        fwrite(&reltab[i].section_idx, sizeof(uint16_t), 1, fp);

        RelocationTableNode *reltab_node = NULL;
        for (reltab_node = reltab[i].first; reltab_node; reltab_node = reltab_node->next)
            fwrite(&reltab_node->record, sizeof(RelocationRecord), 1, fp);


        if (txt_fp)
        {
            const char *section_name = "<UNKNOWN SECTION>";
            SymbolTableEntry *section = find_section(symtab, reltab[i].section_idx);
            if (section) /* should never be NULL */
                section_name = section->sym_name;

            fprintf(txt_fp, "### %s RELOCATION TABLE ###\n", section_name);
            fprintf(txt_fp, "+------------------------------------------------------------------------------+\n");
            fprintf(txt_fp, "|%7s|%32s|%19s|%17s|\n", "INDEX", "RELOCATION_TYPE", "OFFSET", "SYMBOL");
            fprintf(txt_fp, "+------------------------------------------------------------------------------+\n");
            for (reltab_node = reltab[i].first; reltab_node; reltab_node = reltab_node->next)
            {
                RelocationRecord r = reltab_node->record;

                char type[32];
                if (r.rel_type == REL_TYPE_ABS16) strcpy(type, "REL_TYPE_ABS16");
                if (r.rel_type == REL_TYPE_PCREL16) strcpy(type, "REL_TYPE_PCREL16");

                fprintf(txt_fp, "|%#7x|%32s|%#19x|%#17x|\n", r.rel_num, type, r.offset, r.sym_num);
            }
            fprintf(txt_fp, "+------------------------------------------------------------------------------+\n");
        }
    }

    return 0;
}

int read_reltabs(RelocationTable *reltab, FILE *fp)
{
    if (!reltab || !fp) return 1; /* error */

    uint32_t ntabs = (uint32_t) -1;
    fread(&ntabs, sizeof(uint32_t), 1, fp);
    if (ntabs > MAX_NUM_SECTIONS_IN_MODULE) return 1; /* error */

    unsigned i;
    for (i = 0; i < ntabs; ++i)
    {
        fread(&reltab[i].rel_cnt, sizeof(uint32_t), 1, fp);
        fread(&reltab[i].section_idx, sizeof(uint16_t), 1, fp);

        reltab[i].first = NULL;
        reltab[i].last = NULL;

        unsigned j;
        for (j = 0; j < reltab[i].rel_cnt; ++j)
        {
            RelocationTableNode *node = (RelocationTableNode *) malloc(sizeof(RelocationTableNode));
            if (!node)
                memory_alloc_error("object input/output", "RelocationTableNode", sizeof(RelocationTableNode));
            fread(&node->record, sizeof(RelocationRecord), 1, fp);

            if (!reltab[i].first)
                reltab[i].first = node;
            else
                reltab[i].last->next = node;
            reltab[i].last = node;
            node->next = NULL;
        }
    }

    return 0;
}

void free_reltabs(RelocationTable *reltab, int ntabs)
{
    if (!reltab) return;
    int i;
    for (i = 0; i < ntabs; ++i)
    {
        while (reltab[i].first)
        {
            RelocationTableNode *temp = reltab[i].first;
            reltab[i].first = reltab[i].first->next;
            free(temp);
        }
        reltab[i].last = NULL;
        reltab[i].rel_cnt = 0;
    }
}

/* Section Header Table and Sections */

int write_section_hdrtab(SectionHeaderTable *hdrtab, FILE *fp, FILE *txt_fp)
{
    if (!hdrtab || !fp) return 1; /* error */

    fwrite(hdrtab, sizeof(SectionHeaderTable), 1, fp);

    if (txt_fp)
    {
        unsigned i;
        fprintf(txt_fp, "### SECTION HEADER TABLE ###\n");
        fprintf(txt_fp, "+------------------------------------------------------------------------------+\n");
        fprintf(txt_fp, "|%40s|%19s|%17s|\n", "SECTION_INDEX", "SIZE", "LOAD_ADDRESS");
        fprintf(txt_fp, "+------------------------------------------------------------------------------+\n");
        for (i = 0; i < hdrtab->num_sections; ++i)
        {
            fprintf(txt_fp, "|%#40x|%#19x|%#17x|\n",
                    hdrtab->section[i].idx, hdrtab->section[i].size, hdrtab->section[i].load_addr);
        }
        fprintf(txt_fp, "+------------------------------------------------------------------------------+\n");
    }

    return 0;
}

int read_section_hdrtab(SectionHeaderTable *hdrtab, FILE *fp)
{
    if (!hdrtab || !fp) return 1; /* error */
    fread(hdrtab, sizeof(SectionHeaderTable), 1, fp);
    return 0;
}

/* Program Header Table */

int new_segment(ProgramHeaderTable *hdrtab, uint16_t load_addr, uint16_t idx, uint32_t size)
{
    if (!hdrtab) return 1; /* error */

    ProgramHeaderNode *hdr_node = (ProgramHeaderNode *) malloc(sizeof(ProgramHeaderNode));
    if (!hdr_node)
        memory_alloc_error("object input/output", "ProgramHeaderNode", sizeof(ProgramHeaderNode));

    hdr_node->record.load_addr = load_addr;
    hdr_node->record.idx = idx;
    hdr_node->record.size = size;

    if (!hdrtab->first)
        hdrtab->first = hdr_node;
    else
        hdrtab->last->next = hdr_node;
    hdrtab->last = hdr_node;
    hdr_node->next = NULL;

    ++hdrtab->segment_cnt;

    return 0;
}

int write_program_hdrtab(ProgramHeaderTable *hdrtab, FILE *fp, FILE *txt_fp)
{
    if (!hdrtab || !fp) return 1; /* error */

    fwrite(&hdrtab->segment_cnt, sizeof(uint32_t), 1, fp);
    ProgramHeaderNode *hdr_node = NULL;
    for (hdr_node = hdrtab->first; hdr_node; hdr_node = hdr_node->next)
        fwrite(&hdr_node->record, sizeof(SegmentRecord), 1, fp);

    if (txt_fp)
    {
        fprintf(txt_fp, "### PROGRAM HEADER TABLE ###\n");
        fprintf(txt_fp, "+------------------------------------------------------------------------------+\n");
        fprintf(txt_fp, "|%40s|%19s|%17s|\n", "SEGMENT_INDEX", "SIZE", "LOAD_ADDRESS");
        fprintf(txt_fp, "+------------------------------------------------------------------------------+\n");
        for (hdr_node = hdrtab->first; hdr_node; hdr_node = hdr_node->next)
        {
            fprintf(txt_fp, "|%#40x|%#19x|%#17x|\n",
                    hdr_node->record.idx, hdr_node->record.size, hdr_node->record.load_addr);
        }
        fprintf(txt_fp, "+------------------------------------------------------------------------------+\n");
    }

    return 0;
}

int read_program_hdrtab(ProgramHeaderTable *hdrtab, FILE *fp)
{
    if (!hdrtab || !fp) return 1; /* error */

    hdrtab->first = hdrtab->last = NULL;
    hdrtab->segment_cnt = 0;

    /* read total segment count */
    fread(&hdrtab->segment_cnt, sizeof(uint32_t), 1, fp);

    unsigned i;
    for (i = 0; i < hdrtab->segment_cnt; ++i)
    {
        ProgramHeaderNode *node = (ProgramHeaderNode *) malloc(sizeof(ProgramHeaderNode));
        if (!node)
            memory_alloc_error("object input/output", "ProgramHeaderNode", sizeof(ProgramHeaderNode));
        fread(&node->record, sizeof(SegmentRecord), 1, fp);

        if (!hdrtab->first)
            hdrtab->first = node;
        else
            hdrtab->last->next = node;
        hdrtab->last = node;
        node->next = NULL;
    }

    return 0;
}

/* Sections */

int write_section(const unsigned char *content, uint32_t size, FILE *fp, FILE *txt_fp, const char *section_name)
{
    if (!content || !fp) return 1; /* error */
    if (txt_fp && !section_name) return 1; /* error */

    fwrite(content, sizeof(unsigned char), size, fp);

    if (txt_fp)
    {
        fprintf(txt_fp, "### %s SECTION ###\n", section_name);
        unsigned i;
        unsigned br = 27;
        for (i = 0; i < size; ++i)
            fprintf(txt_fp, "%02x%s", *(content + i), ((i + 1) % br == 0) ? "\n" : " ");

        if (i % br != 0) fputc('\n', txt_fp);
    }

    return 0;
}

int read_section(unsigned char *buffer, uint32_t size, FILE *fp)
{
    if (!buffer || !fp) return 1; /* error */
    fread(buffer, sizeof(unsigned char), size, fp);
    return 0;
}


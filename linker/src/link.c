/* File: link.c */
/* Two-pass linker. */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"
#include "obj_format.h"
#include "link.h"

void __attribute__((noreturn)) link_error(const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    fprintf(stderr, "link error: ");
    vfprintf(stderr, format, ap);
    fputc('\n', stderr);
    va_end(ap);
    exit(EXIT_FAILURE);
}

typedef struct rel_req {
    uint16_t section_idx;
    uint16_t rel_type;
    uint16_t offset;
    uint16_t sym_num;
    struct rel_req *next;
} RelocationRequest;

static RelocationRequest *reloc_req_head = NULL;
static RelocationRequest *reloc_req_tail = NULL;

void new_relocation_request(uint16_t section_idx,
        uint16_t rel_type, uint16_t offset, uint16_t sym_num)
{
    RelocationRequest *rel_req = (RelocationRequest *) malloc(sizeof(RelocationRequest));
    if (!rel_req)
        memory_alloc_error("linker", "RelocationRequest", sizeof(RelocationRequest));
    rel_req->section_idx = section_idx;
    rel_req->rel_type = rel_type;
    rel_req->offset = offset;
    rel_req->sym_num = sym_num;
    rel_req->next = NULL;
    if (!reloc_req_head)
        reloc_req_head = rel_req;
    else
        reloc_req_tail->next = rel_req;
    reloc_req_tail = rel_req;
}

static unsigned char obj_code[UINT16_MAX + 1]; /* 2^16 B */

static SymbolTable symtab;
static ProgramHeaderTable prog_hdrtab;

void merge(FILE **obj_fp, int nfiles)
{
    uint16_t next_load_address = ORIGIN_ADDRESS;

    symtab.first = symtab.last = NULL;
    symtab.sym_cnt = 0;
    prog_hdrtab.first = prog_hdrtab.last = NULL;
    prog_hdrtab.segment_cnt = 0;

    int i;
    /* Note: assumed obf_fp[i] is a valid relocatable object file. */
    for (i = 0; i < nfiles; ++i)
    {
        SymbolTable st;
        RelocationTable rtabs[MAX_NUM_SECTIONS_IN_MODULE];
        SectionHeaderTable hdrtab;

        read_symtab(&st, obj_fp[i]);
        read_reltabs(rtabs, obj_fp[i]);
        read_section_hdrtab(&hdrtab, obj_fp[i]);

        uint16_t *sym_reorder_map = (uint16_t *) malloc(st.sym_cnt * sizeof(uint16_t));
        if (!sym_reorder_map)
            memory_alloc_error("linker", "symbol reorder map", st.sym_cnt * sizeof(uint16_t));

        SymbolTableNode *st_node = NULL;
        for (st_node = st.first; st_node; st_node = st_node->next)
        {
            SymbolTableEntry entry = st_node->entry;

            /* first look only for the section entries */
            if (!(entry.sym_type == TYPE_SECTION))
                continue;

            static int segment_counter = 0;
            char segment_name[SYMBOL_MAXLEN + 1];
            sprintf(segment_name, ".SEGMENT.%x", ++segment_counter);

            SymbolTableEntry *new_entry = new_section(&symtab, segment_name);
            sym_reorder_map[entry.sym_num] = new_entry->sym_num;

            unsigned k;
            for (k = 0; k < hdrtab.num_sections; ++k) /* search O(1) time */
                if (hdrtab.section[k].idx == entry.sym_ndx)
                    break; /* always hit break for some value of k */

            uint16_t load_addr;
            extern int ignore_predefined_origin;
            if (ignore_predefined_origin || hdrtab.section[k].load_addr == (uint16_t)-1)
            {
                /* load address not specified */
                load_addr = next_load_address;
                next_load_address += hdrtab.section[k].size;
            }
            else
            {
                /* specified load address */
                load_addr = hdrtab.section[k].load_addr;
            }

            if ((long) load_addr + hdrtab.section[k].size > UINT16_MAX)
                link_error("object code too large to link in %d address space", UINT16_MAX + 1);

            /* Read section content from an object file. */
            /* sections are stored in object files in the same order as the
             * sections appear in the symbol table */
            read_section(obj_code + load_addr, hdrtab.section[k].size, obj_fp[i]);

            new_entry->sym_val = load_addr;
            new_segment(&prog_hdrtab, load_addr, new_entry->sym_num, hdrtab.section[k].size);
        }

        for (st_node = st.first; st_node; st_node = st_node->next)
        {
            SymbolTableEntry entry = st_node->entry;

            /* now, look only for the global symbol entries */
            if (!(entry.sym_type == TYPE_SYMBOL && entry.sym_bind == BIND_GLOBAL))
                continue;

            if (entry.sym_ndx == 0) /* global imported symbol */
            {
                SymbolTableEntry *new_entry = find_symbol(&symtab, entry.sym_name);
                if (!new_entry)
                    new_entry = new_symbol(&symtab, 0, 0, BIND_GLOBAL, entry.sym_name);
                sym_reorder_map[entry.sym_num] = new_entry->sym_num;
            }
            else /* global exported symbol */
            {
                SymbolTableEntry *new_entry = find_symbol(&symtab, entry.sym_name);
                uint16_t new_sym_ndx = sym_reorder_map[entry.sym_ndx];
                SymbolTableEntry *segment = find_section(&symtab, new_sym_ndx);
                uint16_t new_sym_val = segment->sym_val + entry.sym_val;
                if (!new_entry)
                {
                    new_entry = new_symbol(&symtab, new_sym_ndx, new_sym_val, BIND_GLOBAL, entry.sym_name);
                }
                else
                {
                    if (new_entry->sym_ndx != 0)
                        link_error("multiple definitions of symbol '%s'", entry.sym_name);

                    new_entry->sym_ndx = new_sym_ndx;
                    new_entry->sym_val = new_sym_val;
                }
                sym_reorder_map[entry.sym_num] = new_entry->sym_num;
            }

        }

        unsigned j;
        for (j = 0; j < hdrtab.num_sections; ++j)
        {
            uint16_t new_section_idx = sym_reorder_map[rtabs[j].section_idx];
            RelocationTableNode *rel_node;
            for (rel_node = rtabs[j].first; rel_node; rel_node = rel_node->next)
            {
                RelocationRecord rel_rec = rel_node->record;
                uint16_t new_sym_num = sym_reorder_map[rel_rec.sym_num];
                new_relocation_request(new_section_idx, rel_rec.rel_type, rel_rec.offset, new_sym_num);
            }
        }

        free(sym_reorder_map);
        free_symtab(&st);
        free_reltabs(rtabs, hdrtab.num_sections);
    }
}

void expect_START(void)
{
    SymbolTableEntry *start = find_symbol(&symtab, "START");
    if (!start || start->sym_ndx == 0)
        link_error("undefined reference to 'START'");
}

void patch(void)
{
    RelocationRequest *rel_request = NULL;
    uint16_t curr_idx = 0;
    uint16_t curr_load_address = (uint16_t)-1;

    for (rel_request = reloc_req_head; rel_request; rel_request = rel_request->next)
    {
        SymbolTableEntry *sym_entry = find_symbol_with_num(&symtab, rel_request->sym_num);
        if (sym_entry->sym_ndx == 0)
            link_error("undefined reference to '%s'", sym_entry->sym_name);

        if (rel_request->section_idx != curr_idx)
        {
            ProgramHeaderNode *t = prog_hdrtab.first;
            while (t->record.idx != rel_request->section_idx)
                t = t->next;
            curr_idx = t->record.idx;
            curr_load_address = t->record.load_addr;
        }

        int16_t symval = sym_entry->sym_val;
        int16_t patch_ = 0;
        patch_ += (int16_t)obj_code[curr_load_address + rel_request->offset];
        patch_ += (int16_t)obj_code[curr_load_address + rel_request->offset + 1] << 8;

        switch (rel_request->rel_type)
        {
        case REL_TYPE_ABS16:
            patch_ = symval + patch_;
            break;
        case REL_TYPE_PCREL16:
            patch_ = symval - ((int16_t)curr_load_address + (int16_t)rel_request->offset) + patch_;
            break;
        default:
            break;
        }

        char *byte = (char *) &patch_;
        obj_code[curr_load_address + rel_request->offset] = *byte;
        obj_code[curr_load_address + rel_request->offset + 1] = *(byte + 1);
    }
}

void check_overlap(void)
{
    extern int ignore_predefined_origin;
    if (ignore_predefined_origin)
        return; /* linker made sure there is no overlap */

    ProgramHeaderNode *i;
    for (i = prog_hdrtab.first; i; i = i->next)
    {
        ProgramHeaderNode *j;
        for (j = i->next; j; j = j->next)
        {
            if (i->record.size == 0 || j->record.size == 0)
                continue;

            uint16_t start1 = i->record.load_addr;
            uint16_t end1 = start1 + i->record.size - 1;
            uint16_t start2 = j->record.load_addr;
            uint16_t end2 = start2 + j->record.size - 1;
            if (!(start2 > end1 || start1 > end2))
            {
                FILE *fp = fopen("xxx", "wb");
                write_symtab(&symtab, fp, stdout);
                link_error("load address conflict between segments starting at %#x and %#x", start1, start2);
            }
        }
    }
}

int link_files(FILE **obj_fp, int nfiles, const char *out_filename, const char *out_txt_filename)
{
    if (!obj_fp || nfiles < 0 || !out_filename) return 1; /* error */
    /* link files */
    merge(obj_fp, nfiles);
    patch();
    expect_START();
    check_overlap();

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

    /* write program header table */
    write_program_hdrtab(&prog_hdrtab, out_fp, out_txt_fp);

    /* write segment content */
    ProgramHeaderNode *prog_hdr_node;
    for (prog_hdr_node = prog_hdrtab.first; prog_hdr_node; prog_hdr_node = prog_hdr_node->next)
    {
        const char *segment_name = find_section(&symtab, prog_hdr_node->record.idx)->sym_name;
        const unsigned char *content = obj_code + prog_hdr_node->record.load_addr;
        uint32_t size = prog_hdr_node->record.size;
        write_section(content, size, out_fp, out_txt_fp, segment_name);
    }

    fclose(out_fp);
    if (out_txt_fp)
        fclose(out_txt_fp);
    return 0;
}


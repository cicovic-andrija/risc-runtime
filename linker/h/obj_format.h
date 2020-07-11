/* File: obj_format.h */
/* Data structures and operations used to encode object files. */

#ifndef OBJ_FORMAT_H
#define OBJ_FORMAT_H

#define INSTRUCTION_SIZE 2

#define INSTRUCTION_SIZE_LONG 4

#define MAX_NUM_SECTIONS_IN_MODULE 4

#define SYMBOL_MAXLEN 31

#include <stdint.h>
#include <stdio.h>

/* Symbol Table */

enum { BIND_LOCAL, BIND_GLOBAL };

enum { TYPE_UNDEF = 0, TYPE_SECTION, TYPE_SYMBOL };

typedef struct {
    uint16_t sym_num;
    uint16_t sym_type;
    uint16_t sym_ndx;
    uint16_t sym_val;
    uint16_t sym_bind;
    char sym_name[SYMBOL_MAXLEN + 1];
} SymbolTableEntry;

typedef struct symtab_node {
    SymbolTableEntry entry;
    struct symtab_node *next;
} SymbolTableNode;

typedef struct symtab {
    uint32_t sym_cnt;
    SymbolTableNode *first;
    SymbolTableNode *last;
} SymbolTable;

/* Function new_symbol inserts a new symbol entry in a given symbol table. */
/* Returns a pointer to a new symbol. */
SymbolTableEntry *new_symbol(SymbolTable *symtab, uint16_t sym_ndx,
        uint16_t sym_val, uint16_t sym_bind, const char *sym_name);

/* Function new_section inserts a new section symbol entry in a given symbol table. */
/* Returns a pointer to a new section. */
SymbolTableEntry *new_section(SymbolTable *symtab, const char *section_name);

/* Function find_symbol returns a pointer to a symbol table entry for a given symbol. */
SymbolTableEntry *find_symbol(SymbolTable *symtab, const char *sym);

/* Function find_symbol_with_num returns a pointer to a symbol table entry for a given symbol number. */
SymbolTableEntry *find_symbol_with_num(SymbolTable *symtab, uint16_t sym_num);

/* Function find_section returns a pointer to a section symbol table entry for a given section index. */
SymbolTableEntry *find_section(SymbolTable *symtab, uint16_t section_idx);

/* Function write_symtab writes a given symbol table to a binary file fp, and optionally to a text file txt_fp. */
int write_symtab(SymbolTable *symtab, FILE *fp, FILE *txt_fp);

/* Function read_symtab reads a symbol table from a given binary file. */
int read_symtab(SymbolTable *symtab, FILE *fp);

/* Function free_symtab deallocates the memory taken by the given symbol table. */
void free_symtab(SymbolTable *symtab);

/* Relocation Records Table */

enum { REL_TYPE_ABS16, REL_TYPE_PCREL16 };

typedef struct {
    uint16_t rel_num;
    uint16_t rel_type;
    uint16_t offset;
    uint16_t sym_num;
} RelocationRecord;

typedef struct reltab_node {
    RelocationRecord record;
    struct reltab_node *next;
} RelocationTableNode;

typedef struct reltab {
    uint16_t section_idx;
    uint32_t rel_cnt;
    RelocationTableNode *first;
    RelocationTableNode *last;
} RelocationTable;

/* Function new_relocation_record inserts a new relocation record in a given relocation table. */
int new_relocation_record(RelocationTable *reltab, uint16_t rel_type, uint16_t offset, uint16_t sym_num);

/* Function write_reltabs writes a given array of symbol tables to a binary file fp,
 * and optionally to a text file txt_fp. */
int write_reltabs(RelocationTable *reltab, uint32_t ntabs, SymbolTable *symtab, FILE *fp, FILE *txt_fp);

/* Function read_reltabs reads at most MAX_NUM_SECTIONS_IN_MODULE relocation tables from a given binary file. */
int read_reltabs(RelocationTable *reltab, FILE *fp);

/* Function free_reltabs deallocates the memory taken by the specified number of relocation tables. */
void free_reltabs(RelocationTable *reltab, int ntabs);

/* Section Header Table and Sections */

typedef struct {
    uint32_t num_sections;
    struct section_record {
        uint16_t idx;
        uint16_t load_addr;
        uint32_t size;
    } section[MAX_NUM_SECTIONS_IN_MODULE];
} SectionHeaderTable;

/* Function write_section_hdrtab writes a given section header table to a binary file fp, and optionally to a
 * text file txt_fp. */
int write_section_hdrtab(SectionHeaderTable *hdrtab, FILE *fp, FILE *txt_fp);

/* Function read_section_hdrtab reads a section header table from a given binary file. */
int read_section_hdrtab(SectionHeaderTable *hdrtab, FILE *fp);

/* Program Header Table */

typedef struct {
    uint16_t load_addr;
    uint16_t idx;
    uint32_t size;
} SegmentRecord;

typedef struct program_header_node {
    SegmentRecord record;
    struct program_header_node *next;
} ProgramHeaderNode;

typedef struct {
    ProgramHeaderNode *first;
    ProgramHeaderNode *last;
    uint32_t segment_cnt;
} ProgramHeaderTable;

/* Function new_segment inserts a new segment record in a given program header table. */
int new_segment(ProgramHeaderTable *hdrtab, uint16_t load_addr, uint16_t idx, uint32_t size);

/* Function write_program_hdrtab writes a given program header table to a binary file fp,
 * and optionally to a text file txt_fp. */
int write_program_hdrtab(ProgramHeaderTable *hdrtab, FILE *fp, FILE *txt_fp);

/* Function read_program_hdrtab reads a program header table from a given binary file. */
int read_program_hdrtab(ProgramHeaderTable *hdrtab, FILE *fp);

/* Sections */

/* Function write_section writes section content to a binary file fp, and optionally to a text file txt_fp. */
int write_section(const unsigned char *content, uint32_t size, FILE *fp, FILE *txt_fp, const char *section_name);

/* Function read_section reads section content from a given binary file into a given buffer. */
int read_section(unsigned char *buffer, uint32_t size, FILE *fp);

#endif /* OBJ_FORMAT_H */


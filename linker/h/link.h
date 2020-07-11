/* File: link.h */
/* Two-pass linker. */

#ifndef LINK_H
#define LINK_H

#define ORIGIN_ADDRESS 0x0100

/* Function link_files merges and patches given files into an executable file, or exits
 * in case of a linker error. */
int link_files(FILE **obj_fp, int nfiles, const char *out_filename, const char *out_txt_filename);

#endif /* LINK_H */


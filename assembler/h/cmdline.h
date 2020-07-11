/* File: cmdline.h */
/* Command line arguments parsing. */

#ifndef CMDLINE_H
#define CMDLINE_H

/* Function parse_cmdline parses command line arguments.
 * Calls exit or abort in case of error. */
void parse_cmdline(int argc, char *argv[]);

#endif /* CMDLINE_H */


/* File: termina.c */
/* Terminal settings. */

/* Note: non-standard headers, available on POSIX systems */
#include <unistd.h>
#include <termios.h>

#include "terminal.h"

static struct termios original_terminal_state;

void enable_raw_mode(void)
{
    struct termios raw;

    tcgetattr(STDIN_FILENO, &original_terminal_state);
    tcgetattr(STDIN_FILENO, &raw); /* get terminal attributes */

    /* reset various bits in "local flags" field: */
    /* ECHO - turn off echoing */
    /* ICANON - leave canonical mode - start reading byte-by-byte */
    /* IEXTEN - disable CTRL+V (and fix CTRL+O in macOS) */
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN);

    /* ISIG - turn off CTRL_Z and CTRL_C signals */
    /* raw.c_lflag &= ~(ISIG); */

    /* "input flags" field: */
    /* IXON - disable CTRL+S and CTRL+Q */
    /* ICRNL - stop translatin '\r' into '\n' (fixes CTRL+M) */
    raw.c_iflag &= ~(IXON | ICRNL);

    /* "output flags" field: */
    /* OPOST - stop translating "\n" into "\r\n" */
    raw.c_oflag &= ~(OPOST);

    /* miscellaneous flags - doesn't have any observable effect on modern terminal emulators */
    raw.c_iflag &= ~(BRKINT | INPCK | ISTRIP);
    raw.c_cflag |= (CS8);

    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 0;

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw); /* set new attributes */
}

void disable_raw_mode(void)
{
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_terminal_state);
}


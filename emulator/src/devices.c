/* File: devices.c */
/* Peripheral devices emulation. */

#include <ctype.h>
#include <stdio.h>
#include <time.h>

/* Note: non-standard headers, available on POSIX systems */
#include <unistd.h>

#include "cpu.h"
#include "mem.h"
#include "intr.h"
#include "exec.h"
#include "devices.h"

void output_device(char ch)
{
    if (ch > 0 && (ch == 0x0d || isprint(ch)))
    {
        if (ch == 0x0d)
            write(STDOUT_FILENO, "\r\n", 2);
        else
            write(STDOUT_FILENO, &ch, 1);
    }
}

void signal_output_device(void)
{
    if (memory_write && (uint16_t) mar == OUTPUT_DEVICE_ADDRESS)
    {
        char ch = mem[OUTPUT_DEVICE_ADDRESS];
        output_device(ch);
    }
}

void input_device(char ch)
{
    mem[INPUT_DEVICE_ADDRESS] = ch;
    intr = 1;
    ivtentry = INPUT_DEVICE_IVTENTRY;
}

void poll_input_device(void)
{
    char ch;
    int status = read(STDIN_FILENO, &ch, 1);
    if (status == 0)
        return;
    input_device(ch);
}

static clock_t t;

void init_timer(void)
{
    t = clock();
}

void poll_timer(void)
{
    double elapsed = (double)(clock() - t) / CLOCKS_PER_SEC;
    if (elapsed > TIMER_PERIOD_IN_SEC)
    {
        t = clock();
        if (PSW_TEST_FLAG(PSW_FLAG_T))
        {
            intr = 1;
            ivtentry = TIMER_TICK_IVTENTRY;
        }
    }
}


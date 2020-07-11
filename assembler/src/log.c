/* File: log.c */
/* Logging functionality. */

#include <stdarg.h>
#include <stdio.h>
#include <time.h>

#include "log.h"

static FILE *log_fp = NULL;

static const char default_log_filename[] = "logfile.log";

static LogLevel log_level = LOG_NOTHING;

static const char *prefix[] = { "debug: ", "  log: ", "error: " };

/* get_time_string returns current local time in format hh:mm:ss. */
char *get_time_string(void)
{
    static char time_str[12] = { 0 };
    struct tm *now = NULL;
    time_t time_value = time(NULL);

    now = localtime(&time_value);
    sprintf(time_str, "%02d:%02d:%02d", now->tm_hour, now->tm_min, now->tm_sec);
    return time_str;
}

void set_log_level(LogLevel level)
{
    log_level = level;
}

void open_log(const char *log_filename)
{
    close_log();

    if (log_filename != NULL) log_fp = fopen(log_filename, "a");
    else log_fp = fopen(default_log_filename, "a");

    if (!log_fp)
    {
        fprintf(stderr, "error: [%s]: failed to open log file '%s'\n", get_time_string(), log_filename);
        return;
    }

    write_log(LOG_NORMAL, ">>> log file created <<<");
}

void close_log(void)
{
    if (log_fp) fclose(log_fp);
    log_fp = NULL;
}

int write_log(LogLevel level, const char *format, ...)
{
    if (level < log_level) return 0;

    if (!log_fp) open_log(NULL);
    if (!log_fp) return 1;

    va_list ap;
    va_start(ap, format);
    fprintf(log_fp, "[%s]: %s", get_time_string(), prefix[level]);
    vfprintf(log_fp, format, ap);
    fputc('\n', log_fp);
    va_end(ap);
    return 0;
}


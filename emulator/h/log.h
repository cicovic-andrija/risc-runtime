/* File: log.h */
/* Logging functionality. */

#ifndef LOG_H
#define LOG_H

/* log levels determine the severity of log entries */
typedef enum { LOG_DEBUG = 0, LOG_NORMAL = 1, LOG_ERROR = 2, LOG_NOTHING = 3 } LogLevel;

/* Function set_log_level sets new log level. */
void set_log_level(LogLevel level);

/* Function open_log opens new log file. */
void open_log(const char *log_filename);

/* Function close_log closes current log file. */
void close_log(void);

/* Function write_log prints message to the log file only if the level of the
 * message is the same or greater than the log level. Return value
 * is 0 in case of success, 1 in case of error. */
int write_log(LogLevel level, const char *format, ...);

#endif /* LOG_H */


#ifndef MYSYSLOG_H
#define MYSYSLOG_H

#define LOG_ERROR   1
#define LOG_WARNING 2
#define LOG_INFO    3
#define LOG_DEBUG   4

int mysyslog(const char *msg, int level);
int init_logger(const char *log_file);
void close_logger(void);

#endif

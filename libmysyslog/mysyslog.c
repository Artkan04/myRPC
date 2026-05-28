#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <syslog.h>
#include "mysyslog.h"

static FILE *log_file = NULL;
static int use_syslog = 1;

int init_logger(const char *log_path)
{
    if (log_path == NULL)
    {
        use_syslog = 1;
        openlog("myRPC", LOG_PID | LOG_CONS, LOG_USER);
        return 0;
    }

    log_file = fopen(log_path, "a");
    if (log_file == NULL)
    {
        perror("Failed to open log file");
        return -1;
    }

    use_syslog = 0;
    return 0;
}

void close_logger(void)
{
    if (use_syslog)
    {
        closelog();
    }
    else if (log_file != NULL)
    {
        fclose(log_file);
        log_file = NULL;
    }
}

int mysyslog(const char *msg, int level)
{
    if (use_syslog)
    {
        int syslog_level;
        switch (level)
        {
        case LOG_ERROR:
            syslog_level = LOG_ERR;
            break;
        case LOG_WARNING:
            syslog_level = LOG_WARNING;
            break;
        case LOG_INFO:
            syslog_level = LOG_INFO;
            break;
        case LOG_DEBUG:
            syslog_level = LOG_DEBUG;
            break;
        default:
            syslog_level = LOG_INFO;
        }
        syslog(syslog_level, "%s", msg);
        return 0;
    }

    if (log_file == NULL)
        return -1;

    time_t now = time(NULL);
    char *time_str = ctime(&now);
    time_str[strlen(time_str) - 1] = '\0';

    const char *level_str;
    switch (level)
    {
    case LOG_ERROR:
        level_str = "ERROR";
        break;
    case LOG_WARNING:
        level_str = "WARNING";
        break;
    case LOG_INFO:
        level_str = "INFO";
        break;
    case LOG_DEBUG:
        level_str = "DEBUG";
        break;
    default:
        level_str = "UNKNOWN";
    }

    fprintf(log_file, "[%s] [%s] %s\n", time_str, level_str, msg);
    fflush(log_file);
    return 0;
}
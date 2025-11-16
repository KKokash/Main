//
// Created by karam on 11/15/25.
//
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>

#include "logger.h"

#define LOGFILE "gateway.log"

int pipe_fd[2];
pid_t logger_pid = -1;

static void get_timestamp(char *buf, size_t size)
{
    time_t now = time(NULL);
    char *ct = ctime(&now);

    strncpy(buf, ct, size);
    buf[size-1] = '\0';

    size_t len = strlen(buf);
    if (len > 0 && buf[len-1] == '\n')
        buf[len-1] = '\0';
}

int create_log_process()
{
    if (pipe(pipe_fd) == -1) {
        perror("pipe failed");
        exit(EXIT_FAILURE);
    }

    logger_pid = fork();
    if (logger_pid == -1) {
        perror("fork failed");
        exit(EXIT_FAILURE);
    }

    if (logger_pid == 0)
    {
        // CHILD LOGGER
        close(pipe_fd[1]);

        FILE *logf = fopen(LOGFILE, "a");
        if (!logf) exit(EXIT_FAILURE);

        int seq =-1;
        char buffer[256];
        char line[256];
        int line_pos = 0;

        while (1) {
            int n = read(pipe_fd[0], buffer, sizeof(buffer));
            if (n == 0) break;
            if (n < 0) continue;

            for (int i = 0; i < n; i++) {
                char c = buffer[i];

                if (c == '\n') {
                    line[line_pos] = '\0';

                    // shutdown command
                    if (strcmp(line, "END_LOG") == 0) {
                        char ts[64];
                        get_timestamp(ts, sizeof(ts));
                        seq++;
                        fprintf(logf, "%d - %s - Data file closed.\n", seq, ts);
                        fflush(logf);
                        exit(0);
                    }

                    // normal log entry
                    char ts[64];
                    get_timestamp(ts, sizeof(ts));
                    seq++;
                    fprintf(logf, "%d - %s - %s\n", seq, ts, line);
                    fflush(logf);

                    line_pos = 0; // reset for next message
                }
                else {
                    line[line_pos++] = c;
                }
            }
        }
        fclose(logf);
        exit(0);
    }

    // PARENT
    close(pipe_fd[0]);
    return 0;
}

int write_to_log_process(char *msg)
{
    if (logger_pid <= 0)
        return 0;

    write(pipe_fd[1], msg, strlen(msg));
    write(pipe_fd[1], "\n", 1);
    return 1;
}

int end_log_process()
{
    if (logger_pid <= 0)
        return 0;

    write(pipe_fd[1], "END_LOG", 7);
    close(pipe_fd[1]);

    waitpid(logger_pid, NULL, 0);

    return 1;
}
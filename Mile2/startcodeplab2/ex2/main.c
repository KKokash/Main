//
// Created by karam on 11/15/25.
//
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>     // for pipe(), fork(), read(), write(), close()
#include <string.h>     // for strlen()
#include <ctype.h>      // for toupper(), tolower()
#include <sys/wait.h>   // for wait()

#define MAX_LEN 100

int main() {
    int fd[2];
    pid_t pid;
    char message[MAX_LEN] = "Hi There";   // parent message
    char buffer[MAX_LEN];

    // Create the pipe
    if (pipe(fd) == -1) {
        perror("pipe failed");
        exit(EXIT_FAILURE);
    }

    // Create a child process
    pid = fork();

    if (pid < 0) {
        perror("fork failed");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        //  Child process

        // Close write end (we’re only reading)
        close(fd[1]);

        // Read message from pipe
        ssize_t n = read(fd[0], buffer, sizeof(buffer) - 1);
        if (n < 0) {
            perror("read failed");
            exit(EXIT_FAILURE);
        }

        buffer[n] = '\0'; // null-terminate the string

        // Reverse the case
        for (int i = 0; i < strlen(buffer); i++) {
            if (islower(buffer[i]))
                buffer[i] = toupper(buffer[i]);
            else if (isupper(buffer[i]))
                buffer[i] = tolower(buffer[i]);
        }

        // Print result
        printf("Child received and modified: %s\n", buffer);

        // Close read end
        close(fd[0]);

        exit(EXIT_SUCCESS);
    } else {
        //  Parent process

        // Close read end (we’re only writing)
        close(fd[0]);

        // Write message to pipe
        write(fd[1], message, strlen(message));

        // Close write end
        close(fd[1]);

        // Wait for child to finish
        wait(NULL);
        printf("Parent done.\n");
    }

    return 0;
}
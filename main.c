#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>

int checkError(int val, const char *msg) {
    if (val == -1) {
        perror(msg);
        exit(EXIT_FAILURE);
    }
    return val;
}

void child(int pipefd[2]) {

    close(pipefd[0]);

    dup2(pipefd[1], STDOUT_FILENO);
    close(pipefd[1]);

    execlp("./rand", "rand", "-c", "50", "-m", "200", "-M", "500", (char *)NULL);

    perror("execlp failed");
    exit(EXIT_FAILURE);
}

void parent(int pipefd[2]) {

    close(pipefd[1]);

    int value, sum = 0, count = 0;

    while (1) {
        ssize_t bytes_read = checkError(read(pipefd[0], &value, sizeof(int)), "Reading from pipe");
        if (bytes_read == 0) break;

        if (bytes_read != sizeof(int)) continue;

        printf("Read value: %d\n", value);
        sum += value;
        count++;
    }

    close(pipefd[0]);

    if (count > 0) {
        printf("Total: %d\n", sum);
        printf("Average: %.2f\n", (float)sum / count);
    } else {
        printf("No values read.\n");
    }
}

int main() {
    pid_t pid;

    int pipefd[2];

    if (pipe(pipefd) == -1) {
        perror("pipe failed");
        exit(EXIT_FAILURE);
    }

    pid = checkError(fork(), "Forking process");
    switch (pid)
    {
    case 0:
        child(pipefd);
        break;
    
    default:
        parent(pipefd);
        wait(NULL);
        break;
    }

    exit(EXIT_SUCCESS);
}


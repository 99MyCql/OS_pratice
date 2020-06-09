#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>


int main() {
    int pid1 , pid2;
    int fd[2];
    assert(pipe(fd) == 0);
    printf("fd: %d, %d\n", fd[0], fd[1]);

    if ((pid1 = fork()) == 0) {
        dup2(fd[1], 1);
        close(fd[0]);
        close(fd[1]);

        write(1, "hello", 6);   // 给定写的字符串大小，写完则会自动关闭写端
        exit(0);
    }
    waitpid(pid1, NULL, 0);

    if ((pid2 = fork()) == 0) {
        dup2(fd[0], 0);
        close(fd[0]);

        char buf[32];
        read(0, buf, sizeof(buf));
        printf("Receive:%s\n", buf);
        exit(0);
    }
    waitpid(pid2, NULL, 0);

    close(fd[0]);
    close(fd[1]);
    return 0;
}


int main() {
    int pid1 , pid2;
    int fd[2];
    char *argv1[] = {"cat", "test", NULL};
    char *argv2[] = {"wc", "-l", NULL};

    assert(pipe(fd) == 0);
    printf("fd: %d, %d\n", fd[0], fd[1]);

    if ((pid1 = fork()) == 0) {
        dup2(fd[1], 1);
        close(fd[0]);
        close(fd[1]);

        if (execvp(argv1[0], argv1) == -1) {
            perror("command error");
            return 0;
        }
    }
    waitpid(pid1, NULL, 0);
    close(fd[1]);   // 很重要！！！关闭写端，读端才能知道写入数据已经结束了！！！

    if ((pid2 = fork()) == 0) {
        dup2(fd[0], 0);
        close(fd[0]);

        if (execvp(argv2[0], argv2) == -1) {
            perror("command error");
            return 0;
        }
    }
    waitpid(pid2, NULL, 0);
    close(fd[0]);

    return 0;
}

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>


static int MAX_ARGS = 100;  // 命令最大参数
static int MAX_LINE = 1024; // 命令最大长度


// 从 buf 字符串中解析出各个参数，空格分隔。返回参数个数
int parseline(char *buf, char **argv) {
    int len = strlen(buf);      // 求 buf 长度
    assert(len <= MAX_LINE);    // 不能大于最大长度

    int argc = 0;   // 参数个数
    int pre = 0;
    int next = 0;
    while (1) {
        // 找到第一个非空格的字符，即当前参数的第一个字符
        while (pre < len && buf[pre] == ' ')
            pre++;

        // 找到当前参数后的第一个空格字符
        next = pre;
        while (next < len && buf[next] != ' ')
            next++;

        // next == pre 则说明到达 buf 末尾，退出循环
        if (next == pre) break;

        // 将参数拷贝到 argv 中
        assert(argc < MAX_ARGS);
        argv[argc] = (char*)malloc((next-pre) * sizeof(char));
        int i;
        for (i = 0; i < next-pre; i++) {
            argv[argc][i] = buf[pre+i];
        }

        // 寻找下一个参数
        argc++;
        pre = next;
    }

    return argc;
}


void mysys(char *command) {

    // 拷贝命令行字符串到 buf 缓冲区中
    char *buf = (char*)malloc(strlen(command) * sizeof(char));
    strcpy(buf, command);

    // 调用 parseline() 函数解析参数
    char **argv = (char**)malloc(MAX_ARGS * sizeof(char*));
    int argc = parseline(buf, argv);
    assert(argc > 0);
    argv[argc] = NULL;

    // 打印参数
    // printf("argc: %d\n", argc);
    // int i;
    // for (i = 0; i < argc; i++) {
    //     printf("%s\n", argv[i]);
    // }

    // 子进程运行命令
    pid_t pid;
    if ((pid = fork()) == 0) {
        assert(execvp(argv[0], argv) != -1);
    } else {
        waitpid(pid, NULL, 0);
    }
}


int main() {
    printf("--------------------------------------------------\n");
    mysys("echo HELLO WORLD");
    printf("--------------------------------------------------\n");
    mysys("ls /");
    printf("--------------------------------------------------\n");
    return 0;
}

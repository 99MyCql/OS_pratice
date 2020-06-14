#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>


static int g_debug = 0;     // 是否开启调试
static int MAX_ARGS = 100;  // 命令最大参数
static int MAX_LINE = 512;  // 命令最大长度


/**
 * 从 buf 字符串中，根据空格解析出各个参数。
 * params：
 *  - buf: 待解析的字符串
 *  - len: buf 字符串长度
 *  - argv: 从 buf 中解析出的所有参数，类型为字符串数组
 *  - max_argc: 允许的最大参数个数
 * return：
 *  - 参数个数
 */
int parseline(char *buf, int len, char **argv, int max_argc) {
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
        argc++;
        assert(argc <= max_argc);
        argv[argc-1] = (char*)malloc((next-pre) * sizeof(char));
        int i;
        for (i = 0; i < next-pre; i++) {
            argv[argc-1][i] = buf[pre+i];
        }

        // 寻找下一个参数
        pre = next;
    }

    return argc;
}


// print argv
void print_argv(char **argv, int argc) {
    printf("argc: %d\n", argc);
    int i;
    for (i = 0; i < argc; i++) {
        printf("%s ", argv[i]);
    }
    printf("\n");
}


// my system()
void mysys(char *command) {

    // 拷贝命令行字符串到 buf 缓冲区中
    int len = strlen(command);
    char *buf = (char*)malloc(len * sizeof(char));
    strcpy(buf, command);

    // 调用 parseline() 函数解析参数
    char **argv = (char**)malloc((MAX_ARGS+1) * sizeof(char*));
    int argc = parseline(buf, len, argv, MAX_ARGS); // argc 参数个数
    argv[argc] = NULL;  // 在参数最后加上 NULL

    // 打印参数
    if (g_debug) print_argv(argv, argc);

    // 处理特殊命令
    if (strcmp(argv[0], "exit") == 0) {
        exit(0);
    }
    else if (strcmp(argv[0], "cd") == 0) {
        assert(chdir(argv[1]) != -1);
        return;
    }

    // 子进程运行命令
    pid_t pid;
    if ((pid = fork()) == 0) {
        if (execvp(argv[0], argv) == -1) {
            printf("[ERROR]: command error!\n");
        }
    } else {
        waitpid(pid, NULL, 0);
    }
}


int main() {
    while (1) {
        printf("$ ");

        char line[MAX_LINE];
        fgets(line, MAX_LINE, stdin);   // fgets 获取用户输入命令
        line[strlen(line)-1] = '\0';    // fgets 会读取换行，去掉末尾换行
        mysys(line);                    // 调用 mysys() 函数执行命令
    }
    return 0;
}

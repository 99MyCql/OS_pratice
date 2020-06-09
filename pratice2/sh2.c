#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>


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
    int i;
    int pre = 0;
    int next = 0;
    int argc = 0;   // 参数个数
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
        argv[argc] = (char*)malloc((next-pre+1) * sizeof(char));
        for (i = 0; i < next-pre; i++) {
            argv[argc][i] = buf[pre+i];
        }
        argv[argc][i] = '\0';       // 参数末尾加 \0
        argc++;                     // 参数个数++
        assert(argc <= max_argc);   // 不能超过最大参数个数

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


// 在参数数组中检查重定位
int check_redir(char **argv, int argc, char **input, char **output) {
    int i = 0;
    int new_argc = argc;
    for (i = 0; i < argc; i++) {
        // 检查是否存在 > 重定位输出符
        if (argv[i][0] == '>') {
            // 如果是一个单独的 > ，则直接拷贝下一个参数到 output 中
            if (strlen(argv[i]) == 1) {
                i++;
                assert(i < argc);
                *output = (char*)malloc(strlen(argv[i]) * sizeof(char));
                strcpy(*output, argv[i]);
                new_argc -= 2;
            }
            // 如果是跟在 > 后面还有字符
            else {
                *output = (char*)malloc((strlen(argv[i]) - 1) * sizeof(char));
                strcpy(*output, argv[i] + 1);
                new_argc -= 1;
            }
        }
    }
    return new_argc;
}


// my system()
void mysys(char *command) {
    int i = 0;

    // 拷贝命令行字符串到 buf 缓冲区中
    char *buf = (char*)malloc(strlen(command) * sizeof(char));
    strcpy(buf, command);
    if (g_debug) printf("%s\n", buf);
 
    // 调用 parseline() 函数解析参数
    char **argv = (char**)malloc((MAX_ARGS+1) * sizeof(char*));
    int argc = parseline(buf, strlen(buf), argv, MAX_ARGS); // argc 参数个数

    // 打印参数
    if (g_debug) print_argv(argv, argc);
    
    // 检查是否有重定向
    char *input = NULL;
    char *output = NULL;
    int new_argc = check_redir(argv, argc, &input, &output);
    for (i = new_argc; i < argc; i++) free(argv[i]);    // 释放空间
    argv[new_argc] = NULL;  // 在参数最后加上 NULL

    // 打印参数
    if (g_debug) print_argv(argv, new_argc);

    // 处理特殊命令
    if (strcmp(argv[0], "exit") == 0) {
        exit(0);
    }
    else if (strcmp(argv[0], "cd") == 0) {
        if (chdir(argv[1]) == -1) {
            perror("cd");
        }
        return;
    }

    // 子进程运行命令
    pid_t pid;
    if ((pid = fork()) == 0) {
        int input_fd = 0;   // 标准输入
		int output_fd = 1;  // 标准输出

        // 重定位标准输出
		if (output != NULL) {
        	output_fd = open(output, O_CREAT|O_RDWR, 0666);
        	dup2(output_fd, 1);
        	close(output_fd);
		}

        if (execvp(argv[0], argv) == -1) {
            perror("command error");
            return ;
        }
    } else {
        waitpid(pid, NULL, 0);
    }

    // 释放空间
    for (i = 0; i < new_argc; i++) free(argv[i]);
    free(argv);
    free(buf);
    if (input != NULL) free(input);
    if (output != NULL) free(output);
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

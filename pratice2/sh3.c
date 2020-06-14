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


// 从 buf 字符串中，根据空格解析出各个参数，并返回参数个数
int parseline(char *buf, int len, char **argv) {
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
        assert(argc <= MAX_ARGS);   // 不能超过最大参数个数

        // 寻找下一个参数
        pre = next;
    }

    return argc;
}


// 打印 argv 数组
void print_argv(char **argv, int argc) {
    printf("argc: %d\n", argc);
    int i;
    for (i = 0; i < argc; i++) {
        printf("%s ", argv[i]);
    }
    printf("\n");
}


// 在参数数组中检查重定位符 '<' 或 '>'，如果存在，则打开对应的重定位文件
// 并返回去除重定位符和文件字符串之后的参数长度。
int check_redir(char **argv, int argc, int *input_fd, int *output_fd) {
    int i = 0;
    int new_argc = argc;
    for (i = 0; i < argc; i++) {
        assert(argv[i] != NULL);

        // 检查是否存在 > 或 < 重定位符
        if (argv[i][0] == '>' || argv[i][0] == '<') {
            int fd;

            // 如果是一个单独的重定位符，则下一个参数为重定位文件名，如：> out
            if (strlen(argv[i]) == 1) {
                i++;
                assert(i < argc);
                fd = open(argv[i], O_CREAT|O_RDWR, 0666);
                new_argc -= 2;
            }
            // 否则，是紧跟在重定位符后面的字符串为重定位文件名，如：>out
            else {
                fd = open(argv[i]+1, O_CREAT|O_RDWR, 0666);
                new_argc -= 1;
            }

            if (argv[i][0] == '>' || argv[i-1][0] == '>') *output_fd = fd;
            else *input_fd = fd;
        }
    }
    return new_argc;
}


// 检查数组命令中是否存在管道符 '|'，如果存在，则根据第一个管道符，划分参数数组
// 并返回是否存在管道符的标志
int check_pipe(char **argv, int argc, int *first_argc, int *next_argc) {
    int i = 0;
    int has_pipe = 0;
    *first_argc = argc;
    *next_argc = 0;
    for (i = 0; i < argc; i++) {
        if (argv[i][0] == '|') {
            has_pipe = 1;
            *first_argc = i;
            if (strlen(argv[i]) == 1) {
                *next_argc = argc - i - 1;
                assert(*next_argc > 0);
            } else {
                *next_argc = argc - i;
                strcpy(argv[i], argv[i]+1);
            }
            break;
        }
    }
    return has_pipe;
}


// 运行子程序，返回子进程进程号
int run_subproc(char **argv, int argc, int input_fd, int output_fd) {
    // 处理特殊程序
    if (strcmp(argv[0], "exit") == 0) {
        exit(0);
    }
    else if (strcmp(argv[0], "cd") == 0) {
        if (chdir(argv[1]) == -1)
            perror("cd");
        return -1;
    }

    // 拷贝参数数组
    char **argv_temp = (char**)malloc((argc+1) * sizeof(char*));
    int i = 0;
    for (i = 0; i < argc; i++) {
        argv_temp[i] = (char*)malloc(strlen(argv[i]) * sizeof(char));
        strcpy(argv_temp[i], argv[i]);
    }
    argv_temp[i] = NULL;   // 参数数组后一个为 NULL

    pid_t pid;
    // fork 创建子进程
    if ((pid = fork()) == 0) {
        // 重定位标准输入
        if (input_fd != 0) {
            dup2(input_fd, 0);
            close(input_fd);
        }
        // 重定位标准输出
        if (output_fd != 1) {
            dup2(output_fd, 1);
            close(output_fd);
        }
        // exec 加载程序
        if (execvp(argv_temp[0], argv_temp) == -1) {
            perror("command error");
            return -1;
        }
    }

    // 释放空间
    for (i = 0; i < argc; i++)
        free(argv_temp[i]);
    free(argv_temp);

    return pid;
}


// 运行参数数组化的命令（可能是多个程序命令的结合体：cat /etc/passwd | wc -l）
// 对于存在管道的命令，先运行第一个程序，再递归处理之后的参数
void run_command(char **argv, int argc, int pre_input_fd) {
    if (g_debug) print_argv(argv, argc);    // 打印参数

    // 检查是否存在管道
    int first_argc = argc;  // 第一个程序的参数个数
    int next_argc = 0;      // 之后命令的参数个数
    int has_pipe = check_pipe(argv, argc, &first_argc, &next_argc);

    // 检查第一个程序命令中，是否存在重定向
    int input_fd = pre_input_fd;   // 如果存在重定向，将会覆盖管道输入
    int output_fd = 1;
    first_argc = check_redir(argv, first_argc, &input_fd, &output_fd);

    if (g_debug) print_argv(argv, first_argc);  // 打印参数

    // 如果存在管道符 |
    pid_t pid;
    if (has_pipe) {
        int fd[2];
        pipe(fd);               // 创建管道
        assert(output_fd == 1); // 不能既存在重定向输出，又存在管道输出

        // 先运行第一个程序命令
        pid = run_subproc(argv, first_argc, input_fd, fd[1]);
        close(fd[1]);   // 很重要！！！关闭写端，读端才能知道写入数据已经结束了！！！

        // 再递归运行之后的命令
        run_command(argv+(argc-next_argc), next_argc, fd[0]);
        close(fd[0]);
    }
    // 如果不存在管道，则直接运行第一个程序命令
    else
        pid = run_subproc(argv, first_argc, input_fd, output_fd);

    waitpid(pid, NULL, 0);

    // 关闭打开的文件描述符
    if (input_fd != 0) close(input_fd);
    if (output_fd != 1) close(output_fd);
}


// my system()
void mysys(char *command) {
    int i = 0;

    // 拷贝命令行字符串到 buf 缓冲区中
    char *buf = (char*)malloc(strlen(command) * sizeof(char));
    strcpy(buf, command);

    // 调用 parseline() 函数解析参数
    char **argv = (char**)malloc((MAX_ARGS+1) * sizeof(char*));
    int argc = parseline(buf, strlen(buf), argv); // argc 参数个数

    // 运行命令
    run_command(argv, argc, 0);

    // 释放空间
    for (i = 0; i < argc; i++) {
        if (argv[i] != NULL) free(argv[i]);
    }
    free(argv);
    free(buf);
}


int main() {
    while (1) {
        char line[MAX_LINE];
        printf("$ ");                   // 输出提示符
        fgets(line, MAX_LINE, stdin);   // fgets 获取用户输入命令
        line[strlen(line)-1] = '\0';    // fgets 会读取换行，去掉末尾换行
        mysys(line);                    // 调用 mysys() 函数执行命令
    }
    return 0;
}

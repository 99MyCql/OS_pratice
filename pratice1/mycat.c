#include <stdio.h>      // printf
#include <sys/stat.h>   // open
#include <fcntl.h>      // open
#include <unistd.h>     // read
#include <assert.h>     // assert
#include <string.h>     // memset


int main(int argc, char *argv[]) {
    assert(argc == 2);                  // 必须携带一个参数

    int fd = open(argv[1], O_RDONLY);   // 打开文件
    assert(fd != -1);                   // 打开失败则报错

    char buf[257];                      // 257 包括最后一个字符 \0
    memset(buf, 0, sizeof(char)*257);   // 初始化 buf 数组

    int read_ret = read(fd, buf, 256);  // 读取文件，读 256 字节是为了能在字符串最后添 \0
    assert(read_ret != -1);             // 读取失败则报错
    printf("%s", buf);                  // 输出读取内容
    memset(buf, 0, sizeof(char)*256);   // 清空 buf 数组

    while (read_ret != 0) {             // 连续读文件，直至读到文件末尾
        read_ret = read(fd, buf, 256);
        printf("%s", buf);
        memset(buf, 0, sizeof(char)*256);
    }
    return 0;
}

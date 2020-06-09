#include <stdio.h>      // printf
#include <sys/stat.h>   // open
#include <fcntl.h>      // open
#include <unistd.h>     // read
#include <assert.h>     // assert
#include <string.h>     // memset

int main(int argc, char *argv[]) {
    // 必须两个参数
    assert(argc == 3);

    // 打开源文件
    int source_fd = open(argv[1], O_RDONLY);
    assert(source_fd != -1);

    // 打开目的文件：O_CREAT | O_WRONLY 表示没有则创建、只写打开；0666表示文件的权限
    int des_fd = open(argv[2], O_CREAT | O_WRONLY, 0666);
    assert(des_fd != -1);

    // 初始化缓冲区
    char buf[256];
    memset(buf, 0, sizeof(char)*256);

    // 从源文件读到缓冲区
    int read_ret = read(source_fd, buf, 256);
    assert(read_ret != -1);

    // 将缓冲区内容写到目的文件，并清空缓冲区
    int write_ret = write(des_fd, buf, 256);
    assert(write_ret != -1);
    memset(buf, 0, sizeof(char)*256);

    // 连续读源文件，直至文件末尾
    while (read_ret != 0) {
        read_ret = read(source_fd, buf, 256);
        assert(read_ret != -1);
        write_ret = write(des_fd, buf, 256);
        assert(write_ret != -1);
        memset(buf, 0, sizeof(char)*256);
    }    

    return 0;
}

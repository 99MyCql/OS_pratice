#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <assert.h>


#define BUF_CAPACITY 4
#define ITEM_NUM 8


// 缓冲区结构体
typedef struct {
    char buffer[BUF_CAPACITY];  // 缓冲区数组
    int count;                  // 缓冲区大小
    int in_p;                   // 写指针
    int out_p;                  // 读指针
    pthread_mutex_t mutex;      // 互斥量
    pthread_cond_t empty_buf;   // 缓冲区出现空闲的条件变量
    pthread_cond_t full_buf;    // 缓冲区出现数据的条件变量
} Buffer;

int buffer_is_empty(Buffer *buf) {
    return buf->count == 0;
}

int buffer_is_full(Buffer *buf) {
    return buf->count == BUF_CAPACITY;
}

char get_item(Buffer *buf) {
    char item;
    item = buf->buffer[buf->out_p];
    buf->out_p = (buf->out_p + 1) % BUF_CAPACITY;
    buf->count--;
    assert(buf->count >= 0);
    return item;
}

void put_item(Buffer *buf, char item) {
    buf->buffer[buf->in_p] = item;
    buf->in_p = (buf->in_p + 1) % BUF_CAPACITY;
    buf->count++;
    assert(buf->count <= BUF_CAPACITY);
}


Buffer buf1;
Buffer buf2;


void *produce(void *arg) {
    printf("produce start...\n");
    int i;
    for (i = 0; i < ITEM_NUM; i++) {
        pthread_mutex_lock(&buf1.mutex);        // 加锁
        while (buffer_is_full(&buf1))           // 防止多个生产者同时被唤醒
            // 等待 buf 中出现剩余空间，并释放锁。
            // 与信号量先检查资源数再加锁有所不同，因为条件变量会释放锁
            pthread_cond_wait(&buf1.empty_buf, &buf1.mutex);
        put_item(&buf1, 'a'+i);
        printf("produce item: %c\n", 'a'+i);
        pthread_cond_signal(&buf1.full_buf);    // 通知消费者 buf 中有数据
        pthread_mutex_unlock(&buf1.mutex);      // 解锁
    }
}


void *compute(void *arg) {
    printf("compute start...\n");
    int i;
    for (i = 0; i < ITEM_NUM; i++) {
        char item;
        pthread_mutex_lock(&buf1.mutex);        // 加锁
        while (buffer_is_empty(&buf1))          // 防止多个消费者同时被唤醒
            pthread_cond_wait(&buf1.full_buf, &buf1.mutex);
        item = get_item(&buf1);
        pthread_cond_signal(&buf1.empty_buf);   // 通知生产者 buf 中有空间
        pthread_mutex_unlock(&buf1.mutex);      // 解锁

        item = item + ('A' - 'a');
        printf("compute item: %c\n", item);

        pthread_mutex_lock(&buf2.mutex);
        while (buffer_is_full(&buf2))
            pthread_cond_wait(&buf2.empty_buf, &buf2.mutex);
        put_item(&buf2, item);
        pthread_cond_signal(&buf2.full_buf);
        pthread_mutex_unlock(&buf2.mutex);
    }
}


void *consume(void *arg) {
    printf("consume start...\n");
    int i;
    for (i = 0; i < ITEM_NUM; i++) {
        pthread_mutex_lock(&buf2.mutex);
        while (buffer_is_empty(&buf2))
            pthread_cond_wait(&buf2.full_buf, &buf2.mutex);
        printf("consume item: %c\n", get_item(&buf2));
        pthread_cond_signal(&buf2.empty_buf);
        pthread_mutex_unlock(&buf2.mutex);
    }
}


// 生产者消费者问题
int main() {
    pthread_t produce_tid, compute_tid, consume_tid;

    buf1.count = 0;
    buf1.in_p = 0;
    buf1.out_p = 0;
    buf2.count = 0;
    buf2.in_p = 0;
    buf2.out_p = 0;

    // 初始化互斥量和条件变量
    pthread_mutex_init(&buf1.mutex, NULL);
    pthread_cond_init(&buf1.empty_buf, NULL);
    pthread_cond_init(&buf1.full_buf, NULL);
    pthread_mutex_init(&buf2.mutex, NULL);
    pthread_cond_init(&buf2.empty_buf, NULL);
    pthread_cond_init(&buf2.full_buf, NULL);

    // 创建线程
    pthread_create(&produce_tid, NULL, produce, NULL);
    pthread_create(&compute_tid, NULL, compute, NULL);
    pthread_create(&consume_tid, NULL, consume, NULL);

    // 等待线程
    pthread_join(produce_tid, NULL);
    pthread_join(compute_tid, NULL);
    pthread_join(consume_tid, NULL);

    return 0;
}

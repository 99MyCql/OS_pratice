#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <assert.h>


#define BUF_CAPACITY 4
#define ITEM_NUM 8


// 通过条件变量和互斥量构造信号量
typedef struct {
    int value;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} sema_t;

void sema_init(sema_t *sema, int value)
{
    sema->value = value;
    pthread_mutex_init(&sema->mutex, NULL);
    pthread_cond_init(&sema->cond, NULL);
}

void sema_wait(sema_t *sema)
{
    pthread_mutex_lock(&sema->mutex);
    while (sema->value <= 0)
        pthread_cond_wait(&sema->cond, &sema->mutex);
    sema->value--;
    pthread_mutex_unlock(&sema->mutex);
}

void sema_signal(sema_t *sema)
{
    pthread_mutex_lock(&sema->mutex);
    ++sema->value;
    pthread_cond_signal(&sema->cond);
    pthread_mutex_unlock(&sema->mutex);
}


typedef struct {
    char buffer[BUF_CAPACITY];
    int in_p;
    int out_p;
    sema_t mutex;
    sema_t empty_buf;
    sema_t full_buf;
} Buffer;

char get_item(Buffer *buf) {
    char item;
    item = buf->buffer[buf->out_p];
    buf->out_p = (buf->out_p + 1) % BUF_CAPACITY;
    return item;
}

void put_item(Buffer *buf, char item) {
    buf->buffer[buf->in_p] = item;
    buf->in_p = (buf->in_p + 1) % BUF_CAPACITY;
}


Buffer buf1;
Buffer buf2;


void *produce(void *arg) {
    printf("produce start...\n");
    int i;
    for (i = 0; i < ITEM_NUM; i++) {
        sema_wait(&buf1.empty_buf);     // 先检查 buf 中有没有剩余空间
        sema_wait(&buf1.mutex);         // 再加锁进行操作，两者顺序不能调换
        put_item(&buf1, 'a'+i);
        sema_signal(&buf1.mutex);       // 释放锁
        printf("produce item: %c\n", 'a'+i);
        sema_signal(&buf1.full_buf);    // 提醒消费者，buf 中有数据了
    }
}


void *compute(void *arg) {
    printf("compute start...\n");
    int i;
    for (i = 0; i < ITEM_NUM; i++) {
        char item;
        sema_wait(&buf1.full_buf);
        sema_wait(&buf1.mutex);
        item = get_item(&buf1);
        sema_signal(&buf1.mutex);
        sema_signal(&buf1.empty_buf);


        item = item + ('A' - 'a');
        printf("compute item: %c\n", item);

        sema_wait(&buf2.empty_buf);
        sema_wait(&buf2.mutex);
        put_item(&buf2, item);
        sema_signal(&buf2.mutex);
        sema_signal(&buf2.full_buf);
    }
}


void *consume(void *arg) {
    printf("consume start...\n");
    int i;
    for (i = 0; i < ITEM_NUM; i++) {
        sema_wait(&buf2.full_buf);
        sema_wait(&buf2.mutex);
        printf("consume item: %c\n", get_item(&buf2));
        sema_signal(&buf2.mutex);
        sema_signal(&buf2.empty_buf);
    }
}


// 生产者消费者问题
int main() {
    pthread_t produce_tid, compute_tid, consume_tid;

    buf1.in_p = 0;
    buf1.out_p = 0;
    buf2.in_p = 0;
    buf2.out_p = 0;

    // 初始化信号量
    sema_init(&buf1.mutex, 1);
    sema_init(&buf1.empty_buf, BUF_CAPACITY);
    sema_init(&buf1.full_buf, 0);
    sema_init(&buf2.mutex, 1);
    sema_init(&buf2.empty_buf, BUF_CAPACITY);
    sema_init(&buf2.full_buf, 0);

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

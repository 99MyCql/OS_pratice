#include <stdio.h>
#include <pthread.h>
#include <math.h>


#define PI acos(-1)         // PI
#define LEIBNIZL_MAX 10000  // 莱布尼兹级数公式项数
#define DEBUG 0             // 是否开启调试


double master_output = 0.0; // 主线程输出
double worker_output = 0.0; // 子线程输出


void master()
{
    // 计算后半部分
    int i = 0;
    for (i = LEIBNIZL_MAX / 2; i < LEIBNIZL_MAX; i++) {
        if (i % 2 == 0) master_output += (double)1 / (2*i + 1);
        else master_output -= (double)1 / (2*i + 1);
        if (DEBUG) printf("master: %d --> %lf\n", i, master_output);
    }
}


void *worker(void *arg)
{
    int i = 0;
    // 计算前半部分
    for (i = 0; i < LEIBNIZL_MAX / 2; i++) {
        if (i % 2 == 0) worker_output += (double)1 / (2*i + 1);
        else worker_output -= (double)1 / (2*i + 1);
        if (DEBUG) printf("worker: %d --> %lf\n", i, worker_output);
    }
    return NULL;
}


int main()
{
    // 创建线程
    pthread_t worker_tid;
    pthread_create(&worker_tid, NULL, worker, NULL);

    master(); 
    
    // 等待线程
    pthread_join(worker_tid, NULL);
    
    // 两个线程结果相加
    double total = master_output + worker_output;
    printf("master_output: %lf, worker_output: %lf, total: %lf, PI/4: %lf\n",
            master_output, worker_output, total, PI/4);
    return 0;
}

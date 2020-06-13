#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>


#define PI acos(-1)         // PI
#define DEBUG 0             // 是否开启调试
#define LEIBNIZL_MAX 10000  // 莱布尼兹级数公式项数
#define PTHREAD_NUM 5       // 线程数


// 线程参数
typedef struct {
    int start;
    int end;
} Param;

// 线程结果
typedef struct {
    double sum;
} Result;


// 线程运行函数
void *worker(void *arg)
{
    Param *param = (Param*)arg;                         // 获取参数
    Result *result = (Result*)malloc(sizeof(Result));   // 创建返回

    int i = 0;
    for (i = param->start; i < param->end; i++) {
        if (i % 2 == 0) result->sum += (double)1 / (2*i + 1);
        else result->sum -= (double)1 / (2*i + 1);

        if (DEBUG) printf("worker: %d --> %lf\n", i, result->sum);
    }

    return result;
}


int main()
{
    int i = 0;

    // 创建线程，并设置线程的计算范围
    pthread_t worker_tid[PTHREAD_NUM];
    for (i = 0; i < PTHREAD_NUM; i++) {
        Param *param = (Param*)malloc(sizeof(Param));
        param->start = i * (LEIBNIZL_MAX / PTHREAD_NUM);
        param->end = (i + 1) * (LEIBNIZL_MAX / PTHREAD_NUM);
        pthread_create(&worker_tid[i], NULL, worker, param);
    }

    // 等待线程，并将所有线程的结果相加
    double total = 0.0;
    for (i = 0; i < PTHREAD_NUM; i++) {
        Result *result;
        pthread_join(worker_tid[i], &result);
        total += result->sum;
        free(result);
    }

    printf("total: %lf, PI/4: %lf\n", total, PI/4);
    return 0;
}

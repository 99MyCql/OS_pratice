#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>


#define DEBUG 0
#define PTHREAD_NUM 2


// 线程参数
typedef struct {
    int start;
    int end;
    int *array;
} Param;


// 打印数组
void print_array(int *array, int len) {
    int i = 0;
    for (i = 0; i < len; i++) printf("%d ", array[i]);
    printf("\n");
}


// 选择排序
void select_sort(int *array, int len) {
    int i = 0, j = 0;
    for (i = 0; i < len; i++) {
        int min_pos = i;
        for (j = i; j < len; j++) {
            if (array[j] < array[min_pos]) min_pos = j;
        }
        int temp = array[i];
        array[i] = array[min_pos];
        array[min_pos] = temp;
    }
}


// 合并两个数组
int* merge(int *arr1, int arr1_len, int *arr2, int arr2_len) {
    int *ans_arr = (int*)malloc(sizeof(int) * (arr1_len + arr2_len));
    int i = 0, j = 0, k = 0;
    while (i < arr1_len && j < arr2_len) {
        if (arr1[i] < arr2[j])
            ans_arr[k++] = arr1[i++];
        else
            ans_arr[k++] = arr2[j++];
    }

    // 处理两个数组中的剩余数
    while (i < arr1_len)
        ans_arr[k++] = arr1[i++];

    while (j < arr2_len)
        ans_arr[k++] = arr2[j++];

    return ans_arr;
}


// 线程运行函数
void *worker(void *arg) {
    Param *param = (Param*)arg;
    if (DEBUG) {
        printf("worker start: ");
        print_array(param->array + param->start, param->end - param->start);
    }

    select_sort(param->array + param->start, param->end - param->start);

    if (DEBUG) {
        printf("worker end: ");
        print_array(param->array + param->start, param->end - param->start);
    }
    return NULL;
}


int main() {
    int i = 0;
    int array_num = 10;
    int array[] = {6,3,7,8,5,2,1,9,5,4};
    int part_len = array_num / PTHREAD_NUM;

    // 创建线程
    pthread_t workers[PTHREAD_NUM];
    for (i = 0; i < PTHREAD_NUM; i++) {
        Param *param = (Param*)malloc(sizeof(Param));
        param->start = i * part_len;
        param->end = (i + 1) * part_len;
        param->array = array;
        pthread_create(&workers[i], NULL, worker, param);
    }

    // 等待线程结束
    for (i = 0; i < PTHREAD_NUM; i++) {
        pthread_join(workers[i], NULL);
    }

    // 归并两个数组
    int *ans_arr = merge(array, part_len, array + part_len, part_len);
    printf("result: ");
    print_array(ans_arr, array_num);

    return 0;
}

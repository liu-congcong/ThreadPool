# ThreadPool

Thread Pool

## Example

```C
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include "threadPool.h"

typedef struct
{
    int x;
    pthread_mutex_t mutex;
} Result;

typedef struct
{
    int min;
    int max;
    Result *result;
} Temp;

int sum(void *arg)
{
    Temp *temp = (Temp *)arg;
    int x = 0;
    for (int i = temp->min; i < temp->max + 1; i++)
    {
        x += i;
    }
    pthread_mutex_lock(&temp->result->mutex);
    temp->result->x += x;
    pthread_mutex_unlock(&temp->result->mutex);
    free(arg);
    return 0;
}


int main(int argc, char *argv[])
{
    /* 0 + ... + x */
    int x = atoi(argv[1]);
    int threads = atoi(argv[2]);

    Result result;
    result.x = 0;
    pthread_mutex_init(&result.mutex, NULL);

    int step = ceil((float)x / threads);
    ThreadPool *threadPool = threadPoolCreate(threads);
    for (int i = 0; i < threads; i++)
    {
        Temp *temp = malloc(sizeof(Temp));
        temp->result = &result;
        temp->min = step * i + 1;
        temp->max = step * (i + 1) > x ? x : step * (i + 1);
        threadPoolPut(threadPool, sum, temp);
    }
    threadPoolFree(threadPool);
    printf("sum: %d\n", result.x);
    return 0;
}
```

```shell
gcc -o threadPoolExample threadPoolExample.c threadPool.c -lpthread
```

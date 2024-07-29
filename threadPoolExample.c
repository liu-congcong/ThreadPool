#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "threadPool.h"

int work(void *arg)
{
    int x = *(int *)arg;
    printf("%d\n", x);
    return 0;
}

int main(int argc, char *argv[])
{
    ThreadPool *threadPool = threadPoolCreate(10);
    assert(threadPool);
    for (int i = 0; i < 100; i++)
    {
        int *arg = malloc(sizeof(int));
        *arg = i;
        assert(!threadPoolPut(threadPool, work, arg));
    }
    assert(!threadPoolFree(threadPool));
    return 0;
}
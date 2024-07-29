#ifndef __THREADPOOL_H__
#define __THREADPOOL_H__

#include <pthread.h>

typedef struct Task
{
    int (*function)(void *);
    void *arg;
    struct Task *next;
} Task;

typedef struct ThreadPool
{
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    pthread_t *threads;
    int nThreads;
    Task *tasks;
    int nTasks;
    int flag;
} ThreadPool;

ThreadPool* threadPoolCreate(int nThreads);
int threadPoolPut(ThreadPool *threadPool, int (*function)(void *), void *arg);
int threadPoolFree(ThreadPool *pool);

#endif

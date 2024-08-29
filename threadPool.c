#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "threadPool.h"

static void *threadPoolWorker(void *threadPool)
{
    ThreadPool *x = (ThreadPool *)threadPool;
    while (1)
    {
        pthread_mutex_lock(&x->mutex);
        while (!x->nTasks && !x->flag)
        {
            pthread_cond_wait(&x->cond, &x->mutex);
        }
        if (x->nTasks > 0)
        {
            Task *task = x->tasks;
            x->tasks = task->next;
            x->nTasks--;
            pthread_mutex_unlock(&x->mutex);
            (*task->function)(task->arg);
            free(task);
        }
        else if (x->flag)
        {
            pthread_mutex_unlock(&x->mutex);
            pthread_exit(NULL);
        }
    }
    return NULL;
}

ThreadPool *threadPoolCreate(int nThreads)
{
    ThreadPool *threadPool = malloc(sizeof(ThreadPool));
    threadPool->nTasks = 0;
    threadPool->flag = 0;
    pthread_mutex_init(&threadPool->mutex, NULL);
    pthread_cond_init(&threadPool->cond, NULL);
    threadPool->threads = malloc(sizeof(pthread_t) * nThreads);
    threadPool->tasks = NULL;
    threadPool->nThreads = nThreads;
    for (int i = 0; i < nThreads; i++)
    {
        pthread_create(&threadPool->threads[i], NULL, threadPoolWorker, threadPool);
    }
    return threadPool;
}

int threadPoolPut(ThreadPool *threadPool, int (*function)(void *), void *arg)
{
    while (threadPool->nTasks > threadPool->nThreads)
    {
        sleep(1);
    }
    Task *task = malloc(sizeof(Task));
    task->function = function;
    task->arg = arg;
    pthread_mutex_lock(&threadPool->mutex);
    task->next = threadPool->tasks;
    threadPool->tasks = task;
    threadPool->nTasks++;
    pthread_cond_signal(&threadPool->cond);
    pthread_mutex_unlock(&threadPool->mutex);
    return 0;
}

int threadPoolFree(ThreadPool *threadPool)
{
    pthread_mutex_lock(&threadPool->mutex);
    threadPool->flag = 1;
    pthread_cond_broadcast(&threadPool->cond);
    pthread_mutex_unlock(&threadPool->mutex);
    for (int i = 0; i < threadPool->nThreads; i++)
    {
        pthread_join(threadPool->threads[i], NULL);
    }
    pthread_mutex_destroy(&threadPool->mutex);
    pthread_cond_destroy(&threadPool->cond);
    free(threadPool->threads);
    free(threadPool);
    return 0;
}

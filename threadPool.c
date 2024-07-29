#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <assert.h>
#include "threadPool.h"

static int function(void *arg)
{
    return 0;
}

static void *threadPoolWorker(void *threadPool)
{
    ThreadPool *x = (ThreadPool *)threadPool;
    while (1)
    {
        assert(!pthread_mutex_lock(&x->mutex));
        while (x->nTasks == 0 && !x->flag)
        {
            assert(!pthread_cond_wait(&x->cond, &x->mutex));
        }

        if (x->nTasks == 0 && x->flag)
        {
            assert(!pthread_mutex_unlock(&x->mutex));
            pthread_exit(NULL);
        }
        if (x->nTasks > 0)
        {
            Task *task = x->tasks;
            x->tasks = task->next;
            x->nTasks--;
            assert(!pthread_mutex_unlock(&x->mutex));
            assert(!(*task->function)(task->arg));
            free(task->arg);
            free(task);
        }
    }
    return NULL;
}

ThreadPool* threadPoolCreate(int nThreads)
{
    ThreadPool *threadPool = malloc(sizeof(ThreadPool));
    assert(threadPool);
    threadPool->nTasks = 0;
    threadPool->flag = 0;
    assert(!pthread_mutex_init(&threadPool->mutex, NULL));
    assert(!pthread_cond_init(&threadPool->cond, NULL));
    threadPool->threads = malloc(sizeof(pthread_t) * nThreads);
    assert(threadPool->threads);
    threadPool->tasks = NULL;
    threadPool->nThreads = nThreads;
    assert(threadPool->threads);
    for (int i = 0; i < nThreads; i++)
    {
        assert(!pthread_create(&threadPool->threads[i], NULL, threadPoolWorker, threadPool));
    }
    return threadPool;
}

int threadPoolPut(ThreadPool *threadPool, int (*function)(void *), void *arg)
{
    while (threadPool->nTasks >= 10 * threadPool->nThreads)
    {
        sleep(1);
    }
    assert(!pthread_mutex_lock(&threadPool->mutex));
    Task *task = malloc(sizeof(Task));
    assert(task);
    task->function = function;
    task->arg = arg;
    task->next = threadPool->tasks;
    threadPool->tasks = task;
    threadPool->nTasks++;
    assert(!pthread_cond_signal(&threadPool->cond));
    assert(!pthread_mutex_unlock(&threadPool->mutex));
    return 0;
}

int threadPoolFree(ThreadPool *pool)
{
    assert(!pthread_mutex_lock(&pool->mutex));
    pool->flag = 1;
    assert(!pthread_cond_broadcast(&pool->cond));
    assert(!pthread_mutex_unlock(&pool->mutex));
    for (int i = 0; i < pool->nThreads; i++)
    {
        assert(!pthread_join(pool->threads[i], NULL));
    }
    assert(!pthread_mutex_destroy(&pool->mutex));
    assert(!pthread_cond_destroy(&pool->cond));
    free(pool->threads);
    free(pool);
    return 0;
}

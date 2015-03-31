/*
 * uthreads.cpp
 *
 *  Created on: Mar 16, 2015
 *      Author: orenbm21
 */

#include "uthreads.h"
#include "Thread.h"
#include <list>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <sys/time.h>
#include <algorithm>

enum State
{
    READY, RUNNING, BLOCKED, NOT_EXIST
};

using std::list;
using std::queue;
using std::priority_queue;

static const int SAVE_SIGNAL_MASK = 1;

Thread *gThreads[MAX_THREAD_NUM];
State gThreadsState[MAX_THREAD_NUM];
priority_queue<int> gVacantTids;

// TODO: think of option to change global members to a class
int gQuantumUsecs;
int gTotalQuanta;
int gNumOfThreads;

list<int> gRedThreads;
list<int> gOrangeThreads;
list<int> gGreenThreads;

int gRunningThreadId;

int getNextThread()
{
    int nextId;
    if (!gRedThreads.empty())
    {
        nextId = gRedThreads.front();
        gRedThreads.pop_front();
    }
    else if (!gOrangeThreads.empty())
    {
        nextId = gOrangeThreads.front();
        gOrangeThreads.pop_front();
    }
    else
    {
        nextId = gGreenThreads.front();
        gGreenThreads.pop_front();
    }
    return nextId;
}

void resetTimer()
{
    struct itimerval tv;
    tv.it_value.tv_sec = 0;  /* first time interval, seconds part */
    tv.it_value.tv_usec = gQuantumUsecs; /* first time interval, microseconds part */
    tv.it_interval.tv_sec = 0;  /* following time intervals, seconds part */
    tv.it_interval.tv_usec = gQuantumUsecs; /* following time intervals, microseconds part */

    // TODO: err msg
    setitimer(ITIMER_VIRTUAL, &tv, nullptr);

}

void incrementGlobalQuanta()
{
    ++gTotalQuanta;
}

void moveToReady(int tid)
{
    gThreadsState[tid] = READY;

    switch (gThreads[tid]->getPriority())
    {
        case RED:
            gRedThreads.push_back(tid);
            break;
        case ORANGE:
            gOrangeThreads.push_back(tid);
            break;
        case GREEN:
            gGreenThreads.push_back(tid);
            break;
    }
}

int saveCurrentState()
{
    return sigsetjmp(gThreads[gRunningThreadId]->getBuf(), SAVE_SIGNAL_MASK);
}

void swapRunningThread()
{
    gRunningThreadId = getNextThread();
    gThreadsState[gRunningThreadId] = RUNNING;

    gThreads[gRunningThreadId]->incrementQuanta();
    incrementGlobalQuanta();

    resetTimer();
    siglongjmp(gThreads[gRunningThreadId]->getBuf(), 1);
}

void timer_handler(int sig)
{
    int res = saveCurrentState();

    if (res != 0)
    {
        return;
    }

    moveToReady(gRunningThreadId);

    swapRunningThread();
}

void initThreadStates()
{
    gThreadsState[0] = RUNNING;
    for (int i = 1; i < MAX_THREAD_NUM; ++i)
    {
        gThreadsState[i] = NOT_EXIST;
        gVacantTids.push(i);
    }
}

void initMainThread()
{
    gThreads[0] = new Thread(0, nullptr,ORANGE);
    gRunningThreadId = 0;
    gNumOfThreads = 1;
}

int uthread_init(int quantum_usecs)
{
    //TODO: when should we return -1?


    gQuantumUsecs = quantum_usecs;
    signal(SIGVTALRM, timer_handler);

    initMainThread();

    // TODO: 1?
    gTotalQuanta = 1;

    initThreadStates();
    resetTimer();

    return 0;
}

int uthread_spawn(void (*f)(void), Priority pr)
{
    if (gNumOfThreads >= MAX_THREAD_NUM)
    {
        return -1;
    }

    int minVacantId = gVacantTids.top();
    gVacantTids.pop();

    gThreads[minVacantId] = new Thread(minVacantId, f, pr);
    gNumOfThreads++;
    moveToReady(minVacantId);

    return minVacantId;
}

void addVacantId(int tid)
{
    gVacantTids.push(tid);
}

void removeThreadFromReady(int tid)
{
    Priority pr = gThreads[tid]->getPriority();
    switch (pr)
    {
        case GREEN:
            gGreenThreads.remove(tid);
            break;
        case ORANGE:
            gOrangeThreads.remove(tid);
            break;
        case RED:
            gRedThreads.remove(tid);
            break;
    }

}

void deleteThread(int tid)
{
    gThreadsState[tid] = NOT_EXIST;
    delete gThreads[tid];
    addVacantId(tid);
}

int uthread_terminate(int tid)
{
    if (tid == 0)
    {
        deleteThread(tid);
        exit(0);
    }
    switch (gThreadsState[tid])
    {
        case NOT_EXIST:
            // TODO: err msg
            return -1;
        case READY:
            removeThreadFromReady(tid);
            deleteThread(tid);
            break;
        case RUNNING:
            deleteThread(tid);
            swapRunningThread();
            break;
        case BLOCKED:
            deleteThread(tid);
            break;
    }

    return 0;
}

bool isVacant(int tid)
{
    return gThreadsState[tid] == NOT_EXIST;
}

int uthread_suspend(int tid)
{
    if (tid == 0 || isVacant(tid))
    {
        // TODO add error msg
        return -1;
    }

    gThreadsState[tid] = BLOCKED;
    switch (gThreadsState[tid])
    {
        case BLOCKED:
            break;
        case RUNNING:
            int res = saveCurrentState();
            if (res == 0)
            {
                swapRunningThread();
            }
            break;
        case READY:
            removeThreadFromReady(tid);
            break;
    }

    return 0;
}

int uthread_resume(int tid)
{
    switch (gThreadsState[tid])
    {
        case NOT_EXIST:
            //TODO: err msg
            return -1;
        case RUNNING:
        case READY:
            return 0;
        case BLOCKED:
            switch (gThreads[tid]->getPriority())
            {
                case RED:
                    gRedThreads.push_back(tid);
                    break;
                case ORANGE:
                    gOrangeThreads.push_back(tid);
                    break;
                case GREEN:
                    gGreenThreads.push_back(tid);
                    break;
            }

            gThreadsState[tid] = READY;
            return 0;
    }
}

int uthread_get_tid()
{
    return gRunningThreadId;
}

int uthread_get_total_quantums()
{
    return gTotalQuanta;
}

int uthread_get_quantums(int tid)
{
    switch (gThreadsState[tid])
    {
        case NOT_EXIST:
            //TODO: err msg
            return -1;
        default:
            return gThreads[tid]->getQuanta();
    }
}





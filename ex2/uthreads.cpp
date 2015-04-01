/*
 * uthreads.cpp
 *
 *  Created on: Mar 16, 2015
 *      Author: orenbm21
 */

#include "uthreads.h"
#include "Thread.h"
#include <list>
#include <signal.h>
#include <sys/time.h>
#include <algorithm>
#include <iostream>

enum State
{
    READY, RUNNING, BLOCKED, NOT_EXIST
};


using std::list;
using std::queue;
using std::priority_queue;

static const int SYSTEM_CALL_OK = 0;
static const int SAVE_SIGNAL_MASK = 1;
static const int MAIN_THREAD_ID = 0;

// TODO: think of option to change global members to a class
int gQuantumUsecs;
int gTotalQuanta;
int gNumOfThreads;
int gRunningThreadId;
sigset_t gTimerSet;

Thread *gThreads[MAX_THREAD_NUM];
State gThreadsState[MAX_THREAD_NUM];
priority_queue<int> gVacantTids;

list<int> gRedThreads;
list<int> gOrangeThreads;
list<int> gGreenThreads;

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

    if (setitimer(ITIMER_VIRTUAL, &tv, nullptr)!=SYSTEM_CALL_OK)
    {
        std::cerr << "system error: Setting timer interval failed.\n";
        exit(1);
    }

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

void blockTimer()
{
    if ( sigprocmask(SIG_BLOCK,&gTimerSet, nullptr) != SYSTEM_CALL_OK )
    {
        std::cerr << "system error: Blocking timer signal failed.\n";
        exit(1);
    }
}

void unblockTimer()
{
    if (sigprocmask(SIG_UNBLOCK,&gTimerSet, nullptr) != SYSTEM_CALL_OK)
    {
        std::cerr << "system error: Unblocking timer signal failed.\n";
        exit(1);
    }
}

void swapRunningThread(bool shouldUnblock)
{
    gRunningThreadId = getNextThread();
    gThreadsState[gRunningThreadId] = RUNNING;

    gThreads[gRunningThreadId]->incrementQuanta();
    incrementGlobalQuanta();

    resetTimer();
    if (shouldUnblock)
    {
        unblockTimer();
    }

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

    swapRunningThread(false);
}

void initThreadStates()
{
    gNumOfThreads=0;
    gTotalQuanta=0;

    gRedThreads.clear();
    gGreenThreads.clear();
    gOrangeThreads.clear();
    gVacantTids = priority_queue<int>();

    for (int i = 0; i < MAX_THREAD_NUM; ++i)
    {
        if (i != MAIN_THREAD_ID)
        {
            gThreadsState[i] = NOT_EXIST;
            gVacantTids.push(i);
        }
    }
}

void initMainThread()
{
    gThreads[MAIN_THREAD_ID] = new Thread(MAIN_THREAD_ID, nullptr,ORANGE);
    gRunningThreadId = MAIN_THREAD_ID;
    gThreadsState[gRunningThreadId] = RUNNING;

    ++gNumOfThreads;
    ++gTotalQuanta;
}

int uthread_init(int quantum_usecs)
{
    if (quantum_usecs<=0)
    {
        std::cerr << "thread library error: quantum duration must be greater than 0\n";
        return -1;
    }

    gQuantumUsecs = quantum_usecs;

    if (signal(SIGVTALRM, timer_handler) == SIG_ERR)
    {
        std::cerr << "system error: Setting timer handler failed.\n";
        exit(1);
    }

    initThreadStates();

    initMainThread();

    if ( sigemptyset(&gTimerSet) != SYSTEM_CALL_OK )
    {
        std::cerr << "system error: Setting signal set failed.\n";
        exit(1);
    }
    if ( sigaddset(&gTimerSet, SIGVTALRM) != SYSTEM_CALL_OK )
    {
        std::cerr << "system error: Setting signal set failed.\n";
        exit(1);
    }

    resetTimer();

    return 0;
}

int uthread_spawn(void (*f)(void), Priority pr)
{
    blockTimer();
    if (gNumOfThreads >= MAX_THREAD_NUM)
    {
        std::cerr << "thread library error: Thread count reached limit, cannot spawn a new one\n";
        unblockTimer();
        return -1;
    }

    int minVacantId = gVacantTids.top();
    gVacantTids.pop();

    gThreads[minVacantId] = new Thread(minVacantId, f, pr);
    gNumOfThreads++;
    moveToReady(minVacantId);

    unblockTimer();
    return minVacantId;
}

void addVacantId(int tid)
{
    gVacantTids.push(tid);
}

void removeThreadFromReady(const int tid, const Priority &pr)
{
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

void cleanThreadPool()
{
    for (int tid = 0; tid < MAX_THREAD_NUM; ++tid)
    {
        switch (gThreadsState[tid])
        {
            case RUNNING:
            case READY:
            case BLOCKED:
                deleteThread(tid);
                break;
            case NOT_EXIST:
                break;
        }
    }
}

int uthread_terminate(int tid)
{
    blockTimer();
    if (tid == 0)
    {
        cleanThreadPool();
        exit(0);
    }
    switch (gThreadsState[tid])
    {
        case NOT_EXIST:
            std::cerr << "thread library error: Cannot terminate a non existing thread (id: " << tid << "\n";
            unblockTimer();
            return -1;
        case READY:
            Priority pr = gThreads[tid]->getPriority();
            removeThreadFromReady(tid,pr);
            break;
        case RUNNING:
            swapRunningThread(true);
            break;
        case BLOCKED:
            break;
    }

    deleteThread(tid);
    unblockTimer();
    return 0;
}

bool isVacant(int tid)
{
    return gThreadsState[tid] == NOT_EXIST;
}

int uthread_suspend(int tid)
{
    blockTimer();
    if (tid == MAIN_THREAD_ID)
    {
        std::cerr << "thread library error: Cannot suspend the main thread (id: " << tid << "\n";
        unblockTimer();
        return -1;
    }
    if (tid == isVacant(tid))
    {
        std::cerr << "thread library error: Cannot suspend a non existing thread (id: " << tid << "\n";
        unblockTimer();
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
                swapRunningThread(true);
            }
            break;
        case READY:
            removeThreadFromReady(tid, gThreads[tid]->getPriority());
            break;
    }

    unblockTimer();
    return 0;
}

int uthread_resume(int tid)
{
    blockTimer();
    switch (gThreadsState[tid])
    {
        case NOT_EXIST:
            std::cerr << "thread library error: Cannot resume a non existing thread (id: " << tid << "\n";
            unblockTimer();
            return -1;
        case RUNNING:
        case READY:
            break;
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
            break;
    }

    unblockTimer();
    return 0;
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
            std::cerr << "thread library error: Cannot get time for a non existing thread (id: " << tid << "\n";
            return -1;
        default:
            return gThreads[tid]->getQuanta();
    }
}





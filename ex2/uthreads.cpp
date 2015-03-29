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

// TODO: think of option to change global members to a class

int gQuantumUsecs;
int gNumOfThreads;
int gId;
list<int> *gVacantIdList;
list<Thread> gRedThreads;
list<Thread> gOrangeThreads;
list<Thread> gGreenThreads;

Thread *gRunningThread;
list<Thread> gBlockedThreads;
State gThreadsState[MAX_THREAD_NUM];

list<int> *createIdList()
{
    list<int> *idList = new list<int>();
    for (int i = 0; i < MAX_THREAD_NUM; i++)
    {
        idList->push_back(i);
    }
    return idList;
}

void createQueues()
{
    gRedThreads = list<Thread>();
    gOrangeThreads = list<Thread>();
    gGreenThreads = list<Thread>();
}

void swapRunningThread(int sig)
{
    //TODO: fill this
}

void runProcess(int quantum_usecs)
{
    signal(SIGVTALRM, swapRunningThread);

    struct itimerval tv;
    tv.it_value.tv_sec = 0;  /* first time interval, seconds part */
    tv.it_value.tv_usec = quantum_usecs; /* first time interval, microseconds part */
    tv.it_interval.tv_sec = 0;  /* following time intervals, seconds part */
    tv.it_interval.tv_usec = quantum_usecs; /* following time intervals, microseconds part */

    setitimer(ITIMER_VIRTUAL, &tv, nullptr);
}

int uthread_init(int quantum_usecs)
{
    //TODO: when should we return -1?
    //TODO: what about main thread?
    gQuantumUsecs = quantum_usecs;
    gNumOfThreads = 0;
    gVacantIdList = createIdList();
    gBlockedThreads = list<Thread>();
    createQueues();

    runProcess(quantum_usecs);

    return 0;
}

int uthread_spawn(void (*f)(void), Priority pr)
{
    if (gNumOfThreads >= MAX_THREAD_NUM)
    {
        return -1;
    }
    // check if minVacantId is -1? shouldn't be -1 because of the previous check
    int minVacantId = gVacantIdList->front();
    gVacantIdList->pop_front();

    Thread newThread(minVacantId, f, pr);

    switch (pr)
    {
        case RED:
            gRedThreads.push_back(newThread);
            break;
        case ORANGE:
            gOrangeThreads.push_back(newThread);
            break;
        case GREEN:
            gGreenThreads.push_back(newThread);
            break;
    }
    gThreadsState[minVacantId] = READY;
    gNumOfThreads++;
    return minVacantId;
}

int getNextThread(Thread &next)
{
    if (!gRedThreads.empty())
    {
        next = gRedThreads.front();
        gRedThreads.pop_front();
    }
    else if (!gOrangeThreads.empty())
    {
        next = gOrangeThreads.front();
        gOrangeThreads.pop_front();
    }
    else
    {
        if (gGreenThreads.empty())
        {
            return -1;
        }
        else
        {
            next = gGreenThreads.front();
            gGreenThreads.pop_front();
        }
    }
    gThreadsState[next.getId()] = RUNNING;
    return next.getId();
}

void addVacantId(int id)
{
    for (auto iter = gVacantIdList->begin(); iter != gVacantIdList->end(); iter++)
    {
        if (*iter > id)
        {
            gVacantIdList->insert(iter, id);
        }
    }
}

int removeThreadFromList(list<Thread> &source, int tid, Thread &removed)
{
    for (auto iter = source.begin(); iter != source.end(); iter++)
    {
        if (iter->getId() == tid)
        {
            removed = Thread(*iter);
            source.erase(iter);

            return 0;
        }
    }
    return -1;
}

Thread removeThreadFromReady(int tid)
{
    Thread *removedThread;
    int result = removeThreadFromList(gRedThreads, tid, *removedThread);

    if (result == -1)
    {
        result = removeThreadFromList(gOrangeThreads, tid, *removedThread);
        if (result == -1)
        {
            result = removeThreadFromList(gGreenThreads, tid, *removedThread);
        }
    }
    return *removedThread;
}

int uthread_terminate(int tid)
{
    if (tid == 0)
    {
        exit(0);
    }
    switch (gThreadsState[tid])
    {
        case READY:
            // looking for the thread in the READY lists
            removeThreadFromReady(tid);
            break;
        case RUNNING:
            getNextThread(*gRunningThread);
            break;
        case BLOCKED:
            // looking for the thread in the blocked list
            Thread *stub;
            removeThreadFromList(gBlockedThreads, tid, *stub);
            break;
    }
    gThreadsState[tid] = NOT_EXIST;
    // enabling use of the terminated tid
    addVacantId(tid);
    return 0;
}

bool isVacant(int tid)
{
    return std::binary_search(gVacantIdList->begin(), gVacantIdList->end(), tid);
}

int uthread_suspend(int tid)
{
    if (tid == 0 || isVacant(tid))
    {
        // TODO add error msg
        return -1;
    }
    Thread *blockedThread;
    switch (gThreadsState[tid])
    {
        case BLOCKED:
            return 0;
        case READY:
            *blockedThread = (removeThreadFromReady(tid));
            break;
        case RUNNING:
            blockedThread = gRunningThread;
            getNextThread(*gRunningThread);
            break;
    }

    gBlockedThreads.push_back(*blockedThread);
    gThreadsState[tid] = BLOCKED;

    return 0;
}

int uthread_resume(int tid)
{
    if (isVacant(tid))
    {
        // TODO add err msg
        return -1;
    }
    switch (gThreadsState[tid])
    {
        case RUNNING:
        case READY:
            return 0;

    }
    Thread *resumedThread = removeThreadFromList(gBlockedThreads, tid, <#initializer#>);
    if (resumedThread != -1)
    {
        switch (resumedThread->pr)
        {
            case RED:
                gRedThreads->push_back(*resumedThread);
                break;
            case ORANGE:
                gOrangeThreads->push_back(*resumedThread);
                break;
            case GREEN:
                gGreenThreads->push_back(*resumedThread);
                break;
        }
    }
    gThreadsState[tid] = READY;
    return 0;
}

int uthread_get_tid()
{
    return gRunningThread->getId();
}

int uthread_get_total_quantums();

int uthread_get_quantums(int tid);




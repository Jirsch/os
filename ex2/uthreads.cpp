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

enum State { READY, RUNNING, BLOCKED, NOT_EXIST};

using std::list;

// think of option to change global members to a class

int gQuantumUsecs;
int gNumOfThreads;
int gId;
list<int>* gVacantIdList;
list<Thread>* gRedThreads;
list<Thread>* gOrangeThreads;
list<Thread>* gGreenThreads;

Thread* gRunningThread;
list<Thread>* gBlockedThreads;
State gThreadsState[MAX_THREAD_NUM];

list<int>* createIdList(){
    list<int>* idList = new list<int>();
    for (int i = 0; i < MAX_THREAD_NUM; i++){
        idList->push_back(i);
    }
    return idList;
}

void createQueues(){
    gRedThreads = new list<Thread>();
    gOrangeThreads = new list<Thread>();
    gGreenThreads = new list<Thread>();
}

void swapRunningThread(int sig){

}

void runProcess(int quantum_usecs){
    signal(SIGVTALRM, swapRunningThread);

    struct itimerval tv;
    tv.it_value.tv_sec = 0;  /* first time interval, seconds part */
    tv.it_value.tv_usec = quantum_usecs; /* first time interval, microseconds part */
    tv.it_interval.tv_sec = 0;  /* following time intervals, seconds part */
    tv.it_interval.tv_usec = quantum_usecs; /* following time intervals, microseconds part */



    setitimer(ITIMER_VIRTUAL, &tv, NULL);


}

int uthread_init(int quantum_usecs)
{
    // when should we return -1?

    gQuantumUsecs = quantum_usecs;
    gNumOfThreads = 0;
    gVacantIdList = createIdList();
    gBlockedThreads = new list<Thread>();
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
    int minVacantId = gVacantIdList->pop_front();
    Thread newthread = Thread(minVacantId,f, pr);

    switch (pr)
    {
        case RED:
            gRedThreads->push_back(newthread);
            break;
        case ORANGE:
            gOrangeThreads->push_back(newthread);
            break;
        case GREEN:
            gGreenThreads->push_back(newthread);
            break;
    }
    gThreadsState[minVacantId] = READY;
    gNumOfThreads++;
    return minVacantId;
}

Thread* getNextThread(){
    Thread* nextThread;
    if (!gRedThreads->empty()){
        nextThread = gRedThreads->pop_front();
    }
    else if (!gOrangeThreads->empty()){
        nextThread = gOrangeThreads->pop_front();
    }
    else{
        // if gGreenThreads is empty will return null
        nextThread = gGreenThreads->pop_front();
    }
    gThreadsState[nextThread->getId()] = RUNNING;
    return nextThread;
}

void addVacantId(int id){
    for (auto iter = gVacantIdList->begin(); iter != gVacantIdList->end(); iter++){
        if (*iter > id){
            gVacantIdList->insert(iter, id);
        }
    }
}

Thread* removeThreadFromList(list<Thread>* list, int tid){
    for (auto iter = list->begin(); iter != list->end(); iter++){
        if (*iter->getId() == tid){
            list->erase(iter);
            return *iter;
        }
    }
    return -1;
}

Thread* removeThreadFromReady(int tid){
    Thread* removedThread = removeThreadFromList(gRedThreads, tid);
    if (removedThread == -1){
        removedThread = removeThreadFromList(gOrangeThreads, tid);
        if (removedThread == -1){
            removedThread = removeThreadFromList(gGreenThreads, tid);
        }
    }
    return removedThread;
}

int uthread_terminate(int tid){
    if (tid == 0){
        exit(0);
    }
    switch(gThreadsState[tid])
    {
        case READY:
            // looking for the thread in the READY lists
            removeThreadFromReady(tid);
            break;
        case RUNNING:
            gRunningThread = getNextThread();
            break;
        case BLOCKED:
            // looking for the thread in the blocked list
            removeThreadFromList(gBlockedThreads, tid);
            break;
    }
    gThreadsState[tid] = NOT_EXIST;
    // enabling use of the terminated tid
    addVacantId(tid);
    return 0;
}

int uthread_suspend(int tid){
    Thread blockedThread;
    switch(gThreadsState[tid]){
        case READY:
            blockedThread = removeThreadFromReady(tid);
            break;
        case RUNNING:
            blockedThread = gRunningThread;
            gRunningThread = getNextThread();
            break;
    }
    if (blockedThread != -1){
        gBlockedThreads->push_back(blockedThread);
    }
    gThreadsState[tid] = BLOCKED;
    return 0;
}

int uthread_resume(int tid){
    Thread* resumedThread = removeThreadFromList(gBlockedThreads, tid);
    if (resumedThread != -1){
        switch (resumedThread->pr){
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

int uthread_get_tid(){
    return gRunningThread->getId();
}

int uthread_get_total_quantums();

int uthread_get_quantums(int tid);




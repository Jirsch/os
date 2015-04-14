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

// global members for the program
int gQuantumUsecs;
int gTotalQuanta;
int gNumOfThreads;
int gRunningThreadId;
sigset_t gTimerSet;

Thread *gThreads[MAX_THREAD_NUM];

// an array with the state of each thread
State gThreadsState[MAX_THREAD_NUM];

// will hold the ID numbers that aren't in use
priority_queue<int, std::vector<int>, std::greater<int> > gVacantTids;

// the lists containing the READY threads
list<int> gRedThreads;
list<int> gOrangeThreads;
list<int> gGreenThreads;

/*
* gets the next thread from the READY state lists. the priority is red->orange->green
*/
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
    else if(!gGreenThreads.empty())
    {
        nextId = gGreenThreads.front();
        gGreenThreads.pop_front();
    }
    else
    {
    	nextId = -1;
    }
    return nextId;
}

/*
* resets the timer to the given round-robin quanta in Usecs
*/
void resetTimer()
{
    struct itimerval tv;
    tv.it_value.tv_sec = 0;  /* first time interval, seconds part */
    tv.it_value.tv_usec = gQuantumUsecs; /* first time interval, microseconds part */
    tv.it_interval.tv_sec = 0;  /* following time intervals, seconds part */
    tv.it_interval.tv_usec = gQuantumUsecs; /* following time intervals, microseconds part */

    if (setitimer(ITIMER_VIRTUAL, &tv, NULL)!=SYSTEM_CALL_OK)
    {
        std::cerr << "system error: Setting timer interval failed.\n";
        exit(1);
    }
}

void incrementGlobalQuanta()
{
    ++gTotalQuanta;
}

/*
* moves the given thread to its priority list
*/
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
    if (sigprocmask(SIG_BLOCK,&gTimerSet, NULL) != SYSTEM_CALL_OK )
    {
        std::cerr << "system error: Blocking timer signal failed.\n";
        exit(1);
    }
}

void unblockTimer()
{
    if (sigprocmask(SIG_UNBLOCK,&gTimerSet, NULL) != SYSTEM_CALL_OK)
    {
        std::cerr << "system error: Unblocking timer signal failed.\n";
        exit(1);
    }
}

/*
* switches the RUNNING thread at the end of the round-robin quantum
*/
void swapRunningThread(bool notToReady)
{
//	std::cout << "Swap from " << gRunningThreadId;

	gThreads[gRunningThreadId]->incrementQuanta();
	incrementGlobalQuanta();
	int runningThread = gRunningThreadId;
	gRunningThreadId = getNextThread();

	if(gRunningThreadId == -1)
	{
		gRunningThreadId = runningThread;
	}
	if (!notToReady)
	{
		moveToReady(runningThread);
	}


    gThreadsState[gRunningThreadId] = RUNNING;

//    std::cout << "Swapped to " << gRunningThreadId << std::endl;


    resetTimer();

    unblockTimer();

    siglongjmp(gThreads[gRunningThreadId]->getBuf(), 1);
}

void timer_handler(int sig)
{
	int res = saveCurrentState();
//	std::cout << "timer handler" << std::endl;
    if (res != 0)
    {
        return;
    }
    swapRunningThread(false);
}

void initThreadStates()
{
    gNumOfThreads=0;
    gTotalQuanta=0;

    gRedThreads.clear();
    gGreenThreads.clear();
    gOrangeThreads.clear();
    gVacantTids = priority_queue<int, std::vector<int>, std::greater<int> >();

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
    gThreads[MAIN_THREAD_ID] = new Thread(MAIN_THREAD_ID, NULL,ORANGE);
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

    // get the id for the next thread
    int minVacantId = gVacantTids.top();
    gVacantTids.pop();

    gThreads[minVacantId] = new Thread(minVacantId, f, pr);
    gNumOfThreads++;
    moveToReady(minVacantId);

    unblockTimer();
    return minVacantId;
}

/*
* add the gived tid to the available ids list
*/
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
    gNumOfThreads--;
}

/*
* delete all existing threads
*/
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
    Priority pr;
    State state = gThreadsState[tid];
    deleteThread(tid);


    switch (state)
    {
        case NOT_EXIST:
            std::cerr << "thread library error: Cannot terminate a non existing thread (id: " << tid << "\n";
            unblockTimer();
            return -1;
        case READY:
            pr = gThreads[tid]->getPriority();
            removeThreadFromReady(tid,pr);
            break;
        case RUNNING:
            swapRunningThread(true);
            break;
        case BLOCKED:
            break;
    }
    unblockTimer();
    return 0;
}

/*
* return true if the tid is available
*/
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
    if (isVacant(tid))
    {
        std::cerr << "thread library error: Cannot suspend a non existing thread (id: " << tid << "\n";
        unblockTimer();
        return -1;
    }
    int res;
    switch (gThreadsState[tid])
    {
        case BLOCKED:
            break;
        case RUNNING:
        	// if the RUNNING thread suspends itself
        	gThreadsState[tid] = BLOCKED;
            res = saveCurrentState();
            if (res == 0)
            {
                swapRunningThread(true);
            }
            break;
        case READY:
            removeThreadFromReady(tid, gThreads[tid]->getPriority());
            break;
    }
    gThreadsState[tid] = BLOCKED;
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
//	std::cout << "Get id = " << gRunningThreadId << std::endl;
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
     //   	std::cout << "Get quantums for "<< tid << " = " << gThreads[tid]->getQuanta() << std::endl;
            return gThreads[tid]->getQuanta();
    }
}





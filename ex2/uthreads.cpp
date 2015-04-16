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


#define LIB_ERR_MSG "thread library error: "
#define SYS_ERR_MSG "system error: "
#define TO_SEC 1000000
#define NO_THREADS -1
#define SUCCESS 0
#define FAILURE -1

enum State
{
    READY, RUNNING, BLOCKED, NOT_EXIST
};

using std::list;
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
    	nextId = NO_THREADS;
    }

    return nextId;
}

/*
* resets the timer to the given round-robin quanta in Usecs
*/
void resetTimer()
{
    struct itimerval tv;
	int quantumSec = gQuantumUsecs / TO_SEC;
	int quantumUsecs = gQuantumUsecs % TO_SEC;

    tv.it_value.tv_sec = quantumSec;  /* first time interval, seconds part */
    tv.it_value.tv_usec = quantumUsecs; /* first time interval, microseconds part */
    tv.it_interval.tv_sec = quantumSec;  /* following time intervals, seconds part */
    tv.it_interval.tv_usec = quantumUsecs; /* following time intervals, microseconds part */

    if (setitimer(ITIMER_VIRTUAL, &tv, NULL)!=SYSTEM_CALL_OK)
    {
        std::cerr << SYS_ERR_MSG << "Setting timer interval failed.\n";
        exit(1);
    }
}

/*
 * add one quantum to the global quanta
 */
void incrementGlobalQuanta()
{
    ++gTotalQuanta;
}

/*
* return true if the tid is available
*/
bool isVacant(int tid)
{
    return gThreadsState[tid] == NOT_EXIST;
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

/*
 * saves the current state of running thread at the end of the quanta
 */
int saveCurrentState()
{
    return sigsetjmp(gThreads[gRunningThreadId]->getBuf(), SAVE_SIGNAL_MASK);
}

/*
 * blocks the timer from running
 */
void blockTimer()
{
    if (sigprocmask(SIG_BLOCK,&gTimerSet, NULL) != SYSTEM_CALL_OK )
    {
        std::cerr << SYS_ERR_MSG << "Blocking timer signal failed.\n";
        exit(1);
    }
}

/*
 * unblocks the timer and resume running
 */
void unblockTimer()
{
    if (sigprocmask(SIG_UNBLOCK,&gTimerSet, NULL) != SYSTEM_CALL_OK)
    {
        std::cerr << SYS_ERR_MSG << "Unblocking timer signal failed.\n";
        exit(1);
    }
}

/*
* switches the RUNNING thread at the end of the round-robin quantum
*/
void swapRunningThread(bool notToReady)
{

	int runningThread = gRunningThreadId;

	// finding the next available thread from the READY lists
	gRunningThreadId = getNextThread();

	// if the lists are empty the RUNNING thread continues to run
	if(gRunningThreadId == NO_THREADS)
	{
		gRunningThreadId = runningThread;
	}

	else if (!notToReady)
	{
		moveToReady(runningThread);
	}

    gThreadsState[gRunningThreadId] = RUNNING;

    resetTimer();

    gThreads[gRunningThreadId]->incrementQuanta();
    incrementGlobalQuanta();

    unblockTimer();

    siglongjmp(gThreads[gRunningThreadId]->getBuf(), 1);
}

/*
 * this method is called after every loop of the timer
 */
void timer_handler(int sig)
{
	int res = saveCurrentState();

    if (res != 0)
    {
        return;
    }

    swapRunningThread(false);
}

/*
 * initializing the global variables and Data-structures
 */
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
/*
 * initialize the main thread, put in the RUNNING state and increment quanta
 */
void initMainThread()
{
    gThreads[MAIN_THREAD_ID] = new Thread(MAIN_THREAD_ID, NULL,ORANGE);

    gRunningThreadId = MAIN_THREAD_ID;

    gThreadsState[gRunningThreadId] = RUNNING;

    gThreads[MAIN_THREAD_ID]->incrementQuanta();

    ++gNumOfThreads;
    ++gTotalQuanta;
}

int uthread_init(int quantum_usecs)
{
    if (quantum_usecs<=0)
    {
        std::cerr << LIB_ERR_MSG << "quantum duration must be greater than 0\n";
        return FAILURE;
    }

    gQuantumUsecs = quantum_usecs;

    if (signal(SIGVTALRM, timer_handler) == SIG_ERR)
    {
        std::cerr << SYS_ERR_MSG << "Setting timer handler failed.\n";
        exit(1);
    }

    initThreadStates();
    initMainThread();

    if ( sigemptyset(&gTimerSet) != SYSTEM_CALL_OK )
    {
        std::cerr << SYS_ERR_MSG << "Setting signal set failed.\n";
        exit(1);
    }

    if ( sigaddset(&gTimerSet, SIGVTALRM) != SYSTEM_CALL_OK )
    {
        std::cerr << SYS_ERR_MSG << "Setting signal set failed.\n";
        exit(1);
    }

    resetTimer();

    return SUCCESS;
}

int uthread_spawn(void (*f)(void), Priority pr)
{
    blockTimer();

    if (gNumOfThreads >= MAX_THREAD_NUM)
    {
        std::cerr << LIB_ERR_MSG << "Thread count reached limit, cannot spawn a new one\n";
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
* add the given tid to the available ids list
*/
void addVacantId(int tid)
{
    gVacantTids.push(tid);
}

/*
 * removes the given thread id from its READY list
 */
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

/*
 * delete the given thread and update the global vars
 */
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

/*
 * return the state of the given thread
 */
State getState(int tid)
{
	if (tid < 0 || tid > MAX_THREAD_NUM || isVacant(tid))
	{
		return NOT_EXIST;
	}
	else
	{
		return gThreadsState[tid];
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
    State state = getState(tid);

    if (state != NOT_EXIST)
    {
    	deleteThread(tid);
    }

    switch (state)
    {
        case NOT_EXIST:
            std::cerr << LIB_ERR_MSG << "Cannot terminate a non existing thread (id: "
            	<< tid << ")\n";
            unblockTimer();
            return FAILURE;

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
    return SUCCESS;
}

int uthread_suspend(int tid)
{
    blockTimer();

    if (tid == MAIN_THREAD_ID)
    {
        std::cerr << LIB_ERR_MSG << "Cannot suspend the main thread (id: " << tid << ")\n";
        unblockTimer();
        return FAILURE;
    }

    if (tid < 0 || isVacant(tid))
    {
        std::cerr << LIB_ERR_MSG << "Cannot suspend a non existing thread (id: " << tid << ")\n";
        unblockTimer();
        return FAILURE;
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
            gThreadsState[tid] = BLOCKED;
            break;
    }

    unblockTimer();
    return SUCCESS;
}

int uthread_resume(int tid)
{
    blockTimer();
    State state = getState(tid);

    switch (state)
    {
        case NOT_EXIST:
            std::cerr << LIB_ERR_MSG << "Cannot resume a non existing thread (id: " << tid << ")\n";
            unblockTimer();
            return FAILURE;

        case RUNNING:
        case READY:
            break;

        case BLOCKED:
            moveToReady(tid);
            break;
    }

    unblockTimer();
    return SUCCESS;
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
    State state = getState(tid);

    switch (state)
    {
        case NOT_EXIST:
            std::cerr << LIB_ERR_MSG << "Cannot get time for a non existing thread (id: "
            	<< tid << ")\n";
            return FAILURE;

        default:
            return gThreads[tid]->getQuanta();
    }
}

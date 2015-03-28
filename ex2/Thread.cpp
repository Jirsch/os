//
// Created by Jonathan Hirsch on 3/28/15.
//

#include "Thread.h"


Thread::Thread(int tid, void (*func)(void), Priority pr)
{
    // TODO allocate memory to stack
    this->f = func;
    this->tid = tid;
    this->pr = pr;
    this->quantumsFinished = START; // change?
//	this->blockingThreadId = NOT_BLOCKED;
    this->stack = new char[STACK_SIZE];
}

bool Thread::operator ==(const Thread& thread, int id){
    return (thread.tid == id);
}

Thread::~Thread()
{
    // TODO Auto-generated destructor stub
}

int Thread::getQuantums()
{
    return this->quantumsFinished;
}

int Thread::getId()
{
    return this->tid;
}


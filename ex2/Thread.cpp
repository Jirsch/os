//
// Created by Jonathan Hirsch on 3/28/15.
//

#include "Thread.h"


Thread::Thread(int tid, void (*func)(void), Priority pr)
{
    this->f = func;
    this->tid = tid;
    this->pr = pr;
    this->quantumsFinished = START; // TODO: change?
    //this->blockingThreadId = NOT_BLOCKED;
    this->stack = new char[STACK_SIZE];
}

Thread::Thread(const Thread &rhs)
{
    this->f = rhs.f;
    this->tid = rhs.tid;
    this->pr = rhs.pr;
    this->quantumsFinished = rhs.quantumsFinished;

    this->stack = new char[STACK_SIZE];
    strncpy(this->stack, rhs.stack, STACK_SIZE);
}

bool Thread::operator==(const Thread &thread, int id)
{
    return (thread.tid == id);
}

Thread &Thread::operator=(const Thread &rhs)
{
    if (this == &rhs)
    {
        return *this;
    }
    this->f = rhs.f;
    this->tid = rhs.tid;
    this->pr = rhs.pr;
    this->quantumsFinished = rhs.quantumsFinished;

    delete[] this->stack;
    this->stack = new char[STACK_SIZE];
    strncpy(this->stack, rhs.stack, STACK_SIZE);

    return *this;
}

Thread::~Thread()
{
    delete[] this->stack;
}

int Thread::getQuantums()
{
    return this->quantumsFinished;
}

int Thread::getId()
{
    return this->tid;
}

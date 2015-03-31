//
// Created by Jonathan Hirsch on 3/28/15.
//

#include <signal.h>
#include "Thread.h"


Thread::Thread(int tid, void (*func)(void), Priority pr)
{
    this->f = func;
    this->tid = tid;
    this->pr = pr;
    this->quantumsFinished = START;

    if (f != nullptr)
    {
        address_t sp, pc;
        sp = (address_t) stack + STACK_SIZE - sizeof(address_t);
        pc = (address_t) f;

        (this->buf.__jmpbuf)[JB_SP] = translate_address(sp);
        (this->buf.__jmpbuf)[JB_PC] = translate_address(pc);
        sigemptyset(&this->buf.__saved_mask);
    }
}

Thread::Thread(const Thread &rhs)
{
    this->f = rhs.f;
    this->tid = rhs.tid;
    this->pr = rhs.pr;
    this->quantumsFinished = rhs.quantumsFinished;

    strncpy(this->stack, rhs.stack, STACK_SIZE);
}

//bool Thread::operator==(const Thread &thread, int id)
//{
//    return (thread.tid == id);
//}

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

    strncpy(this->stack, rhs.stack, STACK_SIZE);

    return *this;
}

Thread::~Thread()
{
}

void Thread::incrementQuanta()
{
    ++(this->quantumsFinished);
}

//
// Created by Jonathan Hirsch on 3/28/15.
//


#include "Thread.h"
#define START 0
#ifdef __x86_64__
/* code for 64 bit Intel arch */

typedef unsigned long address_t;
#define JB_SP 6
#define JB_PC 7

/* A translation is required when using an address of a variable.
   Use this as a black box in your code. */
address_t translate_address(address_t addr)
{
    address_t ret;
    asm volatile("xor    %%fs:0x30,%0\n"
            "rol    $0x11,%0\n"
    : "=g" (ret)
    : "0" (addr));
    return ret;
}

//#else
///* code for 32 bit Intel arch */
//
//typedef unsigned int address_t;
//#define JB_SP 4
//#define JB_PC 5
//
///* A translation is required when using an address of a variable.
//   Use this as a black box in your code. */
//address_t translate_address(address_t addr)
//{
//    address_t ret;
//    asm volatile("xor    %%gs:0x18,%0\n"
//		"rol    $0x9,%0\n"
//                 : "=g" (ret)
//                 : "0" (addr));
//    return ret;
//}

#endif

Thread::Thread(int tid, void (*func)(void), Priority pr)
{
    this->f = func;
    this->tid = tid;
    this->pr = pr;
    this->quantumsFinished = START;

    if (f != NULL)
    {
        address_t sp, pc;
        sp = (address_t) stack + STACK_SIZE - sizeof(address_t);
        pc = (address_t) f;

        (this->buf->__jmpbuf)[JB_SP] = translate_address(sp);
        (this->buf->__jmpbuf)[JB_PC] = translate_address(pc);
        sigemptyset(&this->buf->__saved_mask);
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

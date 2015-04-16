/*   *--------------------------------------------------------------------------------------------*
* 											OS (2015) - Ex2
*
* 		Name : Thread.h
* 		General : Class representing a Thread. Each Thread has ID, priority, stack, state,
* 		environment to save its status and the number of quantums that were started for the thread.
*
*    *--------------------------------------------------------------------------------------------*
*/

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
    _f = func;
    _tid = tid;
    _pr = pr;
    _quantumsFinished = START;

    if (_f != NULL)
    {
        address_t sp, pc;
        sp = (address_t) _stack + STACK_SIZE - sizeof(address_t);
        pc = (address_t) _f;

        (_buf->__jmpbuf)[JB_SP] = translate_address(sp);
        (_buf->__jmpbuf)[JB_PC] = translate_address(pc);
        sigemptyset(&_buf->__saved_mask);
    }
}

Thread::Thread(const Thread &rhs)
{
    _f = rhs._f;
    _tid = rhs._tid;
    _pr = rhs._pr;
    _quantumsFinished = rhs._quantumsFinished;

    strncpy(_stack, rhs._stack, STACK_SIZE);
}

Thread &Thread::operator=(const Thread &rhs)
{
    if (this == &rhs)
    {
        return *this;
    }

    _f = rhs._f;
    _tid = rhs._tid;
    _pr = rhs._pr;
    _quantumsFinished = rhs._quantumsFinished;

    strncpy(_stack, rhs._stack, STACK_SIZE);

    return *this;
}

Thread::~Thread()
{
}

void Thread::incrementQuanta()
{
    ++(_quantumsFinished);
}

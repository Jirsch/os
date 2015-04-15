//
// Created by Jonathan Hirsch on 3/28/15.
//

#ifndef THREAD_H_
#define THREAD_H_

#define NOT_BLOCKED -1


#include <setjmp.h>
#include "uthreads.h"
#include <signal.h>
#include <cstdlib>
#include <string.h>





class Thread
{

    private:
    int tid;
    void (*f)(void);
    Priority pr;
    unsigned int quantumsFinished;
    char stack[STACK_SIZE];
    sigjmp_buf buf;

    public:
    Thread(int tid, void (*func)(void), Priority pr);
    Thread(const Thread &rhs);

    Priority const &getPriority() const
    {
        return pr;
    }


    sigjmp_buf& getBuf()
    {
        return buf;
    }


    int getId() const
    {
        return tid;
    }


    unsigned int getQuanta() const
    {
        return quantumsFinished;
    }

    void incrementQuanta();

    virtual ~Thread();

    Thread &operator=(const Thread &rhs);
};

#endif /* THREAD_H_ */


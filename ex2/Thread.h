//
// Created by Jonathan Hirsch on 3/28/15.
//

#ifndef THREAD_H_
#define THREAD_H_

#define NOT_BLOCKED -1
#define START 0
#include "uthreads.h"

class Thread {

    public:
    Thread(int tid, void (*func)(void), Priority pr);
    virtual ~Thread();
    int getQuantums();
    int getId();

    bool operator==(const Thread& thread, int id);
    private:
    int tid;
    void (*f)(void);
    Priority pr;
    unsigned int quantumsFinished;
//	int blockingThreadId;
    char stack[STACK_SIZE];
};

#endif /* THREAD_H_ */


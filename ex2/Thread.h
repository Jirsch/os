/*   *--------------------------------------------------------------------------------------------*
* 											OS (2015) - Ex2
*
* 		Name : Thread.h
* 		General : Class representing a Thread. Each Thread has ID, priority, stack, state,
* 		environment to save its status and the number of quantums that were started for the thread.
*
*    *--------------------------------------------------------------------------------------------*
*/
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

    int _tid;
    void (*_f)(void);
    Priority _pr;
    unsigned int _quantumsFinished;
    char _stack[STACK_SIZE];
    sigjmp_buf _buf;

    public:

    /*
     * constructor
     * tid - thread id
     * func - the function of the thread
     * pr - the priority of the thread (ordered RED->ORANGE-GREEN)
     */
    Thread(int tid, void (*func)(void), Priority pr);

    /*
     * copy constructor
     */
    Thread(const Thread &rhs);

    /*
     * return the priority of the thread
     */
    Priority const &getPriority() const
    {
        return _pr;
    }

    /*
     * return the buffer of the thread
     */
    sigjmp_buf& getBuf()
    {
        return _buf;
    }

    /*
     * return the thread id
     */
    int getId() const
    {
        return _tid;
    }

    /*
     * return the number of quanta that the thread had finished
     */
    unsigned int getQuanta() const
    {
        return _quantumsFinished;
    }

    /*
     * add one quanta to the thread
     */
    void incrementQuanta();

    /*
     * destructor
     */
    virtual ~Thread();

    /*
     * operator =
     */
    Thread &operator=(const Thread &rhs);
};

#endif /* THREAD_H_ */


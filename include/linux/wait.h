#ifndef     _WAIT_H_
#define     _WAIT_H_

#include "list.h"

struct wait_queue
{
    struct task_struct  *task;
    struct wait_queue 	*next;
};

struct semaphore
{
    int count;
    struct wait_queue *wait;
};

struct __wait_queue_head {
    //		spinlock_t lock;
    struct list_head task_list;
};

typedef struct __wait_queue_head wait_queue_head_t;


extern 	void  	wake_up_interruptible(struct wait_queue **wq);
extern 	void 	interruptible_sleep_on(struct wait_queue **wq);

extern void add_wait_queue(struct wait_queue** wq, struct wait_queue *wait);
extern void remove_wait_queue(struct wait_queue **wq, struct wait_queue *wait);

extern void down(struct semaphore *sem);
extern void up(struct semaphore *sem);

#endif

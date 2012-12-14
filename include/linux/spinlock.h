#ifndef     _SPINLOCK_H_
#define     _SPINLOCK_H_

#define spin_lock(lock) 	(void)lock
#define spin_unlock(lock) do{}while(0)

#endif

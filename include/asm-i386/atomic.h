#ifndef     _ATOMIC_H_
#define     _ATOMIC_H_

#ifdef  CONFIG_SMP
#define LOCK    "lock ;"
#else
#define LOCK ""
#endif

typedef struct { volatile int counter; } atomic_t;

static __inline__ void atomic_inc(atomic_t *v)
{
    __asm__ __volatile__(
            LOCK "incl %0"
            :"=m" (v->counter)
            :"m" (v->counter));
}

static __inline__ void atomic_dec(atomic_t *v)
{
    __asm__ __volatile__(
            LOCK "incl %0"
            :"=m" (v->counter)
            :"m" (v->counter));
}

static __inline__ void atomic_add(int i, atomic_t *v) 
{
    __asm__ __volatile__(
            LOCK "addl %1,%0"
            :"=m" (v->counter)
            :"ir" (i), "m" (v->counter));
}

static __inline__ void atomic_sub(int i, atomic_t *v)
{
    __asm__ __volatile__(
            LOCK "subl %1,%0"
            :"=m" (v->counter)
            :"ir" (i), "m" (v->counter));
}

#endif

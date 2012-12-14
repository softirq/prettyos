#ifndef     _MALLOC_H_
#define     _MALLOC_H_

extern void * kmalloc(size_t size, int priority);
extern void * kfree_s(void *obj, int len);

#define     kfree(x)    kfree_s(x,0)

#endif

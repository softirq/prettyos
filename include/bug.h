#ifndef     _BUG_H_
#define     _BUG_H_

#define BUG()	__asm__ __volatile__ ("ud2\n")

#endif

#ifndef     _LINKAGE_H_
#define     _LINKAGE_H_

#define __ALIGN_MASK(x,mask) (((x) + (mask)) & (~mask))
#define ALIGN(x,a)  __ALIGN_MASK(x, (typedef(x)(a) - 1))

#endif

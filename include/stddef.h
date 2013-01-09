#ifndef     _STDDEF_H_
#define     _STDDEF_H_

#undef  NULL
#define NULL ((void *)0)

#define     VALUE_MASK(value)   (~(value) + 1)

#define     ARRAY_SIZE(array)   (sizeof(array)/sizeof(array[0]))

#endif

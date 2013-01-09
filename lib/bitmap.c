#include "type.h"
#include "bitmap.h"
/* set the address bit  */
#if 0
inline int set_bit(int nr, void * addr)
{
    int oldbit;
    __asm__ __volatile__("btsl %2,%1\n\tsbbl %0,%0"
            :"=r" (oldbit),"=m" (addr)
            :"r" (nr));
    return oldbit;
}

/* clear the address bit */
inline int clear_bit(int nr, void * addr)
{
    int oldbit;

    __asm__ __volatile__("btrl %2,%1\n\tsbbl %0,%0"
            :"=r" (oldbit),"=m" (addr)
            :"r" (nr));
    return oldbit;
}

#endif

inline void __set_bit(int nr, unsigned char *ch)
{
    *ch |= (1<<nr & 0xFF);
}

inline void __clear_bit(int nr, unsigned char *ch)
{
    *ch &= (~(1<<nr) & 0xFF);
}

/*  bit scan forward 
 *   * find the first 1 */
inline int find_first_bit(unsigned char ch)
{
    unsigned int _res;
    __asm__ (
            "cld\n\t"
            "bsf %%eax,%%ecx\n\t"
            :"=c"(_res)
            :"a"((unsigned int)ch),"c"(0)
            );	
    return _res;
}

/* bit scan forward
 * find the first 0 */
inline int find_first_zero(unsigned char ch) 
{
    return find_first_bit(~ch & 0xFF);
}

inline int get_first_bit(unsigned char *bitmap,size_t size)
{
    int i = 0;
    for(;i < size; ++i)
    {   
        if(*(bitmap + i) == 0xff)
            continue;
        int first = find_first_zero(*(bitmap+i));
        return first + sizeof(unsigned char ) * i;
    }   

    return -1; 
}

inline int set_first_bit(unsigned char *bitmap, size_t size)
{
    int i = 0;
    for(;i < size; ++i)
    {   
        if(*(bitmap + i) == 0xff)
            continue;
        int first = find_first_zero(*(bitmap+i));
        __set_bit(first, (bitmap+i));
        return first + sizeof(unsigned char ) * i;
    }

    return -1;
}

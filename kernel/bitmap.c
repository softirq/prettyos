inline int set_bit(int nr, void * addr)
{
    int oldbit;
    __asm__ __volatile__("btsl %2,%1\n\tsbbl %0,%0"
            :"=r" (oldbit),"=m" (addr)
            :"r" (nr));
    return oldbit;
}

inline int clear_bit(int nr, void * addr)
{
    int oldbit;

    __asm__ __volatile__("btrl %2,%1\n\tsbbl %0,%0"
            :"=r" (oldbit),"=m" (addr)
            :"r" (nr));
    return oldbit;
}

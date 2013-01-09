#ifndef     _BITMAP_H_
#define     _BITMAP_H_

extern inline void __clear_bit(int nr, unsigned char *ch);
extern inline void __set_bit(int nr, unsigned char * ch);
extern inline int find_first_bit(unsigned char ch) ;
extern inline int find_first_zero(unsigned char ch);
extern inline int get_first_bit(unsigned char *bitmap,size_t size);
extern inline int set_first_bit(unsigned char *bitmap, size_t size);

#endif

#ifndef     _FS_BITMAP_H_
#define     _FS_BITMAP_H_

extern unsigned short get_imap_bit(int dev);
extern unsigned short get_zmap_bit(int dev);
extern int clear_imap_bit(int dev,int nr);
extern int clear_zmap_bit(int dev,int nr);

#endif

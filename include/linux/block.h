#ifndef     _BLOCK_H_
#define     _BLOCK_H_

extern int nr_sectors;

extern int get_block_nums(int dev, struct m_inode *inode, int num);
extern int get_block_nr(int dev, struct m_inode *inode);
#endif

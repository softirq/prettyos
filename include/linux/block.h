#ifndef     _BLOCK_H_
#define     _BLOCK_H_

extern int nr_sectors;

extern int set_block_nums(int dev, struct d_inode *inode, int num);
extern int set_block_nr(int dev, struct d_inode *inode);
extern inline int get_first_block(struct m_inode *inode);
extern inline int get_pos_block(struct m_inode *inode, int pos);

#endif

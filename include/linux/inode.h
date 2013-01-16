#ifndef     _INODE_H_
#define     _INODE_H_

#include "list.h"

//memory inode接点结构
struct m_inode
{
	unsigned short	i_mode;			//文件类型和属性
	unsigned short 	i_size;			//文件的大小
//	unsigned short	i_nlinks;		//连接数(有多少个文件目录项指向该i节点)
	unsigned short	i_start_sect;		//文件起始块
	unsigned short	i_nr_sects;		//文件数据块数

	unsigned short	i_dev;
	unsigned short	i_count;		//当前被使用的次数
	unsigned short	i_num;			//文件的inode号
	unsigned short	i_lock;			//文件的琐标记
	unsigned short i_dirt;			//脏标记
	struct vm_area_struct *i_mmap;  /* for shm areas,  the list of attaches, otherwise unused. */
    struct list_head list;
};

extern unsigned short nr_inodes_count;
extern struct list_head inode_lists;


int free_inode(struct m_inode *inode);
#endif

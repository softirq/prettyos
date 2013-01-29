#ifndef     _INODE_H_
#define     _INODE_H_

#include "list.h"

//第一个inode (即/目录的inode)
#define ROOT_INODE	1
#define NR_INODE	32
//extern struct m_inode inode_table[NR_INODE];
extern struct m_inode *root_inode;

struct inode_operations
{
    struct m_inode * (*lookup)(int dev, int num);
    int (*write)(struct m_inode *inode);
    int (*release)(struct m_inode *inode);
};

//memory inode接点结构
struct m_inode
{
	unsigned short	i_mode;			//文件类型和属性
	unsigned short 	i_size;			//文件的大小
//	unsigned short	i_nlinks;		//连接数(有多少个文件目录项指向该i节点)
	unsigned short	i_start_sect;		//文件起始块
	unsigned short	i_nr_sects;		//文件数据块数
    unsigned int    i_flags;
    unsigned int    i_data[13];

	unsigned short	i_dev;
	short	i_count;		//当前被使用的次数
	unsigned short	i_num;			//文件的inode号
	unsigned short	i_lock;			//文件的琐标记
	unsigned short i_dirty;			//脏标记
	struct vm_area_struct *i_mmap;  /* for shm areas,  the list of attaches, otherwise unused. */
    struct list_head i_list;
    struct inode_operations *i_ops;
    struct address_space *i_mapping;
};

//disk inode节点结构	
struct d_inode
{
	unsigned short	i_mode;			//文件类型和属性
	unsigned short 	i_size;			//文件的大小
//	unsigned short	i_nlinks;		//连接数(有多少个文件目录项指向该i节点)
//先使用简单的方式
//	unsigned short	i_zone[9];		//直接块 间接块 二次间接块
	unsigned short	i_start_sect;		//文件起始块
	unsigned short	i_nr_sects;		//文件数据块数
    unsigned int    i_flags;
    unsigned int    i_data[13];
};

extern unsigned short nr_inodes_count;
//extern struct list_head inode_lists;

extern struct inode_operations pfs;

extern struct m_inode * iget(int dev,int num);
extern int iput(struct m_inode *inode);
extern int  write_inode(struct m_inode *inode);
extern int  create_block(struct m_inode *inode,int block);

#endif

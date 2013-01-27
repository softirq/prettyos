#ifndef     _FS_H_
#define     _FS_H_

#include "page.h"
#include "inode.h"
#include "buffer_head.h"
#include "file.h"
#include "namei.h"
#include "read_write.h"
#include "link.h"
#include "bitmap.h"
#include "block.h"
#include "radix-tree.h"

// filesystem magic
#define MAGIC_FS	0x0001
#define NR_SUPER_BLOCK_SECTS		1	//超级块占的扇区个数
#define NR_INODE_MAP_SECTS	1	//inode_map所占的扇区个数
#define NR_ZONE_MAP_SECTS	1	//zone_map所占的扇区个数
#define BITS_SECTOR		SECTOR_SIZE * 8	//每个扇区的位图个数
#define BITS_INODE		NR_INODE_MAP_SECTS * BITS_SECTOR
#define BITS_ZONE		NR_ZONE_MAP_SECTS * BITS_SECTOR
#define NR_INODES		BITS_SECTOR	
#define NR_ZONES		BITS_SECTOR

//inode 占32个字节
//#define INODE_SIZE 	32
#define INODE_SIZE      64	

//all inode sector number
#define INODE_SECTS 		(NR_INODE_MAP_SECTS * BITS_SECTOR)/(SECTOR_SIZE/INODE_SIZE)

//super block 
struct super_block {
    unsigned short s_magic;		//magic 
    unsigned short s_ninodes;		//inode numbER
    unsigned short s_nzones;		//文件系统的数据块数(数据块跟扇区大小一致)
    unsigned short s_nimap_sects;		//inode位图所占的数据块数
    unsigned short s_nzmap_sects;		//数据块位图所占的数据块数
    unsigned short s_firstzone;		//第一个数据块的块号
    unsigned short s_dev;			//所代表的分区
    unsigned int file_max_size;		//文件的最大长度
    struct m_inode root_inode;		//根inode
    struct list_head    s_inodes;   //all inode
    struct list_head    s_dirty;    //dirty inode
    struct list_head    s_io;       //ready write to disk
    struct list_head    s_files;    //all files;
};

struct file_system_type {
    struct super_block *(*read_super) (struct super_block *, void *, int);
    char name[16];
    int requires_dev;
    struct file_system_type * next;
};

struct address_space_operations {
    int (*writepage)(struct page *);
    int (*readpage)(struct file *,struct page *);
    int (*prepare_write)(struct file *, struct page *,unsigned, unsigned);
    int (*commit_write)(struct file *, struct page *, unsigned, unsigned);
};

struct address_space{
    struct inode *host;
    struct radix_tree_root page_tree;
    struct list_head    clean_pages;
    struct list_head    dirty_pages;
    struct address_space_operations *a_ops;
};

#define NR_SUPER	2

extern struct super_block super_block[];

extern int      init_fs();
extern void     mk_fs();

struct super_block * get_super_block(int dev);
extern void  read_super_block(int dev);

#define 	SEL_IN 		1
#define 	SEL_OUT 	2
#define 	SEL_EX 		4

extern  u8 hd_buf[];

#endif

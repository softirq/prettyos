#ifndef     _FS_H_
#define     _FS_H_

#include "inode.h"
#include "buffer_head.h"
#include "file.h"
#include "namei.h"
#include "read_write.h"
#include "link.h"
#include "bitmap.h"

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
#define INODE_SIZE 	32

//所有inode结构所占的扇区个数
#define INODE_SECTS 		(NR_INODE_MAP_SECTS * BITS_SECTOR)/(SECTOR_SIZE/INODE_SIZE)

//超级块结构
struct super_block
{
    unsigned short s_magic;		//magic 
    unsigned short s_ninodes;		//inode numbER
    unsigned short s_nzones;		//文件系统的数据块数(数据块跟扇区大小一致)
    unsigned short s_nimap_sects;		//inode位图所占的数据块数
    unsigned short s_nzmap_sects;		//数据块位图所占的数据块数
    unsigned short s_firstzone;		//第一个数据块的块号
    unsigned short s_dev;			//所代表的分区
    unsigned int file_max_size;		//文件的最大长度
    struct m_inode root_inode;		//根inode
};

struct file_system_type {
    struct super_block *(*read_super) (struct super_block *, void *, int);
    char name[16];
    int requires_dev;
    struct file_system_type * next;
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

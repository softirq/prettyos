#ifndef     _FS_H_
#define     _FS_H_

#define NAME_LEN	12		//文件名的长度
//文件系统magic
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
#define DENTRY_SZIE 	16
//所有inode结构所占的扇区个数
#define INODE_SECTS 		(NR_INODE_MAP_SECTS * BITS_SECTOR)/(SECTOR_SIZE/INODE_SIZE)
//一个目录最多可以拥有的文件或者目录个数
#define MAX_NUM_FILES	1024
//分配给文件默认的扇区数
#define NR_DEFAULT_SECTS	20	//默认分配20个扇区

#define NR_HASH_BUFFER		107
//extern long buffer_memory_start;
struct buffer_head
{
	char *b_data;
	unsigned long b_blocknr;
	unsigned short b_dev;
	unsigned char b_uptodate;
	unsigned char b_dirt;
	unsigned char b_count;
	unsigned char b_lock;
	struct wait_queue  *b_wait;

    /* list */
	struct buffer_head *b_prev;
	struct buffer_head *b_next;

    /* free list  */
	struct buffer_head *b_prev_free; 
	struct buffer_head *b_next_free;
};

//目录项结构
struct dir_entry
{
	u16	inode_num;		//对应的inode号
	char  	file_name[NAME_LEN];	//文件名
};

#define DIR_ENTRY_SIZE  	16
//disk inode节点结构	
struct d_inode
{
	u16	i_mode;			//文件类型和属性
	u16 	i_size;			//文件的大小
//	u16	i_nlinks;		//连接数(有多少个文件目录项指向该i节点)
//先使用简单的方式
//	u16	i_zone[9];		//直接块 间接块 二次间接块
	u16	i_start_sect;		//文件起始块
	u16	i_nr_sects;		//文件数据块数
	
};
//memory inode接点结构
struct m_inode
{
	u16	i_mode;			//文件类型和属性
	u16 	i_size;			//文件的大小
//	u16	i_nlinks;		//连接数(有多少个文件目录项指向该i节点)
	u16	i_start_sect;		//文件起始块
	u16	i_nr_sects;		//文件数据块数

	u16	i_dev;
	u16	i_count;		//当前被使用的次数
	u16	i_num;			//文件的inode号
	u16	i_lock;			//文件的琐标记
	u16	i_dirt;			//脏标记
	struct vm_area_struct *i_mmap;  /* for shm areas,  the list of attaches, otherwise unused. */
};
//超级块结构
struct super_block
{
		u16		s_magic;		//文件系统魔数
		u16		s_ninodes;		//文件系统的节点个数
		u16		s_nzones;		//文件系统的数据块数(数据块跟扇区大小一致)
		u16		s_nimap_sects;		//inode位图所占的数据块数
		u16		s_nzmap_sects;		//数据块位图所占的数据块数
		u16		s_firstzone;		//第一个数据块的块号
		u16		s_dev;			//所代表的分区
		u32		file_max_size;		//文件的最大长度
	struct 	m_inode		root_inode;		//根inode
};

struct file
{
	u16	f_mode;			//文件的操作模式(rwx)
	u16	f_flag;			//文件打开和控制的标志
	u16	f_count;		//对应的文件描述符数
	loff_t	f_pos;			//对应的偏移量
	struct m_inode 	*f_inode;
//	struct 	dir_entry de;		//对应的目录项
	struct file_operations *f_op;
};

struct file_operations
{
	loff_t (*lseek)(struct file *,loff_t,int);
	ssize_t (*read)(struct file *,char *,size_t,loff_t);
	ssize_t (*write)(struct file *,char *,size_t,loff_t *);
	int (*open)(struct m_inode *,struct file *);
	int (*release)(struct m_inode *,struct file *);
	int (*mmap)(struct m_inode *,struct file *,struct vm_area_struct *);
	int (*lock)(struct file *);
};


struct inode_operations
{
	
};

struct dentry_operations
{

};

//每个进程最多打开的文件数
#define NR_OPEN			0x20
//系统同时可以打开的文件个数
#define NR_FILE			0x80	
struct file file_table[NR_FILE];

//第一个inode (即/目录的inode)
#define ROOT_INODE	1
#define NR_INODE	32
extern struct m_inode inode_table[NR_INODE];
extern struct m_inode *root_inode;

#define NR_SUPER	8

#define DE_MATCH	0x0001
#define DE_DEL		0x0002

extern struct super_block super_block[];

/* fs*/
       /*      fs.c    */
extern int      init_fs();
extern void     mk_fs();
       /*      inode.c         */
struct m_inode* iget(int dev,int num);
extern void     iput(struct m_inode *inode);
extern void     write_inode(struct m_inode *inode);
extern int      create_block(struct m_inode *inode,int block);
       /*      super.c         */
struct   super_block * get_super_block(int dev);
extern void     read_super_block(int dev);
       /*      namei.c         */
extern int      open_namei(char *pathname,int flag,int mode,struct m_inode **res_inode);
extern int      add_entry(struct m_inode *dir,int inode_num,char *name);
struct m_inode * namei(char *pathname);
extern int      del_entry(struct m_inode *dir,char *name,int namelen,struct dir_entry **res_de);
struct m_inode * dir_namei(char *pathname,char ** name,int *namelen);
       /*      open.c          */
struct m_inode * create_file(struct m_inode *dir,char *basename,int namelen);
extern int      open(char* filename,int mode,int flag);
extern int      sys_close(unsigned int fd);
       /*      file_dev.c      */
extern int      file_read(struct m_inode *inode,struct file *filp,char *buf,int count);
       /*      read_write.c    */
extern int      sys_read(unsigned int fd,char *buf,int count);
// int sys_open(char* filename,int mode,int flag);
       /*      bitmap.c        */
unsigned short get_imap_bit(int dev);
unsigned short get_zmap_bit(int dev);
extern int clear_imap_bit(int dev,int nr);
extern int clear_zmap_bit(int dev,int nr);
       /*      link.c  */
extern int do_unlink(char *name);

extern void init_buffer(const long buffer_start,const long buffer_end);
struct buffer_head * getblk(int dev,int block);
extern void brelse(struct buffer_head *bh);

#define 	SEL_IN 		1
#define 	SEL_OUT 	2
#define 	SEL_EX 		4

#define 	FILE_READ 	0x01
#define 	FILE_WRITE 	0x02

//sector size 
#define SECTOR_SIZE 	512

//buffer size
#define     BUFFER_SIZE 	SECTOR_SIZE
#define     BUFFER_ALIGN    BUFFER_SIZE 
extern  u8 hd_buf[];

//定义文件类型
#define I_ALL		0X1D00

#define I_REGULAR       0x1000
#define I_BLOCK_SPECIAL 0x0600
#define I_DIRECTORY	0x0400
#define I_CHAR_SPECIAL  0x0200
#define I_NAMED_PIPE	0x0100

#define S_ISBLK(m)	((m & I_ALL) == I_BLOCK_SPECIAL)
#define S_ISCHR(m)	((m & I_ALL) == I_CHAR_SPECIAL)	
#define S_ISDIR(m)	((m & I_ALL) == I_DIRECTORY)	
#define S_ISREG(m)	((m & I_ALL) == I_REGULAR)	
#define S_ISPIFO(m)	((m & I_ALL) == I_NAMED_PIFO)	


#endif

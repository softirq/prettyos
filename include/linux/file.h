#ifndef     _FILE_H_
#define     _FILE_H_

#include "list.h"
#include "inode.h"

/* file type */
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

//one process max open file num
#define NR_OPEN			0x20
//the filesystem could open the file num
#define NR_FILE			0x80	

struct file
{
    unsigned short f_mode;			//文件的操作模式(rwx)
    unsigned short f_flag;			//文件打开和控制的标志
    unsigned short f_count;		//对应的文件描述符数
    loff_t f_pos;			//对应的偏移量
    struct m_inode *f_inode;
    struct file_operations *f_op;
};

struct file_operations
{
    int (*lseek)(int fd, off_t offset ,int origin);
    int (*read)(struct m_inode *inode,struct file *filp,char *buf,int count);
    int (*write)(struct m_inode *inode,struct file *filp,char *buf,int count);
    int (*open)(char *filename, int mode, int flag);
    int (*release)(struct m_inode *,struct file *);
    int (*mmap)(struct m_inode *,struct file *,struct vm_area_struct *);
    int (*lock)(struct file *);
};

extern struct list_head file_lists;
extern unsigned short nr_file_count;

extern struct file_operations general_fop; 
extern int general_read(struct m_inode *inode,struct file *filp,char *buf,int count);
extern int general_write(struct m_inode *inode,struct file *filp,char *buf,int count);
extern int general_lseek(int fd, off_t offset, int origin);
extern struct m_inode * new_file(struct m_inode *dir,char *basename,int namelen, int mode, int flags);
extern int open(char* filename,int mode,int flag);
extern int sys_close(int fd);
extern int close(int fd);

#endif

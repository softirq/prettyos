#include "type.h"
#include "const.h"
#include "sched.h"
#include "stdlib.h"
#include "hd.h"
#include "blk_drv.h"
#include "errno.h"
#include "printf.h"

int sys_read(int fd,char *buf,int count)
{
    u16 mode;
    struct file *fp;
    struct m_inode *inode;
    if(fd >= NR_OPEN || count < 0 || !(fp = current->filp[fd]))
        return -EINVAL;
    inode = fp->f_inode;	
    mode = inode->i_mode;
    /*
       if(S_ISCHR(mode))
       {
    //		rw_char();
    return 0;
    }
    if(S_ISBLK(mode))
    {
    //		block_read();
    return 0;
    }
    */
    printk("read mode = %x",mode);
    if(S_ISDIR(mode) || S_ISREG(mode))
    {
        if((general_file_read(inode,fp,buf,count)) < 0)
            return -3;

        return 0;
    }
    return -4;
}

int sys_write(int fd,char *buf,int count)
{
    u16 mode;
    struct file *fp;
    struct m_inode *inode;
    if(fd <= 0 || fd >= NR_OPEN || count < 0 || !(fp = current->filp[fd]))
    {
        printk("sys_write error.");
        return -EINVAL;
    }

    inode = fp->f_inode;
    mode = inode->i_mode;

    printk("mode=%x.",mode);
    if(S_ISDIR(mode) || S_ISREG(mode))
    {
        if((general_file_write(inode,fp, buf, count)) < 0)
            return -3;
        return 0;
    }
    else
    {
        printk("special file.");
    }

    return -4;	
}

int sys_lseek(int fd,off_t offset,int origin)
{
    return general_file_lseek(fd, offset, origin);
}

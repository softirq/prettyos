#include "type.h"
#include "const.h"
#include "sched.h"
#include "stdlib.h"
#include "hd.h"
#include "blk_drv.h"
#include "errno.h"

int sys_read(unsigned int fd,char *buf,int count)
{
    u16 mode;
    struct file *f;
    struct m_inode *inode;
    if(fd >= NR_OPEN || count < 0 || !(f=current->filp[fd]))
        return -EINVAL;
    if(!count)
        return 0;
    inode = f->f_inode;	
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
    if(S_ISDIR(mode) || S_ISREG(mode))
    {
        file_read(inode,f,buf,count);
        return 0;
    }
    return -EINVAL;
}

int sys_write(unsigned int fd,char *buf,int count)
{
    u16 mode;
    struct file *f;
    struct m_inode *inode;
    if(fd >= NR_OPEN || count < 0 || (f=current->filp[fd]))
        return -EINVAL;
    if(!count)
        return 0;		
    inode = f->f_inode;
    mode = inode->i_mode;
    if(S_ISCHR(mode))
    {
        //		rw_char();
        return 0;
    }
    if(S_ISBLK(mode))
    {
        //		block_write();
        return 0;
    }
    if(S_ISDIR(mode) || S_ISREG(mode))
    {
        //		file_write();
        return 0;
    }
    return -EINVAL;	
}

int sys_lseek(unsigned int fd,off_t offset,int origin)
{
    struct file *f;
    int tmp;
    if(fd >= NR_OPEN || !(f =  current->filp[fd]) || !(f->f_inode))
        return -EBADF;
    switch(origin)
    {
        case 0:		//顶部
            if(offset < 0)	
                return -EINVAL;
            f->f_pos = offset;
            break;
        case 1:		//当前位置
            if(f->f_pos + offset < 0)
                return -EINVAL;
            f->f_pos += offset;
            break;
        case 2:		//尾部
            if((tmp = f->f_inode->i_size + offset) < 0)
                return -EINVAL;
            f->f_pos = tmp;
            break;
        default:
            return -EINVAL;
    }
    return f->f_pos;
}

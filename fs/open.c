#include "type.h"
#include "const.h"
#include "wait.h"
#include "sched.h"
#include "panic.h"
#include "stdlib.h"
#include "hd.h"
#include "mm.h"
#include "blk_drv.h"
#include "file.h"
#include "printf.h"

//创建一个新的文件/目录
struct m_inode * create_file(struct m_inode *dir,char *basename,int namelen)
{
    int inode_nr = get_imap_bit(dir->i_dev);
    printk("create_file:inode_nr = %d\n",inode_nr);
    if(inode_nr <= 0)
    {
        disp_str("there is no free inode 1 \n");
        return NULL;
    }
    struct m_inode *inode = iget(dir->i_dev,inode_nr);
    if(!inode)
    {
        disp_str("there is no free inode 2 \n");
    }
    add_entry(dir,inode->i_num,basename);
    inode->i_mode = dir->i_mode;
    inode->i_dirt = 1;
    disp_str("create file successful \n");
    return inode;
}

static struct file * get_empty_filp()
{
    struct file *fp = NULL;

    if(nr_file_count++ >= NR_OPEN)
        return NULL;

    fp = (struct file *)kmem_get_obj(file_cachep);

    return fp;
}

int open(char* filename,int mode,int flag)
    //int sys_open(char* filename,int mode,int flag)
{
    struct m_inode *inode;

    struct file *fp;
    int i;
    int fd = -1;                    //句柄
    //搜索没用过的句柄
    for(i = 0;i < NR_OPEN;i++)
    {       
        if(current->filp[i] == NULL)
        {
            fd = i;
            break;
        }

    }
    if(fd < 0 || fd > NR_OPEN)
    {
        /*panic("filp is full (PID %d)\n",proc2pid(current)); */
    }

    fp = get_empty_filp();
    if(fp == NULL)
        return -2;

    (current->filp[fd] = fp)->f_count++;
    //	printk("root_inode->i_dev = %d\n",root_inode->i_dev);
    if((i = open_namei(filename,mode,flag,&inode)) != 0)
    {
        current->filp[fd] = NULL;
        fp->f_count = 0;
        return i;
    }
    //返回文件句柄
    //	printk("open inode->i_num = %d\n",inode->i_num);
    fp->f_inode = inode;
    fp->f_count = 1;
    fp->f_flag = flag;
    fp->f_mode = mode;
    fp->f_pos = 0;
    //	printk("open:fd =  %d\n",fd);
    return fd;
}

int sys_close(unsigned int fd)
{
    struct file *filp;
    if(fd >= NR_OPEN)
        return -1;
    if(!(filp = current->filp[fd]))
        return -1;
    if(filp->f_count == 0)
        panic("Close:file count is 0");
    if(--filp->f_count)
        return 0;
    iput(filp->f_inode);
    return 0;
}


#include "type.h"
#include "const.h"
/*#include "wait.h"*/
#include "stdlib.h"
#include "hd.h"
#include "blk_drv.h"
#include "fcntl.h"
#include "errno.h"
#include "file.h"

struct list_head file_lists;
unsigned short nr_file_count = 0;

//读文件
int file_read(struct m_inode *inode,struct file *filp,char *buf,int count)
{
    int i,c,m = 0;
    char *p;
    //	char *ptr = buf;
    struct buffer_head *bh;
    struct dir_entry *de;
    if(!inode || !filp)
        return -EINVAL;
    if(!buf || count <= 0)
        return 0;
    int nr_start_sect = inode->i_start_sect;
    //	printk("inode->i_dev = %d inode->i_num = %d inode->i_start_sect = %d\n",inode->i_dev,inode->i_num,inode->i_start_sect);
    //文件所占的磁盘块总数
    off_t pos = filp->f_pos;			
    //开始读取的磁盘块
    nr_start_sect +=  pos/SECTOR_SIZE;
    //	nr_sects = (inode->i_size + SECTOR_SIZE -1)/SECTOR_SIZE;
    while(i < count)
    {
        c = pos % SECTOR_SIZE;
        c = SECTOR_SIZE -c ;
        if(c > count)
        {
            c = count - i;
        }
        //		printk("file_read : inode->i_dev = %d nr_start_sect = %d count = %d\n",inode->i_dev,nr_start_sect + pos/SECTOR_SIZE,count);
        bh = getblk(inode->i_dev,nr_start_sect + pos/SECTOR_SIZE);
        hd_rw(inode->i_dev,nr_start_sect + pos/SECTOR_SIZE,1,ATA_READ,bh);		
        p = bh->b_data + pos;
        de = (struct dir_entry*)bh->b_data;
        //	printk("de->file_name = %s\n",(++de)->file_name);
        filp->f_pos += c;	
        pos += c;
        i += c;
        m++;
        //	printk("c = %d\n",c);
        while(c-- > 0)
        {
            if(*p == 0)
            {
                p++;
                continue;
            }
            else
            {
                *buf = *p;
                //				printk("%d  %c %c\t",512 - c,*p,*buf);
                buf++;
                p++;
            }
        }
        brelse(bh);
    }
    return 0;
}

int file_write(struct m_inode *inode,struct file *filp,char *buf,int count)
{
    int i = 0;
    int block,c;
    off_t pos;
    unsigned char *p;
    if(!inode || !filp)
        return -EINVAL;
    if(!buf || count <= 0)
        return 0;
    //	p = buf;
    if(filp->f_flag & O_APPEND)
        pos = inode->i_size;
    else 
        pos = filp->f_pos;
    while(i < count)
    {
        if(!(block = create_block(inode,pos/SECTOR_SIZE)))
            break;
        c = pos % SECTOR_SIZE;	
        c = SECTOR_SIZE -c;
        if(c > count) 
            c = count - i;	
        p = hd_buf + pos;			
        pos += c;
        if(pos > inode->i_size)
        {
            inode->i_size = pos;
            inode->i_dirt = 1;
        }
        i += c;
        while(c-- > 0)
            *(p++) = *(buf++);
    }
    return 0;	
}


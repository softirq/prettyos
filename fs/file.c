#include "type.h"
#include "const.h"
#include "sched.h"
#include "stdlib.h"
#include "hd.h"
#include "blk_drv.h"
#include "fcntl.h"
#include "errno.h"
#include "file.h"
#include "printf.h"

/*struct list_head file_lists;*/
unsigned short nr_file_count = 0;

//读文件
int general_file_read(struct m_inode *inode,struct file *filp,char *buf,int count)
{
    int i,c,m = 0;
    char *p;
    //	char *ptr = buf;
    struct buffer_head *bh;
    struct dir_entry *de;
    int block_nr = 0;

    if(!inode || !filp)
        return -EINVAL;
    if(!buf || count <= 0)
        return 0;
    int nr_start_sect = get_first_block(inode); 
    printk("read start_sect = %d.",nr_start_sect);
    //	printk("inode->i_dev = %d inode->i_num = %d inode->i_start_sect = %d\n",inode->i_dev,inode->i_num,inode->i_start_sect);
    //文件所占的磁盘块总数
    off_t pos = filp->f_pos;			
    //开始读取的磁盘块
    nr_start_sect +=  pos/SECTOR_SIZE;
    //	nr_sects = (inode->i_size + SECTOR_SIZE -1)/SECTOR_SIZE;
    while(i < count)
    {
        block_nr = get_pos_block(inode, pos);
        c = pos % SECTOR_SIZE;
        c = SECTOR_SIZE -c ;
        if(c > count)
        {
            c = count - i;
        }
        //		printk("file_read : inode->i_dev = %d nr_start_sect = %d count = %d\n",inode->i_dev,nr_start_sect + pos/SECTOR_SIZE,count);
        bh = getblk(inode->i_dev,block_nr);
        hd_rw(inode->i_dev,block_nr,1,ATA_READ,bh);		
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

int general_file_write(struct m_inode *inode,struct file *filp,char *buf,int count)
{
    int i = 0,block_nr,c;
    off_t pos = 0;
    char *p = NULL;
    struct buffer_head *bh = NULL;
    int nr_start_sect = 0; 

    printk("write 5");
    if(!inode || !filp || !buf || count < 0)
    {
        printk("write parameters error.");
        return -EINVAL;
    }

    /*int nr_start_sect = inode->i_start_sect;*/
    if((nr_start_sect = get_first_block(inode)) == 0)
    {
        set_block_nums(inode->i_dev ,(struct d_inode *)inode, NR_DEFAULT_SECTS);
        if((nr_start_sect = get_first_block(inode)) == 0)
            return -3;
    }
    printk("write start_sect = %d.",nr_start_sect);
    if(filp->f_flag & O_APPEND)
        pos = inode->i_size;
    else 
        pos = filp->f_pos;

    while(i < count)
    {
        /*block = nr_start_sect + pos/SECTOR_SIZE;*/
        block_nr = get_pos_block(inode, pos);
        bh = getblk(inode->i_dev,block_nr);

        c = pos % SECTOR_SIZE;	
        p = bh->b_data + c;			

        c = SECTOR_SIZE -c;
        if(c > count) 
            c = count - i;	

        pos += c;
        i += c;

        if(pos > inode->i_size)
        {
            inode->i_size = pos;
            inode->i_dirt = 1;
        }
        while(c-- > 0)
            *(p++) = *(buf++);

        hd_rw(inode->i_dev,block_nr,1,ATA_WRITE,bh);		
        brelse(bh);
    }

    return 0;	
}

int general_file_lseek(int fd, off_t offset, int origin)
{
    struct file *fp;
    int tmp;

    if(fd <= 0 || fd >= NR_OPEN || !(fp =  current->filp[fd]) || !(fp->f_inode))
        return -EBADF;
    switch(origin)
    {
        case 0:		//顶部
            if(offset < 0)	
                return -EINVAL;
            fp->f_pos = offset;
            break;
        case 1:		//当前位置
            if(fp->f_pos + offset < 0)
                return -EINVAL;
            fp->f_pos += offset;
            break;
        case 2:		//尾部
            if((tmp = fp->f_inode->i_size + offset) < 0)
                return -EINVAL;
            fp->f_pos = tmp;
            break;
        default:
            return -EINVAL;
    }
    return fp->f_pos;

}

struct file_operations general_fop = {
    .lseek = general_file_lseek,
    .read = general_file_read,
    .write = general_file_write,
    .open = open,
};

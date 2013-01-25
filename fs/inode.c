#include "type.h"
#include "const.h"
#include "string.h"
#include "stdlib.h"
#include "wait.h"
#include "panic.h"
#include "hd.h"
#include "blk_drv.h"
#include "mm.h"
#include "printf.h"

struct list_head inode_lists;
unsigned short nr_inodes_count  = 0;

void sync_dev(int dev)
{
}

/*write inode to disk*/
int write_inode(struct m_inode *inode)
{
    if(inode == NULL || !inode->i_dirt || !inode->i_dev)
        return -1;

    unsigned short dev = inode->i_dev;
    unsigned short num = inode->i_num;
    struct buffer_head *bh = NULL;
    struct super_block *sb = get_super_block(dev);

    int blk_nr = 1 + NR_SUPER_BLOCK_SECTS + sb->s_nimap_sects + sb->s_nzmap_sects + \
                 (num -1)/(SECTOR_SIZE/INODE_SIZE);

    bh = getblk(dev,blk_nr);
    if(bh == NULL)
        return -2;

    if((hd_rw(dev,blk_nr,1,ATA_READ,bh)) < 0)
        return -3;

    *(struct d_inode *)(bh->b_data + ((num -1)%(SECTOR_SIZE/INODE_SIZE)) * INODE_SIZE) = *(struct d_inode *)inode;	

    if((hd_rw(dev,blk_nr,1,ATA_WRITE,bh)) > 0)
        return -4;

    brelse(bh);

    inode->i_dirt = 0;

    return 0;
}

static int read_inode(struct m_inode* inode)
{
    if(inode == NULL)
        return -1;

    u16 dev = inode->i_dev;
    u16 num = inode->i_num;

    //	struct d_inode *tmp;
    struct buffer_head *bh = NULL;
    struct super_block *sb = get_super_block(dev);
    if(sb == NULL)
        return -2;

    int blk_nr = 1 + NR_SUPER_BLOCK_SECTS + sb->s_nimap_sects + sb->s_nzmap_sects + \
                 (num -1)/(SECTOR_SIZE / INODE_SIZE);
    bh = getblk(dev,blk_nr);
    if(bh == NULL)
        return -3;

    //将inode信息从磁盘读入到内存中
    if((hd_rw(ROOT_DEV,blk_nr,1,ATA_READ,bh)) < 0)
        return -4;

    //gcc 3.4.3之后不支持左值强制类型转换
    *((struct d_inode *)inode) = *((struct d_inode *)(bh->b_data+ ((num -1)%(SECTOR_SIZE/INODE_SIZE)) * INODE_SIZE));
    brelse(bh);

    return 0;
}

struct m_inode* get_empty_inode(int dev)
{
    struct m_inode *inode;
    struct super_block *sb = get_super_block(dev);

    if(nr_inodes_count++ >= sb->s_ninodes)
    {
        return NULL;
    }

    inode = (struct m_inode *)kmem_get_obj(inode_cachep);
    if(inode == NULL)
    {
        return NULL;
    }

    memset((char *)inode,0,sizeof(struct m_inode));
    inode->i_count = 1;
    inode->i_dev = dev;
    inode->i_ops = &pfs;
    list_add(&(inode->list), &inode_lists);

    return inode;
}

static int _bmap(struct m_inode *inode,int block,int flag)
{
    if(block < 0)
        panic("_bmap : block < 0");	
    return 0;	
}
int bmap(struct m_inode *inode,int block)
{
    return _bmap(inode,block,0);
}

int create_block(struct m_inode *inode,int block)
{
    return _bmap(inode,block,1);
}

//从设备dev上查找num号的inode
struct m_inode* iget(int dev,int num)
{
    struct m_inode *inode;
    if(!dev)
    {
        panic("iget with dev == 0\n");
    }

    struct list_head *head, *pos, *n;
    head = &(inode_lists);
    if(list_empty_careful(head))
    {
        /*printk("1");*/
    }
    else
    {
        list_for_each_safe(pos, n, head)
        {
            inode = list_entry(pos, struct m_inode, list);
            if(inode->i_dev == dev && inode->i_num == num)
            {
                return inode;
            }
            else
            {
            }
        }
    }

    inode = get_empty_inode(dev);
    if(inode == NULL)
    {
        return NULL;
    }

    /*inode = empty;*/
    /*get_block_nums(dev, (struct d_inode *)inode, NR_DEFAULT_SECTS);*/

    if((read_inode(inode)) < 0)
        return NULL;

    inode->i_num = num;

    /*inode->i_start_sect = nr_sectors;*/
    /*inode->i_nr_sects = NR_DEFAULT_SECTS;*/
    /*nr_sectors = NR_DEFAULT_SECTS + 1;*/
    /*list_add(&(inode->list), &inode_lists);*/

    return inode;
}

static int free_inode(struct m_inode *inode)
{
    struct super_block *sb;
    if(!inode)
        return -1;
    if(inode->i_count > 1)
    {
        printk("trying to free inode with count = %d\n",inode->i_count);
        panic("free_inode");
    }
    if(!(sb = get_super_block(inode->i_dev)))
        panic("trying to free inode on nonexistent device");
    if(inode->i_num < 1 ||  inode->i_num > sb-> s_ninodes)
        panic("trying to free inode 0 or nonexistant inode");
    if(!inode->i_dev)
    {
        list_del(&(inode->list));
        clear_imap_bit(inode->i_dev,inode->i_num);
        memset((char *)inode,0,sizeof(struct m_inode));
        //加入释放盘块的操作 free_block(inode);
        return 0;
    }
    return -1;
}

/*free the inode*/
void iput(struct m_inode *inode)
{
    if(!inode)
        return;
    if(!inode->i_count)
    {
        //		panic("iput;trying to free free inode");
        return;
    }
    if(inode == root_inode)
    {
        return;
    }
    if(!inode->i_dev)
    {
        inode->i_count--;
        return;
    }
    if(S_ISBLK(inode->i_mode) || S_ISCHR(inode->i_mode))
    {
        //		sync_dev(inode->i_start_sect);
    }
    if((--inode->i_count) > 0)
    {
        //		inode->i_count--;
        return;
    }
    else
    {
        free_inode(inode);	
    }
    if(inode->i_dirt)
    {
        printk("----");
        write_inode(inode);		
    }
    return;
}

struct inode_operations pfs = {
    .lookup = iget,
    .write = write_inode,
    .release = iput,
};

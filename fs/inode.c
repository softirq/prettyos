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

/*struct list_head inode_lists;*/
unsigned short nr_inodes_count  = 0;

void sync_dev(int dev)
{
}

/*write inode to disk*/
int write_inode(struct m_inode *inode)
{
    if(inode == NULL || !inode->i_dirty || !inode->i_dev)
        return -1;

    unsigned short dev = inode->i_dev;
    unsigned short num = inode->i_num;
    struct buffer_head *bh = NULL;
    struct super_block *sb = get_super_block(dev);

    int blk_nr = 1 + NR_SUPER_BLOCK_SECTS + sb->s_nimap_sects + sb->s_nzmap_sects + \
                 (num -1)/(SECTOR_SIZE/INODE_SIZE);

    printk("write inode blk_nr = %d.", blk_nr);
    printk("write num = %d.",num);
    bh = getblk(dev,blk_nr);
    if(bh == NULL)
        return -2;

    if((hd_rw(dev,blk_nr,1,ATA_READ,bh)) < 0)
        return -3;

    *(struct d_inode *)(bh->b_data + ((num -1)%(SECTOR_SIZE/INODE_SIZE)) * INODE_SIZE) = *(struct d_inode *)inode;	

    if((hd_rw(dev,blk_nr,1,ATA_WRITE,bh)) > 0)
        return -4;

    brelse(bh);

    inode->i_dirty = 0;

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

    printk("read inode blk_nr = %d.", blk_nr);
    printk("read num = %d.",num);
    bh = getblk(dev,blk_nr);
    if(bh == NULL)
        return -3;

    //read inode from disk to memory
    if((hd_rw(ROOT_DEV,blk_nr,1,ATA_READ,bh)) < 0)
        return -4;

    //gcc 3.4.3之后不支持左值强制类型转换
    *((struct d_inode *)inode) = *((struct d_inode *)(bh->b_data + ((num -1)%(SECTOR_SIZE/INODE_SIZE)) * INODE_SIZE));
    brelse(bh);

    inode->i_num = num;
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
    list_add(&(inode->i_list), &sb->s_inodes);
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
    struct super_block *sb = get_super_block(dev);
    head = &(sb->s_inodes);
    if(list_empty_careful(head))
    {
        /*printk("1");*/
    }
    else
    {
        list_for_each_safe(pos, n, head)
        {
            inode = list_entry(pos, struct m_inode, i_list);
            if(inode->i_dev == dev && inode->i_num == num)
            {
                return inode;
            }
            else
            {
            }
        }
    }

    if((inode = get_empty_inode(dev)) == NULL)
    {
        return NULL;
    }

    inode->i_ops = &pfs;
    inode->i_num = num;

    if((read_inode(inode)) < 0)
        return NULL;

    printk("iget mode=%x",inode->i_mode);

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
        list_del(&(inode->i_list));
        clear_imap_bit(inode->i_dev,inode->i_num);
        memset((char *)inode,0,sizeof(struct m_inode));
        //加入释放盘块的操作 free_block(inode);
        return 0;
    }
    return -1;
}

/*free the inode*/
int iput(struct m_inode *inode)
{
    if(!inode)
        return -1;
    if(!inode->i_count)
    {
        return -2;
    }
    if(inode == root_inode)
    {
        return -3;
    }
    if(!inode->i_dev)
    {
        return -4;
    }
    if(S_ISBLK(inode->i_mode) || S_ISCHR(inode->i_mode))
    {
        return -5;
    }

    if((--inode->i_count) > 0)
    {
        return 0;
    }
    else
    {
        if(inode->i_dirty)
        {
            write_inode(inode);		
        }

        list_del(&inode->i_list);
        free_inode(inode);	
    }

    return 0;
}

struct inode_operations pfs = {
    .lookup = iget,
    .write = write_inode,
    .release = iput,
};

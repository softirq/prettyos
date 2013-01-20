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
#include "fs.h"

int nr_sectors = 0;

static int sync_block(int dev,int nr,struct buffer_head *bh)
{
    if(bh)
    {
        bh->b_dirt = 0;
        hd_rw(dev,nr,1,ATA_WRITE,bh);
        return 0; 
    }
    return -1;
}

static int new_block(int dev)
{
    struct super_block *sb = NULL;
    int block_nr;

    if((sb = get_super_block(dev)) == NULL)
        panic("triny to get new block from nonexistant device");

    block_nr = sb->s_firstzone + get_zmap_bit(dev);
    printk(".%d.",block_nr);
    if(block_nr <= 0)
    {
        printk("there is no free block");
        return -1;
    }	
    else
        return block_nr;
}

static int free_block(int dev,int nr)
{
    if(nr <= 0)
        return -1;
    struct super_block *sb;
    struct buffer_head *bh;
    if(!(sb = get_super_block(dev)))
    {
        panic("trying to free inode on nonexistent device");
        panic("free_block");
    }
    if(nr < 1 || nr > sb->s_nzones)
        panic("trying to free inode 0 or nonexistant inode");
    bh = getblk(dev,nr);
    if(!dev)
    {
        clear_zmap_bit(dev,nr);
        if(!bh)
        {
            if(bh->b_dirt)	
                sync_block(dev,nr,bh);
        }
        return 0;
    }
    return -1;
}

/* get direct block nr mmap */
static int get_frist_block_nr(int dev, struct d_inode *inode)
{
    int i,nr;

    for(i = 0;i < NR_DEFAULT_SECTS;++i)
    {
        if(inode->i_data[i] == 0)
        {
            if((nr = new_block(dev)) < 0)
                return -1;

            inode->i_data[i] = nr;
            return 0;
        }
    }
    return -1;
}

/* get second block nr map */
static int get_second_block_nr(int dev, struct d_inode *inode)
{
    int i,nr, blk_nr;
    int *i_array;
    struct buffer_head *bh;

    blk_nr = inode->i_data[11];
    bh = getblk(dev,blk_nr);
    if(bh == NULL)
        return -2;

    if((hd_rw(dev,blk_nr,1,ATA_READ,bh)) < 0)
        return -3;

    i_array = (int *)bh->b_data;
    for(i = 0;i < SECTOR_SIZE/sizeof(int);++i)
    {
        if(i_array[i] == 0)
        {
            if((nr = new_block(dev)) < 0)
                return -4;
            i_array[i] = nr;

            if((hd_rw(dev,blk_nr,1,ATA_WRITE,bh)) > 0)
                return -5;

            brelse(bh);
            return 0;
        }
    }

    return -1;
}

/* get third block nr map */
static int get_third_block_nr(int dev, struct d_inode *inode)
{
    int i,k,nr, blk_nr;
    struct buffer_head *bh, *sbh;
    int *i_array, *i_sarray;

    blk_nr = inode->i_data[12];
    bh = getblk(dev,blk_nr);

    if(bh == NULL)
        return -2;

    if((hd_rw(ROOT_DEV,blk_nr,1,ATA_READ,bh)) < 0)
        return -3;

    i_array = (int *)bh->b_data;
    for(i = 0;i < SECTOR_SIZE/sizeof(int);++i)
    {
        if(i_array[i] == 0)
        {
            continue;
        }
        else
        {
            blk_nr = i_array[i];
            sbh = getblk(dev, blk_nr);
            if((hd_rw(ROOT_DEV,blk_nr,1,ATA_READ,sbh)) < 0)
                return -3;

            i_sarray = (int *)sbh->b_data;
            for(k = 0; k < SECTOR_SIZE/sizeof(int);++k)
            {
                if(i_sarray[k] == 0)
                {
                    if((nr = new_block(dev)) < 0)
                        return -4;
                    i_sarray[k] = nr;

                    if((hd_rw(ROOT_DEV,blk_nr,1,ATA_WRITE,sbh)) > 0)
                        return -5;

                    brelse(sbh);
                    brelse(bh);
                    return 0;
                }
            }
        }
    }

    return -1;
}

/* get one block from system*/
int get_block_nr(int dev, struct d_inode *inode)
{
    if(get_frist_block_nr(dev, inode) == 0)
    {
        return 0;
    }

    if(get_second_block_nr(dev, inode) == 0)
    {
        return 0;
    }

    if(get_third_block_nr(dev, inode) == 0)
    {
        return 0;
    }

    return -2;
}

/* get num block the first stored in block_nr
 * and linked to a list*/
int get_block_nums(int dev, struct d_inode *inode, int num)
{
    int i = 0;
    for(;i < num; ++i)
    {
        if(get_block_nr(dev, inode) < 0)
            return -2;
    }

    return 0;
}

/* return the first block number */
inline int get_first_block(struct d_inode *inode)
{
    return inode->i_data[0];
}

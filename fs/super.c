#include "type.h"
#include "const.h"
#include "wait.h"
#include "panic.h"
#include "hd.h"
#include "blk_drv.h"

struct super_block super_block[NR_SUPER];

void read_super_block(int dev)
{
    int i;
    struct super_block *sb;
    struct buffer_head *bh;
    //找到一个空闲的super block
    for(i = 0;i < NR_SUPER;i++)	
    {
        if(super_block[i].s_dev == NO_DEV)
            break;
    }
    bh = getblk(ROOT_DEV,1);	
    hd_rw(ROOT_DEV,1,1,ATA_READ,bh);
    sb = (struct super_block *)(bh->b_data);
    //	printk("========================%d\n",sb->s_nimap_sects);
    super_block[i] = *sb;
    super_block[i].s_dev = dev;
    super_block[i].s_magic = MAGIC_FS;	
    brelse(bh);
}

struct super_block * get_super_block(int dev)
{
    struct super_block *sb = NULL;
repeat:
    sb = super_block;
    for(;sb < super_block + NR_SUPER;sb++)
    {
        if(sb->s_dev == dev)
            return sb;
    }
    read_super_block(ROOT_DEV);

    /*panic("super block of device %d not found\n",dev);*/
    return 0;
}

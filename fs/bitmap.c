#include "type.h"
#include "const.h"
#include "traps.h"
//#include "string.h"
#include "tty.h"
#include "console.h"
#include "wait.h"
#include "mm.h"
#include "sched.h"
#include "global.h"
#include "kernel.h"
#include "lib.h"
#include "hd.h"
#include "blk_drv.h"


static int find_first_zero(unsigned char ch)
{
	unsigned int _res;
	__asm__ (
		"cld\n\t"
		"bsf %%eax,%%ecx\n\t"
		:"=c"(_res)
		:"a"((unsigned int)ch),"c"(0)
	);	
	return _res;
}
static int clear_bit(int nr,unsigned char ch)
{
	unsigned int _res;
	__asm__ (
		"cld\n\t"
		"btr %%ecx,%%eax\n\t"
		:"=a"(_res)
		:"c"(nr),"a"((unsigned int)ch)
	);	
	return _res;
}

int clear_imap_bit(int dev,int nr)
{
	struct buffer_head *bh;
	struct super_block *sb = get_super_block(dev);	
	int blk_nr = 1 + NR_SUPER_BLOCK_SECTS + nr /INODE_SECTS;
	if(blk_nr > (1 + NR_SUPER_BLOCK_SECTS + sb->s_nimap_sects))
	{
		return -1;
	}
	int char_nr = nr / 8;
	bh = getblk(dev,blk_nr);
	hd_rw(ROOT_DEV,blk_nr,1,ATA_READ,bh);
	clear_bit(nr%8,(bh->b_data)[char_nr]);
	hd_rw(ROOT_DEV,blk_nr,1,ATA_WRITE,bh);
	brelse(bh);
	return 0;	
}

int clear_zmap_bit(int dev,int nr)
{
	struct buffer_head *bh;
	struct super_block *sb = get_super_block(dev);	
	int blk_nr = 1 + NR_SUPER_BLOCK_SECTS + NR_INODE_MAP_SECTS +  nr /INODE_SECTS;
	if(blk_nr > (1 + NR_SUPER_BLOCK_SECTS + NR_INODE_MAP_SECTS + sb->s_nzmap_sects))
	{
		return -1;
	}
	int char_nr = nr / 8;
	bh = getblk(dev,blk_nr);
	hd_rw(ROOT_DEV,blk_nr,1,ATA_READ,bh);
	clear_bit(nr%8,(bh->b_data)[char_nr]);
	hd_rw(ROOT_DEV,blk_nr,1,ATA_WRITE,bh);
	brelse(bh);
	return 0;	
}

unsigned short get_imap_bit(int dev)
{
	int i,j;
	int ret = -1;
	int inode_nr = -1;
	unsigned char ch;
//	printk("%d\n",dev);
	struct buffer_head *bh;
	struct super_block *sb = get_super_block(dev);
	int blk_nr = 1 + NR_SUPER_BLOCK_SECTS;
	for(i = 0;i < sb->s_nimap_sects;i++)
	{		
		bh = getblk(dev,blk_nr + i);
		hd_rw(ROOT_DEV,blk_nr + i,1,ATA_READ,bh);
		for(j = 0; j < SECTOR_SIZE;j++)
		{
			ch = (bh->b_data)[j];
			if(ch == 0xff)
				continue;
			else
			{
				ch = ~ch;
		//		ch =~((bh->b_data)[j]) & 0xff;			
				ret = find_first_zero(ch);
				//没有找到或者为第一个inode节点
				if(ret < 0 || ret > 7 || (i == 0 && ret == 0))
				{
					disp_str("no inode is free\n");
				}
				else
				{
					inode_nr = (i * SECTOR_SIZE + j) * 8 + ret;
					if(inode_nr >= BITS_INODE)
						return 0;
					(bh->b_data)[j] |= (1 << ret); 
					hd_rw(ROOT_DEV,blk_nr + i,1,ATA_WRITE,bh);
					brelse(bh);
					break;
				}
			}	
			if(inode_nr < 1)
				return 0;
			else
				return inode_nr;
		}	
    }
    return inode_nr;
}
unsigned short get_zmap_bit(int dev)
{
    int i,j;
    int ret = -1;
    int block_nr = 0;
    unsigned char ch;
    struct super_block *sb = get_super_block(dev);
    struct buffer_head *bh;
    int blk_nr = 1 + NR_SUPER_BLOCK_SECTS + sb->s_nimap_sects;
    for(i = 0;i < sb->s_nzmap_sects;i++)
    {
        bh = getblk(dev,blk_nr + i);
        hd_rw(dev,blk_nr + i,1,ATA_READ,bh);
        for(j = 0; j < SECTOR_SIZE;j++)
        {
            ch = (bh->b_data)[j];
            if(ch == 0xff)
                continue;
            else
            {
                ch = ~ch;
                //	ch =~(bh->b_data)[j] & 0xff;			
                ret = find_first_zero(ch);
                if(ret < 0 || ret > 7|| (i == 0 && ret == 0))	
                {
                    disp_str("no block is free\n");
                }
                else
                {
                    block_nr = (SECTOR_SIZE * i + j) * 8 + ret;
                    if(block_nr >= BITS_ZONE)
                    {
                        return 0;
                    }
                    (bh->b_data)[j] |= (1 << ret);
                    hd_rw(ROOT_DEV,blk_nr + i,1,ATA_WRITE,bh);
                    brelse(bh);
                    break;
                }
            }
            if(block_nr < 1)
                return 0;
            else
                return block_nr;
        }
    }
    return 0;
}

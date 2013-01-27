#include "type.h"
#include "const.h"
#include "console.h"
#include "wait.h"
#include "stdlib.h"
#include "hd.h"
#include "blk_drv.h"
#include "fcntl.h"
#include "string.h"
#include "printf.h"
#include "panic.h"
#include "mm.h"

struct m_inode *root_inode = NULL;

/* make filesystem */
void mk_fs()
{
    int i;
    struct super_block sb;
    struct buffer_head *bh;

    printk("Initialize PFS.");
    sb.s_dev = ROOT_DEV;
    sb.s_magic = MAGIC_FS;
    sb.s_ninodes = NR_INODES;
    sb.s_nzones = NR_ZONES;
    sb.s_nimap_sects = NR_INODE_MAP_SECTS;
    sb.s_nzmap_sects = NR_ZONE_MAP_SECTS;
    sb.s_ninodes = sb.s_nimap_sects * BITS_SECTOR;
    sb.s_nzones = sb.s_nzmap_sects * BITS_SECTOR;
    sb.s_firstzone = 1 + NR_INODE_MAP_SECTS + NR_ZONE_MAP_SECTS + INODE_SECTS + 1;
    //每个文件最大2个扇区，即1K=1024个字节
    sb.file_max_size = SECTOR_SIZE *2;
    //	super block
    //超级块占1个扇区
    bh = getblk(ROOT_DEV,1);
    memset(bh->b_data,0x00,SECTOR_SIZE);	
    memcpy(bh->b_data,&sb,sizeof(struct super_block));

    //将超级块写入到ROOT_DEV分区(在此为第一个逻辑分区)中
    hd_rw(ROOT_DEV,1,1,ATA_WRITE,bh);
    brelse(bh);
    read_super_block(ROOT_DEV);
    //	inode map

    //初始化inode_map 占NR_INODE_MAP_SECTS个扇区
    bh = getblk(ROOT_DEV,1 + NR_SUPER_BLOCK_SECTS);
    memset(bh->b_data,0x0,SECTOR_SIZE);	
    //0号inode保留 1号inode为根目录 然后依次为NR_CONSOLES个控制文件tty0 tty1 tty2
    for(i = 0;i < NR_CONSOLES + 2;i++)
    {
        (bh->b_data)[0] |= (1 << i);
    }

    if((bh->b_data)[0] != 0x1f)
    {
        panic("fs inode_map initialize error\n");
    }
    hd_rw(ROOT_DEV,1 + NR_SUPER_BLOCK_SECTS,1,ATA_WRITE,bh);

    //	memset(hd_buf,0x0,SECTOR_SIZE * NR_INODE_MAP_SECTS);	
    memset(bh->b_data,0x0,SECTOR_SIZE);	
    for(i = 1;i < sb.s_nimap_sects;i++)
    {
        hd_rw(ROOT_DEV,1 + NR_SUPER_BLOCK_SECTS + i,1,ATA_WRITE,bh);
    }
    brelse(bh);
    /*set_imap_bit(ROOT_DEV, 0);*/

    //	zone map
    //	memset(hd_buf,0,SECTOR_SIZE);
    //第0号盘块被保留 从第1号盘块号开始
    bh = getblk(ROOT_DEV,1 + NR_SUPER_BLOCK_SECTS + NR_INODE_MAP_SECTS);	
    /*memset(bh->b_data, 0xff, SECTOR_SIZE);*/
    /*int nr_sects = NR_DEFAULT_SECTS + 1;*/
    bh->b_data[0] = 0x01;
    hd_rw(ROOT_DEV,1 + NR_SUPER_BLOCK_SECTS + NR_INODE_MAP_SECTS,1,ATA_WRITE,bh);

    memset(bh->b_data,0x0,SECTOR_SIZE);
    for(i = 1; i < NR_INODE_MAP_SECTS;i++)
    {
        hd_rw(ROOT_DEV,1 + NR_SUPER_BLOCK_SECTS + NR_INODE_MAP_SECTS + i,1,ATA_WRITE,bh);
    }
    brelse(bh);

    //	inode
    bh = getblk(ROOT_DEV,1 + NR_SUPER_BLOCK_SECTS + NR_INODE_MAP_SECTS + NR_ZONE_MAP_SECTS);	
    memset(bh->b_data,0,SECTOR_SIZE);
    /* root inode */
    struct	d_inode  *dinode = (struct d_inode *)(bh->b_data);
    dinode->i_mode = I_DIRECTORY;
    dinode->i_size = DENTRY_SIZE;
    /*inode->i_start_sect = sb.s_firstzone;*/
    /*inode->i_nr_sects = NR_DEFAULT_SECTS;*/
    set_block_nums(ROOT_DEV,dinode, NR_DEFAULT_SECTS);
    /*nr_sectors = sb.s_firstzone;*/
    //	dinode->i_nlinks = 1;
    /*for(i = 0;i < NR_CONSOLES;i++)*/
    /*{*/
        /*dinode++;*/
        /*dinode->i_mode = I_CHAR_SPECIAL;*/
        /*dinode->i_size = dinode->i_nr_sects = 0;*/
        /*dinode->i_start_sect = MAKE_DEV(DEV_TTY,i);*/
    /*}*/

    hd_rw(ROOT_DEV,1 + NR_SUPER_BLOCK_SECTS + NR_INODE_MAP_SECTS + NR_ZONE_MAP_SECTS,1,ATA_WRITE,bh);
    brelse(bh);
    //	zone
    //初始化根目录中的数据。根目录是个目录，存放的是目录项 4个文件 .  tty0 tty1 tty2
    printk("s_firstzone=%d.",sb.s_firstzone);
    bh = getblk(ROOT_DEV,sb.s_firstzone);
    memset(bh->b_data,0,SECTOR_SIZE);
    struct dentry *de = (struct dentry*)(bh->b_data);
    de->inode_num = 1;
    strcpy(de->file_name,".");
    /*for(i = 0;i < NR_CONSOLES;i++)*/
    /*{*/
        /*de++;*/
        /*de->inode_num = i + 2;*/
        /*sprintf(de->file_name,"dev_tty%d",i);*/
    /*}*/
    hd_rw(ROOT_DEV,sb.s_firstzone,1,ATA_WRITE,bh);
    brelse(bh);
}

int init_fs()
{
    int i;

    /*INIT_LIST_HEAD(&file_lists);*/
    /*INIT_LIST_HEAD(&inode_lists);*/

    for(i = 0;i < NR_SUPER;i++)
    {
        super_block[i].s_dev = NO_DEV;	
    }

    read_super_block(ROOT_DEV);
    struct super_block *sb = get_super_block(ROOT_DEV);
    if(sb->s_magic != MAGIC_FS)
    {
        mk_fs();	
    }

    root_inode = iget(ROOT_DEV,ROOT_INODE);

    return 0;
}

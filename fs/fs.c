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
#include "panic.h"
#include "stdlib.h"
#include "hd.h"
#include "blk_drv.h"
#include "fcntl.h"
#include "string_32.h"

//一个扇区大小
//public u8 fs_buf[SECTOR_SIZE];

struct m_inode inode_table[NR_INODE];
struct m_inode *root_inode;

void mk_fs()
{
    int i,j;
    struct super_block sb;
    struct buffer_head *bh;

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
    //	inode map

    //初始化inode_map 占NR_INODE_MAP_SECTS个扇区
    bh = getblk(ROOT_DEV,1 + NR_SUPER_BLOCK_SECTS);
    memset(bh->b_data,0x0,SECTOR_SIZE);	
    //0号inode保留 1号inode为根目录 然后依次为NR_CONSOLES个控制文件tty0 tty1 tty2
    for(i = 0;i < NR_CONSOLES + 2;i++)
    {
        (bh->b_data)[0] |= (1 << i);
    }
    //	printf("%d\n",hd_buf[0]);
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

    //	zone map
    //	memset(hd_buf,0,SECTOR_SIZE);
    //第0号盘块被保留 从第1号盘块号开始
    bh = getblk(ROOT_DEV,1 + NR_SUPER_BLOCK_SECTS + NR_INODE_MAP_SECTS);	
    int nr_sects = NR_DEFAULT_SECTS + 1;

    for(i = 0;i < nr_sects / 8;i++)
    {
        (bh->b_data)[i] =0xff;
    }				
    for(j = 0; j < nr_sects % 8;j++)
    {
        (bh->b_data)[i] |= 1 << j;
    }	
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
    struct	d_inode  *dinode = (struct d_inode *)(bh->b_data);
    dinode->i_mode = I_DIRECTORY;
    dinode->i_size = DIR_ENTRY_SIZE * (1 + NR_CONSOLES);
    dinode->i_start_sect = sb.s_firstzone;
    dinode->i_nr_sects = NR_DEFAULT_SECTS;
    //	dinode->i_nlinks = 1;
    for(i = 0;i < NR_CONSOLES;i++)
    {
        dinode++;
        dinode->i_mode = I_CHAR_SPECIAL;
        dinode->i_size = dinode->i_nr_sects = 0;
        dinode->i_start_sect = MAKE_DEV(DEV_TTY,i);
    }
    hd_rw(ROOT_DEV,1 + NR_SUPER_BLOCK_SECTS + NR_INODE_MAP_SECTS + NR_ZONE_MAP_SECTS,1,ATA_WRITE,bh);
    brelse(bh);
    //	zone
    //初始化根目录中的数据。根目录是个目录，存放的是目录项 4个文件 .  tty0 tty1 tty2

    bh = getblk(ROOT_DEV,sb.s_firstzone);
    memset(bh->b_data,0,SECTOR_SIZE);
    struct dir_entry *de = (struct dir_entry*)(bh->b_data);
    de->inode_num = 1;
    strcpy(de->file_name,".");

    for(i = 0;i < NR_CONSOLES;i++)
    {
        de++;
        de->inode_num = i + 2;
        sprintf(de->file_name,"dev_tty%d",i);
    }
    hd_rw(ROOT_DEV,sb.s_firstzone,1,ATA_WRITE,bh);
    brelse(bh);
}

int init_fs()
{
    int i;
    for(i = 0;i < NR_FILE;i++)
        memset((char *)&file_table[i],0,sizeof(struct file));
    for(i = 0;i < NR_INODE;i++)
        memset((char *)&inode_table[i],0,sizeof(struct m_inode));
    for(i = 0;i < NR_SUPER;i++)
        super_block[i].s_dev = NO_DEV;	
    mk_fs();	
    read_super_block(ROOT_DEV);
    struct super_block *sb = get_super_block(ROOT_DEV);
    assert(sb->s_magic == MAGIC_FS);
    root_inode = iget(ROOT_DEV,ROOT_INODE);
    /*
       struct buffer_head* bh = getblk(ROOT_DEV,100);
       printk("bh->b_dev = %d\n",bh->b_dev);
       printk("bh->b_blocknr = %d\n",bh->b_blocknr);
       printk("bh->b_data= %s\n",bh->b_data);

       int fd	= open("/",0,0);
       printk("init_fs fd = %d\n",fd);
    //	struct m_inode *inode = current->filp[fd]->f_inode;
    //	printk("inode num = %d\n",inode->i_num);
    //	char buf[100] = {0};
    printk("-----------------------------------------\n");
    int fd2	= open("/sunkang",0,O_CREAT);
    printk("init_fs fd2 = %d\n",fd2);
    struct inode inode = current->filp[fd2]->f_inode;
    printk("inode num = %d\n",inode->i_num);
    printk("-----------------------------------------\n");

    int fd3 = open("/kamus",0,O_CREAT);
    struct m_inode inode = current->filp[fd3]->f_inode;
    printk("init fs fd3 = %d\n",fd3);
    printk("inode num = %d\n",inode->i_num);
    printk("-----------------------------------------\n");

    //	sys_read(fd,buf,sizeof(buf));
    for(i = 0;i < 100;i++)
    {
    printk("%c",buf[i]);
    }
    printk("\n");
    printk("buf = %s\n",buf);
    do_unlink("/sunkang");
    fd2 = open("/sunkang",0,O_CREAT);
    */	
    return 0;
}

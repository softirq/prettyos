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
    /*set_imap_bit(ROOT_DEV, 0);*/

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
    /* root inode */
    struct	m_inode  *inode = (struct m_inode *)(bh->b_data);
    inode->i_mode = I_DIRECTORY;
    inode->i_size = DENTRY_SIZE;
    inode->i_start_sect = sb.s_firstzone;
    inode->i_nr_sects = NR_DEFAULT_SECTS;
    /*get_block_nums(ROOT_DEV,inode, NR_DEFAULT_SECTS);*/
    nr_sectors = sb.s_firstzone;
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

    INIT_LIST_HEAD(&file_lists);
    INIT_LIST_HEAD(&inode_lists);

    for(i = 0;i < NR_SUPER;i++)
    {
        super_block[i].s_dev = NO_DEV;	
    }

    mk_fs();	
    read_super_block(ROOT_DEV);
    struct super_block *sb = get_super_block(ROOT_DEV);

    /*if(sb->s_magic != MAGIC_FS)*/

    /*assert(sb->s_magic == MAGIC_FS);*/
    root_inode = iget(ROOT_DEV,ROOT_INODE);
    if(root_inode == NULL)
    {
        printk("root inode null.");
    }

    struct buffer_head* bh = getblk(ROOT_DEV,100);
    /*printk("bh->b_dev = %d\n",bh->b_dev);*/
    printk("bh->b_blocknr = %d\n",bh->b_blocknr);
    /*printk("bh->b_data= %s\n",bh->b_data);*/

    printk("-----------------------------------------\n");
    int fd = open("/",0,0);
    if(fd < 0)
    {
        printk("open error. fd = %x.", fd);
    }
    close(fd);
    /*printk("init_fs fd = 0x%x\n",fd);*/
    /*struct m_inode *inode = current->filp[fd]->f_inode;*/
    /*printk("inode num = %d\n",inode->i_num);*/

    struct m_inode *inode;
    printk("\n-----------------------------------------\n");
    fd = open("/sunkang",0,O_CREAT);
    if(fd < 0)
    {
        printk("open error. fd = %x.", fd);
    }
    else
    {
        inode = current->filp[fd]->f_inode;
        printk("sunkang inode num = %d\n",inode->i_num);
        close(fd);
    }


    /*[>printk("\n-----------------------------------------\n");<]*/
    /*if((fd = open("/sunkang/kamus",0,O_CREAT)) < 0)*/
    /*{*/
    /*}*/
    /*else*/
    /*{*/
        /*inode = current->filp[fd]->f_inode;*/
        /*printk("kamus inode num = %d\n",inode->i_num);*/
        /*close(fd);*/
    /*}*/

    /*[>printk("\n-----------------------------------------\n");<]*/
    /*if((fd = open("/sunkang/kamus/hahaha",0,O_CREAT)) < 0)*/
    /*{*/
    /*}*/
    /*else*/
    /*{*/
        /*inode = current->filp[fd]->f_inode;*/
        /*printk("haha inode num = %d\n",inode->i_num);*/
        /*close(fd);*/
    /*}*/

    /*char buf[] = "wo shi sunkang";*/
    /*sys_read(fd,buf,sizeof(buf));*/
    /*sys_write(fd2,buf, sizeof(buf));*/
    /*for(i = 0;i < 100;i++)*/
    /*{*/
    /*printk("%c",buf[i]);*/
    /*}*/
    /*char abc[15] = {0};*/
    /*printk("fd = %d.",fd2);*/
    /*sys_read(fd2,abc,sizeof(abc));*/
    /*printk("\n");*/
    /*printk("abc = %s\n",abc);*/

    /*fd2	= mkdir("/test",0,O_CREAT);*/
    /*do_unlink("/sunkang");*/
    /*fd2 = open("/sunkang",0,O_CREAT);*/
    /*printk("init_fs fd2 = %d\n",fd2);*/
    /*inode = current->filp[fd2]->f_inode;*/
    /*printk("inode num = %d\n",inode->i_num);*/

    return 0;
}

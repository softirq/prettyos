#include "type.h"
#include "const.h"
#include "wait.h"
#include "panic.h"
#include "stdlib.h"
#include "hd.h"
#include "blk_drv.h"
#include "fcntl.h"
#include "string.h"
#include "printf.h"

int match(struct dir_entry *de,char *name,int namelen)
{
    if(!de || !de->inode_num || namelen > NAME_LEN)
        return -1; 
    if(namelen < NAME_LEN && (namelen != strlen(de->file_name))) 
        return -1;
    return strncmp(de->file_name,name,namelen);
}

int do_entry(struct m_inode *dir,char *name,int namelen,struct dir_entry **res_de,int flag)
{
    int i,j,m = 0;
    int ret = -1;
    struct dir_entry *de;
    struct buffer_head *bh;

    int dir_start_sect = dir->i_start_sect;
    int nr_dir_sects = (dir->i_size + SECTOR_SIZE -1) / SECTOR_SIZE;			
    int nr_dir_entry = dir->i_size / DIR_ENTRY_SIZE;	

    for(i = 0;i < nr_dir_sects;i++)
    {
        //		printk("%d\n",dir_start_sect + i);
        bh = getblk(dir->i_dev,dir_start_sect + i);
        hd_rw(dir->i_dev,dir_start_sect + i ,1,ATA_READ,bh);	
        de = (struct dir_entry *)(bh->b_data);
        for(j = 0; j < SECTOR_SIZE / DIR_ENTRY_SIZE;j++,de++)
        {
            if(++m > nr_dir_entry)
                return -1;
            if(de->inode_num == 0)
                continue;	
            else
            {
                ret = match(de,name,namelen);
                if(ret != 0)
                    continue;
                else 
                {
                    switch(flag)
                    {	
                        case DE_MATCH:
                            printk("match successful\n");
                            *res_de = de;
                            brelse(bh);
                            return 0;
                        case DE_DEL:
                            *res_de = de;
                            memset((char *)de,0,DIR_ENTRY_SIZE);
                            hd_rw(dir->i_dev,dir_start_sect + i ,1,ATA_WRITE,bh);	
                            brelse(bh);
                            return 0;
                        default:
                            break;
                    }
                }
            }
        }
        brelse(bh);
    }	
    return -1;
}

int find_entry(struct m_inode *dir,char *name,int namelen,struct dir_entry **res_de)
{
    return do_entry(dir,name,namelen,res_de,DE_MATCH);
}

int del_entry(struct m_inode *dir,char *name,int namelen,struct dir_entry **res_de)
{
    return do_entry(dir,name,namelen,res_de,DE_DEL);
}

int add_entry(struct m_inode *dir,int inode_num,char *name)
{
    int i,j;
    struct dir_entry *de;
    struct dir_entry *new_de;
    struct buffer_head *bh;
    int dir_start_sect= dir->i_start_sect;
    int nr_dir_sects = NR_DEFAULT_SECTS;

    for(i = 0;i < nr_dir_sects;i++)
    {
        bh = getblk(dir->i_dev,dir_start_sect + i);
        hd_rw(dir->i_dev,dir_start_sect + i ,1,ATA_READ,bh);	
        de = (struct dir_entry *)(bh->b_data);
        for(j = 0; j < SECTOR_SIZE/DIR_ENTRY_SIZE;j++,de++)
        {
            if(!de->inode_num)
            {
                new_de = de;	
                break;
            }
        }
        if(new_de)
        {
            break;
        }
        brelse(bh);
    }
    if(!new_de)
    {
        panic("no disk room\n");
        return -1;
    }
    new_de->inode_num = inode_num;
    dir->i_size += DIR_ENTRY_SIZE;
    dir->i_dirt = 1;
    strcpy(new_de->file_name,name);
    hd_rw(dir->i_dev,dir_start_sect + i ,1,ATA_WRITE,bh);	
    write_inode(dir);
    brelse(bh);
    return 0;
}

struct m_inode * dir_namei(char *pathname,char ** name,int *namelen)
{
    char ch;
    char *basename;
    while((ch = *pathname++))
    {
        if(ch == '/')
            basename = pathname;
    }
    *namelen = pathname - basename - 1;
    *name = basename;
    //Version 1.0版本简化流程 直接在/目录下
    return root_inode;
}

int open_namei(char *pathname,int mode,int flag,struct m_inode **res_inode)
{
    int namelen;
    char *basename;
    struct dir_entry *de;   /* dentry */
    struct m_inode *dir;    /* dir dentry */
    struct m_inode *inode;  /* file inode */
    printk("pathname=%s.", pathname);
    if((dir = dir_namei(pathname,&basename,&namelen)) == NULL)
    {
        printk("dir_namei.");
        return -1;
    }
    //需要创建的是个目录
    if(!namelen)
    {
        if(dir)
        {
            *res_inode = dir;
            return 0;
        }
        iput(dir);
        return -2;
    }

    if((find_entry(dir,basename,namelen,&de)) != 0) 
    {
        printk("no such file\n");
        if(flag & O_CREAT)
        {
            inode = create_file(dir,basename,namelen);
            if(inode)
            {
                *res_inode = inode;
                //	printk("open_namei:inode->i_num = %d\n",inode->i_num);
                return 0;
            }
            else
                return -3;
        }	
        else
        {
            iput(dir);
            return -4;
        }
    }
    else
    {
        if(!(inode = iget(dir->i_dev,de->inode_num)))
            return -5;
        iput(dir);
        *res_inode = inode;
        return	0; 
    }
    return -6;
}

struct m_inode * namei(char *pathname)
{
    struct m_inode *dir,*inode;
    char *basename;
    int namelen;
    u16 inode_nr,dev;
    struct dir_entry *de;
    //找到目录的inode
    if(!(dir = dir_namei(pathname,&basename,&namelen)))
    {
        return NULL;
    }
    if(!namelen)
        return dir;

    //从目录项中找到该文件
    find_entry(dir,basename,namelen,&de);
    if(!de)
    {
        return NULL;
    }	

    inode_nr = de->inode_num;
    dev = dir->i_dev;
    //获取该文件的inode
    inode = iget(dev,inode_nr);
    if(inode)
    {
        //		inode->i_dirt = 1;
    }
    return inode;
}

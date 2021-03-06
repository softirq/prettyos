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

static int compare(struct dentry *de,char *name,int namelen)
{
    if(!de || !de->inode_num || namelen > NAME_LEN)
        return -1; 
    if(namelen < NAME_LEN && (namelen != strlen(de->file_name))) 
        return -1;
    return strncmp(de->file_name,name,namelen);
}

/* find the name entry */
int do_entry(struct m_inode *dir,char *name,int namelen,struct dentry **res_de,int flag)
{
    int i,j,m = 0;
    int ret = -1;
    struct dentry *de;
    struct buffer_head *bh;

    int dir_start_sect = get_first_block(dir);
    /*int dir_start_sect = dir->i_data[0];*/
    int nr_dir_sects = (dir->i_size + SECTOR_SIZE -1) / SECTOR_SIZE;			
    int nr_dentry = dir->i_size / DENTRY_SIZE;	
    printk("nr_dentry = %d.",nr_dentry);

    for(i = 0;i < nr_dir_sects;i++)
    {
        //		printk("%d\n",dir_start_sect + i);
        bh = getblk(dir->i_dev,dir_start_sect + i);
        hd_rw(dir->i_dev,dir_start_sect + i ,1,ATA_READ,bh);	
        printk("lookup start sec = %d",dir_start_sect + i);
        de = (struct dentry *)(bh->b_data);
        for(j = 0; j < SECTOR_SIZE / DENTRY_SIZE;++j,++de)
        {
            if(++m > nr_dentry)
                return -1;
            if(de->inode_num == 0)
                continue;	
            else
            {
                /*printk("de name = %s.",de->file_name);*/
                ret = compare(de,name,namelen);
                if(ret != 0)
                    continue;
                else 
                {
                    switch(flag)
                    {	
                        case DE_MATCH:
                            printk("compare successful\n");
                            *res_de = de;
                            brelse(bh);
                            return 0;
                        case DE_DEL:
                            *res_de = de;
                            memset((char *)de,0,DENTRY_SIZE);
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
        return 0;
    }	

    return -2;
}

int find_entry(struct m_inode *dir,char *name,int namelen,struct dentry **res_de)
{
    return do_entry(dir,name,namelen,res_de,DE_MATCH);
}

int del_entry(struct m_inode *dir,char *name,int namelen,struct dentry **res_de)
{
    return do_entry(dir,name,namelen,res_de,DE_DEL);
}

int add_entry(struct m_inode *dir,int inode_num,char *name)
{
    int i,j;
    struct dentry *de = NULL, *new_de = NULL;
    struct buffer_head *bh = NULL;
    int dir_start_sect, nr_dir_sects;

    if(dir == NULL || name == NULL)
        return -1;

    if((dir_start_sect = get_first_block(dir)) == 0)
    {
        set_block_nums(dir->i_dev ,(struct d_inode *)dir, NR_DEFAULT_SECTS);
        if((dir_start_sect = get_first_block(dir)) == 0)
            return -3;
    }

    printk("dir_i_num = %d dir_start_sect=%d.",dir->i_num, dir_start_sect);
    nr_dir_sects = NR_DEFAULT_SECTS;

    for(i = 0;i < nr_dir_sects;i++)
    {
        bh = getblk(dir->i_dev,dir_start_sect + i);
        if((hd_rw(dir->i_dev,dir_start_sect + i ,1,ATA_READ,bh)) < 0)
        {
            return -2;
        }

        de = (struct dentry *)(bh->b_data);

        for(j = 0; j < SECTOR_SIZE/DENTRY_SIZE;j++,de++)
        {
            if(!de->inode_num)
            {
                new_de = de;	
                break;
            }
        }
        if(new_de)
            break;

        brelse(bh);
    }

    if(!new_de)
    {
        panic("no disk room\n");
        return -2;
    }

    new_de->inode_num = inode_num;
    dir->i_size += DENTRY_SIZE;
    dir->i_dirty = 1;

    /*printk("add entry name=%s.",name);*/
    strncpy(new_de->file_name,name, strlen(name));

    printk("sector = %d.",dir_start_sect + i);
    printk("size = %d.",dir->i_size);
    if((hd_rw(dir->i_dev,dir_start_sect + i ,1,ATA_WRITE,bh)) < 0)
        return -3;

    /*if((write_inode(dir)) < 0)*/
    /*return -4;*/

    brelse(bh);

    return 0;
}

/* from the dentry find the filename*/
int lookup(struct m_inode *base, char *name, int namelen, struct m_inode **res_inode)
{
    int ret;
    struct dentry *de;
    struct m_inode *inode;

    if(base == NULL || name == NULL || namelen <= 0 || res_inode == NULL)
        return -1;

    /*printk("lookup name = %s.len = %d.",name, namelen );*/
    printk("base inode num=%d.",base->i_num);
    if((ret = do_entry(base ,name,namelen,&de,DE_MATCH)) < 0)
    {
        printk("do entry error.ret = %d.",ret);
        return -2;
    }
    else
    {
        if(!(inode = iget(base->i_dev,de->inode_num)))
            return -3;
    }

    *res_inode = inode;

    return 0;
}

/* find the direntry of the pathname */
int dir_namei(char *pathname,char ** name,int *namelen, struct m_inode **res_inode)
{
    char ch;
    int ret,len;
    char *thisname;
    struct m_inode *baseinode = NULL;
    struct m_inode *inode = NULL;

    if((ch = *pathname) == '/')
    {
        baseinode = root_inode;
        ++pathname;
    }

    while(1)
    {
        thisname = pathname;
        for(len = 0;(ch = *(pathname++)) && (ch != '/'); ++len)
            ;
        if(!ch)
            break;

        /*printk("thisname = %s len = %d.",thisname,len);*/
        if((ret = lookup(baseinode, thisname, len,&inode)) < 0)
        {
            printk("lookup ret = %d.",ret);
            return -3;
            /*break;*/
        }

        baseinode = inode;
    }

    *namelen = pathname - thisname - 1;
    *name = thisname;

    //Version 1.0版本简化流程 直接在/目录下
    /**res_inode = root_inode;*/
    *res_inode = baseinode;

    return 0;
}

/*find the pathname inode for open function*/
int open_namei(char *pathname,int mode,int flags,struct m_inode **res_inode)
{
    int ret;
    int namelen;
    char *name;
    struct dentry *de;   /* dentry */
    struct m_inode *dir;    /* dir dentry */
    struct m_inode *inode;  /* file inode */

    /* get direntry inode */
    if((ret = dir_namei(pathname,&name,&namelen, &dir)) < 0)
    {
        /*printk("dir_namei.ret = %x.",ret);*/
        return -1;
    }
    /*printk("name=%s.namelen=%d.",name,namelen);*/
    /* is a directory */
    if(!namelen || !dir)
    {
        if(dir)
        {
            *res_inode = dir;
            return 0;
        }
        iput(dir);
        return -2;
    }

    /* find the file from the directory */
    /*printk("name = %s.len = %d.", name, namelen);*/
    if(find_entry(dir,name,namelen,&de) != 0) 
    {
        printk("no such file\n");
        if(flags & O_CREAT)
        {
            printk("create mode=%d.", mode);
            inode = new_file(dir,name,namelen,mode,flags);
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
        printk("inode->mode=%x.",inode->i_mode);
        iput(dir);
        *res_inode = inode;
        return	0; 
    }

    return -6;
}

/* only get pathname inode */
struct m_inode * namei(char *pathname)
{
    struct m_inode *dir,*inode;
    char *basename;
    int namelen;
    u16 inode_nr,dev;
    struct dentry *de;

    //fine the dentry inode
    if(dir_namei(pathname,&basename,&namelen, &dir) < 0)
    {
        return NULL;
    }
    if(!namelen)
        return dir;

    //from the dentry find the file
    find_entry(dir,basename,namelen,&de);
    if(!de)
    {
        return NULL;
    }	

    inode_nr = de->inode_num;
    dev = dir->i_dev;
    //get the file inode
    inode = iget(dev,inode_nr);
    if(inode)
    {
        //		inode->i_dirt = 1;
    }

    return inode;
}

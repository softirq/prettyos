#include "type.h"
#include "const.h"
#include "string.h"
#include "wait.h"
#include "hd.h"
#include "blk_drv.h"
#include "stat.h"

static void cp_stat(struct m_inode *inode,struct stat *statbuf)
{
    struct stat tmp;
    tmp.st_dev = inode->i_dev;
    tmp.st_inode = inode->i_num;
    tmp.st_mode = inode->i_mode;
    tmp.st_size = inode->i_size;
}


int sys_stat(char *filename,struct stat *statbuf)
{
    struct m_inode *inode;
    if(!(inode = namei(filename)))
    {
        return -1;
    }
    cp_stat(inode,statbuf);
    iput(inode);
    return 0;
}

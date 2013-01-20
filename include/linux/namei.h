#ifndef     _NAMEI_H_
#define     _NAMEI_H_

#define NAME_LEN	12		        //filename length

#define DE_MATCH	0x0001
#define DE_DEL		0x0002

//一个目录最多可以拥有的文件或者目录个数
#define MAX_NUM_FILES	1024
//分配给文件默认的扇区数
#define NR_DEFAULT_SECTS	11	//默认分配20个扇区

#define DENTRY_SIZE  	16

//dentry 
struct dentry 
{
    unsigned short inode_num;		//the inode number
    char file_name[NAME_LEN];	    // file name 
    const struct dentry_operations *d_ops;
};


struct dentry_operations
{
    int (*d_revalidate)(struct dentry *, unsigned int);
    int (*d_delete)(const struct dentry *); 
    void (*d_release)(struct dentry *); 
    void (*d_prune)(struct dentry *); 
    //void (*d_iput)(struct dentry *, struct inode *); 
    char *(*d_dname)(struct dentry *, char *, int);
    //struct vfsmount *(*d_automount)(struct path *); 
    int (*d_manage)(struct dentry *, bool);
};

extern int open_namei(char *pathname,int flag,int mode,struct m_inode **res_inode);
extern int add_entry(struct m_inode *dir,int inode_num,char *name);
struct m_inode * namei(char *pathname);
extern int del_entry(struct m_inode *dir,char *name,int namelen,struct dentry **res_de);
extern int dir_namei(char *pathname,char ** name,int *namelen, struct m_inode **res_inode);

#endif

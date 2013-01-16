#ifndef     _NAMEI_H_
#define     _NAMEI_H_

#define NAME_LEN	12		        //filename length

#define DE_MATCH	0x0001
#define DE_DEL		0x0002

//一个目录最多可以拥有的文件或者目录个数
#define MAX_NUM_FILES	1024
//分配给文件默认的扇区数
#define NR_DEFAULT_SECTS	20	//默认分配20个扇区

#define DIR_ENTRY_SIZE  	16

#define DIR_ENTRY_SIZE  	16

#define DENTRY_SZIE 	16

//dentry 
struct dir_entry
{
	unsigned short inode_num;		//the inode number
	char file_name[NAME_LEN];	    // file name 
};

struct dentry_operations
{
};

extern int open_namei(char *pathname,int flag,int mode,struct m_inode **res_inode);
extern int add_entry(struct m_inode *dir,int inode_num,char *name);
struct m_inode * namei(char *pathname);
extern int del_entry(struct m_inode *dir,char *name,int namelen,struct dir_entry **res_de);
struct m_inode * dir_namei(char *pathname,char ** name,int *namelen);

#endif

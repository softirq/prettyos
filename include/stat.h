#ifndef     _STAT_H_
#define     _STAT_H_

struct stat
{
	u16	st_dev;
	u16	st_inode;
	u16	st_mode;
	u16	st_size;
	u16	st_uid;
	u16	st_atime;
	u16	st_mtime;
	u16	st_ctime;	
};

#endif

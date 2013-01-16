#ifndef     _READ_WRITE_H_
#define     _READ_WRITE_H_

#define 	FILE_READ 	0x01
#define 	FILE_WRITE 	0x02

extern int sys_read(unsigned int fd,char *buf,int count);
extern int sys_write(unsigned int fd,char *buf,int count);

#endif

#ifndef     _SYS_H_
#define     _SYS_H_

extern int sys_fork();
extern int sys_write(unsigned int fd,char *buf,int count);
extern int sys_exit();
extern int sys_waitpid(int pid);

#endif

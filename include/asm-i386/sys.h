#ifndef     _SYS_H_
#define     _SYS_H_

/* system call */
#define	NR_SYS_CALL	6

extern int sys_fork();
extern int sys_write(int fd,char *buf,int count);
extern int sys_exit();
extern int sys_waitpid(int pid);
extern int	sys_get_ticks();

#endif

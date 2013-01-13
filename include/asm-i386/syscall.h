#ifndef     _SYSCALL_H_ 
#define     _SYSCALL_H_ 

extern	syscall_ptr		sys_call_table[];

void    sys_call();
int     get_ticks();
//  void    write(char *buf,int len);
void    printx(char *buf,int len);

#endif

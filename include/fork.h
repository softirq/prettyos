#ifndef     _FORK_H_
#define     _FORK_H_

#ifndef _PID_T
#define _PID_T
typedef     unsigned int    pid_t;
#endif

#define     MAX_PIDNUM      1024
extern unsigned char pidmap[MAX_PIDNUM/8];

extern int fork();
extern int getpid();
void init_pidmap();
int get_bitmap();

#endif

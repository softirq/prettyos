#ifndef     _PID_H_
#define     _PID_H_

#define     INIT_PID        1
#define     MAX_PIDNUM      1024

typedef     unsigned int    pid_t;
extern unsigned char pidmap[MAX_PIDNUM/8];

extern int getpid();
extern void init_pidmap();
extern pid_t get_pidmap();

#endif

#ifndef     _PID_H_
#define     _PID_H_

typedef     unsigned int    pid_t;
#define     MAX_PIDNUM      1024
extern unsigned char pidmap[MAX_PIDNUM/8];

extern int getpid();
extern void init_pidmap();
extern pid_t get_pidmap();

#endif

#include "type.h"
#include "const.h"
#include "pid.h"
#include "sched.h"
#include "bitmap.h"
#include "string.h"
#include "stddef.h"

int getpid()
{
    return current->pid;
}

void init_pidmap()
{
    bzero((void *)pidmap,sizeof(pidmap)); 
    pidmap[0] |= 0x01;
}

pid_t get_pidmap()
{
    pid_t first = (pid_t)set_first_bit(pidmap,ARRAY_SIZE(pidmap));
    return first;
}


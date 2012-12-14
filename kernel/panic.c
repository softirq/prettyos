#include "type.h"
#include "const.h"
#include "traps.h"
#include "string.h"
#include "tty.h"
#include "console.h"
#include "wait.h"
#include "mm.h"
#include "sched.h"
#include "global.h"
#include "kernel.h"

//系统级错误 系统stop
void panic(const char *fmt,...)
{
    int i;
    char buf[128];
    va_list args = (va_list)((char *)&fmt+4);
    i = vsprintf(buf,fmt,args);
    printf("%c panic %s",MAG_CH_PANIC,buf);
    //	__asm__ __volatile__ ("ud2");
}

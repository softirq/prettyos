#include "type.h"
#include "const.h"
#include "vsprintf.h"
#include "syscall.h"
#include "lib.h"

int printf(const char *fmt,...)
{
	int i;
	char buf[512];
	va_list args = (va_list)((char *)(&fmt) + 4);
	i = vsprintf(buf,fmt,args);
//	write(buf,i);
	buf[i] = 0;
	printx(buf,i);
	return i;
}

//直接放入到显存，不通过虚拟终端
int printk(const char *fmt,...)
{
	int i;
//	int text_color = 0x74;
	char buf[512];
	va_list args = (va_list)((char *)(&fmt) + 4);
	i = vsprintf(buf,fmt,args);
	buf[i] = 0;
//	disp_color_str(buf,text_color);
	disp_str(buf);
	return i;
}

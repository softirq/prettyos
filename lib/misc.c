#include "type.h"
#include "const.h"
#include "traps.h"
//#include "string.h"
#include "tty.h"
#include "wait.h"
#include "mm.h"
#include "sched.h"
#include "console.h"
#include "global.h"
//#include "kernel.h"
#include "lib.h"


tbool is_alphanumeric(char ch)
{
    return ((ch >= ' ') && (ch <= '~'));
}

//public void itoa(int num)
void itoa(char *str, int num)
{
    char *	p = str;
    char	ch;
    int	i;
    tbool	flag = FALSE;

    //	*p++ = '0';
    //	*p++ = 'x';

    if(num == 0)
    {
        *p++ = '0';
    }
    else
    {	
        for(i=28;i>=0;i-=4)
        {
            ch = (num >> i) & 0xF;
            if(flag || (ch > 0))
            {
                flag = TRUE;
                ch += '0';
                if(ch > '9')
                {
                    ch += 7;
                }
                *p++ = ch;
            }
        }
    }

    *p = 0;
}

int strlen(char *str)
{
    int i = 0;
    char *p = str;
    while(*p != '\0')
    {
        i++;
        p++;
    }
    return i;
}

#define min(x,y) ((x > y)?y:x)
int strncmp(char *str1,char *str2,int len)
{
    int i;
    int min_len  = min(strlen(str1),strlen(str2));
    if(len <= 0 || !str1 || !str2)
        return -1;
    if(len > min_len)
        len = min_len;

    for(i = 0;i < len; i++,str1++,str2++)
    {
        if(*str1 == *str2)
            continue;
        else
            return -1;
    }	
    return 0;
}

int strcmp(char *str1,char *str2)
{
    if(!str1 || !str2)
        return -1;
    while(str1 && str2)
    {
        if(*str1++ != *str2++)
            return -1;
    }
    return 0;
}

int strncpy(char *dst,char *src,int size)
{
    int i = 0;
    if(!dst || !src || size < 0)
        return -1;

    while(((*dst++ = *src++) != '\0') && ((++i) < size));

    return 0;
}

int strcpy(char *dst,char *src)
{
    if(!dst || !src)
        return -1;

    while((*dst++ = *src++) != '\0');

    return 0;
}

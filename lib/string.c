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
#include "stdlib.h"

tbool is_alphanumeric(char ch)
{
    return ((ch >= ' ') && (ch <= '~'));
}

/*decimal number to string*/
void itoa(char *str, int value)
{
    char *p = str;
    u8 bit = 0;

    if(value == 0)
    {   
        *p++ = '0';
    }   
    else
    {   
        while(value != 0)
        {   
            bit = value%10;
            *p++ = '0' + bit;
            value = value/10;
        }   
    }   
    *p = 0;
}

/*hexadecimal number to string*/
void htoa(char *str, int value)
{
    char *p = str;
    char ch;
    int	i = 0;
    tbool flag = FALSE;

    if(value == 0)
    {
        *p++ = '0';
    }
    else
    {	
        for(i=28;i>=0;i-=4)
        {
            ch = (value >> i) & 0xF;
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
int memcmp(char *dst, char *src,int len)
{
    if(dst == NULL || src == NULL)
    {
        return 1;
    }
    else
    {
        if(strlen(dst) != strlen(src))
            return -1;
        if(len > strlen(dst))
            return -1;
        while(dst && src && len--)
        {
            if(*dst != *src)
                return -1;
            dst++;
            src++;
        }
        return 0;
    }
    return -1;

}

int memset(char *dst,char ch,int size)
{
    if(size <= 0)
        return -1;
    int i = 0;	
    while(dst && i < size)
    {
        *dst = ch;
        dst++;
        i++;
    }
    return 0;
}

/*
   int strlen(char *str)
   {
   int i = 0;
   char ch;
   char *ptr = str;
   if(str == NULL)
   return 0;
   while((ch = *str) != '\0')
   {
   i++;
   ptr++;
   }
   return i;
   }
   */

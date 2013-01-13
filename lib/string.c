#include "type.h"
#include "const.h"
#include "stdlib.h"

tbool is_alphanumeric(char ch)
{
    return ((ch >= ' ') && (ch <= '~'));
}

/*string length*/
int strlen(const char *str)
{
    int i = 0;

    if(str == NULL)
        return -1;

    while(*str++ != '\0' && ++i);

    return i;
}

/*string reverse*/
int strreverse(char *ptr)
{
    if(ptr == NULL)
        return -1;

    int len = strlen(ptr);
    char *p = ptr + len - 1;

    char tmp;

    while(ptr < p)
    {
        tmp = *ptr;
        *ptr = *p;
        *p = tmp;

        ++ptr;
        --p;
    }

    return 0;
}

/*decimal number to string*/
void itoa(char *str, int value)
{
    if(str == NULL)
        return;

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
        strreverse(str);
    }   
    *p = 0;

}

/*hexadecimal number to string*/
void htoa(char *str, int value)
{
    if(str == NULL)
        return;

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

#define min(x,y) ((x > y)?y:x)
/*compare too string */
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
#undef min

/*compare too string */
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

/*string copy*/
int strncpy(char *dst,char *src,int size)
{
    int i = 0;
    if(!dst || !src || size < 0)
        return -1;

    while(((*dst++ = *src++) != '\0') && ((++i) < size));

    return 0;
}

/*string copy*/
int strcpy(char *dst,char *src)
{
    if(!dst || !src)
        return -1;

    while((*dst++ = *src++) != '\0');

    return 0;
}

/*memory copy*/
int memcmp(char *dst, char *src,int len)
{
    if(dst == NULL || src == NULL || len <= 0)
        return -1;

    else
    {
        if(strlen(dst) != strlen(src))
            return -2;
        if(len > strlen(dst))
            return -3;
        while(dst && src && len--)
        {
            if(*dst != *src)
                return -4;
            dst++;
            src++;
        }
        return 0;
    }

    return -5;
}

inline int memset(char *dst,char ch,int size)
{
    if(dst == NULL || size <= 0)
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

int bzero(void *dst, size_t size)
{
    return memset((char *)dst, 0,size);
}

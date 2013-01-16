#include "type.h"
#include "const.h"
#include "stdlib.h"
#include "string.h"

int vsprintf(char *buf,const char *fmt,va_list args)
{
    int value = 0;
    char *p = buf;
    char tmp[32] = {0};
    va_list next_arg = args;

    for(;*fmt;fmt++)
    {
        if(*fmt != '%')
        {
            *p++ = *fmt;
            continue;
        }
        fmt++;

        switch(*fmt)
        {
            case 's':
                strcpy(p,(*((char **)next_arg)));
                p += strlen(*((char **)next_arg));

                next_arg += 4;
                break;

            case 'c':
                *p++ = *((char *)next_arg);
                next_arg += 4;
                break;

            case 'd':
                value = *((int *)next_arg);
                if(value < 0)
                {
                    *p++ = '-';
                }
                itoa(tmp,value);				
                strcpy(p,tmp);
                p+= strlen(tmp);
                next_arg += 4;
                break;

            case 'p':
            case 'x':

                value = *((int *)next_arg);
                htoa(tmp,value);				
                strcpy(p,tmp);
                p+= strlen(tmp);
                next_arg += 4;
                break;

                /* print address value */

            default:
                break;
        }
    }
    return (p - buf); 
}

int sprintf(char *buf,const char *fmt,...)
{

    //      char buf[128];
    //      va_list args = (va_list)((char *)(&fmt) + 4);
    //      i = vsprintf(buf,fmt,args);

    //	va_list args = (char *)(&fmt + 1);
    va_list args = (va_list)((char *)(&fmt) + 4);
    return vsprintf(buf,fmt,args);
}

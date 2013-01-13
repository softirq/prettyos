#include "type.h"
#include "const.h"
#include "tty.h"
#include "printf.h"
#include "config.h"
#include "stdlib.h"

void spin(char *str)
{
    printf("%s",str);
    while(1)
    {};
}

//用户级错误 进入死循环
void assertion_failure(char *exp,char *file,char *base_file,int line)
{
    printf("%c assert (%s) failed:file:%s,base_file:%s,ln:%d\t",MAG_CH_ASSERT,exp,file,base_file,line);	
    spin("assertion_failure");
    __asm__ __volatile__ ("ud2");
}

void disp_int(int input)
{
    char output[16];
    htoa(output,input);
    disp_str(output);
}

void delay(int time)
{
    int i, j, k;
    for(k=0;k<time;k++)
    {
        for(i=0;i<10;i++)
        {
            for(j=0;j<10000;j++){}
        }
    }
}

void get_boot_params(struct boot_params *bp)
{
    int *p = (int *)(BOOT_PARAM_ADDR);

    /*assert(p[BP_MAGIC] == BOOT_PARAM_MAGIC);*/
    bp->mem_size = p[BP_MEM_SIZE];
    bp->kernel_addr = (unsigned char *)(p[BP_KERNEL_ADDR]);

    return ;
}

int phys_copy(char *dst, char *src,int size)
{
    if(dst == NULL || src == NULL)
        return -1;
    while(size--)
    {
        *dst++ = *src++;
    }
    *dst = '\0';
    return 0;
}

int u_phys_copy(unsigned char *dst, unsigned char *src, int size)
{
    if(dst == NULL || src == NULL)
        return -1;
    while(size--)
    {
        *dst++ = *src++;
    }
    *dst = '\0';
    return 0;
}


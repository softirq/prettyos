#include "type.h"
#include "const.h"
/*#include "traps.h"*/
#include "tty.h"
/*#include "console.h"*/
/*#include "wait.h"*/
/*#include "mm.h"*/
/*#include "sched.h"*/
/*#include "global.h"*/
#include "printf.h"
#include "config.h"
#include "elf.h"
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

int get_kernel_map(unsigned int * base, unsigned int * limit)
{
    struct boot_params bp;
    get_boot_params(&bp);

    //	printk("mem_size = %d kernel_addr = %d\n",bp.mem_size,bp.kernel_addr);
    Elf32_Ehdr *elf_header = (Elf32_Ehdr *)(bp.kernel_addr);

    *base = ~0;
    unsigned int t = 0;
    int i = 0;
    //	printk("elf_header->e_shnum = %d\n",elf_header->e_shnum);
    //	printk("elf_header->e_entry = %d\n",elf_header->e_entry);
    //	printk("elf_header->e_machine= %d\n",elf_header->e_machine);
    for(;i < elf_header->e_shnum;i++)
    {
        Elf32_Shdr * section_header = (Elf32_Shdr *)(bp.kernel_addr + elf_header->e_shoff + i * elf_header->e_shentsize);
        if(section_header->sh_flags & SHF_ALLOC)
        {
            int bottom = section_header->sh_addr;
            int top = section_header->sh_addr + section_header->sh_size;
            //		printk("bottom = %d top = %d\n",bottom,top);
            if(*base > bottom)
                *base = bottom;
            if(t< top)
                t = top;
        }

    }
    *limit = t - *base - 1;
    return 0;
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


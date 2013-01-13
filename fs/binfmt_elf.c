/*#include "type.h"*/
/*#include "const.h"*/
#include "elf.h"

int get_elf_map(unsigned char *address, struct elf32_ehdr *elf_header, unsigned int *base, unsigned int *limit)
{

    *base = ~0;
    unsigned int t = 0;
    int i = 0;
    for(;i < elf_header->e_shnum;i++)
    {
        /*Elf32_Shdr * section_header = (Elf32_Shdr *)(bp.kernel_addr + elf_header->e_shoff + i * elf_header->e_shentsize);*/
        Elf32_Shdr * section_header = (Elf32_Shdr *)(address + elf_header->e_shoff + i * elf_header->e_shentsize);
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


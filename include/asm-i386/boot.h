#ifndef     _BOOT_H_
#define     _BOOT_H_

//boot parameters
//
//kernel img 
struct boot_params 
{
	int mem_size;
	unsigned char *kernel_addr;
};

#endif

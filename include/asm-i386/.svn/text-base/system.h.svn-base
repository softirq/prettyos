#define sti() __asm__ __volatile__ ("sti": : :"memory")
#define cli() __asm__ __volatile__ ("cli": : :"memory")

#define save_flags(x) \
		__asm__ __volatile__("pushfl ; popl %0":"=r" (x): /* no input */ :"memory")

#define restore_flags(x) \
	    __asm__ __volatile__("pushl %0 ; popfl": /* no output */ :"r" (x):"memory")

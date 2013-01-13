#ifndef	_TYPE_H_
#define	_TYPE_H_

typedef	int		    t32;
typedef	short		t16;
typedef	char		t8;
typedef	int			tbool;
typedef char        bool;
//设置 设备号类型
typedef int 			dev_t;

typedef unsigned long long 	u64;
typedef unsigned int 		u32;	//unsigned long = unsigned int
typedef unsigned short 		u16;
typedef unsigned char		u8;

typedef	unsigned int		t_port;
typedef char*			va_list;

typedef	void	(*t_pf_int_handler)	();	
typedef	void	(*t_pf_task)		();
//typedef	void	(*irq_handler)	(int irq);

typedef	void*	syscall_ptr;


typedef long long	 __kernel_loff_t;
typedef __kernel_loff_t 	loff_t;		//内核偏移量

typedef long	off_t;				//文件偏移量

typedef unsigned int __kernel_size_t;
typedef int	__kernel_ssize_t;

#ifndef _SIZE_T
#define _SIZE_T
typedef __kernel_size_t size_t;
#endif

#ifndef _SSIZE_T
#define _SSIZE_T
typedef __kernel_ssize_t	ssize_t;
#endif

#endif

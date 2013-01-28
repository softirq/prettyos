#ifndef     _MM_H_
#define     _MM_H_

#include "page.h"
#include "file.h"
#include "inode.h"
#include "pgtable.h"
#include "buddy.h"
#include "slab.h"
#include "radix-tree.h"

#define     SYS_RESERVED    0x1
#define     SYS_ROM         0x2
#define     SYS_RAM         0x4

struct mm_struct
{
    int count;
    unsigned long start_code, end_code, end_data;
    unsigned long start_brk, brk, start_stack, start_mmap;
    unsigned long arg_start, arg_end, env_start, env_end;
    unsigned long rss;
    unsigned long min_flt, maj_flt, cmin_flt, cmaj_flt;
    int swappable:1;
    unsigned long swap_address;
    unsigned long old_maj_flt;
    unsigned long dec_flt;
    unsigned long swap_cnt;
    struct vm_area_struct *mmap;  	/* mmap list */
    struct vm_area_struct *mmap_avl;  /* mmap val tree */
};

extern unsigned long memory_end; 
extern unsigned long main_memory_start;
extern unsigned long main_memory_end;
extern unsigned long memory_size;
//extern unsigned long buffer_memory_start;
//extern unsigned long buffer_memory_end;

extern struct page **mem_map;
/* number of pages */
extern unsigned long page_fns;

extern int nr_swap_pages;
extern int nr_free_pages;

unsigned long alloc_mem(const size_t size);
int init_mem();

#define oom() ({ panic ("memory fatal error!\n");})

#define  GFP_BUFFER 	0x001
#define  GFP_USER 		0x001
#define  GFP_ATOMIC 	0x002
#define  GFP_KERNEL 	0x003

//extern void free_pages(unsigned long addr, unsigned long order);
extern unsigned long paging_init();
extern int zeromap_page_range(unsigned long address, unsigned long size, pgprot_t prot);

#define free_page(page) 	free_pages((page), 0)
#define MAP_NR(addr) 		(((unsigned long)addr) >> PAGE_SHIFT)

struct vm_area_struct 
{
    unsigned long vm_start;
    unsigned long vm_end;
    struct task_struct *vm_task;

    pgprot_t vm_page_prot;
    short vm_avl_height;
    unsigned short vm_flags;

    struct vm_area_struct *vm_avl_left;	
    struct vm_area_struct *vm_avl_right;	

    struct vm_area_struct *vm_next;	

    struct vm_area_struct *vm_next_share;	
    struct vm_area_struct *vm_prev_share;	

    struct vm_operation_struct *vm_ops;

    unsigned long vm_offset;
    struct 	m_inode *vm_inode;
    unsigned long 	vm_pte; //shared mem;
};

struct vm_operation_struct
{
    void (*open) (struct vm_area_struct *vma);
    void (*close) (struct vm_area_struct *vma);
    void (*map)(struct vm_area_struct *area, unsigned long, size_t);
    void (*unmap)(struct vm_area_struct *area, unsigned long, size_t);
};

struct address_space_operations {
    int (*writepage)(struct page *);
    int (*readpage)(struct file *,struct page *);
    int (*prepare_write)(struct file *, struct page *,unsigned, unsigned);
    int (*commit_write)(struct file *, struct page *, unsigned, unsigned);
};

struct address_space {
    struct inode *host;
    struct radix_tree_root page_tree;
    struct list_head    clean_pages;
    struct list_head    dirty_pages;
    struct address_space_operations *a_ops;
};

#define 	VM_READ 		0x0001
#define 	VM_WRITE 		0x0002
#define 	VM_EXEC 		0x0004
#define 	VM_SHARED 		0x0008

#define 	VM_MAYREAD 		0x0010
#define 	VM_MAYWRITE 	0x0020
#define 	VM_MAYEXEC 		0x0040
#define 	VM_MAYSHARE 	0x0080

#define 	VM_GROWSDOWN 	0x0100
#define 	VM_GROWSUP 		0x0200
#define 	VM_SHM 			0x0400
#define 	VM_DENYWRITE 	0x0800

#define 	VM_EXECUTABLE  	0X1000
#define 	VM_STACK_FLAGS 	0x0177

#define     get_page(p)     atomic_inc(&(p)->count)

/* inactive page list*/
extern struct list_head inactive_list;

#endif

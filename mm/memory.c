#include "type.h"
#include "const.h"
#include "string.h"
#include "stdlib.h"
#include "list.h"
#include "panic.h"
#include "wait.h"
#include "mm.h"
#include "printf.h"
#include "pgtable.h"

static long PAGING_PAGES = 0;
int nr_swap_pages = 0;
int nr_free_pages = 0;
static int nr_mem_map = 0;

unsigned long page_start_mem = 0;

struct page * mem_map = NULL;

struct mem_list buddy_list[NR_MEM_LISTS];

#define  	copy_page(from, to) 	memcpy((void *)from, (void *)to, PAGE_SIZE)
//static unsigned long free = 0;

/*
   static inline void oom()
   {
   panic("out of memory\n");
   return;
   }
   */
/*free page list*/
/*static unsigned long buddy_list_init (unsigned long start_mem, unsigned long end_mem)*/

static void buddy_list_init ()
{
    int i;
    for(i = 0;i < NR_MEM_LISTS; i++)
    {
        INIT_LIST_HEAD(&(buddy_list[i].list));
    }
}

/*tidy the buddy list*/
static int buddy_list_tidy()
{
    int i ;

    for(i = 0;i < NR_MEM_LISTS; i++)
    {
    }

    return 0;
}

int alloc_mem(int pid,int memsize)
{
    /*assert(pid < NR_PROCESS + NR_PROCS);
    int base = 0;
    if(memsize > PROC_IMAGE_SIZE_DEFAULT)
        panic("unsupported memory request %d,shoud be less than %d\n",\
                memsize,PROC_IMAGE_SIZE_DEFAULT);
    [>
       printk("NR_PROCESS = %d\n",NR_PROCESS);
       printk("base = %d\n",base);

<]
    base = (int)PROCS_BASE + (pid - ((int)NR_PROCESS)) * PROC_IMAGE_SIZE_DEFAULT;

    [>	printk("proces_base = %d\n",PROCS_BASE);
        printk("base = %d\n",base);
        printk("memsize = %d\n",memsize);
        printk("memory_size = %d\n",memory_size);
        <]	
    if(base + memsize >= memory_size)
        panic("memory allocation failed pid %d\n",pid);
    return base;
*/
    return 0;
}

inline unsigned long get_free_page(int priority)
{
    unsigned long page;
    page = __get_free_page(priority);
    if(page)
        memset((void *)page, 0 ,PAGE_SIZE);

    return page;
}

void do_no_page (struct vm_area_struct *vma,unsigned long addr, int access_write)
{
    unsigned long page;
    page = get_free_page(GFP_KERNEL);

    if(!page)
    {
        oom();
        return;
    }
    return ;
}

static inline pte_t* get_empty_pgtable(struct task_struct *tsk, unsigned long address)
{
    pgd_t *pgd;
    pmd_t *pmd;
    pte_t *pte;

    pgd = pgd_offset(tsk,address);
    pmd = pmd_alloc(pgd,address);
    if(!pmd)
    {
        oom();
        return NULL;
    }

    pte = pte_alloc(pmd, address);
    if(!pte)	
    {
        oom();
        return NULL;
    }

    return NULL;
}

int zeromap_page_range(unsigned long address, unsigned long size, pgprot_t prot)
{
    return 0;
}

/* this routine handles present pages, when users try to write
   to a shared page.
   */
void do_wp_page(struct vm_area_struct *vma, unsigned long address, int write_access)
{
    pgd_t *pgd;
    pmd_t *pmd;
    pte_t *page_table,pte;
    unsigned long old_page, new_page;

    new_page = get_free_page(GFP_KERNEL);

    pgd = pgd_offset(vma->vm_task, address);
    if(pgd_none(*pgd))
        goto end_wp_page;
    if(pgd_bad(*pgd))
        goto bad_wp_page;
    pmd = pmd_offset(pgd,address);
    if(pmd_none(*pmd))
        goto end_wp_page;
    if(pmd_bad(*pmd))
        goto bad_wp_page;
    page_table = pte_offset(pmd,address);
    pte = *page_table;
    if(!pte_present(pte))
        goto end_wp_page;
    if(pte_write(pte))
        goto  end_wp_page;
    old_page = pte_page(pte)->address;
    if(old_page >= main_memory_end)
        goto bad_wp_page;

    /*(vma->vm_task->mm->min_flt)++;*/

    if(mem_map[MAP_NR(old_page)].flags & PAGE_PRESENT)
    {
        if(new_page)
        {
            if(mem_map[MAP_NR(old_page)].flags & MAP_PAGE_RESERVED)
                /*++(vma->vm_task->mm->rss);*/
            copy_page(old_page, new_page);
            *page_table = pte_mkwrite(pte_mkdirty(mk_pte((unsigned long)&new_page, vma->vm_page_prot)));
            /*free_page(old_page);*/
            return;
        }
        pte_val(*page_table) &= PAGE_BAD;
        /*free_page(old_page);*/
        oom();
        return;
    }
    *page_table = pte_mkdirty(pte_mkwrite(pte));
    if(new_page)
        /*free_page(new_page);*/
    return;

bad_wp_page:
    printk("do_wp_page: bogus page at address %08lx (%08lx)\n",address,old_page);
    goto end_wp_page;

end_wp_page:
    if(new_page)
        /*free_page(new_page);*/
    return;
}

/*
 *  the function for the no-page and wp_page
 */
void handle_pte_fault(struct vm_area_struct *vma, unsigned long address, pte_t *pte,int write_access)
{
    // no page present
    if(!pte_present(*pte))
    {
        do_no_page(vma,address,write_access);
        return;
    }
    *pte = pte_mkyoung(*pte);
    if(!write_access)
    {
        *pte = pte_mkdirty(*pte);
        return;
    }
    // write the shared page 
    do_wp_page(vma, address, write_access);

}

void handle_mm_fault(struct vm_area_struct *vma, unsigned long address,int write_access)
{
    pgd_t * pgd;
    pmd_t * pmd;
    pte_t * pte;

    pgd = pgd_offset(vma->vm_task, address);
    pmd = pmd_alloc(pgd,address);

    if(!pmd)
        goto no_memory;
    pte = pte_alloc(pmd, address);
    if(!pte)
        goto no_memory;
    handle_pte_fault(vma,address,pte,write_access);
no_memory:
    oom();
}

static unsigned long alloc_mem_map(unsigned long start_mem, unsigned long end_mem)
{
    int size = 0;
    if(!mem_map)
    {
        nr_mem_map = (end_mem - start_mem) >> PAGE_SHIFT;;
        size = (nr_mem_map) * (sizeof(struct page) + sizeof(struct page *));
        mem_map = (struct page *)start_mem;
        memset((void *)mem_map, 0 ,size);

        start_mem += size;
    }

    page_start_mem = start_mem; 
    return start_mem;
}

void init_mem(unsigned long start_mem, unsigned long end_mem)
{
    int k;
    struct page *p = NULL;
    unsigned long page_ptr_start_mem = 0;

    buddy_list_init();

    struct boot_params bp;
    get_boot_params(&bp);
    memory_size = bp.mem_size;
    memory_size  &= 0xfffff000;

    end_mem = (end_mem > memory_size)?end_mem:memory_size;
    /*alloc memory to mem_map;*/
    page_start_mem = alloc_mem_map(start_mem, end_mem);
    page_start_mem = (start_mem & PAGE_MASK);

    PAGING_PAGES = (end_mem - page_start_mem) >> PAGE_SHIFT;

    page_ptr_start_mem = start_mem  + sizeof(struct mem_list) * nr_mem_map; 

    /*struct page *p = (struct page *)((unsigned long)mem_map + sizeof(mem_map) * nr_mem_map);*/
    for(k= 0;k < PAGING_PAGES;++k)
    {
        p = (struct page *)(page_ptr_start_mem) + k;

        (mem_map + k )->address = page_start_mem;

        free_page(p);
        page_start_mem += PAGE_SIZE;
    }

    buddy_list_tidy();

    printk("free pages = %d\n",PAGING_PAGES);
    return ;
}

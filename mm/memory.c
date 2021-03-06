/* memory managment */
#include "type.h"
#include "const.h"
#include "string.h"
#include "stdlib.h"
#include "panic.h"
#include "mm.h"
#include "page_alloc.h"
#include "printf.h"

int nr_swap_pages = 0;

unsigned long low_mem_start = 0x600000;
unsigned long low_mem_end = 0x1000000;

/*unsigned long page_start_mem = 0;*/
/* number of the pages */
struct page ** mem_map = NULL;

struct list_head inactive_list;
/*static int count = 0;*/

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

unsigned long alloc_low_mem(int memsize)
{
    if(low_mem_start + memsize > low_mem_end)
    {
        panic("no low memory alloc.\n");
        return -1;
    }

    unsigned long allow_mem = low_mem_start;
    low_mem_start += (memsize + SAFE_GAP);

    return allow_mem;
}

void do_no_page (struct vm_area_struct *vma,unsigned long addr, int access_write)
{
    
    unsigned long address;
    address = get_free_page(GFP_KERNEL);

        /*oom();*/
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
    if(pte_iswrite(pte))
        goto  end_wp_page;
    old_page = pte_page(pte)->address;
    if(old_page >= main_memory_end)
        goto bad_wp_page;

    /*(vma->vm_task->mm->min_flt)++;*/

    /*if(mem_map[MAP_NR(old_page)].flags & PAGE_PRESENT)
      {
      if(new_page)
      {
      if(mem_map[MAP_NR(old_page)].flags & MAP_PAGE_RESERVED)
      [>++(vma->vm_task->mm->rss);<]
      copy_page(old_page, new_page);
     *page_table = pte_mkwrite(pte_mkdirty(mk_pte((unsigned long)&new_page, vma->vm_page_prot)));
     [>free_page(old_page);<]
     return;
     }
     pte_val(*page_table) &= PAGE_BAD;
     [>free_page(old_page);<]
     oom();
     return;
     }*/
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

/* free area init */
static void free_area_init()
{
    INIT_LIST_HEAD(&inactive_list);
}

int init_mem()
{
    int k;
    struct page *p = NULL;
    unsigned long page_ptr_mem = 0;
    unsigned long address = 0;

    /* buddy init */
    buddy_list_init();
    /* slab init */
    kmem_cache_init();

    struct boot_params bp;
    get_boot_params(&bp);
    int memory_size = bp.mem_size;
    memory_size  &= 0xfffff000;

    page_fns = MAP_NR(memory_size);
    /*printk("page_fns = %d\n", page_fns);*/

    mem_map = (struct page **)alloc_low_mem(page_fns * sizeof(struct page *));
    page_ptr_mem = alloc_low_mem(page_fns * sizeof(struct page));

    p = (struct page*)page_ptr_mem;

    for(k = 0;p != NULL && k < page_fns; ++k)
    {
        if(address < low_mem_end )
        {
            p->flags |= (SYS_RESERVED | SYS_ROM);
        }
        else
        {
            p->flags |= SYS_RAM;
        }

        p->address = address;
        address += PAGE_SIZE;
        *(mem_map + k ) = p;

        free_page(p);
        ++p;
    }

    /* tidy the buddy list : merge and sort*/
    buddy_list_tidy();
    init_kmem_cache();
    free_area_init();

    return 0;
}

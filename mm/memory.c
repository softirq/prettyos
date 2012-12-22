#include "type.h"
#include "const.h"
#include "string.h"
#include "stdlib.h"
#include "list.h"
#include "panic.h"
#include "wait.h"
#include "mm.h"
#include "printf.h"
#include "list.h"
/*#include "pgtable.h"*/

static long PAGING_PAGES = 0;
int nr_swap_pages = 0;
int nr_free_pages = 0;

unsigned long low_mem_start = 0x600000;
unsigned long low_mem_end = 0x1000000;

/*unsigned long page_start_mem = 0;*/
/* number of the pages */
unsigned long page_fns = 0;

struct page * mem_map = NULL;

struct mem_list buddy_list[NR_MEM_LISTS];

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

static void buddy_list_init ()
{
    int i;
    for(i = 0;i < NR_MEM_LISTS; i++)
    {
        INIT_LIST_HEAD(&(buddy_list[i].list));
        buddy_list[i].nr_free_pages = 0;
    }
}

/*tidy the buddy list*/
static int buddy_list_tidy()
{
    int i ;
    int count = 0;
    struct mem_list *queue = NULL;
    struct page *page = NULL;
    struct list_head *head, *pos, *n;

    for(i = 0;i < NR_MEM_LISTS; i++)
    {
        queue = buddy_list + i;
        printk("i = %d nr_free_pages = %d  ", i, queue->nr_free_pages);
        head = &(queue->list);

        pos = head->next;
        while(pos && pos != head)
        {
            ++count;
            if(count > 2400)
            {
                printk("%d ", count);
                page = list_entry(pos, Page, list);
            }
            pos = pos->next;
        }
    }

    return 0;
}

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

static inline int add_buddy_queue (struct page *page, unsigned long order)
{
    if(page == NULL || order >= NR_MEM_LISTS)
    {
        return -1;
    }

    struct mem_list *header = buddy_list + order;
    list_add(&(page->list),&(header->list));

    /*struct list_head *head, *item; 
      head = &(header->list); 
      item = &(page->list);
      if(head == NULL || item == NULL)
      {
      return -2;
      }
      [>head->next->prev = item;<]
      item->next = head->next;
      [>item->prev = head;<]
      head->next = item;

      tmp = head->next;
      count = 0;

      while(tmp && tmp != head)
      {
      ++count;
      if(count > 2412)
      {
      printk(".%d.", count);
      }

      tmp = tmp->next;
      }*/

    ++buddy_list[order].nr_free_pages;

    return 0;
}

static inline int free_pages_ok(struct page *page, unsigned long order)
{
    if(page == NULL || order >= NR_MEM_LISTS)
        return -1;

    if(add_buddy_queue(page, order) < 0)
        return -2;

    return 0;
}

int free_pages(struct page *page, unsigned long order)
{
    if(page == NULL || order >= NR_MEM_LISTS)
        return -1;

    if(page->flags & SYS_RAM)
    {
        if(free_pages_ok(page, order) < 0)
            return -2;
    }

    return 0;
}

unsigned long get_free_pages(unsigned long order)
{
    if(order >= NR_MEM_LISTS)
        return -1;

    struct mem_list *queue = buddy_list + order;
    if(queue == NULL)
        return -2;

    unsigned long new_order = order;
    do
    {
        /*struct mem_list *next = queue->next;
          if(queue != next)
          {
          queue->next = next->next;
          queue->next->prev = queue;
          nr_free_pages -= (1 << order);
          return (unsigned long)next;
          }*/
        new_order++;
        queue++;

    }while(new_order < NR_MEM_LISTS);

    return 0;
}

unsigned long __get_free_pages(int priority, unsigned long order)
{
    if(priority == GFP_ATOMIC)
        return get_free_pages(order);

    return 0;
}

void init_mem()
{
    int k;
    struct page *p = NULL;
    unsigned long page_ptr_mem = 0;
    unsigned long address = 0;

    buddy_list_init();

    struct boot_params bp;
    get_boot_params(&bp);
    int memory_size = bp.mem_size;
    memory_size  &= 0xfffff000;

    page_fns = MAP_NR(memory_size);
    printk("page_fns = %d\n", page_fns);

    /*alloc memory to mem_map;*/
    mem_map = (struct page *)alloc_low_mem(page_fns * sizeof(struct page *));
    /* alloc memory to struct page */
    printk("%x.\n", low_mem_start);
    printk("buffer_memory_end = %x\n, buffer_memory_end");
    printk("page size = %d\n", sizeof(struct page));
    page_ptr_mem = alloc_low_mem(page_fns * sizeof(struct page));
    printk("%x %x.\n", page_ptr_mem, page_ptr_mem + page_fns * sizeof(struct page));

    p = (struct page*)page_ptr_mem;

    for(k = 0;p != NULL && k < page_fns; ++k)
    {
        if(address < low_mem_end || address < buffer_memory_end)
        {
            /* system reserved and read only */
            p->flags |= (SYS_RESERVED | SYS_ROM);
        }
        else
        {
            p->flags |= SYS_RAM;
        }
        p->address = address;
        address += PAGE_SIZE;
        *(mem_map + k ) = *p;

        free_page(p);
        ++p;
    }

    buddy_list_tidy();

    return ;
}


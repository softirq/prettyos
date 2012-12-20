#include "type.h"
#include "const.h"
#include "traps.h"
#include "string.h"
#include "tty.h"
#include "console.h"
#include "panic.h"
#include "printf.h"
#include "fork.h"
#include "stdlib.h"
#include "wait.h"
#include "mm.h"
#include "sched.h"
#include "global.h"
#include "pgtable.h"
#include "page.h"
#include "swap.h"
#include "bitops.h"
#include "kstat.h"
#include "proc.h"

pgd_t swapper_pg_dir[1024];

inline int pmd_none(pmd_t pmd)
{
    return !(pmd_val(pmd));
}

inline int pmd_bad(pmd_t pmd)
{
    return (((pmd_val(pmd) & ~PAGE_MASK) != PAGE_TABLE) || (pmd_val(pmd) > main_memory_end));
}

inline int pte_none(pte_t pte)
{
    return !(pte_val(pte));
}

inline int pgd_none(pgd_t pgd)
{
    return !(pgd_val(pgd));
}

inline int pgd_bad(pgd_t pgd)
{
    return 0;
}

inline int pte_present(pte_t pte)
{
    return (pte_val(pte) & PAGE_PRESENT);
}

inline int pte_clear(pte_t pte)
{
    return pte_val(pte) = 0;
}

inline int pmd_clear(pmd_t pmd)
{
    return pmd_val(pmd) = 0;
}

inline int pgd_clear(pgd_t pgd)
{
    return pgd_val(pgd) = 0;
}

inline struct page * pte_page(pte_t pte)
{
    return (mem_map + MAP_NR(pte_val(pte)));
}

inline struct page * pmd_page(pmd_t pmd)
{
    return (mem_map + MAP_NR(pmd_val(pmd)));
}

inline struct page * pgd_page(pgd_t pgd)
{
    return (mem_map + MAP_NR(pgd_val(pgd)));
}

inline void pte_free(pte_t pte)
{
    free_page(pte_page(pte));
}

inline void pmd_free(pmd_t pmd)
{
    free_page(pmd_page(pmd));
}

inline void pgd_free(pgd_t pgd)
{
    free_page(pgd_page(pgd));
}

inline int pte_inuse(pte_t *pte)
{
    return (mem_map[MAP_NR(pte)].flags == PAGE_PRESENT);
}

inline int pmd_inuse(pmd_t *pmd)
{
    return (mem_map[MAP_NR(pmd)].flags == PAGE_PRESENT);
}

inline int pgd_inuse(pgd_t *pgd)
{
    return (mem_map[MAP_NR(pgd)].flags == PAGE_PRESENT);

}

inline pte_t mk_pte(unsigned long address, pgprot_t pgprot)
{
    pte_t pte;
    pte_val(pte) = __va(address) | pgprot_val(pgprot);
    return pte;
}

inline pte_t * pte_offset(pmd_t *pmd, unsigned long address)
{
    return (pte_t *)(pmd_page(*pmd) + ((address > PAGE_SHIFT) & (PTRS_PER_PTE - 1)));
}

inline pmd_t * pmd_offset(pgd_t *pgd, unsigned long address)
{
    return (pmd_t *)pgd;
}
inline pgd_t * pgd_offset(struct task_struct *tsk, unsigned long address)
{
    return (pgd_t *)(tsk->tss.cr3 + (address >> PGDIR_SHIFT));
}

inline pmd_t * pmd_alloc (pgd_t *pgd, unsigned long address)
{
    return (pmd_t *)pgd;
}

inline int pte_write(pte_t pte)
{
    return (pte_val(pte) & PAGE_RW);
}

inline pte_t pte_mkwrite(pte_t pte)
{
    pte_val(pte) |= PAGE_RW;
    return pte;
}

inline pte_t  pte_mkyoung(pte_t pte)
{
    pte_val(pte) |= PAGE_ACCESSED;
    return pte;
}

inline pte_t pte_mkdirty(pte_t pte)
{
    pte_val(pte) |= PAGE_DIRTY;
    return pte;
}

inline pte_t * pte_alloc (pmd_t *pmd, unsigned long address)
{
    address = (address >> PAGE_SHIFT) & (PTRS_PER_PTE -1);
    if(pmd_none(*pmd))
    {
        pte_t *page = (pte_t*)get_free_page(GFP_KERNEL);
        if(pmd_none(*pmd))
        {
            if(page)
            {
                pmd_val(*pmd) = (PAGE_TABLE | (unsigned long)page) ;
                return page + address;
            }
            pmd_val(*pmd) = (PAGE_TABLE | PAGE_BAD);
            return NULL;
        }
        free_page(pte_page(*page));
    }
    if(pmd_bad(*pmd))
    {
        printk("Bad pmd in pte_alloc: %08lx\n", pmd_val(*pmd));
        pmd_val(*pmd) = PAGE_TABLE | PAGE_BAD;
        return NULL;
    }

    return (pte_t *)(pmd_page(*pmd) + address);
}

static inline void forget_pte(pte_t page)
{
    if(pte_none(page))
        return;

    if(pte_present(page))
    {
        free_page(pte_page(page));
        if(mem_map[MAP_NR(&page)].flags == PAGE_PRESENT)
            return;
        if(current->mm->rss <= 0)
            return;
        current->mm->rss--;
        return;
    }
    swap_free(*((swap_entry_t *)&(pte_val(page))));
    return;
}

inline void free_one_pte(pte_t *pgtable)
{
    pte_t page = *pgtable;
    if(pte_none(page))
    {
        return;
    }
    pte_clear(*pgtable);
    if(!pte_present(page))
    {
        return;
    }
    free_page(pte_page(page));
    return;
}

inline void free_one_pmd(pmd_t *pmd)
{
    int tmp;
    pte_t *pte;
    if(pmd_none(*pmd))
    {
        return;
    }
    pte = pte_offset(pmd,0);
    pmd_clear(*pmd);
    if(pte_inuse(pte))
    {
        pte_free(*pte);
        return;
    }

    for(tmp = 0; tmp < PTRS_PER_PTE;tmp++)
    {
        free_one_pte(pte + tmp);
    }
    return;
}

inline void free_one_pgd(pgd_t *pgd)
{
    int tmp;
    pmd_t *pmd;
    if(pgd_none(*pgd))
    {
        return;
    }
    pmd = pmd_offset(pgd, 0);
    pgd_clear(*pgd);
    if(pmd_inuse(pmd))
    {
        pmd_free(*pmd);
        return;
    }
    for(tmp = 0; tmp < PTRS_PER_PMD; tmp++)
    {
        free_one_pmd(pmd + tmp);
    }
    return;
}

inline void free_page_tables(struct task_struct *tsk)
{
    int tmp;
    pgd_t *pgd;
    pgd = pgd_offset(tsk,0);

    if(!pgd || pgd == swapper_pg_dir)
    {
        printk("trying to clear kernel page-directory: not good\n");
        return;
    }
    if(pgd_inuse(pgd))
    {
        free_page(pgd_page(*pgd));
    }

    for(tmp = 0; tmp < PTRS_PER_PGD;tmp++)
    {
        free_one_pgd(pgd + tmp);
    }
    pgd_free(*pgd);

}

static void put_page(pte_t *page_table, pte_t pte)
{
    if(!(pte_none(*page_table)))
    {
        printk("put_page: page already exists %08lx\n", pte_val(*page_table));
        free_page(pte_page(pte));
        return;
    }
    *page_table = pte;
}

inline void get_empty_page(struct vm_area_struct *vma, pte_t *page_table)
{
    unsigned long tmp;
    if(!(tmp = get_free_page(GFP_KERNEL)))
    {
        oom();
                        /*put_page(page_table, PAGE_BADTABLE);*/
        return;
    }

    put_page(page_table,pte_mkwrite(mk_pte(tmp, vma->vm_page_prot)));
}

static inline void unmap_pte_range(pmd_t *pmd, unsigned long addr, unsigned long size)
{
    pte_t *pte;
    unsigned long end;

    if(pmd_none(*pmd))
        return;
    if(pmd_bad(*pmd))
    {
        printk("unmap_pte_range: bad pmd (%08lx)\n", pmd_val(*pmd));
        pmd_clear(*pmd);
        return;
    }

    pte = pte_offset(pmd, addr);
    addr &= ~PMDIR_MASK;
    end = addr + size;
    if(end >= PMDIR_SIZE)
        end = PMDIR_SIZE;
    do
    {
        pte_t page = *pte;
        pte_clear(page);
        forget_pte(page);
        addr += PAGE_SIZE;
        pte++;
    }while(addr < end);

    return;
}

static inline void unmap_pmd_range(pgd_t *pgd, unsigned long addr, unsigned long size)
{
    pmd_t *pmd;
    unsigned long end;

    if(pgd_none(*pgd))
        return;
    if(pgd_bad(*pgd))
    {
        printk("unmap_pmd_range: bad pgd (%08lx)\n", pgd_val(*pgd));
        pgd_clear(*pgd);
        return;
    }

    pmd = pmd_offset(pgd, addr);
    addr &= ~PMDIR_MASK;
    end = addr + size;
    if(end > PGDIR_SIZE)
        end = PGDIR_SIZE;
    do{
        unmap_pte_range(pmd, addr, end - addr);
        addr = (addr + PMDIR_SIZE) & PMDIR_MASK;
        pmd++;
    }while(addr < end);
    return; 
}

int unmap_page_range(unsigned long addr, unsigned long size)
{
    pgd_t *pgd;
    unsigned long end = addr + size;
    pgd = pgd_offset(current, addr);

    if(pgd_none(*pgd))
        return -1;
    do{
        unmap_pmd_range(pgd, addr, end - addr);
        addr = (addr + PGDIR_SIZE) & PGDIR_MASK;
        pgd++;
    }while(addr < end);
    return 0;
}

/*first in boot/loader.asm first setup the pgd talbe in function SetupPaging
 * now set the pte after get the real memory in the system*/
unsigned long paging_init(const unsigned long start_mem, const unsigned long end_mem)
{
    int k = 0;
    unsigned long address = 0;
    int count = 1;

    pgd_t *pg_dir = NULL;
    pte_t *pg_table = NULL;

    pg_dir = swapper_pg_dir;

    while(address < end_mem)
    {
        /*the first 768 entries is the directory mapping*/
        if(count <= HIGH_MEM_ENTRY)
        {
            pg_table = (pte_t *)(pgd_val(*pg_dir));
            /*start_mem += PAGE_SIZE;*/
            for(k = 0;k < PTRS_PER_PTE; ++k,pg_table++)
            {
                if(address < end_mem)
                    *pg_table =  mk_pte(address,PAGE_SHARED);
                address += PAGE_SIZE;
            }
            ++pg_dir;
            ++count;
        }
        /* the rest for vmalloc or persistent mapping or fixmaped */
        else
        {
        }
    }

    /*free_list_init(start_mem, end_mem);*/

    return 0;
}

unsigned long get_free_pages(unsigned long order)
{
    struct mem_list *queue = buddy_list + order;
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

static inline void add_mem_queue (struct page *page, unsigned long order)
{
    struct mem_list *header = buddy_list + order;
    list_add_after(&(header->list), &(page->list));
}

static inline void free_pages_ok(struct page *page, unsigned long order)
{
    add_mem_queue(page, order);
}

void free_pages(struct page *page, unsigned long order)
{
    if(!(page->flags & MAP_PAGE_RESERVED))
    {
        free_pages_ok(page, order);
        return;
    }

    printk("Trying to free free memory (%081x):memory probably corrupted\n",page->address);
    return;
}


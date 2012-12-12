typedef unsigned int mem_map_t;


typedef struct { unsigned long pte; } pte_t;
typedef struct { unsigned long pmd; } pmd_t;
typedef struct { unsigned long pgd; } pgd_t;
typedef struct { unsigned long pgprot; } pgprot_t;


#define pgd_val(x)  ((x).pgd)
#define pmd_val(x)  ((x).pmd)
#define pte_val(x)  ((x).pte)
#define pgprot_val(x)   ((x).pgprot)

#define PAGE_SHIFT 			12
#define PGDIR_SHIFT 		22
#define PGDIR_SIZE 			(1UL << PGDIR_SHIFT)
#define PMDIR_SIZE 			PGDIR_SIZE
#define PGDIR_MASK 			(~(PGDIR_SIZE - 1))
#define PMDIR_MASK 			PGDIR_MASK

#define __PAGE_OFFSET 		(0xC0000000UL)

#define PAGE_SIZE 			(1UL << PAGE_SHIFT)
#define PAGE_MASK 			(~(PAGE_SIZE - 1))

#define __pte(x)        ((pte_t) { (x) } )
#define __pmd(x)        ((pmd_t) { (x) } )
#define __pgd(x)        ((pgd_t) { (x) } )
#define __pgprot(x)     ((pgprot_t) { (x) } )

#define 	virt_to_page(addr)  	 (mem_map + ((unsigned long)(addr - PAGE_OFFSET) >> PAGE_SHIFT))
#define 	page_to_virt(page) 		 (((page - mem_map) >> PAGE_SHIFT) + PAGE_OFFSET)


#define 	PAGE_ALIGN(addr) 		( (addr + PAGE_SIZE -1) & PAGE_MASK )

#define MAP_PAGE_RESERVED 	(1 << 15 )

#define PAGE_OFFSET 	(unsigned long)__PAGE_OFFSET
#define __pa(x) 	((unsigned long)(x) - PAGE_OFFSET)
#define __va(x) 	((void *)((unsigned long)(x) + PAGE_OFFSET))

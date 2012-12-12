pgd_t swapper_pg_dir[1024];

#define PTRS_PER_PTE 	1024
#define PTRS_PER_PMD  	1	
#define PTRS_PER_PGD 	1024


#define 	PAGE_PRESENT 		0x001
#define 	PAGE_RW 			0x002
#define 	PAGE_USER 			0x004
#define 	PAGE_ACCESSED 		0x008
#define 	PAGE_DIRTY 			0X010
#define 	PAGE_BAD 			0x800

#define  	PAGE_SHARED 		__pgprot(PAGE_PRESENT | PAGE_RW | PAGE_USER)
#define 	PAGE_TABLE 			(PAGE_PRESENT | PAGE_RW | PAGE_USER | PAGE_DIRTY)
#define  	PAGE_BADTABLE 		__pgprot(PAGE_BAD)


extern inline pgd_t* pgd_offset(struct task_struct *tsk, unsigned long address);
extern inline int pgd_none(pgd_t pgd);
extern inline int pgd_bad(pgd_t pgd);

extern inline pmd_t * pmd_alloc (pgd_t *pgd, unsigned long address);
extern inline pmd_t * pmd_offset(pgd_t *pgd, unsigned long address);
extern inline int pmd_none(pmd_t pmd);
extern inline int pmd_bad(pmd_t pmd);
extern inline unsigned long  pmd_page (pmd_t pmd);

extern inline pte_t * pte_alloc (pmd_t *pmd, unsigned long address);
extern inline pte_t mk_pte(unsigned long addr, pgprot_t pgprot);
extern inline pte_t  pte_mkyoung(pte_t pte);
extern inline pte_t  pte_mkdirty(pte_t pte);
extern inline pte_t	 pte_mkwrite(pte_t pte);
extern inline int pte_present(pte_t pte);
extern inline int pte_none(pte_t pte);
extern inline int pte_write(pte_t pte);
extern inline pte_t * pte_offset(pmd_t *pmd, unsigned long address);
extern inline unsigned long pte_page(pte_t pte);

extern inline void free_page_tables(struct task_struct *tsk);

extern int unmap_page_range(unsigned long addr, unsigned long size);

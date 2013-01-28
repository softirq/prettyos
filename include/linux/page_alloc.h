#ifndef     _PAGE_ALLOC_H_
#define     _PAGE_ALLOC_H_

extern unsigned long get_free_pages(int order);
extern unsigned long get_free_page(int priority);
extern int free_pages(struct page *page, const int order);
extern unsigned long alloc_mem(const size_t size);

extern unsigned long alloc_page();
extern unsigned long alloc_pages(const int order);

#define  	__get_free_page(priority) 		__get_free_pages((priority),0)
#define     page_address(page)      (page->address)

#endif

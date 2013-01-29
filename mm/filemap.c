/* address_space -- page cache
 * */
#include "type.h"
#include "const.h"
#include "string.h"
#include "stdlib.h"
#include "panic.h"
#include "mm.h"
#include "swap.h"
#include "atomic.h"

static int __add_to_page_cache(struct page *page, struct address_space *mapping, unsigned int index)
{
    list_add(&page->list, &mapping->clean_pages);
    page->mapping = mapping;
    page->index = index;

    return 0;
}

static int _add_to_page_cache(struct page *page, struct address_space *mapping, unsigned int offset)
{
    get_page(page);

    if((radix_tree_insert(&mapping->page_tree, offset, page)) < 0)
        return -1;

    __add_to_page_cache(page, mapping, offset);

    return 0;
}

/*add page to address_space*/
int add_to_page_cache(struct page *page, struct address_space *mapping, unsigned int offset)
{
    if((_add_to_page_cache(page, mapping, offset)) < 0)

        return -1;
    lru_cache_add(page);
    return 0;
}

static int page_cache_read(struct file * file, unsigned long offset)  
{
    struct address_space *mapping = NULL;;
    mapping = file->f_inode->i_mapping;
    return 0;
}

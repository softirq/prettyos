#include "type.h"
#include "const.h"
/*#include "global.h"*/
#include "stdlib.h"
#include "list.h"

inline void INIT_LIST_HEAD(struct list_head *list)
{
    list->next = list;
    list->prev = list;
}
inline void __list_add(struct list_head *item, struct list_head *prev, struct list_head *next)
{
    item->next = next;
    next->prev = item;
    item->prev = prev;
    prev->next = item;
}

inline void list_add_after(struct list_head *item, struct list_head *head)
{
    __list_add(item, head, head->next);
}

inline void list_add_before(struct list_head *item, struct list_head *head)
{
    __list_add(item,head->prev, head);
}

inline void __list_del(struct list_head *prev, struct list_head *next)
{
    next->prev = prev;

    if(prev)
    {
        prev->next = next;
    }
}

inline void list_del(struct list_head *entry)
{
    __list_del(entry->prev, entry->next);
    entry->next = LIST_POISON1;
    entry->prev = LIST_POISON2;
}

inline void list_del_init(struct list_head *entry)
{
    __list_del(entry->prev, entry->next);
    INIT_LIST_HEAD(entry);
}

inline int list_empty(const struct list_head *head)
{
    return (head->next == head);
}

inline int list_empty_careful(const struct list_head *head)
{
    return (head->next == head) && (head == head->prev);
}

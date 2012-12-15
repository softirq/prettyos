#ifndef     _LIST_H_
#define     _LIST_H_

#include "linux/poison.h"

struct list_head
{
    struct list_head *next;
    struct list_head *prev;
};

#define LIST_HEAD_INIT(name) {&(name), &(name)}

#define LIST_HEAD(name) 	\
    struct list_head name = LIST_HEAD_INIT(name)
static inline void INIT_LIST_HEAD(struct list_head *list)
{
    list->next = list;
    list->prev = list;
}
static inline void __list_add(struct list_head *item, struct list_head *prev, struct list_head *next)
{
    item->next = next;
    next->prev = item;
    item->prev = prev;
    prev->next = item;
}

static inline void list_add_after(struct list_head *item, struct list_head *head)
{
    __list_add(item, head, head->next);
}

static inline void list_add_before(struct list_head *item, struct list_head *head)
{
    __list_add(item,head->prev, head);
}

#define list_add 		list_add_after
#define list_add_tial 	list_add_before

static inline void __list_del(struct list_head *prev, struct list_head *next)
{
    next->prev = prev;

    if(prev)
    {
        prev->next = next;
    }
}

static inline void list_del(struct list_head *entry)
{
    __list_del(entry->prev, entry->next);
    entry->next = LIST_POISON1;
    entry->prev = LIST_POISON2;
}

static inline void list_del_init(struct list_head *entry)
{
    __list_del(entry->prev, entry->next);
    INIT_LIST_HEAD(entry);
}

static inline int list_empty(const struct list_head *head)
{
    return (head->next == head);
}

static inline int list_empty_careful(const struct list_head *head)
{
    return (head->next == head) && (head == head->prev);

}

#define list_entry(ptr, type, member) 		\
    container_of(ptr, type, member)

#define list_first_entry(ptr, type, member) \
    list_entry((ptr)->next, type, member)

#define list_for_each_entry(pos, head, memeber) 					\
    for(pos = list_entry((head)->next, typeof(*pos),member); 	\
            &pos->member != (head); 								\
            pos = list_entry(pos->member.next,typeof(*pos), member))

#endif

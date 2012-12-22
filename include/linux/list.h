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
    do{struct list_head name = LIST_HEAD_INIT(name);}while(0)

inline void INIT_LIST_HEAD(struct list_head *list);
inline void __list_add(struct list_head *item, struct list_head *prev, struct list_head *next);
inline void list_add_after(struct list_head *item, struct list_head *head);
inline void list_add_before(struct list_head *item, struct list_head *head);

#define list_add 		list_add_after
#define list_add_tail 	list_add_before

inline void __list_del(struct list_head *prev, struct list_head *next);
inline void list_del(struct list_head *entry);
inline void list_del_init(struct list_head *entry);
inline int list_empty(const struct list_head *head);
inline int list_empty_careful(const struct list_head *head);

#define offsetof(type, member) ((unsigned long)&((type *)0)->member)

#define container_of(ptr, type, member)({ \
                const typeof(((type *)0)->member) *__mptr = (ptr); \
                (type *)((char *)__mptr - offsetof(type, member)); })

#define list_entry(ptr, type, member) \
        container_of(ptr, type, member)

/*  防止在遍历的时候 节点正好被删掉了*/
#define list_for_each_safe(pos, n, head) \
        for(pos = (head)->next, n = pos->next;pos != (head); pos = n, n = pos->next)

#define list_first_entry(ptr, type, member) \
    list_entry((ptr)->next, type, member)

#define list_for_each_entry(pos, head, memeber) 					\
    for(pos = list_entry((head)->next, typeof(*pos),member); 	\
            &pos->member != (head); 								\
            pos = list_entry(pos->member.next,typeof(*pos), member))

#endif

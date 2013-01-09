#include "type.h"
#include "const.h"
#include "traps.h"
#include "tty.h"
#include "console.h"
#include "wait.h"
#include "mm.h"
#include "sched.h"
#include "global.h"
#include "kernel.h"
#include "stdlib.h"
#include "timer.h"
#include "kstat.h"
#include "list.h"
#include "printf.h"
#include "asm-i386/system.h"
#include "rbtree.h"

#if defined(rb_entry)
#undef rb_entry
#define rb_entry(ptr, type, member) container_of(ptr, type, member)
#endif

struct sched_entity *se_search(struct rb_root *root, int vruntime)
{
    struct rb_node *node = root->rb_node;

    while (node) {
        struct sched_entity *se = container_of(node, struct sched_entity, run_node);
        int result;

        result = vruntime - se->vruntime;

        if (result < 0)
            node = node->rb_left;
        else if (result > 0)
            node = node->rb_right;
        else
            return se;
    }
    return NULL;
}

int ts_earse(struct rb_root *root, struct sched_entity *se)
{
    rb_erase(&(se->run_node),root);
    return 0;
}

int ts_insert(struct rb_root *root, struct sched_entity *se)
{
    int result = 0;
    struct rb_node **new = &(root->rb_node), *parent = NULL;

    /*  Figure out where to put new node */
    while (*new) {
        struct sched_entity *this = container_of(*new, struct sched_entity, run_node);
        if(this)
            result = se->vruntime - this->vruntime;
        else
            break;

        parent = *new;
        if (result < 0)
            new = &((*new)->rb_left);
        else if (result > 0)
            new = &((*new)->rb_right);
        else
            return 0;
    }

    /*  Add new node and rebalance tree. */
    rb_link_node(&se->run_node, parent, new);
    rb_insert_color(&se->run_node, root);

    return 1;
}

struct sched_entity * ts_leftmost(struct rb_root *root)
{
    struct rb_node *pnode = rb_first(root);
    struct sched_entity *se = rb_entry(pnode, struct sched_entity, run_node);
    return se;
}

void ts_free(struct sched_entity *se)
{
}

void rb_print(struct rb_node *pnode)
{
    if(pnode != NULL) 
    {
        if (pnode->rb_left != NULL) 
        {
            rb_print(pnode->rb_left);
        }

        printk("key = %d color = %d\t", rb_entry(pnode, struct sched_entity, run_node)->vruntime, rb_color(pnode));
        /*printk("key = %d color = %d\t", rb_entry(pnode, struct sched_entity, run_node)->vruntime, pnode->rb_parent_color);*/

        if (pnode->rb_right != NULL) 
        {
            rb_print(pnode->rb_right);
        }
    }
}

void rb_front_print(struct rb_node *pnode)
{
    if(pnode != NULL) 
    {
        printf("key = %d color = %lu\t ", rb_entry(pnode, struct sched_entity, run_node)->vruntime,rb_color(pnode));
        if (pnode->rb_left != NULL) 
        {
            rb_front_print(pnode->rb_left);
        }


        if (pnode->rb_right != NULL) 
        {
            rb_front_print(pnode->rb_right);
        }
    }
}


#if defined(rb_entry)
#undef rb_entry
#endif

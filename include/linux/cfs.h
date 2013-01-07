#ifndef     _CFS_H_
#define     _CFS_H_

#include "signal.h"
#include "fs.h"
#include "mm.h"
#include "rbtree.h"
#include "list.h"
#include "asm-i386/processor.h"
#include "asm-i386/traps.h"
#include "sched.h"

extern struct sched_entity *se_search(struct rb_root *root, int vruntime);
extern int ts_earse(struct rb_root *root, struct sched_entity *se);
extern int ts_insert(struct rb_root *root, struct sched_entity *se);
extern struct sched_entity * ts_leftmost(struct rb_root *root);
void ts_free(struct sched_entity *se);
void rb_print(struct rb_node *pnode);

#endif

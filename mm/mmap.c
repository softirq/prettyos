#include "type.h"
#include "const.h"
#include "traps.h"
#include "string.h"
#include "tty.h"
#include "panic.h"
#include "errno.h"
#include "console.h"
#include "printf.h"
#include "fork.h"
#include "wait.h"
#include "mm.h"
#include "fs.h"
#include "sched.h"
#include "global.h"
#include "proc.h"
#include "mmap.h"
#include "pgtable.h"
#include "malloc.h"


#define avl_empty   (struct vm_area_struct *) NULL
typedef unsigned long vm_avl_key_t;
#define 	vm_avl_key 		vm_end
#define 	avl_maxheight   41 
#define 	heightof(tree)  ((tree) == avl_empty ? 0 : (tree)->vm_avl_height)

static void avl_rebalance (struct vm_area_struct *** nodeplaces_ptr, int count)
{
    for ( ; count > 0 ; count--) {
        struct vm_area_struct ** nodeplace = *--nodeplaces_ptr;
        struct vm_area_struct * node = *nodeplace;
        struct vm_area_struct * nodeleft = node->vm_avl_left;
        struct vm_area_struct * noderight = node->vm_avl_right;
        int heightleft = heightof(nodeleft);
        int heightright = heightof(noderight);
        if (heightright + 1 < heightleft) {
            /*                                                      */
            /*                            *                         */
            /*                          /   \                       */
            /*                       n+2      n                     */
            /*                                                      */
            struct vm_area_struct * nodeleftleft = nodeleft->vm_avl_left;
            struct vm_area_struct * nodeleftright = nodeleft->vm_avl_right;
            int heightleftright = heightof(nodeleftright);
            if (heightof(nodeleftleft) >= heightleftright) {
                /*                                                        */
                /*                *                    n+2|n+3            */
                /*              /   \                  /    \             */
                /*           n+2      n      -->      /   n+1|n+2         */
                /*           / \                      |    /    \         */
                /*         n+1 n|n+1                 n+1  n|n+1  n        */
                /*                                                        */
                node->vm_avl_left = nodeleftright; nodeleft->vm_avl_right = node;
                nodeleft->vm_avl_height = 1 + (node->vm_avl_height = 1 + heightleftright);
                *nodeplace = nodeleft;
            } else {
                /*                                                        */
                /*                *                     n+2               */
                /*              /   \                 /     \             */
                /*           n+2      n      -->    n+1     n+1           */
                /*           / \                    / \     / \           */
                /*          n  n+1                 n   L   R   n          */
                /*             / \                                        */
                /*            L   R                                       */
                /*                                                        */
                nodeleft->vm_avl_right = nodeleftright->vm_avl_left;
                node->vm_avl_left = nodeleftright->vm_avl_right;
                nodeleftright->vm_avl_left = nodeleft;
                nodeleftright->vm_avl_right = node;
                nodeleft->vm_avl_height = node->vm_avl_height = heightleftright;
                nodeleftright->vm_avl_height = heightleft;
                *nodeplace = nodeleftright;
            }
        }
        else if (heightleft + 1 < heightright) {
            /* similar to the above, just interchange 'left' <--> 'right' */
            struct vm_area_struct * noderightright = noderight->vm_avl_right;
            struct vm_area_struct * noderightleft = noderight->vm_avl_left;
            int heightrightleft = heightof(noderightleft);
            if (heightof(noderightright) >= heightrightleft) {
                node->vm_avl_right = noderightleft; noderight->vm_avl_left = node;
                noderight->vm_avl_height = 1 + (node->vm_avl_height = 1 + heightrightleft);
                *nodeplace = noderight;
            } else {
                noderight->vm_avl_left = noderightleft->vm_avl_right;
                node->vm_avl_right = noderightleft->vm_avl_left;
                noderightleft->vm_avl_right = noderight;
                noderightleft->vm_avl_left = node;
                noderight->vm_avl_height = node->vm_avl_height = heightrightleft;
                noderightleft->vm_avl_height = heightright;
                *nodeplace = noderightleft;
            }
        }
        else {
            int height = (heightleft<heightright ? heightright : heightleft) + 1;
            if (height == node->vm_avl_height)
                break;
            node->vm_avl_height = height;
        }
    }
}


/* Insert a node into a tree, and
 *  * return the node to the left of it and the node to the right of it.                                                       
 *   */
static void avl_insert_neighbours (struct vm_area_struct * new_node, struct vm_area_struct ** ptree,                        
        struct vm_area_struct ** to_the_left, struct vm_area_struct ** to_the_right)                                            
{   
    vm_avl_key_t key = new_node->vm_avl_key;
    struct vm_area_struct ** nodeplace = ptree;
    struct vm_area_struct ** stack[avl_maxheight];                                                                          
    int stack_count = 0;
    struct vm_area_struct *** stack_ptr = &stack[0]; /* = &stack[stackcount] */                                             
    *to_the_left = *to_the_right = NULL;                                                                                    
    for (;;) { 
        struct vm_area_struct * node = *nodeplace;                                                                          
        if (node == avl_empty)                                                                                              
            break;
        *stack_ptr++ = nodeplace; stack_count++;                                                                            
        if (key < node->vm_avl_key) {                                                                                       
            *to_the_right = node;
            nodeplace = &node->vm_avl_left;                                                                                 
        } else {
            *to_the_left = node;
            nodeplace = &node->vm_avl_right;                                                                                
        }                                                                                                                   
    }
    new_node->vm_avl_left = avl_empty;
    new_node->vm_avl_right = avl_empty;                                                                                     
    new_node->vm_avl_height = 1;                                                                                            
    *nodeplace = new_node;
    avl_rebalance(stack_ptr,stack_count);                                                                                   
}           

static void avl_insert (struct vm_area_struct * new_node, struct vm_area_struct ** ptree)
{
    vm_avl_key_t key = new_node->vm_avl_key;
    struct vm_area_struct ** nodeplace = ptree;
    struct vm_area_struct ** stack[avl_maxheight];
    int stack_count = 0;
    struct vm_area_struct *** stack_ptr = &stack[0]; /* = &stack[stackcount] */
    for (;;) {
        struct vm_area_struct * node = *nodeplace;
        if (node == avl_empty)
            break;
        *stack_ptr++ = nodeplace; stack_count++;
        if (key < node->vm_avl_key)
            nodeplace = &node->vm_avl_left;
        else
            nodeplace = &node->vm_avl_right;
    }   
    new_node->vm_avl_left = avl_empty;
    new_node->vm_avl_right = avl_empty;
    new_node->vm_avl_height = 1;
    *nodeplace = new_node;
    avl_rebalance(stack_ptr,stack_count);
}

/* Removes a node out of a tree. */
static void avl_remove (struct vm_area_struct * node_to_delete, struct vm_area_struct ** ptree)                             
{   
    vm_avl_key_t key = node_to_delete->vm_avl_key;                                                                          
    struct vm_area_struct ** nodeplace = ptree;
    struct vm_area_struct ** stack[avl_maxheight];                                                                          
    int stack_count = 0;
    struct vm_area_struct *** stack_ptr = &stack[0]; /* = &stack[stackcount] */                                             
    struct vm_area_struct ** nodeplace_to_delete;                                                                           
    for (;;) { 
        struct vm_area_struct * node = *nodeplace;
        if (node == avl_empty) {
            /* what? node_to_delete not found in tree? */
            printk("avl_remove: node to delete not found in tree\n");
            return;
        }
        *stack_ptr++ = nodeplace; stack_count++;
        if (key == node->vm_avl_key)
            break;
        if (key < node->vm_avl_key)
            nodeplace = &node->vm_avl_left;
        else
            nodeplace = &node->vm_avl_right;
    }
    nodeplace_to_delete = nodeplace;
    /* Have to remove node_to_delete = *nodeplace_to_delete. */
    if (node_to_delete->vm_avl_left == avl_empty) {
        *nodeplace_to_delete = node_to_delete->vm_avl_right;
        stack_ptr--; stack_count--;
    } else {
        struct vm_area_struct *** stack_ptr_to_delete = stack_ptr;
        struct vm_area_struct ** nodeplace = &node_to_delete->vm_avl_left;
        struct vm_area_struct * node;
        for (;;) {
            node = *nodeplace;
            if (node->vm_avl_right == avl_empty)
                break;
            *stack_ptr++ = nodeplace; stack_count++;
            nodeplace = &node->vm_avl_right;
        }
        *nodeplace = node->vm_avl_left;
        /* node replaces node_to_delete */
        node->vm_avl_left = node_to_delete->vm_avl_left;
        node->vm_avl_right = node_to_delete->vm_avl_right;
        node->vm_avl_height = node_to_delete->vm_avl_height;
        *nodeplace_to_delete = node; /* replace node_to_delete */
        *stack_ptr_to_delete = &node->vm_avl_left; /* replace &node_to_delete->vm_avl_left */
    }
    avl_rebalance(stack_ptr,stack_count);
}


/* Look up the nodes at the left and at the right of a given node. */
static void avl_neighbours (struct vm_area_struct * node, struct vm_area_struct * tree, struct vm_area_struct ** to_the_left, struct vm_area_struct ** to_the_right)
{
    vm_avl_key_t key = node->vm_avl_key;                                                                                    

    *to_the_left = *to_the_right = NULL;                                                                                    
    for (;;) {
        if (tree == avl_empty) {                                                                                            
            printk("avl_neighbours: node not found in the tree\n");                                                         
            return;
        }       
        if (key == tree->vm_avl_key)                                                                                        
            break;
        if (key < tree->vm_avl_key) {                                                                                       
            *to_the_right = tree;
            tree = tree->vm_avl_left;                                                                                       
        } else {
            *to_the_left = tree;                                                                                            
            tree = tree->vm_avl_right;                                                                                      
        }       
    }       
    if (tree != node) {                                                                                                     
        printk("avl_neighbours: node not exactly found in the tree\n");                                                     
        return;
    }       
    if (tree->vm_avl_left != avl_empty) {                                                                                   
        struct vm_area_struct * node;
        for (node = tree->vm_avl_left; node->vm_avl_right != avl_empty; node = node->vm_avl_right)                          
            continue; 
        *to_the_left = node;                                                                                                
    }       
    if (tree->vm_avl_right != avl_empty) {                                                                                  
        struct vm_area_struct * node;
        for (node = tree->vm_avl_right; node->vm_avl_left != avl_empty; node = node->vm_avl_left)                           
            continue; 
        *to_the_right = node;                                                                                               
    }       
    if ((*to_the_left && ((*to_the_left)->vm_next != node)) || (node->vm_next != *to_the_right))                            
        printk("avl_neighbours: tree inconsistent with list\n");
}

void remove_shared_vm_struct (struct vm_area_struct *mpnt)
{
    struct m_inode *inode = mpnt->vm_inode;
    if(!inode)
        return;
    if(mpnt->vm_next_share == mpnt)
    {
        if(inode->i_mmap != mpnt)
            printk("inode i_mmap ring corrupted!\n");
        inode->i_mmap = NULL;
        return;
    }

    if(inode->i_mmap == mpnt)
        inode->i_mmap = mpnt->vm_next_share;

    mpnt->vm_prev_share->vm_next_share = mpnt->vm_next_share;
    mpnt->vm_next_share->vm_prev_share = mpnt->vm_prev_share;
}

/* print a list */
static void printk_list (struct vm_area_struct * vma)
{
    printk("[");
    while (vma) {
        printk("%08lX-%08lX", vma->vm_start, vma->vm_end);
        vma = vma->vm_next;
        if (!vma)
            break;
        printk(" ");
    }
    printk("]");
}

/* print a tree */
static void printk_avl (struct vm_area_struct * tree)
{
    if (tree != avl_empty) {
        printk("(");
        if (tree->vm_avl_left != avl_empty) {
            printk_avl(tree->vm_avl_left);
            printk("<");
        }
        printk("%08lX-%08lX", tree->vm_start, tree->vm_end);
        if (tree->vm_avl_right != avl_empty) {
            printk(">");
            printk_avl(tree->vm_avl_right);
        }
        printk(")");
    }
}

static char *avl_check_point = "somewhere";

/* check a tree's consistency and balancing */
static void avl_checkheights (struct vm_area_struct * tree)
{
    int h, hl, hr;

    if (tree == avl_empty)
        return;
    avl_checkheights(tree->vm_avl_left);
    avl_checkheights(tree->vm_avl_right);
    h = tree->vm_avl_height;
    hl = heightof(tree->vm_avl_left);
    hr = heightof(tree->vm_avl_right);
    if ((h == hl+1) && (hr <= hl) && (hl <= hr+1))
        return;
    if ((h == hr+1) && (hl <= hr) && (hr <= hl+1))
        return;
    printk("%s: avl_checkheights: heights inconsistent\n",avl_check_point);
}

/* check that all values stored in a tree are < key */
static void avl_checkleft (struct vm_area_struct * tree, vm_avl_key_t key)
{
    if (tree == avl_empty)
        return;
    avl_checkleft(tree->vm_avl_left,key);
    avl_checkleft(tree->vm_avl_right,key);
    if (tree->vm_avl_key < key)
        return;
    printk("%s: avl_checkleft: left key %lu >= top key %lu\n",avl_check_point,tree->vm_avl_key,key);
}

/* check that all values stored in a tree are > key */
static void avl_checkright (struct vm_area_struct * tree, vm_avl_key_t key)
{
    if (tree == avl_empty)
        return;
    avl_checkright(tree->vm_avl_left,key);
    avl_checkright(tree->vm_avl_right,key);
    if (tree->vm_avl_key > key)
        return;
    printk("%s: avl_checkright: right key %lu <= top key %lu\n",avl_check_point,tree->vm_avl_key,key);
}

/* check that all values are properly increasing */
static void avl_checkorder (struct vm_area_struct * tree)
{
    if (tree == avl_empty)
        return;
    avl_checkorder(tree->vm_avl_left);
    avl_checkorder(tree->vm_avl_right);
    avl_checkleft(tree->vm_avl_left,tree->vm_avl_key);
    avl_checkright(tree->vm_avl_right,tree->vm_avl_key);
}

/* all checks */
static void avl_check (struct task_struct * task, char *caller)
{
    avl_check_point = caller;
    /*  printk("task \"%s\", %s\n",task->comm,caller); */
    /*  printk("task \"%s\" list: ",task->comm); printk_list(task->mm->mmap); printk("\n"); */
    /*  printk("task \"%s\" tree: ",task->comm); printk_avl(task->mm->mmap_avl); printk("\n"); */
    avl_checkheights(task->mm->mmap_avl);
    avl_checkorder(task->mm->mmap_avl);
}

/* Build the AVL tree corresponding to the VMA list. */ 
void build_mmap_avl(struct task_struct * task) 
{   
    struct vm_area_struct * vma; 
    task->mm->mmap_avl = NULL; 
    for (vma = task->mm->mmap; vma; vma = vma->vm_next) 
        avl_insert(vma, &task->mm->mmap_avl);   
}       
/* Look up the first VMA which satisfies  addr < vm_end,  NULL if none. */
struct vm_area_struct * find_vma(struct task_struct *task, unsigned long addr)
{
    struct vm_area_struct *result = NULL;
    struct vm_area_struct *tree;

    for(tree = task->mm->mmap_avl;;)
    {
        if(tree == avl_empty)
            return result;
        if(tree->vm_end > addr)
        {
            if(tree->vm_start < addr)
                return tree;
            result = tree;
            tree = tree->vm_avl_left;
        }
        else
            tree = tree->vm_avl_right;
    }
}

/*
 * merge the list of memory segments if possible. Redundant vm_area_structs are freed. This assumes that the list is ordered by address.
 * we don't need to traverse the entire list, only those segments which intersect or are adjacent to a given interval.
 */
void merge_segments (struct task_struct * task, unsigned long start_addr, unsigned long end_addr)
{
    struct vm_area_struct *prev, *mpnt, *next;
    mpnt = find_vma(task, start_addr);
    if(!mpnt)
        return;
    avl_neighbours(mpnt, task->mm->mmap_avl, &prev, &next);

    if(!prev)
    {
        prev = mpnt;
        mpnt = next;
    }
    for(; mpnt && prev->vm_start < end_addr; prev = mpnt, mpnt = next)
    {
        next = mpnt->vm_next;
        if(mpnt->vm_inode != prev->vm_inode)
            continue;
        if(mpnt->vm_pte != prev->vm_pte)
            continue;
        if(mpnt->vm_ops != prev->vm_ops)
            continue;
        if(mpnt->vm_flags != prev->vm_flags)
            continue;
        if(mpnt->vm_start != prev->vm_end)
            continue;

        if((mpnt->vm_inode != NULL) || (mpnt->vm_flags & VM_SHM))
        {
            if(prev->vm_offset + prev->vm_end - prev->vm_start != mpnt->vm_offset)
                continue;
        }

        avl_remove(mpnt, &task->mm->mmap_avl);
        prev->vm_end = mpnt->vm_end;
        prev->vm_next = mpnt->vm_next;
        if(mpnt->vm_ops && mpnt->vm_ops->close)
        {
            mpnt->vm_offset += mpnt->vm_end - mpnt->vm_start;
            mpnt->vm_start = mpnt->vm_end;
            mpnt->vm_ops->close(mpnt);
        }

        remove_shared_vm_struct(mpnt);
        if(mpnt->vm_inode)
            mpnt->vm_inode->i_count++;

        kfree_s(mpnt,sizeof(*mpnt));
        mpnt = prev;

    }
    return;
}
/*
 * insert vm struct into process list sotred by address and inot the inode's i_mmap ring
 */
void insert_vm_struct (struct task_struct *task, struct vm_area_struct *vma)
{
    struct vm_area_struct *share;
    struct m_inode *inode;

    struct vm_area_struct *prev, *next;

    avl_insert_neighbours(vma, &task->mm->mmap_avl, &prev, &next);
    if((prev? prev->vm_next:task->mm->mmap)!= next)
        printk("insert_vm_struct : tree inconsistent with list.\n");
    if(prev)
        prev->vm_next = vma;
    else
        task->mm->mmap = vma;

    inode = vma->vm_inode;
    if(!inode)
        return;

    if((share = inode->i_mmap))
    {
        vma->vm_next_share = share->vm_next_share;
        vma->vm_next_share->vm_prev_share = vma;
        share->vm_next_share = vma;
        vma->vm_prev_share = share;
    }
    else
        inode->i_mmap = vma->vm_next_share = vma->vm_prev_share = vma;
}
/*
 * no file mmap , zero the mmap area
 */
static int anon_map(struct m_inode *ino, struct file * file, struct vm_area_struct * vma)
{
    if (zeromap_page_range(vma->vm_start, vma->vm_end - vma->vm_start, vma->vm_page_prot))
        return -ENOMEM;
    return 0;
}

void unmap_fixup(struct vm_area_struct *area, unsigned long addr, size_t len)
{
    struct vm_area_struct *mpnt;
    unsigned long end = addr + len;

    if(addr < area->vm_start || addr >= area->vm_end || end <= area->vm_start || end > area->vm_end || end < addr)
    {
        printk("unmap_fixup: area = %lx-%lx, unmap %lx-%lx!!\n",
                area->vm_start, area->vm_end, addr, end);
        return;
    }

    if(addr == area->vm_start && end == area->vm_end)
    {
        if(area->vm_ops && area->vm_ops->close)
            area->vm_ops->close(area);
        if(area->vm_inode)
            iput(area->vm_inode);
        return;
    }

    if(end == area->vm_end)
        area->vm_end = addr;
    else if(addr == area->vm_start)
    {
        area->vm_offset += (end - area->vm_start);
        area->vm_start = end;
    }
    else
    {
        mpnt = (struct vm_area_struct *)kmalloc(sizeof(*mpnt), GFP_KERNEL);
        if(!mpnt)
            return;
        *mpnt = *area;
        mpnt->vm_offset += (end - area->vm_start);
        mpnt->vm_start = end;
        if(mpnt->vm_inode)
            mpnt->vm_inode->i_count++;
        if(mpnt->vm_ops && mpnt->vm_ops->open)
            mpnt->vm_ops->open(mpnt);
        //				area->vm_end = addr;
        insert_vm_struct(current,mpnt);
    }
    mpnt = (struct vm_area_struct *)kmalloc(sizeof(*mpnt), GFP_KERNEL);
    if(!mpnt)
        return;
    *mpnt = *area;
    mpnt->vm_end = addr;

    if(mpnt->vm_ops && mpnt->vm_ops->open)
        mpnt->vm_ops->open(mpnt);
    if(area->vm_ops && area->vm_ops->close)
    {
        area->vm_end = area->vm_start;
        area->vm_ops->close(area);
    }
    insert_vm_struct(current,mpnt);
}
unsigned long get_unmmapped_area(unsigned long len)
{
    struct vm_area_struct *vma;
    //	unsigned long gap_start = 0, gap_end;

    for(vma = current->mm->mmap; ; vma = vma->vm_next)
    {
        /*
           if(gap_start < SHM_RANGE_START)
           gap_start = SHM_RANGE_START;
           if(!vma || (gap_end = vma->vm_start) > SHM_RANGE_END)
           gap_end = SHM_RANGE_END;
           gap_start = PAGE_ALIGN(gap_start);
           gap_end &= PAGE_MASK;

           if((gap_start <= gap_end) && (gap_end - gap_start)>= len)
           return gap_start;
           if(!vma)
           return;
           gap_start = vma->vm_end;
           */
    }
    return 0;
}

unsigned long do_munmap(unsigned long addr, int len)
{
    struct vm_area_struct *mpnt, *prev, *next, **npp, *free;
    if((addr & ~PAGE_MASK) || (addr > PAGE_OFFSET) || (addr + len) > PAGE_OFFSET)
        return -EINVAL;
    if((len = PAGE_ALIGN(len)) == 0)
        return 0;

    mpnt = find_vma(current, addr);
    if(!mpnt)
        return 0;

    avl_neighbours(mpnt, current->mm->mmap_avl, &prev, &next);

    npp = (prev? &prev->vm_next: &current->mm->mmap);
    free = NULL;

    for(; mpnt && mpnt->vm_start < addr + len; mpnt = *npp)
    {
        *npp = mpnt->vm_next;
        mpnt->vm_next = free;
        free = mpnt;
        avl_remove(mpnt, &current->mm->mmap_avl);
    }
    if(free == NULL)
        return 0;

    while(free)
    {
        unsigned long st, end;

        mpnt = free;
        free = free->vm_next;

        remove_shared_vm_struct(mpnt);

        st = addr < mpnt->vm_start?mpnt->vm_start:addr;
        end = addr + len;
        end = end > mpnt->vm_end? mpnt->vm_end:end;

        if(mpnt->vm_ops && mpnt->vm_ops->unmap)
            mpnt->vm_ops->unmap(mpnt, st, end-st);

        unmap_fixup(mpnt, st, end-st);
        kfree(mpnt);
    }

    unmap_page_range(addr, len);
    return 0;
}

unsigned long do_mmap(struct file *file,unsigned long addr, unsigned long len, unsigned long prot, unsigned long flags, unsigned long off)
{
    int error;
    struct vm_area_struct *vma;

    if(len <= 0)
        return -EINVAL;

    if((len = PAGE_ALIGN(len)) == 0)
        return addr;

    if(addr > PAGE_OFFSET || len > PAGE_OFFSET || (addr + len) > PAGE_OFFSET)
        return -EINVAL;

    if(!file)
    {
        switch (flags & MAP_TYPE)
        {
            case MAP_SHARED:
                if((prot & PROT_WRITE) && (file->f_mode & FILE_WRITE))
                    return -EACCES;
                break;
            case MAP_PRIVATE:
                if(!(file->f_mode & FILE_READ))
                    return -EACCES;
            default:
                return -EINVAL;
        }
        if(file->f_inode->i_count > 0 && flags & MAP_DENYWRITE)
            return -ETXTBSY;
    }
    else if((flags & MAP_TYPE) != MAP_PRIVATE)
        return -EINVAL;

    if(flags & MAP_FIXED)
    {
        if(addr & ~ PAGE_MASK)
            return -EINVAL;
        if(len > PAGE_OFFSET || addr + len > PAGE_OFFSET)
            return -EINVAL;
    }
    else
    {
        addr = get_unmmapped_area(len);
        if(!addr)
            return -ENOMEM;
    }

    if(file && (!file->f_op || !file->f_op->mmap))
        return -ENODEV;

    vma = (struct vm_area_struct *)kmalloc(sizeof(struct vm_area_struct),GFP_KERNEL);
    vma->vm_task = current;
    vma->vm_start = addr;
    vma->vm_end = addr + len;
    vma->vm_flags = prot & (VM_READ | VM_WRITE | VM_EXEC);
    vma->vm_flags |= flags & (VM_GROWSDOWN | VM_DENYWRITE | VM_EXECUTABLE);

    if(file)
    {
        if(file->f_mode & FILE_READ)
            vma->vm_flags |= VM_MAYREAD | VM_MAYWRITE | VM_MAYEXEC;
        if(flags & MAP_SHARED)
            vma->vm_flags |= VM_SHARED | VM_MAYSHARE;

        if(file->f_mode & FILE_WRITE)
            vma->vm_flags &= ~(VM_MAYWRITE | VM_SHARED);
    }
    else
        vma->vm_flags |= VM_MAYREAD | VM_MAYWRITE | VM_MAYEXEC;


    //	vma->vm_page_prot = protection_map[vma->vm_flags & 0X0F];
    vma->vm_ops = NULL;
    vma->vm_offset = off;
    vma->vm_inode = NULL;
    vma->vm_pte = 0;

    do_munmap(addr, len);

    if(file)
        error = file->f_op->mmap(file->f_inode, file, vma);
    else 
        error = anon_map(NULL, NULL, vma);
    if(error)
    {
        kfree(vma);
        return error;
    }

    insert_vm_struct(current, vma);
    merge_segments(current, vma->vm_start, vma->vm_end);
    return addr;

    return 0;
}

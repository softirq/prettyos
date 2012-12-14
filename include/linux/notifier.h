#ifndef     _INOTIFY_H_
#define     _INOTIFY_H_

#define 	NETDEV_UP 		0x001
#define 	NETDEV_DOWN 	0x002
#define 	NETDEV_REBOOT 	0x003
#define  	NETDEV_STOP_MASK 	0x004 	//don't call further

#define     NOTIFY_DONE         0x0000
#define     NOTIFY_OK           0x0001

struct notifier_block
{
    int (*notifier_call)(unsigned long, void *);
    struct notifier_block *next;
    int priority;
};

extern inline int notifier_chain_register(struct notifier_block **list,struct notifier_block *n)
{
    struct notifier_block *prev;
    while(*list)
    {
        if(n->priority < (*list)->priority)
            break;
        prev = *list;
        list = (struct notifier_block **)(&((*list)->next));
    }
    n->next = *list;
    *list = n;
    prev->next = n;
    return 0;
}

extern inline int notifier_chain_unregister(struct notifier_block **list,struct notifier_block *n)
{
    while(*list)
    {
        if((*list) == n)
        {
            *list = n->next;
            return 0;
        }
        list = &((*list)->next);
    }
    return 0;
}

extern inline int notifier_call_chain(struct notifier_block **list,unsigned long val,void *arg)
{
    int ret = NETDEV_DOWN;
    struct notifier_block *nb;
    for(nb = *list; nb != NULL;nb=nb->next)
    {
        ret = nb->notifier_call(val,arg);
        if(ret & NETDEV_STOP_MASK)
            return ret;
    }
    return ret;
    return 0;
}

#endif

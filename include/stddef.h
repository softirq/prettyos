#undef  NULL
#define NULL ((void *)0)

#define offsetof(TYPE,MEMBER)  (size_t)(&((TYPE*)0->MEMBER))
/* 
 * #define container_of(ptr, type, memeber) ({ \
		const typeof((type *)0->member) *_mptr = ptr;\
		(type*)((char *)_mptr - offsetof(type,member));)}
*/

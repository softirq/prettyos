#ifndef	_CONST_H_
#define	_CONST_H_

#define NULL	0

#define	TRUE	1
#define	FALSE	0

#define ASSERT

#ifdef 	ASSERT
void assertion_failure(char *exp,char *file,char *base_file,int line);
#define assert(exp) do{ \
    if(exp);\
        else	 \
    assertion_failure(#exp,__FILE__,__BASE_FILE__,__LINE__); \
}while(0)
#else
#define assert(exp)
#endif


#define MIN(a,b)	(((a)>(b)?(b):(a)))
#define MAX(a,b)	(((a)>(b)?(a):(b)))

#define ALIGN(value,agn) ((value) & (~(agn) + 1))

#endif /* _Pretty_CONST_H_ */


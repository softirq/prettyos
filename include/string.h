#ifndef     _STRING_H_
#define     _STRING_H_

extern void     assertion_failure(char *exp,char *file,char *base_file,int line);

extern	void*	memcpy(void* p_dst, void* p_src, int size);
extern	void*	mymemcpy(void* p_dst, void* p_src, int size);
extern	int strcpy(char* p_dst, char* p_src);
extern int      strncpy(char *dest,char *src,int size);
extern	int 	strlen(const char *str);
extern int      strncmp(char *str1,char *str2,int len);
extern int      strcmp(char *str1,char *str2);

#endif	

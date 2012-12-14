#ifndef     _STRING_H_
#define     _STRING_H_

extern void     assertion_failure(char *exp,char *file,char *base_file,int line);
extern int      strncmp(char *str1,char *str2,int len);
//extern int      strncpy(char *dest,char *src,int pos,int size);
extern int      strncpy(char *dest,char *src,int size);
extern int      strcpy(char *dst, char *src);
extern int      strcmp(char *str1,char *str2);
extern int 	    memcpy(void *dst, void *src, int len);

#endif

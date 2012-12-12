//从端口读字符串
#define port_read(port,buf,nr) \
       __asm__ (       \
               "cld\n\t"\
               "rep\n\t"\
               "insw"\
               ::"d"(port),"D"(buf),"c"(nr))
           //          :"ecx","edi")
//向端口写字符串
#define port_write(port,buf,nr) \
       __asm__ (       \
               "cld\n\t" \
               "rep\n\t" \
               "outsw" \
               ::"d"(port),"S"(buf),"c"(nr))
//              :"ecx","esi")

#define NO_DEV 		0
#define DEV_FLOPPY 	1
#define DEV_CHARACTER	2
#define DEV_HD		3
#define DEV_TTY		4


void ll_rw_swap_file(int rw, int dev, unsigned int *b, int nb, char *buf);

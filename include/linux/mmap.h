#ifndef     _MMAP_H_
#define     _MMAP_H_

#define 	PROT_READ 		0x01
#define 	PROT_WRITE 		0x02
#define 	PROT_EXEC 		0x04
#define 	PROT_NONE 		0x0

#define  	MAP_TYPE 		0x0f
#define 	MAP_SHARED 		0x01
#define 	MAP_PRIVATE 	0x02
#define 	MAP_FIXED 		0x04 		//interpret address exactly
#define 	MAP_DENYWRITE 	0x08 		//

#endif

#ifndef     _PROC_H_
#define     _PROC_H_

#define PROCS_BASE 		0xA00000
#define PROC_IMAGE_SIZE_DEFAULT 0x80000
#define PROC_ORIGIN_STACK 	0x400

int get_limit(struct descriptor *dp);
int get_base(struct descriptor *dp);

#endif

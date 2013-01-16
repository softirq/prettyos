#ifndef     _BUFFER_HEAD_H_
#define     _BUFFER_HEAD_H_

#define     BUFFER_HEAD_PAGE    8

//sector size 
#define SECTOR_SIZE 	512
//buffer size
#define     BUFFER_SIZE 	SECTOR_SIZE
#define     BUFFER_ALIGN    BUFFER_SIZE 
#define     NR_HASH_BUFFER		107

//extern long buffer_memory_start;
struct buffer_head
{
	char *b_data;
	unsigned long b_blocknr;
	unsigned short b_dev;
	unsigned char b_uptodate;
	unsigned char b_dirt;
	unsigned char b_count;
	unsigned char b_lock;
	struct wait_queue  *b_wait;

    /* list */
	struct buffer_head *b_prev;
	struct buffer_head *b_next;

    /* free list  */
	struct buffer_head *b_prev_free; 
	struct buffer_head *b_next_free;
};

extern void buffer_init();
struct buffer_head * getblk(int dev,int block);
extern void brelse(struct buffer_head *bh);

#endif

#define  MAX_SWAPFILES 	8

struct swap_info_struct
{
		unsigned int flags;
		struct inode *swap_file;
		unsigned int swap_device;
		unsigned short *swap_map;
		unsigned short *lock_map;
		unsigned int lowest_bit;
		unsigned int highest_bit;
		unsigned long max;
};

typedef struct 
{
		        unsigned long val;
}swap_entry_t;

extern void swap_free(swap_entry_t entry);

#define SWAP_TYPE(x)    (((x).val >> 1) & 0x3f)
#define SWAP_OFFSET(x)  ((x).val >> 8)
#define SWAP_ENTRY(type,offset) ((swap_entry_t){(type << 1) | (offset << 8)})
#define pte_to_swap_entry(pte) (swap_entry_t) {(pte).val}
#define swap_to_pte_entry(x) ((pte_t){(x).val})

#define 	SHM_SWP_TYPES  	0x41
#define 	SWAP_MAP_MAX 	16

#define 	READ 	0
#define 	WRITE  	1

#define read_swap_page(nr,buf) \
		rw_swap_page(READ,(nr),(buf))

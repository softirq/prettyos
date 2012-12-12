//#include "timer.h"

#define IP_FRAG_TIME    (30*HZ)

#define 	 IP_OFFSET  	0x1FFF
#define  	IP_FLAGS 		0xE000

#define  	IP_DF 		0x0010  	// don't fragment
#define  	IP_MF 		0x0020  	// more fragment

extern void ip_init();
extern int 	ip_id_count;
extern void ip_send_checksum(struct iphdr *iph);

/*
 * describe an IP fragment.
 */
struct ipfrag
{
		int 	offset; 	 // offset of fragment in IP datagram
		int 	end; 		// last byte of data in datagram
		int 	len; 			//length of this fragment
		struct sk_buff 	*skb;  	// complete received gragment
		struct timer_list 	timer;
		unsigned char 	*ptr;
		struct ipfrag 	*next;
		struct ipfrag 	*prev;
};

/*
 * a queue for ipfrag list
 */
struct ipfq
{
		unsigned 	char  	 	*mac; 		//pointer to MAC header
		struct 		iphdr 		*iph;   	//pointer to IP header
		int 					len;  				//total length of original datagram
		short  					ihlen;  			// length of the IP header
		short 					maclen; 			//length of the MAC header
		struct 		timer_list 	timer;  		// when will this queue expire
		struct 		ipfrag 		*fragments;
		struct 		ipfq 		*next;
		struct 		ipfq 		*prev;
		struct 		net_device 		*dev;
};

static inline unsigned short ip_fast_cksum(unsigned char *buff, int wlen)
{
		return 0;
}

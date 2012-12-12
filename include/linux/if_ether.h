
#define ETH_ALEN 	6
#define ETH_HLEN 	14


#define ETH_P_IP  	0x0800
#define ETH_P_ARP 	0x0806


struct ethhdr
{
	unsigned char h_dest[ETH_ALEN];
	unsigned char h_source[ETH_ALEN];
	unsigned short h_proto;
};

#include "stddef.h"
#include "linux/netdevice.h"
#include "linux/ip.h"
#include "linux/in.h"
#include "protocol.h"



static struct inet_protocol icmp_protocol = 
{
	NULL,
	NULL,
	IPPROTO_ICMP,
	NULL,
	"ICMPD"	
};


struct inet_protocol *inet_protocol_base = &icmp_protocol;

struct inet_protocol *inet_protos[MAX_INET_PROTOS] = 
{
			NULL,
};


void inet_add_protocol(struct inet_protocol *prot)
{
		unsigned char hash;

		hash = prot->protocol & (MAX_INET_PROTOS - 1);
		prot->next = inet_protos[hash];
		inet_protos[hash] = prot;
}

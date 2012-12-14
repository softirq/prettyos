#include "linux/net.h"
#include "linux/inet.h"
#include "unix/unix.h"
#include "stddef.h"

struct net_proto protocols[] =
{
	{"UNIX", 	unix_proto_init},
	{"INET", 	inet_proto_init},
	{NULL, 		NULL}
};

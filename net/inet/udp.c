#include "type.h"
#include "stddef.h"

#include "linux/timer.h"
#include "linux/net.h"
#include "linux/socket.h"
#include "linux/skbuff.h"
#include "linux/tcp.h"
#include "sock.h"

struct  proto udp_prot = {
	128,
	0,
	{NULL,},
	"UDP",
	0,
	0
};

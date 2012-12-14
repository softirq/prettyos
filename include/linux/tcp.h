#ifndef     _TCP_H_
#define     _TCP_H_
enum
{
    TCP_ESTABLISHED = 1,
    TCP_SYN_SENT,
    TCP_SYN_RECV,
    TCP_FIN_WAIT1,
    TCP_FIN_WAIT2,
    TCP_TIME_WAIT,
    TCP_CLOSE,
    TCP_CLOSE_WAIT,
    TCP_LAST_ACK,
    TCP_LISTEN,
    TCP_CLOSING
};

struct tcphdr 
{
    u16 	source;
    u16 	dest;
    u32 	seq;
    u32 	ack_seq;
    u16 	len:4,
            reserve:6,
            urg:1,
            ack:1,
            psh:1,
            rst:1,
            syn:1,
            fin:1;
    u16 	window;
    u16 	check;
    u16 	urg_ptr;

};

#define HEADER_SIZE 64      /* maximum header size      */       

#endif

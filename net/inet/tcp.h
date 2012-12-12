extern struct proto tcp_prot;

extern void 	tcp_send_probe(struct sock *sk);

extern inline int before(unsigned long seq1, unsigned long seq2)
{
		return (int)((seq1 - seq2) < 0);
}

extern inline int after(unsigned long seq1, unsigned long seq2)
{
		return (int)((seq1 - seq2) > 0);
}


#define TCP_TIMEOUT_LEN 	(15*60*HZ) /* should be about 15 mins       */ 
#define TCP_ACK_TIME    	(3*HZ)  /* time to delay before sending an ACK  */
#define TCP_FIN_TIMEOUT 	(3*60*HZ) /* BSD style FIN_WAIT2 deadlock breaker */   
#define TCP_TIMEOUT_INIT 	(3*HZ) /* RFC 1122 initial timeout value   */
#define MAX_ACK_SIZE    	40 + MAX_HEADER
#define MAX_RESET_SIZE  	40 + MAX_HEADER

#define MAX_WINDOW  16384                                        
#define MIN_WINDOW  2048  
#define MAX_SYN_SIZE    44 + MAX_HEADER                               
#define MAX_FIN_SIZE    40 + MAX_HEADER                                    


#include "type.h"
#include "const.h"
#include "traps.h"
//#include "string.h"
#include "tty.h"
#include "console.h"
#include "wait.h"
#include "mm.h"
#include "sched.h"
#include "global.h"
#include "kernel.h"
#include "lib.h"

int get_signal_bitmap()
{
	return current->signal;
}

void do_signal(int signr)
{

//	disp_int(~(i<<signr));
	current->signal &= ~(1 << signr );
	disp_str("do signal\n");

}


int send_signal(int signr,int pid)
{
	if(signr < 0 || signr > NR_SIGNALS || signr == SIGKILL)
		return -1;
	if(pid < 0 || pid > NR_PROCESS)
		return -1;
	struct task_struct *t = pid2proc(pid);
	t->signal |= 1 << ((signr -1));
	t->sigpending = TRUE;
	return 0;
	
}

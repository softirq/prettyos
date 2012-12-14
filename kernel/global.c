#define GLOBAL_VARIABLES_HERE

#include "type.h"
#include "const.h"
#include "traps.h"
#include "tty.h"
#include "console.h"
#include "wait.h"
#include "mm.h"
#include "sched.h"
#include "global.h"
#include "kernel.h"
#include "sys.h"

long jiffies = 0;

//public	PROCESS	proc_table[NR_TASKS + NR_NATIVE_PROCS];
struct task_struct proc_table[NR_PROCESS + NR_PROCS];

char	task_stack[STACK_SIZE_TOTAL];

//system process tables
TASK	\
            task_table[NR_SYSTEM_PROCS] = { 
                {task_tty, STACK_SIZE_TTY, "tty"}
            };

//user process tables;only for test now
TASK 	\
            user_proc_table[NR_USER_PROCS] = { 
                {init, 	STACK_SIZE_INIT,   "init"},
                {TestA, STACK_SIZE_TESTA, "TestA"},
                {TestB, STACK_SIZE_TESTB, "TestB"},
                {TestC, STACK_SIZE_TESTC, "TestC"},
                {TestD, STACK_SIZE_TESTD, "TestD"}
            };

//console tables
TTY			tty_table[NR_CONSOLES];

//console tables
CONSOLE			console_table[NR_CONSOLES];

//irq handler table
irq_handler_ptr		irq_table[NR_IRQ];

//system call table
syscall_ptr		\
                    sys_call_table[NR_SYS_CALL] = {
                        sys_get_ticks,
                        sys_write,
                        sys_printx,
                        sys_fork,
                        sys_exit,
                        sys_waitpid
                    };

#define set_trap_gate(n,addr) 			init_idt_desc(n,DA_386TGate,addr,PRIVILEGE_KRNL)
#define set_system_gate(n,addr) 		init_idt_desc(n,DA_386TGate,addr,PRIVILEGE_USER)
#define set_intr_gate(n,addr) 			init_idt_desc(n,DA_386IGate,addr,PRIVILEGE_KRNL)
#define set_syscall_gate(n,addr) 		init_idt_desc(n,DA_386IGate,addr,PRIVILEGE_USER)

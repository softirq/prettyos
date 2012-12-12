#define 	DK_NDRIVE 		4
struct kernel_stat
{
		unsigned int cpu_user, cpu_system, cpu_nice;
		unsigned int dk_drive[DK_NDRIVE];
		unsigned int pgpgin, pgpgout;
		unsigned int pswpin, pswpout;
		unsigned int interrupts[16];
		unsigned int ipackets, opackets;
		unsigned int ierrors, oerrors;
		unsigned int collisions;
		unsigned int context_swtch;
};


extern struct kernel_stat kstat;

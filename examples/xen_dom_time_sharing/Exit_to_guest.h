#ifndef __HDLR_EXIT_TO_GUEST
#define __HDLR_EXIT_TO_GUEST

typedef struct XenRuntime
{
	unsigned long long 	exitToGuest[MAX_DOMS];
	unsigned long long 	runtime;
	unsigned int		cpuId;
	list_head 		cpuList;
} XenRuntime;

int exit_to_guest_init(EventHandler *handler);
int exit_to_guest_handler(EventHandler *handler, Event *event);
int exit_to_guest_finalize(EventHandler *handler);
void exit_to_guest_reset(void);

unsigned long long get_last_exit_to_guest(unsigned int cpuId, unsigned int domId);
unsigned long long get_latest_exit_to_guest(unsigned int cpuId);

#endif

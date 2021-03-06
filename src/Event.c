#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "Macros.h"
#include "Event.h"

static Event *eventList = NULL;
static unsigned long long eventCount = 0;
static unsigned long long numEvents = 0;

unsigned long long get_total_event_records()
{
	return numEvents;
}

int compare(const void *a, const void *b)
{
	long long result = ( ((Event *)a)->ns - ((Event *)b)->ns);

	if(result > 0)		return 1;
	else if(result < 0)	return -1;
	else			return 0;
}

/* Description:	Generic event parse function
   Parses event passed as function argument.
   Returns FAIL if event not found.
 */
int parse_event(Event *ev, FILE *fp, unsigned int ev_id)
{
	Event tmp_ev;

	/* Init struct */
	clear_event(&tmp_ev);

	while(!feof(fp))
	{
		if(parse_next_event(&tmp_ev, fp) == FAIL)
		{
			fprintf(stderr, "%s:%d: parse_next_event returned FAIL\n", __FILE__, __LINE__);
			return FAIL;
		}

		if(tmp_ev.event_id == ev_id)
		{
			memcpy(ev, &tmp_ev, sizeof(tmp_ev));
			return SUCCESS;
		}
	}

	/* fprintf(stdout, "%s:%d:parse_event: EOF file reached", __FILE__, __LINE__); */
	return FAIL;
}

/* Description:	Generic event parse function
   Parses the next event in log file. 
   Uses modular parse functions
 */
int parse_next_event(Event *ev, FILE *fp)
{
/* CODE CLEANUP: Modify the Event reading code to incorporate 32, 64 bit compatibility.
 *               Also remove t_rec struct and use only Event struct
 */  
	unsigned int cpu;
	int i = 0;
	t_rec rec;

	if(fread(&cpu, sizeof(cpu), 1, fp) != 1)
	{
		if(!feof(fp))
		{
			fprintf(stderr, "%s:%d: File read failed - failed to read cpu id\n", __FILE__, __LINE__);
		}

		return FAIL;

	}

	if(fread(&rec, sizeof(rec), 1, fp) != 1)
	{
		if(!feof(fp))
		{
			fprintf(stderr, "%s:%d: File read failed - failed to read t_rec data\n", __FILE__, __LINE__);
		}

		return FAIL;
	}
	/*	
	printf("cpu:%u, cycles:%llu, ns:%llu, event:%x, data0:%lx, data1:%lx, data2:%lx, data3:%lx, data4:%lx\n", 
			cpu, rec.cycles, rec.ns, rec.event, rec.data[0], rec.data[1], rec.data[2], rec.data[3], rec.data[4]);
	*/
	ev->cpu = cpu;
	ev->event_id = rec.event;
	ev->tsc = rec.cycles;
	ev->ns = rec.ns;
	ev->lastNs = 0;
	ev->n_data = MAX_EV_DATA;

	for(i = 0; i < MAX_EV_DATA; i++)
	{
		ev->data[i] = rec.data[i];
	}

	return SUCCESS;
}

int sort_events_by_ns(FILE *fp)
{
	int i = 0;
	unsigned long fsize;

	fseek(fp, 0, SEEK_END);
	fsize = ftell(fp);
	rewind(fp);

	// No. of events = file_size/(size of cpu + t_rec)
	numEvents = fsize/(sizeof(unsigned int) + sizeof(t_rec));

	eventList = (Event *)malloc(sizeof(Event) * numEvents);

	for(i = 0; i < numEvents; i++)
	{
		if(parse_next_event(&eventList[i], fp) != SUCCESS)
			break;
	}

	qsort(eventList, numEvents, sizeof(Event), compare);

	return SUCCESS;
}

int return_next_event(Event *ev)
{
	if(eventCount < numEvents)
	{
		/* Keep track of last ns timestamp for lost_records check */
		if(eventCount > 0)
		{
			unsigned long long i = 0;
			while((++i < eventCount) 
				&& (eventList[eventCount - i].cpu != eventList[eventCount].cpu));

			if((eventCount - i - 1) > 0)
				eventList[eventCount].lastNs = eventList[eventCount - i].ns;
		}
		memcpy(ev, &eventList[eventCount++], sizeof(Event));
		return SUCCESS;
	}
	else
		return FAIL;
}

void free_events(void)
{
	free(eventList);
}

/* Description:	Helper function
   Sets all event data to 0
   Used to init and reset Event struct.
 */
void clear_event(Event *ev)
{
	ev->event_id = 0;
	ev->n_data = 0;
	ev->tsc_in = 0;
	ev->tsc = 0;
	memset(ev->data, 0, MAX_EV_DATA);
}

/* Description:	Helper function
   Checks null pointers.
 */
int check_null(void *ev, void *fp)
{
	if((ev == NULL) || (fp == NULL))
	{
		return FAIL;
	}
	else
		return SUCCESS;
}

unsigned long long get_first_ns_ev_list(int cpu)
{
	unsigned long long i;

	if(eventList != NULL)
	{
		for(i = 0; i < numEvents; i++)
		{
			if(eventList[i].cpu == cpu)
				return eventList[i].ns;
		}
	}

	return 0;
}

unsigned long long get_last_ns_ev_list(int cpu)
{
	long long i;

	if(eventList != NULL)
	{
		for(i = numEvents - 1; i != 0; i--)
		{
			if(eventList[i].cpu == cpu)
				return eventList[i].ns;
		}
	}

	return 0;
}

void print_line(void)
{
	printf("==================================================================\n");
}


/* VisualOS is an educational visual simulator of an operating system.   
   Copyright (C) 2000,2004 Manuel Estrada Sainz <ranty@debian.org>
   
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.

*/

#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <IO/IO.h>
#include <CLOCK/CLOCK.h>

#include "simulation.h"
/**
 * IO_BLOCK:
 * @event: process event.
 *
 * Extracts the block from a IO event.
 */

/**
 * init_CPU_simulation:
 *
 * Initializes the simulation code.
 */
void init_CPU_simulation (void)
{
	srand((unsigned int)time(NULL));
}
static int io_event_cmp(const simul_io_event_t *e1,
		const simul_io_event_t *e2)
{
	return (e1->time - e2->time);
}
static void sort_simulation_io_events(proc_t *proc)
{
	simul_data_t *data = proc->simul_data;

	qsort(data->io_events,
			data->last_io_event - data->io_events +1,
			sizeof(simul_io_event_t),
			(int (*)(const void *, const void *)) io_event_cmp);
}
/**
 * fix_simulation_in_proc:
 * @proc: process whose simulation data should be fixed.
 *
 * Makes the simulation parameters of a process coherent, prevents: multiple
 * events at the same time, events after process termination, determines the
 * next event ...
 */
void fix_simulation_in_proc(proc_t *proc)
{
	gint time = get_time();
	simul_data_t *data = proc->simul_data;
	simul_io_event_t *next_event=NULL;
	/* sort io_events */
	sort_simulation_io_events(proc);
	/* filter events ocurring at the same time*/

	/* process should not terminate before doing all it's IO */
	if (data->io_events && data->end_time <= data->last_io_event->time){
		data->end_time = data->last_io_event->time+1;
#ifdef DEBUG
		g_print("Process termination scheduled before las IO event: "
								"fixed\n");
#endif
	}
	/*fill next_event */
	if (data->start_time > time){
		proc->next_event.type = EVENT_START;
		proc->next_event.time = data->start_time;
		return;
	} 
	if (data->io_events){
		simul_io_event_t *event=data->io_events;
		while(event <= data->last_io_event){
			if (event->time > proc->time){
				next_event = event;
				data->next_io_event = event+1;
				break;
			}
			++event;
		}
	}
	if(next_event){
		proc->next_event.type=EVENT_IO;
		proc->next_event.time=next_event->time;
		IO_BLOCK(proc->next_event) = next_event->block;
	} else {
		proc->next_event.type=EVENT_TERM;
		if (data->end_time > proc->time)
			proc->next_event.time=data->end_time;
		else
			proc->next_event.time=proc->time +1;
	}
}
/**
 * init_CPU_simulation_in_proc:
 * @proc: process involved.
 *
 * prepares the data structures for simulation in process @proc.
 */
void init_CPU_simulation_in_proc(proc_t *proc)
{
	proc->next_event.data = g_new(event_data_t, 1);
	proc->pending_event.data = g_new(event_data_t, 1);
}
/**
 * end_CPU_simulation_in_proc:
 * @proc: the process involved.
 * 
 * cleans up simulation data in @proc to prepare it for termination.
 *
 * Note: currently does nothing.
 */
void end_CPU_simulation_in_proc(proc_t *proc)
{
}

/**
 * free_CPU_simulation_data:
 * @data: the data to be freed
 *
 * Frees all dynamic memory asociated to @data, including @data itself.
 */
void free_CPU_simulation_data(simul_data_t *data)
{
	g_free(data->io_events);
	g_free(data->pages);
	g_free(data);
}

/**
 * cpy_CPU_simulation_data:
 * @dest: the target of the copy
 * @src: the source for the copy
 *
 * Copies the contents of @src into @dest, allocating dynamic memory when
 *  needed.
 */
void cpy_CPU_simulation_data(simul_data_t *dest, simul_data_t *src)
{
	g_free(dest->io_events);
	g_free(dest->pages);

	*dest = *src;
	if(src->io_events){
		gint n_io_events = src->last_io_event - src->io_events +1;
		dest->io_events = (simul_io_event_t *)
			g_memdup(src->io_events,
				 n_io_events*sizeof(simul_io_event_t));
		dest->last_io_event = dest->io_events + n_io_events -1;
	}
	dest->pages = (simul_mem_t *)g_memdup(src->pages, src->n_pages);
}
/**
 * dup_CPU_simulation_data:
 * @data: a pointer to the data to be copied
 *
 * Duplicates a simul_data_t structure.
 *
 * returns: a pointer to newly allocated memory with the same content of @data
 */
simul_data_t *dup_CPU_simulation_data(simul_data_t *data)
{
	simul_data_t *new_data = g_new0(simul_data_t, 1);
	cpy_CPU_simulation_data(new_data, data);
	return new_data;
}
/**
 * free_CPU_proc_simulation_data:
 * @proc: a pointer to a proc_t structure
 *
 * Free all simulation related dynamic memory from the @proc structure.
 *
 */
void free_CPU_proc_simulation_data(proc_t *proc)
{
	free_CPU_simulation_data(proc->simul_data);
	g_free(proc->pending_event.data);
	g_free(proc->next_event.data);
}
/**
 * next_CPU_simulation_in_proc:
 * @proc: the process involved.
 *
 * Once the process had an event it prepares the process for its next
 * event, making it a termination event if necesary.
 */
void next_CPU_simulation_in_proc(proc_t *proc)
{
	simul_data_t *data = proc->simul_data;
	/* will switch the event private data pointers insted of the contents
	 * of the memory */
	event_data_t *event_data = proc->pending_event.data;
	/* next event will now be pending */
	memcpy(&proc->pending_event, &proc->next_event, sizeof(event_t));

	proc->next_event.data=event_data;
	if (proc->pending_event.type==EVENT_TERM)
		return;

	if (data->io_events && data->next_io_event <= data->last_io_event){
		proc->next_event.time = data->next_io_event->time;
		event_data->io_block = data->next_io_event->block;
		proc->next_event.type = EVENT_IO;
		data->next_io_event++;
	} else {
		proc->next_event.type = EVENT_TERM;
		if (data->end_time > proc->time) 
			proc->next_event.time = data->end_time;
		else
			proc->next_event.time = proc->time + 5;
	}
}
/**
 * CPU_proc_next_page:
 * @proc: the process involved.
 * 
 * Makes the process move to its next memory page and should be called once
 * for each "clock tick" that @proc is running.
 *
 * returns: the new page which the process is using.
 */
gint CPU_proc_next_page(proc_t *proc)
{
	simul_data_t *data = proc->simul_data;
	g_return_val_if_fail(data->pages != NULL, 0);
	g_return_val_if_fail(data->cur_access < data->n_pages, 0);
	
	if(++data->cur_access >= data->n_pages){
		/* keep using the last page */
		--data->cur_access;
	}
	return data->pages[data->cur_access].page;
}
/**
 * CPU_proc_current_page:
 * @proc: the process involved.
 *
 * returns: the page which @proc is using on this very moment.
 */
gint CPU_proc_current_page(proc_t *proc)
{
	simul_data_t *data = proc->simul_data;
	g_return_val_if_fail(data->pages != NULL, 0);
	g_return_val_if_fail(data->cur_access < data->n_pages, 0);

	return data->pages[data->cur_access].page;
}
/**
 * CPU_proc_current_page_is_write:
 * @proc: the process involved.
 *
 * Checks if @proc is writing to memory or only reading.
 *
 * returns: TRUE when @proc is writing to its current page.
 */
gboolean CPU_proc_current_page_is_write(proc_t *proc)
{
	simul_data_t *data = proc->simul_data;
	
	return data->pages[data->cur_access].write;
}

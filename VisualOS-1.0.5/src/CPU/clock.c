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

#include <gtk/gtk.h>

#include <CLOCK/CLOCK.h>
#include <MEM/MEM.h>
#include <IO/IO.h>

#include "drawings/main.h"
#include "simulation.h"
#include "queues.h"
#include "combos.h"
#include "clock.h"
#include "info.h"

static gint heart_beat;
static gint heart_beat_timer;
static gint time=0;

/**
 * reset_CPU_timer:
 *
 * Resets the "time unit" counter so we will have a full timeslice until
 * the next interupt.
 *
 * returns: nothing important.
 */
gint reset_CPU_timer(void)
{
	heart_beat_timer = heart_beat;
	return 0;
}
/**
 * set_CPU_heart_beat:
 * @freq: new frequency in "time units".
 * 
 * Set timer interrupt frequency. Which means, the calling frequency
 * of algorithm function @clock.
 *
 * Zero means that the timer interupt is not desired.
 *
 * returns: nothing important.
 */
gint set_CPU_heart_beat(gint freq)
{
	heart_beat = freq;
	heart_beat_timer = freq;
	return 0;
}
	
static gint CPU_tick (gint clock_time)
{
	proc_t *proc=get_CPU_current_proc();
	cpu_algorithm_t *algorithm=get_CPU_current_algorithm();

	time=clock_time;
	if (proc!=NULL){
		gint page = CPU_proc_current_page(proc);
		gboolean write = CPU_proc_current_page_is_write(proc);

		if (!mem_touch_page(proc, page, write)){
			suspend_proc(proc);
			algorithm->next(proc);
			redraw_CPU_drawing();
			return 0;
		}
		CPU_proc_next_page(proc);
		proc->time++;
	}
	if ((proc!=NULL)&&(proc->next_event.time <= proc->time)){
		if (proc->next_event.time < proc->time)
			g_warning("next event on the past\n");
		next_CPU_simulation_in_proc(proc);
		switch (proc->pending_event.type){
			case EVENT_IO:
				/* if executed else where behaivior changes*/
				io_request_block(IO_BLOCK(proc->pending_event));
				suspend_proc(proc);
				algorithm->next(proc);
				break;
			case EVENT_TERM:
				remove_proc_from_queue(proc);
				algorithm->next(proc);
				destroy_process(proc);
				break;
		}
	} else {
		if ((heart_beat != CPU_NO_HEART_BEAT)&&
				(--heart_beat_timer == 0)){
			heart_beat_timer = heart_beat;
			algorithm->clock();
		}
	}
	redraw_CPU_drawing();
	return 0;
}
static void block_ready (gint block)
{
	proc_queue_t queue=get_CPU_wait_queue();
	while (!proc_queue_end(queue)){
		proc_t *proc=proc_data(queue);
		if(proc->pending_event.type == EVENT_IO)
			if(IO_BLOCK(proc->pending_event)==block){
				proc->pending_event.type = EVENT_NONE;
				wakeup_proc(proc);
				return;
			}
		queue=proc_queue_next(queue);
	}
	g_warning("CPU_clock: could not find a process waiting for block %d",
			block);
}
static void page_ready (gint pid, gint page)
{
	proc_queue_t queue=get_CPU_wait_queue();
	proc_t *proc=get_proc_by_pid(pid);

	g_return_if_fail(proc != NULL);

	if (proc_queue_find(queue, proc)){
		wakeup_proc(proc);
			return;
	}

	g_warning("CPU_clock: process %d is not waiting for page %d",
			pid, page);
}
void init_CPU_clock(void)
{
	io_register_block_ready(block_ready);
	mem_register_page_ready(page_ready);
	clock_register_tick(CPU_tick);
}


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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <gtk/gtk.h>

#include "algorithm_api.h"
#include "spn.h"

static gint spn_select (void);
static gint spn_unselect (void);
static gint spn_select_proc (proc_t *proc);
static gint spn_init_proc (proc_t *proc);
static gint spn_end_proc (proc_t *proc);
static gint spn_event (proc_t *proc);
static gint spn_next (proc_t *proc);

static cpu_algorithm_t spn_algorithm = {
	"Shortest Process Next",
	spn_select,
	spn_unselect,
	NULL, 
	spn_select_proc,
	NULL,/*properties WidGet*/
	NULL,/*process_properties WidGet*/
	spn_init_proc,
	spn_end_proc,
	spn_event,
	spn_next
};

static proc_t *spn_next_proc(proc_queue_t queue)
{
	/* we return the shortest process in the queue */
	proc_t *shortest = proc_data(queue);
	gint short_burst = shortest->next_event.time - shortest->time;
	
	while (!proc_queue_end(queue)){
		proc_t *proc;
		gint burst;
		
		proc = proc_data(queue);
		burst = proc->next_event.time - proc->time;
		if (burst < short_burst){
			shortest = proc;
			short_burst = burst;
		}
		queue = proc_queue_next(queue);
	}
	return shortest;
}

gint spn_init (void)
{
	register_CPU_algorithm (&spn_algorithm);
	return 0;
}
static gint spn_select_proc (proc_t *proc)
{
	return 0;
}
static gint spn_select (void)
{
	set_CPU_heart_beat(CPU_NO_HEART_BEAT);
	
	request_nqueues (1);
	return 0;
}
static gint spn_unselect (void)
{
	return 0;
}
static gint spn_init_proc (proc_t *proc)
{
	const proc_queues_t *queues=get_CPU_queues();

	if (queues->current==NULL)
		move_proc_to_CPU(proc);
	else 
		move_proc_to_queue(proc, 0);
	return 0;
}
static gint spn_end_proc (proc_t *proc)
{
	return 0;
}
static gint spn_event (proc_t *proc)
{
	if (get_CPU_current_proc()==NULL)
		move_proc_to_CPU(proc);
	else
		move_proc_to_queue(proc, 0);
	return 0;
}
static gint spn_next (proc_t *proc)
{
	const proc_queues_t *queues=get_CPU_queues();

	g_return_val_if_fail (get_CPU_current_proc() == NULL, 0);
	
	if (!proc_queue_empty(queues->queue[0])){
		proc_t *next_proc = spn_next_proc(queues->queue[0]);
		move_proc_to_CPU(next_proc);
	}

	return 0;
}

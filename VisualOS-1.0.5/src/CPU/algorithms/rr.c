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
#include "rr.h"

static gint rr_select (void);
static gint rr_unselect (void);
static gint rr_clock (void);
static gint rr_select_proc (proc_t *proc);
static gint rr_init_proc (proc_t *proc);
static gint rr_end_proc (proc_t *proc);
static gint rr_event (proc_t *proc);
static gint rr_next (proc_t *proc);

static cpu_algorithm_t rr_algorithm = {
	"Round Robin",
	rr_select,
	rr_unselect,
	rr_clock, 
	rr_select_proc,
	NULL,/*properties WidGet*/
	NULL,/*process_properties WidGet*/
	rr_init_proc,
	rr_end_proc,
	rr_event,
	rr_next
};

static const gfloat default_params[]={2};
static properties_t *properties;
static property_t algorithm_params[] = {
	{N_("Time Slice"), 1, 99, 1}
};
#define TIME_SLICE 0

static void rr_property_change_notify(void)
{
	gfloat *value;

	value = properties_get_values(properties);
	set_CPU_heart_beat(value[TIME_SLICE]);
}
static proc_t *rr_next_proc(proc_queue_t queue)
{
	/* we return the first process in the queue */
	return proc_data(queue);
}

gint rr_init (void)
{
	register_CPU_algorithm (&rr_algorithm);
	return 0;
}
static gint rr_select_proc (proc_t *proc)
{
	return 0;
}
static gint rr_select (void)
{
	set_CPU_heart_beat(default_params[TIME_SLICE]);
	
	properties = properties_create (algorithm_params, 1,
			rr_property_change_notify);
	properties_set_values(properties, default_params);
	rr_algorithm.properties = properties_get_widget (properties);
	
	request_nqueues (1);
	return 0;
}
static gint rr_unselect (void)
{
	properties_destroy (properties);
	rr_algorithm.properties = NULL;
	return 0;
}
static gint rr_clock(void)
{
	const proc_queues_t *queues=get_CPU_queues();
	proc_t *proc=queues->current;
	if (proc==NULL)
		return 0;
	move_proc_to_queue(proc, 0);
	if (!proc_queue_empty(queues->queue[0])){
		proc = rr_next_proc(queues->queue[0]);
		move_proc_to_CPU(proc);
	}

	return 0;
}
static gint rr_init_proc (proc_t *proc)
{
	const proc_queues_t *queues=get_CPU_queues();

	if (queues->current==NULL)
		move_proc_to_CPU(proc);
	else 
		move_proc_to_queue(proc, 0);
	return 0;
}
static gint rr_end_proc (proc_t *proc)
{
	return 0;
}
static gint rr_event (proc_t *proc)
{
	if (get_CPU_current_proc()==NULL){
		move_proc_to_CPU(proc);
		reset_CPU_timer();
	} else 
		move_proc_to_queue(proc, 0);

	return 0;
}
static gint rr_next (proc_t *proc)
{
	const proc_queues_t *queues=get_CPU_queues();

	g_return_val_if_fail (get_CPU_current_proc() == NULL, 0);
	if (!proc_queue_empty(queues->queue[0])){
		proc_t *next_proc = rr_next_proc(queues->queue[0]);
		move_proc_to_CPU(next_proc);
		reset_CPU_timer();
	}

	return 0;
}

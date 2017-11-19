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
#include "hrrn.h"

typedef struct hrrn_data_s {
	gint last_run;
} hrrn_data_t;

static gint hrrn_select (void);
static gint hrrn_unselect (void);
static gint hrrn_select_proc (proc_t *proc);
static gint hrrn_init_proc (proc_t *proc);
static gint hrrn_end_proc (proc_t *proc);
static gint hrrn_event (proc_t *proc);
static gint hrrn_next (proc_t *proc);

#define last_run(x) (((hrrn_data_t *)x->algorithm_data)->last_run)
#define waiting(x) (get_time() - last_run(x))

static cpu_algorithm_t hrrn_algorithm = {
	"Highest Response Ratio Next",
	hrrn_select,
	hrrn_unselect,
	NULL, 
	hrrn_select_proc,
	NULL,/*properties WidGet*/
	NULL,/*process_properties WidGet*/
	hrrn_init_proc,
	hrrn_end_proc,
	hrrn_event,
	hrrn_next
};

static gint hrrn_set_last_run_time(proc_t *proc)
{
	hrrn_data_t *proc_data;

	if (proc->algorithm_data == NULL){
		proc_data = g_new(hrrn_data_t, 1);
		proc->algorithm_data = proc_data;
	} else
		proc_data = (hrrn_data_t *)proc->algorithm_data;
	
	/* we use this pointer to get read of the ugly cast:
	((proc_t *)proc->algorithm_data)->last_run=get_time(); */
	proc_data->last_run=get_time();

	return 0;
}

static proc_t *hrrn_next_proc(proc_queue_t queue)
{
	/* we return the shortest process in the queue */
	proc_t *next = proc_data(queue);
	/* highest response ratio */
	gfloat highest_rr = (gfloat)(waiting(next) + burst(next))/ 
				(gfloat)burst(next);
	
	while (!proc_queue_end(queue)){
		proc_t *proc;
		gfloat rr;
		
		proc = proc_data(queue);
		rr = (gfloat)(waiting(proc) + burst(proc))/ 
				(gfloat)burst(proc);
		if (rr > highest_rr){
			next = proc;
			highest_rr = rr;
		}
		queue = proc_queue_next(queue);
	}
	return next;
}

gint hrrn_init (void)
{
	register_CPU_algorithm (&hrrn_algorithm);
	return 0;
}
static gint hrrn_select_proc (proc_t *proc)
{
	return 0;
}
static gint hrrn_select (void)
{
	proc_t *current_proc = get_CPU_current_proc();

	set_CPU_heart_beat(CPU_NO_HEART_BEAT);
	
	request_nqueues (1);
	if(current_proc != NULL)
		hrrn_set_last_run_time(current_proc);
	proc_queue_foreach(get_proc_list(),
			(GFunc)hrrn_set_last_run_time,
			NULL);
	return 0;
}
static gint hrrn_unselect (void)
{
	deallocate_algorithm_private_data(get_proc_list());
	return 0;
}
static gint hrrn_init_proc (proc_t *proc)
{
	const proc_queues_t *queues=get_CPU_queues();

	hrrn_set_last_run_time(proc);

	if (queues->current==NULL)
		move_proc_to_CPU(proc);
	else {
		move_proc_to_queue(proc, 0);
	}
	return 0;
}
static gint hrrn_end_proc (proc_t *proc)
{
	return 0;
}
static gint hrrn_event (proc_t *proc)
{
	if (get_CPU_current_proc()==NULL)
		move_proc_to_CPU(proc);
	else
		move_proc_to_queue(proc, 0);

	return 0;
}
static gint hrrn_next (proc_t *proc)
{
	const proc_queues_t *queues=get_CPU_queues();

	hrrn_set_last_run_time(proc);

	g_return_val_if_fail (get_CPU_current_proc()==NULL, 0);

	if (!proc_queue_empty(queues->queue[0])){
		proc_t *next_proc = hrrn_next_proc(queues->queue[0]);
		if (next_proc!=NULL)
			move_proc_to_CPU(next_proc);
	}

	return 0;
}

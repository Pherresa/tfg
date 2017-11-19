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
#include <gnome.h>

#include <CLOCK/CLOCK.h>
#include <events.h>

#include "algorithms/algorithms.h"
#include "drawings/main.h"
#include "cpu_config.h"
#include "status.h"
#include "queues.h"
#include "info.h"

static proc_queues_t *queues = NULL;

void set_queue_in_proc (gpointer proc, gpointer new_queue);

/**
 * get_CPU_queues:
 *
 * Retrive the processor's status.
 *
 * returns: a pointer to the proc_queues_t structure.
 */
const proc_queues_t *get_CPU_queues(void)
{
	return queues;
}
/**
 * get_CPU_current_proc:
 *
 * returns: The currently running process, which may be NULL if the processor
 * is idle.
 */
proc_t *get_CPU_current_proc (void)
{
	return queues->current;
}
/**
 * get_CPU_queue:
 * @nqueue: requested queue.
 *
 * returns: ready queue number @nqueue.
 */
proc_queue_t get_CPU_queue(gint nqueue)
{
	return queues->queue[nqueue];
}
/**
 * get_CPU_wait_queue:
 *
 * returns: the blocked process queue.
 */
proc_queue_t get_CPU_wait_queue(void)
{
	return queues->wait;
}
/**
 * set_queue_in_proc:
 * @proc: process involved.
 * @new_queue: value to be set in @nqueue member.
 *
 * This is to be used in g_slist_foreach to set the queue number acording 
 * to the new queue when coping a whole queue into another 
 */
void set_queue_in_proc (gpointer proc, gpointer new_queue)
{
	((proc_t *)proc)->nqueue=*(gint *)new_queue;
	return;
}
/**
 * request_nqueues:
 * @nqueues: requested number of queues.
 *
 * Sets the number of queues for handling ready processes.
 * 
 * NOTE: when shrinking the lower queues, those which will be deleted, must be
 * empty or otherwise they will be concatenated to the first queue.
 * 
 * returns: nothing important.
 */
gint request_nqueues (gint nqueues)
{
#ifdef DEBUG
	g_print ("request_nqueues: Allocating %d queues\n", nqueues);
#endif
	lock_CPU_drawing();
	if (queues == NULL){
#ifdef DEBUG
		g_print ("request_nqueues: queues was NULL\n");
#endif
		queues = g_new (proc_queues_t, 1);
		queues->current = NULL;
		/* why nqueues+1? this way the array of queues will be
		 * null terminated and we get a sigserv if we access beond
		 * the end */
		queues->queue = g_new0 (proc_queue_t, nqueues+1);
		queues->nqueues = nqueues;
		proc_queue_init(queues->wait);
	} else if (nqueues != queues->nqueues) {
		gint i;

		/* we use g_malloc0 to make sure we get an array full of 
		 * null pointers */
		proc_queue_t *new_queue = g_new0 (proc_queue_t, nqueues);
#ifdef DEBUG
		g_print ("request_nqueues: queues was not NULL\n");
#endif
		for (i=0; (i < nqueues)&&(i < queues->nqueues); i++)
			new_queue[i]=queues->queue[i];

		/* this wont get executed unless the number of queues
		 * is shrinking in which case the processes from the 
		 * lower queues are put into queue 0 */
		for (i=nqueues; i < queues->nqueues; ++i){
			gint aux=0;
			proc_queue_foreach(queues->queue[i],
					(GFunc)set_queue_in_proc,
					(gpointer) &aux);
			proc_queue_concat(new_queue[0], new_queue[0],
					queues->queue[i]);
#ifdef DEBUG
			g_print ("request_nqueues: queue %d merged "
					"with queue 0\n", i);
#endif
		}
		g_free (queues->queue);
		queues->queue = new_queue;
		queues->nqueues = nqueues;
	}
#ifdef DEBUG
	g_print ("request_nqueues: %d queues allocated\n", nqueues);
#endif
	unlock_CPU_drawing();
	redraw_CPU_drawing();
	return 0;
}
/**
 * move_proc_to_queue:
 * @proc: process to be moved.
 * @new_queue: target queue for @proc.
 *
 * Move @proc to queue number @new_queue.
 *
 * NOTE: The @proc should not be blocked.
 */
void move_proc_to_queue (proc_t *proc, gint new_queue)
{
	GString *str;

	g_return_if_fail(proc->nqueue != CPU_WAITING);
	str = g_string_new(NULL);
	lock_CPU_drawing();
	g_return_if_fail(new_queue >= 0);

	if (CPU_config->stop_clock)
		CLOCK_stop();
	
	remove_proc_from_queue(proc);
	proc_queue_append (queues->queue[new_queue], proc);
	proc->nqueue = new_queue;
	g_string_sprintf(str, _("process %d moved to queue %d"),
			proc->pid, new_queue);
	CPU_status_set(str->str);
	g_string_free(str, TRUE);
	unlock_CPU_drawing();
	redraw_CPU_drawing();
}
/**
 * move_proc_to_CPU:
 * @proc: process to run.
 *
 * Starts running process @proc.
 *
 * NOTE: the processor should be idle.
 */
void move_proc_to_CPU (proc_t *proc)
{
	GString *str;

	g_return_if_fail (queues->current == NULL);
	g_return_if_fail (proc->nqueue != CPU_WAITING);

	str = g_string_new(NULL);
	lock_CPU_drawing();

	if (CPU_config->stop_clock)
		CLOCK_stop();
	
	remove_proc_from_queue(proc);

	queues->current = proc;
	proc->nqueue = CPU_CURRENT;
	unlock_CPU_drawing();
	redraw_CPU_drawing();
	system_event(SYS_EVENT_PROC_RUNNING, proc);
	g_string_sprintf(str, _("process %d is now running"), proc->pid);
	CPU_status_set(str->str);
	g_string_free(str, TRUE);
}
/**
 * remove_proc_from_queue:
 * @proc: process involved.
 *
 * Remove @proc from any queue or from the processor.
 *
 * NOTE: @proc should not be blocked.
 */
void remove_proc_from_queue (proc_t *proc)
{
	g_return_if_fail(proc->nqueue != CPU_WAITING);
	
	lock_CPU_drawing();

	if (CPU_config->stop_clock)
		CLOCK_stop();
	
	if (proc->nqueue >= 0)
		proc_queue_remove (queues->queue[proc->nqueue], proc);
	else if (proc->nqueue == CPU_CURRENT){
		queues->current = NULL;
		system_event(SYS_EVENT_PROC_QUEUED, proc);
	}

	proc->nqueue = CPU_NO_QUEUE;
	unlock_CPU_drawing();
	redraw_CPU_drawing();
}
/**
 * suspend_proc:
 * @proc: process involved.
 * 
 * Move @proc out of the way when it blocks.
 *
 * returns: nothing important.
 */
gint suspend_proc(proc_t *proc)
{
	GString *str = g_string_new(NULL);
	if (CPU_config->stop_clock)
		CLOCK_stop();
	
	remove_proc_from_queue(proc);
	proc->nqueue= CPU_WAITING;
	proc_queue_append (queues->wait, proc);
	system_event(SYS_EVENT_PROC_WAITING, proc);
	g_string_sprintf(str, _("process %d is now blocked"), proc->pid);
	CPU_status_set(str->str);
	return 0;
}
/**
 * wakeup_proc:
 * @proc: process involved.
 *
 * Move a process back when it becomes ready again letting the current
 * algorithm decide were to put it.
 *
 * returns: nothing important.
 */
gint wakeup_proc(proc_t *proc)
{
	GString *str = g_string_new(NULL);
	cpu_algorithm_t *algorithm=get_CPU_current_algorithm();

	if (CPU_config->stop_clock)
		CLOCK_stop();
	
	proc_queue_remove (queues->wait, proc);
	proc->nqueue = CPU_NO_QUEUE;
	algorithm->event(proc);
	system_event(SYS_EVENT_PROC_READY, proc);
	g_string_sprintf(str, _("process %d is now ready"), proc->pid);
	CPU_status_set(str->str);

	return 0;
}

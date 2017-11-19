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

#ifndef PROCESS_H
#define PROCESS_H
#include <gtk/gtk.h>

/* encapsulation for process lists */
/*#define PROC_QUEN_ENCAP_CHECK*/
#ifdef PROC_QUEN_ENCAP_CHECK
/* hack to make the compiler cry when not using the encapsulation */
typedef int proc_queue_t;
#define DECLARE_PROC_QUEUE(queue) proc_queue_t queue=0
#define proc_queue_empty(queue) (queue == 0)
#define proc_data(element) NULL
#define proc_queue_next(element) ((proc_queue_t)0)
#define proc_queue_find(queue, proc) ((proc_queue_t)0)
#define proc_queue_len(queue) (5)
#define proc_queue_init(queue) (queue=0)
#define proc_queue_foreach(queue, func, data) 0
#define proc_queue_concat(dest, orig1, orig2) 0
#define proc_queue_remove(queue, proc) 0
#define proc_queue_append(queue, proc) 0
#define proc_queue_nth(queue, n) NULL
#else
/* true encapsulation */
typedef GSList * proc_queue_t;
#define DECLARE_PROC_QUEUE(queue) proc_queue_t queue=NULL
#define proc_queue_empty(queue) (queue == NULL)
#define proc_data(element) ((proc_t *)(element->data))
#define proc_queue_next(element) (element->next)
#define proc_queue_find(queue, proc) (g_slist_find(queue, proc))
#define proc_queue_len(queue) g_slist_length(queue)
#define proc_queue_init(queue) (queue=NULL)
#define proc_queue_foreach(queue, func, data) g_slist_foreach(queue, \
							      func, data)
#define proc_queue_concat(dest, orig1, orig2) \
(dest = g_slist_concat(orig1, orig2))
#define proc_queue_remove(queue, proc) (queue = g_slist_remove (queue, proc))
#define proc_queue_append(queue, proc) (queue = g_slist_append (queue, proc))
#define proc_queue_nth(queue, n) 	((proc_t *)g_slist_nth_data(queue, n))
#endif

#define proc_queue_end(element) proc_queue_empty(element)

/* This struct is what they all know of events (I/O, ...) */
typedef struct event_s {
	gint time;
	gint type;
	void *data;
} event_t;
/* this are the types of events */
enum {EVENT_NONE, EVENT_IO, EVENT_TERM, EVENT_START};

/* This struct is what we know of processes */
typedef struct proc_s {
	gint pid;
	gint time;
	event_t pending_event;
	event_t next_event;
	gint nqueue;
	GtkWidget *list_item;
	GdkGC *color_gc;
	void *simul_data;
	void *algorithm_data;
	void *stats_data;
} proc_t;
#define burst(proc) (proc->next_event.time - proc->time)

/* this are special values for nqueues */
enum {CPU_WAITING=-3,		/*process is in the wait queue */
	CPU_NO_QUEUE=-2,	/*process is nowhere not even in the CPU*/
	CPU_CURRENT=-1		/* process is running in the CPU */

};

proc_t *create_process (void);
proc_t *new_process (void);
void insert_process(proc_t *proc);

void save_processes_to_file (void);
void load_processes_from_file (void);

void free_process (proc_t *proc);
gint destroy_process (proc_t *proc);
proc_queue_t get_proc_list(void);
proc_t *get_proc_by_pid(gint pid);
void select_process (proc_t *proc);
proc_t *get_CPU_selected_proc (void);
#endif

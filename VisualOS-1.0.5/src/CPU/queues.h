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

#ifndef CPU_QUEUES_H
#define CPU_QUEUES_H
#include <gtk/gtk.h>

#include <process.h>

typedef struct {		/* Processor status */
	gint nqueues;		/* Number of queues */
	proc_t *current;	/* Currently running process */
	proc_queue_t *queue;	/* Ready process queues */
	proc_queue_t wait;	/* Blocked processes */
} proc_queues_t;

const proc_queues_t *get_CPU_queues(void);
proc_t *get_CPU_current_proc (void);
proc_queue_t get_CPU_queue(gint nqueue);
proc_queue_t get_CPU_wait_queue(void);
gint request_nqueues (gint nqueues);
void move_proc_to_queue (proc_t *proc, gint new_queue);
void move_proc_to_CPU (proc_t *proc);
void remove_proc_from_queue (proc_t *proc);
gint suspend_proc(proc_t *proc);
gint wakeup_proc(proc_t *proc);
#endif

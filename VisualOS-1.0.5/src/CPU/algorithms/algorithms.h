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

#ifndef ALGORITHMS_H
#define ALGORITHMS_H
#include <gtk/gtk.h>

#include <CPU/simulation.h>

typedef struct { /*This struct is all that we know about each algorithm*/
	gchar * name;				
	gint (*select) (void);
	gint (*unselect) (void);		/* These two functions will be
                                                   called before and after the
                                                   use of an algorithm to let
                                                   it keep a low memory usage
                                                   when not in use.*/
	gint (*clock) (void);			/* timer interrupt. */
	gint (*select_proc) (proc_t *proc);	/* notifies the algorithm of a
						   process selection by the
						   user */
	GtkWidget * process_properties;
	GtkWidget * properties;			/* Each algorithm will maintain
						   it's own properties widgets.
						   NULL means "no properties".
						   They should be set to NULL
						   when destroyed. If not
						   destroyed in "unselect" the
						   system will destroy them.*/
	gint (*init_proc) (proc_t *proc);	/* This function should allocate
						   and initialice algorith data
						   and anything else to get
						   a new process going, like
						   sticking it into a queue. */
	gint (*end_proc) (proc_t *proc);	/* This function should free the
						   algorithm specific data of
						   proc but should not take it
						   out of its queue */
	gint (*event) (proc_t *proc);		/* This function is called when
						   ever a process gets waked up
						   by an event and we have to
						   put it in some queue. */
	gint (*next) (proc_t *proc);		/* This function is called when
						   ever the current process gets
						   suspended waiting for some
						   event and we have to choose
						   another one to run.
						   It receives the suspended
						   process as argument just in
						   case its needed. */
} cpu_algorithm_t;


GSList * init_CPU_algorithms(void);
gint register_CPU_algorithm (cpu_algorithm_t * algorithm);
gint deallocate_algorithm_private_data(proc_queue_t proc_list);
#endif

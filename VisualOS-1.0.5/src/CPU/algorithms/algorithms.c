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
#include <stdio.h>

#include <events.h>
#include <CPU/info.h>

#include "algorithms.h"
#include "manual.h"
#include "test.h"
#include "fcfs.h"
#include "rr.h"
#include "spn.h"
#include "srt.h"
#include "hrrn.h"

static GSList *CPU_algorithms = NULL;
static void proc_select (sys_event_t type, proc_t *proc)
{
	get_CPU_current_algorithm()->select_proc(proc);
}
/**
 * init_CPU_algorithms:
 * 
 * Initializes the CPU algorithms code.
 *
 * Mainly will call init functions for each algorithm.
 *
 * returns: a pointer to the algorithm structs linked list
 */
GSList * init_CPU_algorithms(void)
{
	/* Por si se llamara a esta funcion mas de una vez 
	   solo en la primera llamada se inicializan los algoritmos */
	if (CPU_algorithms==NULL){
		fcfs_init();
		rr_init();
		spn_init();
		srt_init();
		hrrn_init();
		manual_init();
#ifdef DEBUG
		test_init();
#endif
	}
	system_event_receive(SYS_EVENT_PROC_SELECT,
			(sys_event_callback *)proc_select);
	return CPU_algorithms;
}
/**
 * register_CPU_algorithm:
 * @algorithm: algorithm struct to register.
 *
 * Each algorithm should call this function in it's initialization function
 * to register its algorithm struct.
 *
 * returns: nothing important.
 */
gint register_CPU_algorithm (cpu_algorithm_t * algorithm)
{
	CPU_algorithms = g_slist_append(CPU_algorithms, algorithm);
	return 0;
}
static gint deallocate_algorithm_data(proc_t *proc)
{
	g_free(proc->algorithm_data);
	proc->algorithm_data = NULL;
	return 0;
}
/**
 * deallocate_algorithm_private_data:
 * @proc_list: queue of processes.
 *
 * This function uses the algorithm's private data of each process
 * as argument to g_free.
 *
 * This is for convinience of algorithm writers.
 *
 * returns: nothing important.
 */
gint deallocate_algorithm_private_data(proc_queue_t proc_list)
{
	proc_queue_foreach(proc_list,
			(GFunc)deallocate_algorithm_data,
			NULL);
	return 0;
}

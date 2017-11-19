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

#ifndef CPU_SIMULATION_H
#define CPU_SIMULATION_H
#include <gtk/gtk.h>

#include <process.h>

#include "editor/editor.h"

typedef struct {	/* data for an IO event */
	gint block;	/* block to read */
	gint time;	/* time to read @block */
} simul_io_event_t;
typedef struct {	/* data for a page access */
	gint8 page;	/* page to access */
	gint8 write;	/* is it a write access? */
} simul_mem_t;
typedef struct {		/* simulation data for a process */
	gint start_time;	/* time of creation */
	gint end_time;		/* length of the process */
	/* IO properties */
	/* this list is never modified until process destruction */
	simul_io_event_t *io_events;		/* list of all io events */
	simul_io_event_t *next_io_event;	/* pointer to the next event */
	simul_io_event_t *last_io_event;	/* pointer to the last event */
	/* MEM properties */
	simul_mem_t *pages;	/* list of memory accesses */
	gint n_pages;		/* number of memory accesses */
	gint cur_access;	/* index to the current access */
} simul_data_t;
typedef struct {	/* io event data known by all the code */
	gint io_block;	/* block to access */
} event_data_t;
#define IO_BLOCK(event) (((event_data_t *)(event.data))->io_block)
void init_CPU_simulation (void);
void init_CPU_simulation_in_proc(proc_t *proc);
void end_CPU_simulation_in_proc(proc_t *proc);
void cpy_CPU_simulation_data(simul_data_t *dest, simul_data_t *src);
simul_data_t *dup_CPU_simulation_data(simul_data_t *data);
void free_CPU_simulation_data(simul_data_t *data);
void free_CPU_proc_simulation_data(proc_t *proc);
void next_CPU_simulation_in_proc(proc_t *proc);
/* returns the memory page which is currently in use by the process */
gint CPU_proc_current_page(proc_t *proc);
/* sets a new "current page" for the process and returns it */
gint CPU_proc_next_page(proc_t *proc);
/* is proc accessing current page for writeing? */
gboolean CPU_proc_current_page_is_write(proc_t *proc);
void fix_simulation_in_proc(proc_t *proc);

#endif



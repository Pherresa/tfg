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

#include <IO/IO.h>
#include <CPU/simulation.h>

#include "parser.h"

GString *get_proc_in_gstring(const proc_t *proc)
{
	simul_data_t *data = proc->simul_data;
	simul_io_event_t *event;
	GString *string = g_string_new("\nproc {\n");
	gint i;

	g_string_sprintfa(string, "\tstart_time=%d\n", data->start_time);
	g_string_sprintfa(string, "\tend_time=%d\n", data->end_time);
	g_string_sprintfa(string, "\tIO{\n");
	if(data->io_events)
	for (event = data->io_events; event <= data->last_io_event; ++event){
		g_string_sprintfa(string, "\t\tblock=%d time=%d\n",
				event->block, event->time);
	}
	g_string_sprintfa(string, "\t}\n");
	g_string_sprintfa(string, "\tMEM{\n");
	if(data->pages)
	for (i=0; i < data->n_pages; ++i){
		g_string_sprintfa(string, "\t\tpage=%d write=%d\n",
				data->pages[i].page, data->pages[i].write);
	}
	g_string_sprintfa(string, "\t}\n");
	g_string_sprintfa(string, "}\n");
	return string;
}


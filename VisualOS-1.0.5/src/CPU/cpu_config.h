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

#include <glib.h>
#include <glade/glade.h>

#include "editor/io.h"
#include "editor/memory.h"

typedef struct {
	gboolean stop_clock;	/* clock should be stoped when something
				   interesting happens */
	gboolean auto_fill_procs;	/* process properties should be
					   filled automaticly without any
					   user interaction */
	prop_io_params_t prop_io_params;   /* current parameters for processes
					      IO parameters autofilling */
	prop_mem_params_t prop_mem_params; /* current parametes for processes
					      Memory parametes autofilling */
	struct {			/* parameters related with the 
					   graphical representation of the 
					   subsystem */
		gint max_graph_history;	/* Maximun pixmap width for the
					   different representations */
		gint pix_size_step;
	} drawing;
} cpu_config_t;

extern const cpu_config_t *CPU_config;
cpu_config_t *get_CPU_config (void);
void init_CPU_config(GladeXML *xml);

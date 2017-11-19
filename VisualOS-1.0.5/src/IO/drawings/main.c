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
#include <stdio.h>

#include <IO/simulation.h>
#include <IO/info.h>
#include <drawing.h>

#include "round.h"
#include "new.h"

static GtkWidget *drawing = NULL;

GtkWidget *new_IO_drawing(void);
	
void redraw_IO_drawing (void);
void update_queue_entries(void);
gint unlock_IO_drawing(void);
gint lock_IO_drawing(void);

	

GtkWidget *new_IO_drawing(void)
{
	drawing = create_drawing();

	init_IO_drawing_round(drawing);
	init_IO_drawing_new(drawing);

        return drawing;
}

static gint drawing_locked = FALSE;

gint lock_IO_drawing(void)
{
	drawing_locked = TRUE;
	return 0;
}
gint unlock_IO_drawing(void)
{
	drawing_locked = FALSE;
	return 0;
}

static void append_request_in_entry(io_request_t *request, GtkWidget *entry)
{
	gchar buff[10];

	sprintf(buff, " %d(%d)", request->block, request->track);
	gtk_entry_append_text(GTK_ENTRY(entry), buff);
}

void update_queue_entries(void)
{
	GtkWidget *reading_entry = get_IO_reading_entry();
	GtkWidget *requested_entry = get_IO_requested_entry();
	io_queue_t reading = get_IO_reading_queue();
	io_queue_t requested = get_IO_requested_queue();

	gtk_entry_set_text(GTK_ENTRY(reading_entry),"");
	gtk_entry_set_text(GTK_ENTRY(requested_entry),"");
	io_queue_foreach( reading,
			append_request_in_entry,
			reading_entry);
	io_queue_foreach( requested,
			append_request_in_entry,
			requested_entry);
}

void redraw_IO_drawing (void)
{
	/* we are called before the drawing gets created so we have to make
	 * sure it exists before doing anything*/
	if (drawing==NULL)
		return;
	g_return_if_fail (!drawing_locked);

	update_drawing(drawing);
}


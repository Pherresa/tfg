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

#include <unistd.h>
#include <gtk/gtk.h>
#include <glib.h>

#include <messaging.h>
#include <interface.h>
#include <SCHED.h>

#include "algorithms/algorithms.h"
#include "drawings/main.h"
#include "simulation.h"
#include "io_config.h"
#include "status.h"
#include "combo.h"
#include "info.h"
#include "IO.h"

void IO_main (void);



static gboolean delete_event (GtkWidget * window, GdkEvent * event, 
		gpointer pointer)
{
	gtk_widget_hide(GTK_WIDGET(window));
	return TRUE;
}
GtkWidget *app;
static void misc_show(void)
{
	gtk_widget_show(GTK_WIDGET(app));
}

void IO_main (void)
{
	GSList *algorithms;
	GtkWidget *requested;
	GtkWidget *reading;
	GtkWidget *combo;
	GtkWidget *drawing;
	GtkWidget *box;
	GladeXML *xml;

	mesg_subsystem_setup(IO, MESG_WITH_GTK);

	xml = glade_xml_new(get_xml_file(), "IO");
	glade_xml_signal_connect(xml, "on_IO_delete_event",
				 GTK_SIGNAL_FUNC(delete_event));

	app = glade_xml_get_widget(xml, "IO");

	algorithms = init_IO_algorithms();

	requested=glade_xml_get_widget(xml, "IO_requested_entry");
	reading=glade_xml_get_widget(xml, "IO_reading_entry");

	set_IO_requested_entry(requested);
	set_IO_reading_entry(reading);
	
	combo = glade_xml_get_widget(xml, "IO_algorithm_combo");
	init_IO_algorithm_combo(xml, algorithms);

	init_IO_status(xml);
	init_IO_config(xml);

	box = glade_xml_get_widget(xml, "IO_mainbox");
	drawing=new_IO_drawing();
	gtk_container_add(GTK_CONTAINER(box), drawing);
	gtk_widget_show (GTK_WIDGET(drawing));

	mesg_callback_register(MISC_SHOW, (receive_callback)misc_show);
	io_server_init();
	init_IO_simulation();
	gtk_main();
	return;
}









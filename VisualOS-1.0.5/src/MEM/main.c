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
#include <unistd.h>
#include <gtk/gtk.h>
#include <glib.h>

#include <messaging.h>
#include <interface.h>
#include <SCHED.h>

#include "MEM.h"
#include "status.h"
#include "info.h"
#include "page_info.h"
#include "swap.h"
#include "drawings/main.h"
#include "misc_menu_callbacks.h"
#include "combo.h"
#include "mem_config.h"

void MEM_main (void);


GtkWidget *app = NULL;
static gboolean delete_event (GtkWidget * window, GdkEvent * event, 
		gpointer pointer)
{
	gtk_widget_hide(GTK_WIDGET(app));
	return TRUE;
}
static void misc_show(void)
{
	gtk_widget_show(GTK_WIDGET(app));
}
void MEM_main (void)
{
	GSList *algorithms;
	GladeXML *xml;
	GtkWidget *drawing;
	GtkWidget *box;

	mesg_subsystem_setup(MEM, MESG_WITH_GTK);

	xml = glade_xml_new(get_xml_file(), "MEM");
	glade_xml_signal_connect(xml, "on_MEM_delete_event",
			GTK_SIGNAL_FUNC(delete_event));
	algorithms = init_MEM_algorithms();
	set_MEM_algorithms (algorithms);
	init_MEM_algorithm_combo(xml, algorithms);

	app = glade_xml_get_widget(xml, "MEM");
	box = glade_xml_get_widget(xml, "MEM_mainbox");
	drawing=new_MEM_drawing();
	gtk_widget_set_usize(GTK_WIDGET(drawing), 200, 150);
	gtk_container_add(GTK_CONTAINER(box), drawing);
	gtk_widget_show(GTK_WIDGET(drawing));

	mesg_callback_register(MISC_SHOW, (receive_callback)misc_show);

	init_MEM_status(xml);
	init_MEM_misc_menu_callbacks(xml);
	init_page_info();
	init_MEM_config(xml);
	MEM_swap_init();
	mem_server_init(xml);
	gtk_main();
	return;
}




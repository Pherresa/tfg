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
#include <gtk/gtk.h>
#include <glade/glade.h>

#include <messaging.h>
#include <process.h>
#include <interface.h>

#include "misc_menu_callbacks.h"
#include "drawings/main.h"
#include "simulation.h"
#include "cpu_config.h"
#include "status.h"
#include "combos.h"
#include "queues.h"
#include "clock.h"
#include "stats.h"
#include "main.h"
#include "info.h"
#include "CPU.h"

#ifdef DEBUG
static void list_queues(void)
{
	const proc_queues_t *queues;
	proc_queue_t queue;
	gint i;
	queues = get_CPU_queues();
	if (queues->current != NULL)
		g_print ("Process %d running\n",queues->current->pid);
	else
		g_print ("No process running\n");
	for (i=0; i<queues->nqueues; i++){
		queue = queues->queue[i];
		g_print ("Printing queue number %d\n",i);
		while (queue){
			proc_t *proc = proc_data(queue);
			g_print("%d ",proc->pid);
			queue = proc_queue_next(queue);
		}
		g_print ("\n");
	}
}
#endif
static gboolean delete_event (GtkWidget * window, GdkEvent * event, 
			      gpointer pointer)
{
	return TRUE;
}
static void quit(void)
{
	mesg_broadcast(MISC_QUIT, NULL, 0);
	gtk_main_quit();
}

static void setup_menu(GladeXML *xml)
{
	glade_xml_signal_connect(xml, "on_CPU_menu_file_open_activate",
			GTK_SIGNAL_FUNC(load_processes_from_file));
	glade_xml_signal_connect(xml, "on_CPU_menu_file_save_activate",
			GTK_SIGNAL_FUNC(save_processes_to_file));
	glade_xml_signal_connect(xml, "on_CPU_exit",
			GTK_SIGNAL_FUNC(quit));
}
void CPU_main (void)
{
	GtkWidget *drawing = NULL;
	GtkWidget *mainbox = NULL;
	GSList *algorithms = NULL;
	GladeXML *xml;

	mesg_subsystem_setup (CPU, MESG_WITH_GTK);

	xml = glade_xml_new(get_xml_file(), "CPU");
	glade_xml_signal_connect(xml, "on_CPU_delete_event",
			GTK_SIGNAL_FUNC(delete_event));

	setup_menu(xml);
	algorithms = init_CPU_algorithms();
	set_CPU_algorithms (algorithms);

	set_CPU_window(glade_xml_get_widget(xml, "CPU"));
	/* some function within init_CPU_simulation needs the CPU_window*/
	init_CPU_simulation();

	mainbox = glade_xml_get_widget(xml, "CPU_mainbox");

	init_CPU_proc_gui(xml);

	init_CPU_algorithm_combo(xml, algorithms);

	drawing=new_CPU_drawing();
	gtk_widget_set_usize(GTK_WIDGET(drawing), 200, 150);
	gtk_container_add(GTK_CONTAINER(mainbox), drawing);
	gtk_widget_show(GTK_WIDGET(drawing));

	init_CPU_clock();
	init_CPU_stats();
	init_CPU_prop_editor(xml);
	init_CPU_misc_menu_callbacks(xml);
	cpu_server_init();
	init_CPU_status(xml);
	init_CPU_config(xml);

	gtk_main();
#ifdef DEBUG
	list_queues();
#endif
	return;
}		




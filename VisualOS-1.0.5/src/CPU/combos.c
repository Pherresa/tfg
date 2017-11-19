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
#include <gnome.h>
#include <stdio.h>

#include <process.h>
#include <events.h>

#include "editor/editor.h"
#include "combos.h"
#include "info.h"

static GtkWidget *algorithm_box = NULL;
static GtkWidget *proc_box = NULL;

static GtkWidget *pid_label;
static GtkWidget *time_label;
static GtkWidget *burst_label;

static void proc_select (sys_event_t type, proc_t *proc)
{
	GString *pid_string = g_string_new(_("pid:"));
	GString *time_string = g_string_new(_("time:"));
	GString *burst_string = g_string_new(_("burst:"));

	if (proc != NULL){
		g_string_sprintf (pid_string, _("pid: %d"), proc->pid);
		g_string_sprintf (time_string, _("time: %d"), proc->time);
		g_string_sprintf (burst_string, _("burst: %d"), burst(proc));
	}
	
	gtk_label_set(GTK_LABEL(pid_label), pid_string->str);
	gtk_label_set(GTK_LABEL(time_label), time_string->str);
	gtk_label_set(GTK_LABEL(burst_label), burst_string->str);

	g_string_free (pid_string, TRUE);
	g_string_free (time_string, TRUE);
	g_string_free (burst_string, TRUE);
}
static void edit_button_callback(GtkWidget *widget)
{
	proc_t *proc=get_CPU_selected_proc();
	if (proc != NULL)
		edit_process_properties(proc);
}
/**
 * init_CPU_proc_gui:
 * @xml: Glade interface object.
 *
 * Setup the process related GUI.
 */
void init_CPU_proc_gui(GladeXML *xml)
{
	proc_box= glade_xml_get_widget(xml, "CPU_proc_box");
	glade_xml_signal_connect(xml, "on_CPU_new_proc_clicked",
			GTK_SIGNAL_FUNC (create_process));
	glade_xml_signal_connect(xml, "on_CPU_edit_proc_clicked",
			GTK_SIGNAL_FUNC (edit_button_callback));

	pid_label = glade_xml_get_widget(xml, "CPU_proc_pid");
	time_label = glade_xml_get_widget(xml, "CPU_proc_time");
	burst_label = glade_xml_get_widget(xml, "CPU_proc_burst");
	system_event_receive(SYS_EVENT_PROC_SELECT,
			(sys_event_callback *) proc_select);
	return;
}	
static void algorithm_select (GtkWidget *item, cpu_algorithm_t *algorithm)
{
	cpu_algorithm_t *current;
	static gint repeat=0;
	static GtkWidget *properties;

	/* curiosusly each signal calls this function twice so I have to
	 * filter out one of them */
	repeat = !repeat;
	if (repeat)
		return;
	
	current = get_CPU_current_algorithm();
	if (current != NULL){
		current->unselect();
		if (current->properties != NULL)
			gtk_widget_destroy(GTK_WIDGET(current->properties));
		if (current->process_properties != NULL)
			gtk_widget_destroy(GTK_WIDGET(
						current->process_properties));
		current->process_properties = current->properties = NULL;
	}
	algorithm->select();


	/* if we don't have any of the properties widgets stick a dummy widget 
	 * in it's place */
	if (algorithm->properties == NULL){
		properties = gtk_label_new( _("No algorithm properties\n"
						"available"));
		algorithm->properties = properties;
	}else
		properties = algorithm->properties;

	gtk_box_pack_start (GTK_BOX(algorithm_box), properties, TRUE, TRUE, 0);
	gtk_widget_show(GTK_WIDGET(properties));

	if (algorithm->process_properties != NULL){
		gtk_box_pack_start (GTK_BOX(proc_box),
				algorithm->process_properties, TRUE, TRUE, 0);
		gtk_widget_show(GTK_WIDGET(algorithm->process_properties));
	}

	set_CPU_current_algorithm (algorithm);
	
}
	
/**
 * init_CPU_algorithm_combo:
 * @xml: Glade interface object.
 * @algorithms: the list of available algorithms.
 *
 * Setup the algorithm combo in @xml with the algorithms in @algorithms.
 */
void init_CPU_algorithm_combo(GladeXML *xml, GSList *algorithms) 
{
	GtkWidget * item;
	GtkWidget * list;

	algorithm_box = glade_xml_get_widget(xml, "CPU_algorithm_box");
	list = GTK_COMBO(
			glade_xml_get_widget(xml, "CPU_algorithm_combo"))->list;
	g_return_if_fail(GTK_IS_LIST(list));

	while (algorithms != NULL){
		cpu_algorithm_t *algorithm;
		algorithm = (cpu_algorithm_t *)algorithms->data;
		item = gtk_list_item_new_with_label (algorithm->name);
		gtk_widget_show (GTK_WIDGET(item));
		gtk_signal_connect(GTK_OBJECT(item), "select", 
				GTK_SIGNAL_FUNC(algorithm_select), 
				algorithm);
		gtk_container_add (GTK_CONTAINER (list), item);
		algorithms = algorithms->next;
	}
	return;
}


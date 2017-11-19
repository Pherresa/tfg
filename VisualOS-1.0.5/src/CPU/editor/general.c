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

#include <string.h>
#include <gtk/gtk.h>
#include <glade/glade.h>
#include <gnome.h>
#include <stdlib.h>

#include <IO/IO.h>
#include <CLOCK/CLOCK.h>
#include <CPU/simulation.h>
#include <CPU/cpu_config.h>

#include "general.h"
#include "util.h"

static GtkLabel *pid_label;
static GtkLabel *time_label;
static GtkSpinButton *start_entry;
static GtkSpinButton *end_entry;

GtkWidget *create_CPU_prop_general_editor(GladeXML *xml)
{
	pid_label = GTK_LABEL(glade_xml_get_widget(xml, "proc_prop_pid"));
	
	time_label = GTK_LABEL(glade_xml_get_widget(xml, "proc_prop_time"));

	start_entry = GTK_SPIN_BUTTON(
			glade_xml_get_widget(xml, "proc_prop_start_time"));
	end_entry = GTK_SPIN_BUTTON(
			glade_xml_get_widget(xml, "proc_prop_end_time"));
	
	return glade_xml_get_widget(xml, "proc_prop_general");
}

void read_CPU_prop_general(proc_t *proc)
{
	simul_data_t *data = proc->simul_data;
	GString *string = g_string_new(NULL);

	g_string_sprintf(string, _("pid: %d"), proc->pid);
	gtk_label_set_text(pid_label, string->str);
	g_string_sprintf(string, _("time: %d"), proc->time);
	gtk_label_set_text(time_label, string->str);

	gtk_spin_button_set_value(start_entry, data->start_time);
	gtk_spin_button_set_value(end_entry, data->end_time);

	g_string_free (string, TRUE);
}
void write_CPU_prop_general(simul_data_t *data)
{
	data->start_time = gtk_spin_button_get_value_as_int(start_entry);
	data->end_time =gtk_spin_button_get_value_as_int(end_entry);
}
void autofill_CPU_prop_general(proc_t *proc)
{
	gfloat avg_create = CPU_config->prop_io_params.avg_create; 
	gfloat avg_burst = CPU_config->prop_io_params.avg_burst;
	simul_data_t *data = proc->simul_data;
	gint sys_time = get_time();
	static gint last_start=0;

	data->start_time=last_start+(gint)exp_dist(avg_create);
	if (data->start_time < sys_time)
		data->start_time = sys_time;
	last_start=data->start_time;
	if (data->io_events !=NULL)
		data->end_time=data->last_io_event->time
					+ exp_dist(avg_burst)+1;
	else
		data->end_time=0; /* end time not set */
}

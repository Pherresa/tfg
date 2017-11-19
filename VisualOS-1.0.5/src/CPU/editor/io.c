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
#include <stdlib.h>

#include <IO/IO.h>
#include <CLOCK/CLOCK.h>
#include <CPU/simulation.h>
#include <CPU/cpu_config.h>

#include "io.h"
#include "util.h"

static GtkWidget *clist = NULL;
static GtkWidget *block_spin = NULL;
static GtkWidget *time_spin = NULL;

static void add_event( GtkWidget *widget)
{
	gchar *text[2];

	text[0] = gtk_entry_get_text(
			&GTK_SPIN_BUTTON(block_spin)->entry);
	text[1] = gtk_entry_get_text(
			&GTK_SPIN_BUTTON(time_spin)->entry);
	while (*text[1] == '0' && *(text[1]+1) !='\0')
		text[1]++;
	gtk_clist_append(GTK_CLIST(clist), text);
}
static void replace_event( GtkWidget *widget)
{
	gchar *text[2];
	gint row;

	if (GTK_CLIST(clist)->selection == NULL)
		return;

	row = GPOINTER_TO_INT(GTK_CLIST(clist)->selection->data);

	text[0] = gtk_entry_get_text(
			&GTK_SPIN_BUTTON(block_spin)->entry);
	text[1] = gtk_entry_get_text(
			&GTK_SPIN_BUTTON(time_spin)->entry);
	while (*text[1] == '0' && *(text[1]+1) !='\0')
		text[1]++;
	
	gtk_clist_set_text(GTK_CLIST(clist), row, 0, text[0] );
	gtk_clist_set_text(GTK_CLIST(clist), row, 1, text[1] );
	gtk_clist_sort(GTK_CLIST(clist));

}
static void remove_event( GtkWidget *widget)
{
	if (GTK_CLIST(clist)->selection == NULL)
		return;
	gtk_clist_remove(GTK_CLIST(clist),
			GPOINTER_TO_INT(GTK_CLIST(clist)->selection->data));
}
static void select_row_callback(GtkWidget *widget, gint row, gint column,
			 GdkEventButton *event, gpointer data)
{
	gchar *text;
	gtk_clist_get_text(GTK_CLIST(widget), row, 0, &text );
	gtk_entry_set_text(&GTK_SPIN_BUTTON(block_spin)->entry, text);
	
	gtk_clist_get_text(GTK_CLIST(widget), row, 1, &text );
	gtk_entry_set_text(&GTK_SPIN_BUTTON(time_spin)->entry, text);
}
static gint time_compare(GtkCList *clist,
			  GtkCListRow * row1,
			  GtkCListRow * row2)
{
	gchar *text1 = GTK_CELL_TEXT (row1->cell[clist->sort_column])->text;
	gchar *text2 = GTK_CELL_TEXT (row2->cell[clist->sort_column])->text;

	gint len1 = strlen(text1);
	gint len2 = strlen(text2);

	if (len1 != len2)
		return len1 - len2;
	else
		return strcmp(text1, text2);
}

static void init_add_replace_remove(GladeXML *xml)
{
	GtkAdjustment *adj;

	block_spin = glade_xml_get_widget(xml, "proc_prop_io_block_spin");
	adj = (GtkAdjustment *)gtk_adjustment_new(1, 0, get_IO_max_data_block(),
							1, 10, 0);
	gtk_spin_button_set_adjustment(GTK_SPIN_BUTTON(block_spin), adj);

	time_spin = glade_xml_get_widget(xml, "proc_prop_io_time_spin");
	adj = (GtkAdjustment *)gtk_adjustment_new(1, 0, G_MAXINT, 1, 10, 0);
	gtk_spin_button_set_adjustment(GTK_SPIN_BUTTON(time_spin), adj);

	glade_xml_signal_connect(xml, "proc_prop_io_remove",
					GTK_SIGNAL_FUNC(remove_event));
	glade_xml_signal_connect(xml, "proc_prop_io_replace",
					GTK_SIGNAL_FUNC(replace_event));
	glade_xml_signal_connect(xml, "proc_prop_io_add",
					GTK_SIGNAL_FUNC(add_event));
}
static gboolean disk_button_press (GtkWidget *widget, GdkEventButton *event,
			gpointer user_data)
{
	if (event->type != GDK_BUTTON_PRESS)
		return 0;

	gtk_spin_button_set_value (GTK_SPIN_BUTTON(block_spin),
					get_IO_max_data_block()
					*event->x/widget->allocation.width);
	return 0;
}

GtkWidget *create_CPU_prop_io_editor(GladeXML *xml)
{
	clist = glade_xml_get_widget(xml, "proc_prop_io_list");
	gtk_clist_set_compare_func(GTK_CLIST(clist),
				  (GtkCListCompareFunc)time_compare);
	gtk_clist_set_sort_column(GTK_CLIST(clist), 1);
	gtk_clist_set_auto_sort(GTK_CLIST(clist), TRUE);
	glade_xml_signal_connect(xml, "proc_prop_io_list_select",
					GTK_SIGNAL_FUNC(select_row_callback));
	glade_xml_signal_connect(xml, "on_proc_prop_io_disk_button_press",
					GTK_SIGNAL_FUNC(disk_button_press));
	init_add_replace_remove(xml);
	return glade_xml_get_widget(xml, "proc_prop_io");
}
static void dump_clist(GtkWidget *clist, simul_data_t *data)
{
	gint n_rows = GTK_CLIST(clist)->rows;
	char *endptr;
	char *prev="";
	gint i,j;

	g_free(data->io_events);
	data->io_events = g_new(simul_io_event_t, n_rows);

	for (i=0, j=0; i<n_rows; ++i){
		gchar *text;
		gtk_clist_get_text(GTK_CLIST(clist), i, 0, &text );
		data->io_events[j].block = strtol(text, &endptr, 10);
		if (*endptr != '\0')
			continue;
		gtk_clist_get_text(GTK_CLIST(clist), i, 1, &text );
		data->io_events[j].time = strtol(text, &endptr, 10);
		if (strcmp(prev, text) == 0){
			g_warning("there is more than one event at "
					"the same time");
			continue;
		}
		prev = text;
		if (*endptr != '\0')
			continue;
		++j;
	}
	data->io_events = g_renew(simul_io_event_t, data->io_events, j);
	data->next_io_event = data->io_events;
	data->last_io_event = data->io_events+j-1;

}
void read_CPU_prop_io(proc_t *proc)
{
	simul_data_t *data = proc->simul_data;
	simul_io_event_t *event = data->io_events;
	GString *time = g_string_new("");
	GString *block = g_string_new("");
	gchar *text[2];

	gtk_clist_clear(GTK_CLIST(clist));
	if (!event)
		return;
	for (event = data->io_events; event <= data->last_io_event; ++event){
		g_string_sprintf(block, "%d", event->block);
		g_string_sprintf(time, "%d", event->time);
		text[0] = block->str;
		text[1] = time->str;
		gtk_clist_append(GTK_CLIST(clist), text);
	}
}
void write_CPU_prop_io(simul_data_t *data)
{
	dump_clist(clist, data);
}
void autofill_CPU_prop_io(proc_t *proc)
{
	simul_data_t *data = proc->simul_data;
	gfloat avg_blocks = CPU_config->prop_io_params.avg_blocks;
	gfloat avg_burst = CPU_config->prop_io_params.avg_burst;
	gfloat avg_events = CPU_config->prop_io_params.avg_io_accesses;
	gint n_events = exp_dist(avg_events);
	gint n_blocks = exp_dist(avg_blocks)+1;
	gint *blocks;
	gint time=0;
	gint i;

	blocks = g_new(gint, n_blocks);

	for (i=0; i<n_blocks; ++i)
		blocks[i] = my_rand(0, get_IO_max_data_block());
	
	g_free(data->io_events);
	data->io_events = g_new(simul_io_event_t, n_events);
	for (i=n_events-1; i >= 0; --i){
		time += (gint)exp_dist(avg_burst) +1;
		data->io_events[i].block = blocks[my_rand(0, n_blocks-1)];
		data->io_events[i].time = time;
	}
	data->next_io_event = data->io_events;
	data->last_io_event = data->io_events+n_events-1;
	if (data->end_time==0){ /* if end_time not set, set it */
		if (data->io_events)
			data->end_time=data->last_io_event->time;
		data->end_time += exp_dist(avg_burst)+1;
	}
	g_free(blocks);
}

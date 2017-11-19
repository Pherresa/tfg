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
#include <string.h>
#include <gtk/gtk.h>
#include <gnome.h>
#include <glade/glade.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#include <IO/IO.h>
#include <CLOCK/CLOCK.h>
#include <CPU/simulation.h>
#include <interface.h>
#include <CPU/info.h>
#include <CPU/cpu_config.h>

#include "editor.h"
#include "general.h"
#include "advanced.h"
#include "memory.h"
#include "io.h"
#include "file.h"
#include "parser.h"
#include "util.h"

static GtkWidget *window = NULL;
static GtkWidget *notebook = NULL;
static GtkWidget *auto_fill_widget = NULL;
static GtkWidget *autofill_button = NULL;
static GtkWidget *average_blocks = NULL;
static GtkWidget *average_io_accesses = NULL;
static GtkWidget *average_burst = NULL;
static GtkWidget *average_create = NULL;
static proc_t *proc = NULL;
static simul_data_t simul_data_backup;

void autofill_params_ok(void);

static struct {
	gchar *name;
	GtkWidget * (*create)(GladeXML  *xml);
	void (*read)(proc_t *);
	void (*write)(simul_data_t *);
	void (*autofill)(proc_t *);
} *cur_property=NULL, prop_list[] = {
	{N_("General"),
		create_CPU_prop_general_editor,
		read_CPU_prop_general,
		write_CPU_prop_general,
		autofill_CPU_prop_general},
	{N_("I/O"),
		create_CPU_prop_io_editor,
		read_CPU_prop_io,
		write_CPU_prop_io,
		autofill_CPU_prop_io},
	{N_("Memory"),
		create_CPU_prop_memory_editor,
		read_CPU_prop_memory,
		write_CPU_prop_memory,
		autofill_CPU_prop_memory},
	{N_("Advanced"),
		create_CPU_prop_advanced_editor,
		read_CPU_prop_advanced,
		write_CPU_prop_advanced,
		NULL},
	{NULL, NULL, NULL, NULL, NULL}};
	

static gint delete_event( GtkWidget *widget,
			  GdkEvent  *event,
			  gpointer   data )
{
	gtk_widget_hide (GTK_WIDGET(widget));
	gtk_main_quit();
	return TRUE;
}
static gboolean editor_canceled;
static void ok_clicked (void)
{
	editor_canceled = FALSE;
	delete_event(window, NULL, NULL);
}
static void cancel_clicked (void)
{
	editor_canceled = TRUE;
	delete_event(window, NULL, NULL);
}
static void switch_page (GtkWidget *notebook, GtkNotebookPage *page,
				gint new_page)
{
	gint old_page = gtk_notebook_get_current_page (GTK_NOTEBOOK(notebook));

	if (!GTK_WIDGET_VISIBLE(window))
		return;
	prop_list[old_page].write(proc->simul_data);
	prop_list[new_page].read(proc);
	cur_property = &prop_list[new_page];
	if(cur_property->autofill==NULL)
		gtk_widget_set_sensitive(autofill_button, FALSE);
	else
		gtk_widget_set_sensitive(autofill_button, TRUE);
}
static void file_save(void)
{
	gint page = gtk_notebook_get_current_page (GTK_NOTEBOOK(notebook));
	prop_list[page].write(proc->simul_data);
	write_CPU_prop_file(proc);
}
static void file_load(void)
{
	gint page = gtk_notebook_get_current_page (GTK_NOTEBOOK(notebook));
	read_CPU_prop_file(proc);
	prop_list[page].read(proc);
}

static void auto_fill_params_callback(void)
{
	prop_io_params_t *params = &get_CPU_config()->prop_io_params;

	gtk_window_set_modal(GTK_WINDOW(window), FALSE);
	gtk_window_set_modal(GTK_WINDOW(auto_fill_widget), TRUE);
	gtk_widget_show(GTK_WIDGET(auto_fill_widget));
	if (GTK_WIDGET_VISIBLE(window))
		gtk_window_set_transient_for (GTK_WINDOW(auto_fill_widget),
						GTK_WINDOW(window));
	else
		gtk_window_set_transient_for (GTK_WINDOW(auto_fill_widget),
						GTK_WINDOW(get_CPU_window()));
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(average_blocks),
					params->avg_blocks);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(average_burst),
					params->avg_burst);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(average_create),
					params->avg_create);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(average_io_accesses),
					params->avg_io_accesses);

	gtk_main();
	
	params->avg_blocks = gtk_spin_button_get_value_as_float(
					GTK_SPIN_BUTTON(average_blocks));
	params->avg_burst = gtk_spin_button_get_value_as_float(
					GTK_SPIN_BUTTON(average_burst));
	params->avg_create = gtk_spin_button_get_value_as_float(
					GTK_SPIN_BUTTON(average_create));
	params->avg_io_accesses = gtk_spin_button_get_value_as_float(
					GTK_SPIN_BUTTON(average_io_accesses));
	gtk_window_set_modal(GTK_WINDOW(auto_fill_widget), FALSE);
	gtk_window_set_modal(GTK_WINDOW(window), TRUE);
}
static void auto_fill_callback(void)
{
	if(cur_property->autofill == NULL)
		return;
	cur_property->autofill(proc);
	cur_property->read(proc);
}
void autofill_params_ok(void)
{
	gtk_widget_hide(GTK_WIDGET(auto_fill_widget));
}
void init_CPU_prop_editor(GladeXML *main_xml)
{
	GtkWidget *prop;
	GladeXML  *xml_autofill;
	GladeXML  *xml;
	gint i;

	srand((guint) time(NULL));
	
	xml = glade_xml_new(get_xml_file(), "proc_properties");

	window = glade_xml_get_widget(xml, "proc_properties");
	g_return_if_fail(window != NULL);
	gtk_window_set_transient_for (GTK_WINDOW(window),
					GTK_WINDOW(get_CPU_window()));
	gtk_signal_connect (GTK_OBJECT (window), "delete_event",
			    GTK_SIGNAL_FUNC (delete_event), NULL);

	notebook = glade_xml_get_widget(xml, "proc_prop_notebook");
	gtk_signal_connect (GTK_OBJECT (notebook), "switch_page",
			    GTK_SIGNAL_FUNC (switch_page), NULL);

	for (i=0; prop_list[i].create != NULL; ++i){
		prop = prop_list[i].create(xml);
		if (prop->parent != NULL)
			continue;
		gtk_notebook_append_page(GTK_NOTEBOOK(notebook), prop,
				gtk_label_new(_(prop_list[i].name)));
		gtk_widget_show(GTK_WIDGET(prop));
	}

	/* make sure the we start on the first page and cur_property is
	 * set */
	cur_property = &prop_list[0];
	gtk_notebook_set_page(GTK_NOTEBOOK(notebook), 0);

	glade_xml_signal_connect(xml, "proc_prop_load",
			GTK_SIGNAL_FUNC(file_load));
	glade_xml_signal_connect(xml, "proc_prop_autofill",
			GTK_SIGNAL_FUNC(auto_fill_callback));
	autofill_button = glade_xml_get_widget(xml,
						"proc_prop_autofill_button");
	glade_xml_signal_connect(xml, "proc_prop_save",
			GTK_SIGNAL_FUNC(file_save));
	glade_xml_signal_connect(xml, "proc_prop_ok",
			GTK_SIGNAL_FUNC(ok_clicked));
	glade_xml_signal_connect(xml, "proc_prop_cancel",
				 GTK_SIGNAL_FUNC(cancel_clicked));
	
	glade_xml_signal_connect(main_xml, "proc_prop_edit_params",
			GTK_SIGNAL_FUNC(auto_fill_params_callback));
	glade_xml_signal_connect(xml, "proc_prop_edit_params",
			GTK_SIGNAL_FUNC(auto_fill_params_callback));

	xml_autofill = glade_xml_new(get_xml_file(), "autofill_params");
	glade_xml_signal_autoconnect(xml_autofill);
	auto_fill_widget = glade_xml_get_widget(xml_autofill, "autofill_params");
	average_blocks = glade_xml_get_widget(xml_autofill, "average_blocks");
	average_burst = glade_xml_get_widget(xml_autofill, "average_burst");
	average_create = glade_xml_get_widget(xml_autofill, "average_create");
	average_io_accesses = glade_xml_get_widget(xml_autofill, "average_io_accesses");
	
	parse_proc_init();
}
void auto_fill_process_properties(proc_t *proc)
{
	gint i;

	if (!proc->simul_data)
		proc->simul_data = g_new0(simul_data_t, 1);

	for (i=0; prop_list[i].name != NULL; i++)
		if(prop_list[i].autofill != NULL)
			prop_list[i].autofill(proc);

	fix_simulation_in_proc(proc);
}
gboolean edit_process_properties(proc_t *proc_to_edit)
{
	simul_data_t *data;
	gint page;

	proc = proc_to_edit;

	if (!proc->simul_data)
		auto_fill_process_properties(proc);
	data = proc->simul_data;
	cpy_CPU_simulation_data(&simul_data_backup, data);
	
	gtk_notebook_set_page(GTK_NOTEBOOK(notebook), 0);
	prop_list[0].read(proc);

	gtk_widget_popup(GTK_WIDGET(window), 10, 10);

	gtk_main();

	page = gtk_notebook_get_current_page (GTK_NOTEBOOK(notebook));
	prop_list[page].write(proc->simul_data);

	if(editor_canceled)
		cpy_CPU_simulation_data(proc->simul_data, &simul_data_backup);
	fix_simulation_in_proc(proc);
	proc = NULL;

	return editor_canceled;
}

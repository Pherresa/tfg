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
#include "config.h"
#endif
#include <glib.h>
#include <gnome.h>
#include <glade/glade.h>

#include <interface.h>

#include "cpu_config.h"

static struct {
	GtkSpinButton *avg_blocks;
	GtkSpinButton *avg_burst;
	GtkSpinButton *avg_create;
	GtkSpinButton *avg_io_accesses;
	GtkGammaCurve *mem_read;
	GtkGammaCurve *mem_write;
	GtkAdjustment *mem_n_pages;
	GtkSpinButton *max_graph_history;
	GtkSpinButton *pix_size_step;
} widgets;

static GtkWidget *pref_dialog = NULL;

static cpu_config_t cpu_config = {
	FALSE,		/* stop_clock */
	FALSE,		/* auto_fill_procs */
	{	/* prop_io_params */
		4,	/* avg_blocks */
		8, 	/* avg_burst */
		7,	/* avg_create */
		5, 	/* avg_io_accesses */
	}
};
/**
 * CPU_config:
 *
 * This is a pointer to the configuration data, but should be used only
 * for reading.
 */

const cpu_config_t *CPU_config = &cpu_config;

/**
 * get_CPU_config:
 *
 * This is the right way to modify the configuration.
 *
 * Returns: a writable pointer to the configuration data.
 */
cpu_config_t *get_CPU_config (void)
{
	return &cpu_config;
}
static void config_get_float_vector(const gchar *path, gint vector_size,
				    gfloat *vector, gfloat default_val)
{
	gint i, total_elements;
	gchar **elements;
	
	gnome_config_get_vector(path, &total_elements, &elements);
	if (total_elements == vector_size)
		for (i=0; i<vector_size; i++)
			vector[i] = atof(elements[i]);
	/* if things don't look good, give the default */
	else
		for (i=0; i<vector_size; i++)
			vector[i] = default_val;
	for (i=0; i<total_elements; i++)
		g_free(elements[i]);
	g_free(elements);
}
static void config_set_float_vector(const gchar *path, gint vector_size,
				    gfloat *vector)
{
	gint i;
	gchar *elements[vector_size];
	GString *str;

	for (i=0; i<vector_size; i++){
		str = g_string_new(NULL);
		g_string_sprintf(str, "%f", vector[i]);
		elements[i] = str->str;
		g_string_free(str, FALSE);
	}
	
	gnome_config_set_vector(path, vector_size, (const gchar **) elements);
	for (i=0; i<vector_size; i++)
		g_free(elements[i]);
}
static void apply_prefs(GtkWidget *widget, gint page_num, gpointer data)
{
	g_print("applying preferences\n");

	gnome_config_push_prefix ("/" PACKAGE "/CPU_Preferences/");

	cpu_config.prop_io_params.avg_blocks =
		gtk_spin_button_get_value_as_int(widgets.avg_blocks);
	gnome_config_set_int("avg_blocks",
			     cpu_config.prop_io_params.avg_blocks);

	cpu_config.prop_io_params.avg_burst =
		gtk_spin_button_get_value_as_int(widgets.avg_burst);
	gnome_config_set_int("avg_burst",
			     cpu_config.prop_io_params.avg_burst);

	cpu_config.prop_io_params.avg_create =
		gtk_spin_button_get_value_as_int(widgets.avg_create);
	gnome_config_set_int("avg_create",
			     cpu_config.prop_io_params.avg_create);

	cpu_config.prop_io_params.avg_io_accesses =
		gtk_spin_button_get_value_as_int(widgets.avg_io_accesses);
	gnome_config_set_int("avg_io_accesses",
			     cpu_config.prop_io_params.avg_io_accesses);

	gtk_curve_get_vector (GTK_CURVE(widgets.mem_read->curve), MAX_PAGES*10,
			      cpu_config.prop_mem_params.read_usage);
	config_set_float_vector("mem_read_usage", MAX_PAGES*10,
				cpu_config.prop_mem_params.read_usage); 

	gtk_curve_get_vector (GTK_CURVE(widgets.mem_write->curve), MAX_PAGES*10,
			      cpu_config.prop_mem_params.write_usage);
	config_set_float_vector("mem_write_usage", MAX_PAGES*10,
				cpu_config.prop_mem_params.write_usage); 

	cpu_config.prop_mem_params.n_pages = widgets.mem_n_pages->value;
	gnome_config_set_int("mem_num_pages",
			     cpu_config.prop_mem_params.n_pages);

	cpu_config.drawing.max_graph_history =
		gtk_spin_button_get_value_as_int(widgets.max_graph_history);
	gnome_config_set_int("max_graph_history",
			     cpu_config.drawing.max_graph_history);

	cpu_config.drawing.pix_size_step =
		gtk_spin_button_get_value_as_int(widgets.pix_size_step);
	gnome_config_set_int("pixel_size_step",
			     cpu_config.drawing.pix_size_step);

	gnome_config_pop_prefix();
	gnome_config_sync ();
}
static void load_prefs(void)
{

	gnome_config_push_prefix ("/" PACKAGE "/CPU_Preferences/");

	cpu_config.prop_io_params.avg_blocks =
		gnome_config_get_int("avg_blocks=4");
	gtk_spin_button_set_value(widgets.avg_blocks,
				  cpu_config.prop_io_params.avg_blocks);  

	cpu_config.prop_io_params.avg_burst =
		gnome_config_get_int("avg_burst=8");
	gtk_spin_button_set_value(widgets.avg_burst,
				  cpu_config.prop_io_params.avg_burst);  

	cpu_config.prop_io_params.avg_create =
		gnome_config_get_int("avg_create=7");
	gtk_spin_button_set_value(widgets.avg_create,
				  cpu_config.prop_io_params.avg_create);  

	cpu_config.prop_io_params.avg_io_accesses =
		gnome_config_get_int("avg_io_accesses=5");
	gtk_spin_button_set_value(widgets.avg_io_accesses,
				  cpu_config.prop_io_params.avg_io_accesses);  


	config_get_float_vector("mem_read_usage", MAX_PAGES*10,
				cpu_config.prop_mem_params.read_usage, 5); 
	gtk_curve_set_vector (GTK_CURVE(widgets.mem_read->curve), MAX_PAGES*10,
			      cpu_config.prop_mem_params.read_usage);

	config_get_float_vector("mem_write_usage", MAX_PAGES*10,
				cpu_config.prop_mem_params.write_usage, 5); 
	gtk_curve_set_vector (GTK_CURVE(widgets.mem_write->curve),
			      MAX_PAGES*10,
			      cpu_config.prop_mem_params.write_usage);

	cpu_config.prop_mem_params.n_pages =
		gnome_config_get_int("mem_num_pages=5");
	gtk_adjustment_set_value(widgets.mem_n_pages,
				 CPU_config->prop_mem_params.n_pages);


	cpu_config.drawing.max_graph_history =
		gnome_config_get_int("max_graph_history=300");
	gtk_spin_button_set_value(widgets.max_graph_history,
				  cpu_config.drawing.max_graph_history);  
	cpu_config.drawing.pix_size_step =
		gnome_config_get_int("pixel_size_step=5");
	gtk_spin_button_set_value(widgets.pix_size_step,
				  cpu_config.drawing.pix_size_step);  

	gnome_config_pop_prefix();
}
static void show_CPU_prefs(void)
{
	if(GTK_WIDGET_VISIBLE(pref_dialog))
		gdk_window_raise(GTK_WIDGET(pref_dialog)->window);
	else
		gtk_widget_show(GTK_WIDGET(pref_dialog));
}
static void pref_modified_cb (void)
{
	gnome_property_box_changed(GNOME_PROPERTY_BOX(pref_dialog));
}
static void fill_widgets(GladeXML *xml)
{
	widgets.avg_blocks = GTK_SPIN_BUTTON(
		glade_xml_get_widget(xml, "average_blocks"));
	widgets.avg_burst = GTK_SPIN_BUTTON(
		glade_xml_get_widget(xml, "average_burst"));
	widgets.avg_create = GTK_SPIN_BUTTON(
		glade_xml_get_widget(xml, "average_create"));
	widgets.avg_io_accesses = GTK_SPIN_BUTTON(
		glade_xml_get_widget(xml, "average_io_accesses"));
	widgets.mem_read = GTK_GAMMA_CURVE(
		glade_xml_get_widget(xml, "CPU_pref_mem_read"));
	widgets.mem_write = GTK_GAMMA_CURVE(
		glade_xml_get_widget(xml, "CPU_pref_mem_write"));
	widgets.mem_n_pages = GTK_ADJUSTMENT(GTK_RANGE(GTK_SCALE(
		glade_xml_get_widget(xml, "mem_n_pages")))
					     ->adjustment);
	widgets.max_graph_history = GTK_SPIN_BUTTON(
		glade_xml_get_widget(xml, "max_graph_history"));
	widgets.pix_size_step = GTK_SPIN_BUTTON(
		glade_xml_get_widget(xml, "pix_size_step"));
}

void init_CPU_config(GladeXML *main_xml)
{
	GladeXML *xml;

	xml = glade_xml_new(get_xml_file(), "CPU_preferences");
	pref_dialog = GTK_WIDGET(glade_xml_get_widget(xml, "CPU_preferences"));
	gnome_dialog_close_hides(GNOME_DIALOG(pref_dialog), TRUE);

	fill_widgets(xml);

	/* Hide the Gamma buttons as they are not very usefull */
	gtk_widget_hide(GTK_WIDGET(widgets.mem_read->button[3]));
	gtk_widget_hide(GTK_WIDGET(widgets.mem_write->button[3]));

	load_prefs();
	/* We connect signals after load_prefs to stop the Apply button
	   from being active */
	glade_xml_signal_connect(xml, "on_CPU_preferences_apply",
				 GTK_SIGNAL_FUNC(apply_prefs));
	glade_xml_signal_connect(xml, "on_CPU_property_changed",
				 GTK_SIGNAL_FUNC(pref_modified_cb));
	glade_xml_signal_connect(main_xml, "on_CPU_preferences_activate",
				 GTK_SIGNAL_FUNC(show_CPU_prefs));
}







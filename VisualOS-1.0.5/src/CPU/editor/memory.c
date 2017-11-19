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

#include <gnome.h>
#include <glade/glade.h>
#include <stdio.h>

#include <CPU/simulation.h>
#include <CPU/cpu_config.h>

#include "memory.h"
#include "util.h"

static GtkCList *list = NULL;
static GtkGammaCurve *read_curve = NULL;
static GtkGammaCurve *write_curve = NULL;
static GtkAdjustment *n_pages_adj = NULL;

static void load_defaults(void)
{

	/* reset the curve to something more meaning full for this case */
	gtk_curve_set_vector (GTK_CURVE(read_curve->curve), MAX_PAGES*10,
			      (gfloat *)
			      CPU_config->prop_mem_params.read_usage);
	gtk_curve_set_vector (GTK_CURVE(write_curve->curve), MAX_PAGES*10,
			      (gfloat *)
			      CPU_config->prop_mem_params.write_usage);
	gtk_adjustment_set_value(n_pages_adj,
				 CPU_config->prop_mem_params.n_pages);
}

GtkWidget *create_CPU_prop_memory_editor(GladeXML *xml)
{

	list = GTK_CLIST(glade_xml_get_widget(xml, "proc_prop_mem_list"));
	read_curve = GTK_GAMMA_CURVE(glade_xml_get_widget(xml,
					"proc_prop_mem_read_curve"));
	write_curve = GTK_GAMMA_CURVE(glade_xml_get_widget(xml,
					"proc_prop_mem_write_curve"));
	/* Hide the Gamma buttons, as they give trouble with modal windows*/
	gtk_widget_hide(GTK_WIDGET(read_curve->button[3]));
	gtk_widget_hide(GTK_WIDGET(write_curve->button[3]));

	n_pages_adj = GTK_ADJUSTMENT(GTK_RANGE(GTK_SCALE(
		glade_xml_get_widget(xml, "proc_prop_mem_num_pages")))
				     ->adjustment);
	glade_xml_signal_connect(xml, "on_proc_properties_show",
				 GTK_SIGNAL_FUNC(load_defaults));

	return glade_xml_get_widget(xml, "proc_prop_mem");
}
void write_CPU_prop_memory(simul_data_t *data)
{
}
void read_CPU_prop_memory(proc_t *proc)
{
	simul_data_t *data = proc->simul_data;
	GString *page = g_string_new("");
	GString *accesses = g_string_new("");
	gchar *text[2];
	gint i;

	gtk_clist_clear(list);
	g_return_if_fail (data->pages != NULL);

	for (i=0; i < data->n_pages; i++){
		g_string_sprintf(page, "%d", data->pages[i].page);
		if (data->pages[i].write)
			g_string_sprintf(accesses, _("WRITE"));
		else
			g_string_sprintf(accesses, _("READ"));
		text[0] = page->str;
		text[1] = accesses->str;
		gtk_clist_append(list, text);
	}
}
void autofill_CPU_prop_memory(proc_t *proc)
{
	simul_data_t *data = proc->simul_data;
	const gint values_per_page = 10;
	gint n_pages = n_pages_adj->value;
	gint n_values = n_pages*values_per_page;
	gfloat *curve_values[2];
	gfloat *values;
	gint i,j, sum, page_sum[2];

	/* we will average "values_per_page" values taken evenly for
	 * each page */
	curve_values[0] = g_new(gfloat, n_values);
	curve_values[1] = g_new(gfloat, n_values);
	values = g_new(gfloat, n_pages*2);
	
	/* get all curve values */
	gtk_curve_get_vector(GTK_CURVE(read_curve->curve), n_values,
				curve_values[0]);
	gtk_curve_get_vector(GTK_CURVE(write_curve->curve), n_values,
				curve_values[1]);
	/* arrange a vector for weighted random selection
	 * alternatively read and write values */
	page_sum[0] = page_sum[1] = 0;
	for (i=0, j=0, sum=0; i<n_values;++i){
		page_sum[0] += curve_values[0][i];
		page_sum[1] += curve_values[1][i];
		if((i%values_per_page) == values_per_page-1){
			sum += page_sum[0];
			values[j++] = sum;
			sum += page_sum[1];
			values[j++] = sum;
			page_sum[0] = page_sum[1] = 0;
		}
	}
	/* lets generate the page accesses */
	g_free(data->pages);
	data->n_pages = data->end_time;
	data->pages = g_new0(simul_mem_t, data->n_pages);
	
	for (i=0; i< data->n_pages; i++){
		gint val = my_rand(0,sum);
		for(j=0; j< n_pages*2; j++)
			if(val <= values[j])
				break;
		data->pages[i].page=j/2;
		data->pages[i].write=j%2;
	}
	g_free(curve_values[0]);
	g_free(curve_values[1]);
	g_free(values);
}






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

#include <CPU/simulation.h>

#include "advanced.h"
#include "parser.h"

static GtkWidget *text;
GtkWidget *create_CPU_prop_advanced_editor(GladeXML *xml)
{
	text = glade_xml_get_widget(xml, "proc_prop_advanced_text");
	return glade_xml_get_widget(xml, "proc_prop_advanced");
}
void write_CPU_prop_advanced(simul_data_t *data)
{
	simul_data_t *backup = g_new0(simul_data_t,1 );

	gchar *str = gtk_editable_get_chars(GTK_EDITABLE(text), 0, -1);

	cpy_CPU_simulation_data(backup, data);
	if(get_simulation_from_string (data, str) == NULL){
		g_print("error parsing data, restoring the backup\n");
		cpy_CPU_simulation_data(data, backup);
	}
	g_free(backup);
}
void read_CPU_prop_advanced(proc_t *proc)
{
	GString *string;
	gint pos=0;
	
	gtk_editable_delete_text(GTK_EDITABLE(text), 0, -1);
	string = get_proc_in_gstring(proc);
	gtk_editable_insert_text (GTK_EDITABLE(text),
				string->str, string->len, &pos);
	g_string_free (string, TRUE);
}

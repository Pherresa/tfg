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

#include "combo.h"
#include "info.h"

static GtkWidget *algorithm_box = NULL;

static void algorithm_select (GtkWidget *item, mem_algorithm_t *algorithm)
{
	mem_algorithm_t *current;
	static gint repeat=0;
	static GtkWidget *properties;

	/* curiosusly each signal calls this function twice so I have to
	 * filter out one of them */
	repeat = !repeat;
	if (repeat)
		return;
	
	current = get_MEM_current_algorithm();
	if (current != NULL){
		current->unselect();
		if (current->properties != NULL)
			gtk_widget_destroy(GTK_WIDGET(current->properties));
		current->properties = NULL;
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

	set_MEM_current_algorithm (algorithm);
}

void init_MEM_algorithm_combo(GladeXML *xml, GSList *algorithm_list) 
{
	GtkWidget * item;
	GtkWidget * list;

	algorithm_box = glade_xml_get_widget(xml, "MEM_algorithm_box");
	list = GTK_COMBO(
		glade_xml_get_widget(xml, "MEM_algorithm_combo"))->list;
	g_return_if_fail(GTK_IS_LIST(list));

	while (algorithm_list != NULL){
		mem_algorithm_t *algorithm;
		algorithm = algorithm_list->data;
		item = gtk_list_item_new_with_label (algorithm->name);
		gtk_widget_show (GTK_WIDGET(item));
		gtk_signal_connect(GTK_OBJECT(item), "select", 
				GTK_SIGNAL_FUNC(algorithm_select), 
				algorithm);
		gtk_container_add (GTK_CONTAINER (list), item);
		algorithm_list = algorithm_list->next;
	}
	return;
}


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
#include <math.h>
#include <stdlib.h>

#include <property_widgets.h>


static void property_notify (GtkAdjustment *adj, properties_t *properties)
{
	properties->notify();
}
static void property_changed(GtkAdjustment *adj, gfloat *value)
{
	*value = adj->value;
}
/**
 * properties_create:
 * @property: properties descriptions.
 * @num_properties: number of elemente in @property.
 * @notify: to be called when the properties change.
 *
 * Creates a GtkWidget for the properties described in @property.
 *
 * Returns: a pointer which identifies the newly created #properties_t.
 */
properties_t *properties_create(const property_t property[], 
				const gint num_properties,
				void (*notify)(void))
{
	gint i;
	GtkWidget *box;
	properties_t *properties;
	
	properties = g_new(properties_t, 1);
	properties->num_properties = num_properties;
	properties->notify = notify;
	properties->entry = g_new(GtkWidget *, num_properties);
	properties->value = g_new(gfloat, num_properties);
	properties->widget = gtk_vbox_new(FALSE, 0);
	for (i=0; i<num_properties; ++i){
		GtkWidget *hbox;
		GtkWidget *label;
		GtkWidget *entry;
		GtkAdjustment *adj;
		gdouble dummy;
		gint num_decimals;

		if (modf (property[i].step, &dummy) == 0)
			num_decimals = 0;
		else
			num_decimals = 1;
				
		
		hbox = gtk_hbox_new(FALSE, 0);
		gtk_widget_show(GTK_WIDGET(hbox));

		label = gtk_label_new(_(property[i].label));
		gtk_widget_show(GTK_WIDGET(label));
		gtk_box_pack_start (GTK_BOX(hbox), label, 
				TRUE, FALSE, 0);
/*		entry = gtk_entry_new_with_max_length(3);
		gtk_entry_set_text(GTK_ENTRY(entry), "0");*/
		adj = (GtkAdjustment *)gtk_adjustment_new(0, 
				property[i].min, property[i].max, 
				property[i].step,
				5.0, 0.0);
		entry = gtk_spin_button_new (adj, 1.0, num_decimals);

		gtk_signal_connect(GTK_OBJECT(adj),"value_changed",
				GTK_SIGNAL_FUNC(property_changed),
				&(properties->value[i]));
		gtk_signal_connect(GTK_OBJECT(adj),"value_changed",
				GTK_SIGNAL_FUNC(property_notify),
				(gpointer)properties);
		properties->entry[i] = entry;
		gtk_box_pack_start (GTK_BOX(hbox), entry,
				TRUE, FALSE, 0);
		gtk_widget_show(GTK_WIDGET(entry));

		gtk_box_pack_start (GTK_BOX(properties->widget), hbox,
				TRUE, FALSE, 0);
	}
	box = gtk_hbox_new(FALSE, 0);
	gtk_widget_show(GTK_WIDGET(box));

	gtk_box_pack_start (GTK_BOX(properties->widget), box,
			TRUE, FALSE, 5);
	return properties;
}	
/**
 * properties_destroy:
 * @properties: a properties pointer obtained with #properties_create.
 *
 * Destroys the properties widget, freeing all it's memory.
 *
 * Returns: nothing interesting.
 */
gint properties_destroy(properties_t *properties)
{
	gtk_widget_destroy(GTK_WIDGET(properties->widget));
	if (properties->num_properties > 0){
		g_free (properties->entry);
		g_free (properties->value);
	}
	g_free (properties);
	return 0;
}
/**
 * properties_get_widget:
 * @properties: a properties pointer obtained with #properties_create.
 *
 * Retrives the widget created to hold all the properties.
 * 
 * Returns: A pointer to the widget.
 */
GtkWidget *properties_get_widget(const properties_t *properties)
{
	return properties->widget;
}
/**
 * properties_get_values:
 * @properties: a properties pointer obtained with #properties_create.
 *
 * Retrives the current values of all the properties representied in
 * @properties.
 *
 * Returns: An array with the values.
 */
gfloat *properties_get_values(const properties_t *properties)
{
	return properties->value;
}
/**
 * properties_set_values:
 * @properties: a properties pointer obtained with #properties_create.
 * @value: and array with the values.
 *
 * Sets the values of all the properties stored in @properties.
 *
 * Returns: nothing interesting.
 */
gint properties_set_values(properties_t *properties, const gfloat value[])
{
	gint i;
	for (i=0; i < properties->num_properties; ++i){
		gtk_adjustment_set_value(
				GTK_ADJUSTMENT(GTK_SPIN_BUTTON(
					properties->entry[i])->adjustment),
				value[i]);
		properties->value[i] = value[i];
	}
	return 0;
}

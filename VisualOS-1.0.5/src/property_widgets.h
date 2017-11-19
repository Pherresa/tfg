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

#ifndef PROPERTY_WIDGETS_H
#define PROPERTY_WIDGETS_H

#include <gtk/gtk.h>

typedef struct {
	void (*notify)(void);	/* will be called when ever a properties
				 * change.*/
	GtkWidget *widget;	/* is the main widget.*/
	gint num_properties;	/* number of entry and value elements */
	GtkWidget **entry;	/* is an array of all entry widgets */
	gfloat *value;		/* is an array of all property values */
} properties_t;

typedef struct {
	gchar *label;	/* Some text to identify de value */
	gfloat min;	/* The minimun value */
	gfloat max;	/* The maximun value */
	gfloat step;	/* Value increments allowed */
} property_t;
properties_t *properties_create(const property_t property[], 
				const gint num_properties,
				void (*notify)(void));
gint properties_destroy(properties_t *properties);
GtkWidget *properties_get_widget(const properties_t *properties);
gfloat *properties_get_values(const properties_t *properties);
gint properties_set_values(properties_t *properties,
		const gfloat value[]);

#endif

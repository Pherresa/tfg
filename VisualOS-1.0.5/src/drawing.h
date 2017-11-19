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

#ifndef DRAWING_H
#define DRAWING_H
#include <gtk/gtk.h>
#include <gnome.h>

typedef enum {
	DRAWING_FIXED_SIZE=1,		/* the widget will not change it's
					   size scrollbars should be used */
	DRAWING_FIXED_RATIO=1<<1	/* the widget will change it's size
					   if needed but wants to keep it's
					   width/height ratio */
} drawing_flags_t;


typedef struct {				/* descrives a drawing style */
	GtkWidget *widget;			/* widget to be shown */
	gchar *name;				/* name for this style */
	void (*update)(GtkWidget *widget);	/* function to update the
						   style */
	drawing_flags_t flags;			/* one of drawing_flags_t */
} drawing_style_t;

GtkWidget *create_drawing(void);
void register_drawing_style(GtkWidget *drawing, drawing_style_t *style);
void update_drawing(GtkWidget *drawing);
#endif

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

#ifndef GDK_HELPER_H
#include <gtk/gtk.h>
#define GDK_HELPER_H

GdkGC *new_gdk_GC_with_color(guint8 red, guint8 green, guint8 blue);
void resize_gdk_pixmap(GdkPixmap **pixmap, gint new_width, gint new_height, GdkGC *fill);
void enlarge_gdk_pixmap(GdkPixmap **pixmap, gint new_width, gint new_height, GdkGC *fill);

void draw_gdk_text_centered (GdkDrawable *drawable, GdkFont *font, GdkGC *gc,
				gint x, gint y, gint width, gint height,
				const gchar *text, gint text_length);
void fill_gdk_window(GdkPixmap *pixmap, GdkGC *fill);
#endif

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
#include <gtk/gtk.h>
#include <stdio.h>

#include <drawing.h>
#include <gdk-helper.h>
#include <IO/info.h>
#include <IO/simulation.h>
#include <IO/geometry.h>

#include "new.h"

static GdkPixmap *pixmap = NULL;
static GdkGC *white_gc = NULL;
static GdkGC *default_gc = NULL;
static GdkFont *font = NULL;

static void redraw (GtkWidget *widget);

static drawing_style_t drawing_style= {
	NULL,
	N_("route"),
	redraw,
	0
};

static gint expose_event(GtkWidget * widget, GdkEventExpose * event)
{
	gdk_draw_pixmap(widget->window,
			widget->style->fg_gc[GTK_WIDGET_STATE(widget)],
			pixmap,
			event->area.x, event->area.y,
			event->area.x, event->area.y,
			event->area.width, event->area.height);

	return FALSE;
}
static gfloat track_width;
static gint track_pos;
static gfloat step;
static gint height;
static void draw_request(io_request_t *request, GtkWidget *drawing)
{
	gint track = request->track;

	gdk_draw_line(pixmap,
		      default_gc,
		      track_pos, height,
		      track*track_width, height+step);
	gdk_draw_rectangle(pixmap,
			   default_gc,
			   TRUE,
			   track*track_width-1,
			   height+step-1,
			   3, 3);
	height += step;
	track_pos = track*track_width;
}
static void redraw (GtkWidget *widget)
{
	io_queue_t requests = get_IO_reading_queue();

	/* calculate the width of a track so that all track fit between the
	 * two circles */
	track_width = (widget->allocation.width)/(gfloat)get_IO_ntracks();
	track_pos = track_width * get_IO_head_pos();
	step = widget->allocation.height / (gfloat)io_queue_len(requests);
	if (step > track_width)
		step = track_width;
	height = 0;

	/*clear the pixmap*/
	gdk_draw_rectangle(pixmap,
			   white_gc,
			   TRUE,
			   0, 0,
			   widget->allocation.width,
			   widget->allocation.height);
	/* put a line for each request */
	io_queue_foreach( get_IO_reading_queue(),
			draw_request,
			widget );

	if(!GTK_WIDGET_REALIZED(widget))
		return;
	/* dump the pixmap into the drawing area */
	gdk_draw_pixmap(widget->window,
			widget->style->fg_gc[GTK_WIDGET_STATE(widget)],
			pixmap,
			0, 0,
			0, 0,
			widget->allocation.width, 
			widget->allocation.height);
	return;
}

static gint configure_event(GtkWidget * widget, GdkEventConfigure * event)
{
	resize_gdk_pixmap(&pixmap, widget->allocation.width,
			  widget->allocation.height, white_gc);
	redraw(widget);

	return TRUE;
}
static void setup_resources (void)
{
	GdkGCValues values;
	GdkColormap *colormap=gdk_colormap_get_system();

	gdk_color_black (colormap, &values.foreground);
	default_gc = gdk_gc_new_with_values (pixmap, &values,
						GDK_GC_FOREGROUND);
	gdk_color_white(colormap, &values.foreground);
	white_gc = gdk_gc_new_with_values (pixmap, &values,
						GDK_GC_FOREGROUND);
	font = gdk_font_load ( "-*-*-bold-*-*-*-14-*-*-*-*-*-iso8859-1");
	if (font == NULL)
		font = gdk_font_load ( "-*-*-*-*-*-*-14-*-*-*-*-*-iso8859-1");
	return;
}

void init_IO_drawing_new(GtkWidget *drawing)
{
	GtkWidget *widget;
	widget = gtk_drawing_area_new();
	gtk_drawing_area_size(GTK_DRAWING_AREA(widget), 10, 10);

	pixmap = gdk_pixmap_new(NULL, 1, 1,
				gdk_visual_get_system()->depth);
	setup_resources();
	gtk_signal_connect(GTK_OBJECT(widget), "expose_event",
			   (GtkSignalFunc) expose_event, NULL);
	gtk_signal_connect(GTK_OBJECT(widget), "configure_event",
			   (GtkSignalFunc) configure_event, NULL);
	drawing_style.widget=widget;
	register_drawing_style(drawing, &drawing_style);
}

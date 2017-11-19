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

#include "round.h"

static GdkPixmap *pixmap = NULL;
static GdkGC *white_gc = NULL;
static GdkGC *default_gc = NULL;
static GdkGC *dark_gc = NULL;
static GdkGC *mid_gc = NULL;
static GdkFont *font = NULL;

static void redraw (GtkWidget *widget);

static drawing_style_t drawing_style= {
	NULL,
	N_("round"),
	redraw,
	0,/*DRAWING_FIXED_RATIO*/
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
static void draw_request(io_request_t *request, GtkWidget *drawing)
{
	gint track = request->track;

	gdk_draw_rectangle(pixmap,
			   default_gc,
			   TRUE,
			   track*track_width,
			   drawing->allocation.height/2,
			   2,
			   2);
	
}
static gint draw_IO_drawing(GtkWidget *widget)
{
	gint track_pos;
	gchar buff[30];
	gint ntracks = get_IO_ntracks();
	gint last_data_track = get_IO_last_data_track();
	gint disk_diameter = widget->allocation.width*20/7;
	gint disk_offset = (widget->allocation.height - disk_diameter)/2;
	gint disk_diameter2 = disk_diameter/3;

	/* calculate the width of a track so that all track fit between the
	 * two circles */
	track_width = (disk_diameter-disk_diameter2)/2.0 /ntracks;
	track_pos = track_width * get_IO_head_pos();

	/*clear the pixmap*/
	gdk_draw_rectangle(pixmap,
			   white_gc,
			   TRUE,
			   0, 0,
			   widget->allocation.width,
			   widget->allocation.height);
	/* put the disk */
	gdk_draw_arc (pixmap,
		       dark_gc,
		       TRUE,
		       0,
		       disk_offset,
		       disk_diameter,
		       disk_diameter,
		       0*64,
		       360*64);
	gdk_draw_arc (pixmap,
		       mid_gc,
		       TRUE,
		       (last_data_track+1)*track_width,
		       (last_data_track+1)*track_width + disk_offset,
		       disk_diameter - (last_data_track)*track_width*2,
		       disk_diameter - (last_data_track)*track_width*2,
		       0*64,
		       360*64);
	gdk_draw_arc (pixmap,
		       white_gc,
		       TRUE,
		       disk_diameter/2-disk_diameter2/2,
		       disk_diameter/2-disk_diameter2/2 +disk_offset,
		       disk_diameter2,
		       disk_diameter2,
		       0*64,
		       360*64);
	/* write the current head position */
	gdk_draw_text(pixmap,
		font, default_gc,
		1,13,
		buff, sprintf(buff,_("Track: %d"),get_IO_head_pos()));
	/* put a dot for each request */
	io_queue_foreach( get_IO_reading_queue(),
			draw_request,
			widget );
	/* put the head */
	gdk_draw_line(pixmap,
			default_gc,
			(disk_diameter-disk_diameter2)/4,
			disk_diameter + disk_offset,
			track_pos,
			disk_diameter/2 + disk_offset);
	return 0;
}

static void redraw (GtkWidget *widget)
{
	draw_IO_drawing(widget);
	if(!GTK_WIDGET_REALIZED(widget))
		return;
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

	draw_IO_drawing(widget);

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
	
	dark_gc = new_gdk_GC_with_color(142, 142, 161);
	mid_gc = new_gdk_GC_with_color(199, 199, 208);

 	font = gdk_font_load ( "-*-*-bold-*-*-*-14-*-*-*-*-*-iso8859-1");
 	if (font == NULL)
 		font = gdk_font_load ( "-*-*-*-*-*-*-14-*-*-*-*-*-iso8859-1");
 	return;
}
void init_IO_drawing_round(GtkWidget *drawing)
{
	GtkWidget *widget;
	widget = gtk_drawing_area_new();
	gtk_drawing_area_size(GTK_DRAWING_AREA(widget), 100, 100);

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

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
#include <events.h>
#include <MEM/info.h>
#include <MEM/page_info.h>

#include "physical.h"

static GdkPixmap *pixmap = NULL;
static GdkGC *default_gc = NULL;
static GdkGC *green_gc = NULL;
static GdkGC *red_gc = NULL;
static GdkGC *white_gc = NULL;
static GdkFont *font = NULL;
static GtkWidget *widget = NULL;

static void redraw (GtkWidget *widget);

static drawing_style_t drawing_style= {
	NULL,
	N_("Physical"),
	redraw,
	DRAWING_FIXED_SIZE
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
#define TABLE_WIDTH 60
#define ROW_HEIGHT 60
#define HSPACE 5
#define VSPACE 5
static gint table_width=TABLE_WIDTH;
static gint row_height=ROW_HEIGHT;
static gint hspace = HSPACE, vspace = VSPACE;
static gint min_width= TABLE_WIDTH + HSPACE*2;
static gint n_cols=1;

static void draw_frame(frame_info_t *frame, gint x, gint y)
{
	GString *str = g_string_new(NULL);
	GdkGC *gc;

	g_return_if_fail(frame != NULL);

	gc = FRAME_MODIFIED(frame) ? default_gc : green_gc;
	if (frame->proc == NO_PROC)
		gc = red_gc;


	gdk_draw_rectangle(pixmap, default_gc, FALSE,
			   x, y,
			   table_width-1, row_height/3);
	g_string_sprintf(str, _("frame:%d"), frame->frame);
	draw_gdk_text_centered (pixmap, font, default_gc, x, y,
			table_width, row_height/3,
			str->str, str->len);
	y += row_height/3;
	gdk_draw_rectangle(pixmap, default_gc, FALSE,
			   x, y,
			   table_width-1, row_height/3);
	if (frame->proc == NO_PROC)
		g_string_sprintf(str, _("pid:-"));
	else
		g_string_sprintf(str, _("pid:%d"), frame->proc);
	draw_gdk_text_centered (pixmap, font, gc, x, y,
			table_width, row_height/3,
			str->str, str->len);
	y += row_height/3;
	gdk_draw_rectangle(pixmap, default_gc, FALSE,
			   x, y,
			   table_width-1, row_height/3);
	if (frame->page == NO_PAGE)
		g_string_sprintf(str, _("page:-"));
	else
		g_string_sprintf(str, _("page:%d"), frame->page);
	draw_gdk_text_centered (pixmap, font, gc, x, y,
			table_width, row_height/3,
			str->str, str->len);
}
static void draw_frames(GtkWidget *widget)
{
	frame_info_t *frame = get_frames_list();
	gint x=hspace, y=vspace;
	gint max_x=x, max_y=y;
	gint col=0;

	g_return_if_fail(n_cols > 0);

	fill_gdk_window(pixmap, white_gc);

	while (frame){
		max_y = y + row_height;
		max_x = n_cols*(table_width+hspace) + hspace*2;
		enlarge_gdk_pixmap(&pixmap, max_x+1, max_y+1, white_gc);

		draw_frame(frame, x, y);

		if (++col >= n_cols){
			col = 0;
			y = max_y + vspace;
			x = hspace;
		} else {
			x += table_width + hspace;
		}
		frame = mem_frames_next(frame);
	}
	resize_gdk_pixmap(&pixmap, max_x+1, max_y+1, white_gc);
	gtk_drawing_area_size(GTK_DRAWING_AREA(widget), min_width, max_y+1);
}
static void size_allocate (GtkWidget *widget, GtkAllocation *allocation)
{
	gint old_n_cols = n_cols;

	n_cols = allocation->width/(table_width+hspace);
	if (n_cols != old_n_cols)
		redraw(widget);
}
static void redraw (GtkWidget *widget)
{
	draw_frames(widget);
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
}
static gint button_press_event(GtkWidget *widget, GdkEventButton *event)
{
	gint frame = NO_FRAME;
	gint col = event->x / (hspace + table_width);
	gint row = event->y / (vspace + row_height);

	if (event->button != 1)
		return TRUE;

	frame = row*n_cols + col;
	system_event(SYS_EVENT_FRAME_SELECT, GINT_TO_POINTER(frame));

	return TRUE;
}
static void realize (GtkWidget *widget)
{
	GdkGCValues values;
	
	gdk_gc_get_values(white_gc, &values);
	gdk_window_set_background(widget->window, &values.foreground);
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
	green_gc = new_gdk_GC_with_color(34, 139, 34);
	red_gc = new_gdk_GC_with_color(255, 0, 0);
	font = gdk_font_load ( "-*-*-bold-*-*-*-14-*-*-*-*-*-iso8859-1");
	if (font == NULL)
		font = gdk_font_load ( "-*-*-*-*-*-*-14-*-*-*-*-*-iso8859-1");

	table_width=gdk_string_width(font, _("frame:000"));
	return;
}
void init_MEM_drawing_physical(GtkWidget *drawing)
{
	widget = gtk_drawing_area_new();
	gtk_drawing_area_size(GTK_DRAWING_AREA(widget), min_width, 10);

	gtk_widget_set_events (widget, GDK_EXPOSURE_MASK
			       | GDK_BUTTON_PRESS_MASK);
	
	pixmap = gdk_pixmap_new(NULL, 1, 1,
			gdk_visual_get_system()->depth);
	setup_resources();
	gtk_signal_connect(GTK_OBJECT(widget), "realize",
			   (GtkSignalFunc) realize, NULL);
	gtk_signal_connect(GTK_OBJECT(widget), "size-allocate",
			   (GtkSignalFunc) size_allocate, NULL);
	gtk_signal_connect(GTK_OBJECT(widget), "expose_event",
			   (GtkSignalFunc) expose_event, NULL);
	gtk_signal_connect(GTK_OBJECT (widget), "button_press_event",
			   (GtkSignalFunc) button_press_event, NULL);
	drawing_style.widget=widget;
	register_drawing_style(drawing, &drawing_style);
}


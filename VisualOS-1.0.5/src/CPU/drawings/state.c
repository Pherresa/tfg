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

#include <CPU/queues.h>
#include <CPU/info.h>
#include <drawing.h>
#include <process.h>
#include <events.h>

#include "shared.h"
#include "CPU.xpm"

static GdkPixmap *pixmap = NULL;
static GdkPixmap *CPU_pixmap = NULL;
static GtkWidget *widget = NULL;

static void redraw (GtkWidget *widget);
void init_CPU_drawing_state(GtkWidget *drawing);

static drawing_style_t drawing_style= {
	NULL,
	N_("State"),
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
		
static gint total_width=305, total_height=215;
static gint proc_width=90, proc_height=50;

/*Defined but not used
static gint hspace=10, vspace=10;
*/

static gint running_x=210, running_y=20;

static gint ready_x=150, ready_y=80;
static gint waiting_x=150, waiting_y=160;
static gint ready_susp_x=20, ready_susp_y=80;
static gint waiting_susp_x=20, waiting_susp_y=160;

static proc_t *proc=NULL;
static void draw_CPU_proc(proc_t *proc, GtkWidget *drawing,
		gint x, gint y,
		gint width, gint height)
{
	static GdkFont *pid_font=NULL;
	static GdkFont *burst_font;
	static gint pid_height;
	static gint burst_height;
	static gint margin = 4;
	char buff[10];

	if (pid_font == NULL){
		pid_font = gdk_font_load (
				"-*-*-bold-*-*-*-24-*-*-*-*-*-iso8859-1");
		if (pid_font == NULL)
			pid_font = gdk_font_load (
				"-*-*-*-*-*-*-24-*-*-*-*-*-iso8859-1");
		pid_height = gdk_string_height (pid_font, "0123456789");
		burst_font = gdk_font_load (
				"-*-*-normal-i-*-*-20-*-*-*-*-*-iso8859-1");
		if (burst_font == NULL)
			burst_font = gdk_font_load (
				"-*-*-*-*-*-*-20-*-*-*-*-*-iso8859-1");

		burst_height = gdk_string_height (burst_font, "0123456789");
	}
	if(proc==NULL)
		return;
	/*write the process' pid*/
	y += margin + pid_height;
	sprintf(buff,"%d",proc->pid);
	gdk_draw_string(pixmap,
		pid_font, drawing->style->black_gc,
		x + width/3, y,
		buff);
	/*write the process' burst*/
	y += margin + burst_height;
	sprintf(buff,"%d",proc->next_event.time- proc->time);
	gdk_draw_string(pixmap,
		burst_font, drawing->style->black_gc,
		x + width/3, y ,
		buff);
}
static void gdk_draw_varrow (GdkDrawable  *drawable, GdkGC *gc,
				gint x1, gint y1,
				gint x2, gint y2)
{
	gint len=10;
	gint width=3;

	if (y2 < y1)
		len = -len;

	gdk_draw_line (drawable, gc, x1, y1, x2, y2);

	gdk_draw_line (drawable, gc, x2 -width, y2 -len, x2, y2);
	gdk_draw_line (drawable, gc, x2 +width, y2 -len, x2, y2);
}

static void gdk_draw_harrow (GdkDrawable  *drawable, GdkGC *gc,
				gint x1, gint y1,
				gint x2, gint y2)
{
	gint len=10;
	gint width=3;

	if (x2 < x1)
		len = -len;

	gdk_draw_line (drawable, gc, x1, y1, x2, y2);

	gdk_draw_line (drawable, gc, x2 -len, y2 -width, x2, y2);
	gdk_draw_line (drawable, gc, x2 -len, y2 +width, x2, y2);
}

static void gdk_draw_arrow (GdkDrawable  *drawable, GdkGC *gc,
				gint x1, gint y1,
				gint x2, gint y2,
				gint width1, gint width2)
{
	gint len=10;

	if (y2 < y1)
		len = -len;

	gdk_draw_line (drawable, gc, x1, y1, x2, y2);

	gdk_draw_line (drawable, gc, x2 -width1, y2 -len, x2, y2);
	gdk_draw_line (drawable, gc, x2 +width2, y2 -len, x2, y2);
}

static void draw_skeleton(GtkWidget *widget)
{
	gdk_draw_arc(pixmap, cpu_ready_gc, TRUE,
			ready_x, ready_y, proc_width, proc_height,
			0, 360*64);
	gdk_draw_arc(pixmap, widget->style->black_gc, FALSE,
			ready_x, ready_y, proc_width, proc_height,
			0, 360*64);
	
	gdk_draw_arc(pixmap, cpu_running_gc, TRUE,
			running_x, running_y, proc_width, proc_height,
			0, 360*64);
	gdk_draw_arc(pixmap, widget->style->black_gc, FALSE,
			running_x, running_y, proc_width, proc_height,
			0, 360*64);
	
	gdk_draw_arc(pixmap, cpu_waiting_gc, TRUE,
			waiting_x, waiting_y, proc_width, proc_height,
			0, 360*64);
	gdk_draw_arc(pixmap, widget->style->black_gc, FALSE,
			waiting_x, waiting_y, proc_width, proc_height,
			0, 360*64);

	gdk_draw_arc(pixmap, cpu_waiting_susp_gc, TRUE,
		waiting_susp_x, waiting_susp_y, proc_width, proc_height,
			0, 360*64);
	gdk_draw_arc(pixmap, widget->style->black_gc, FALSE,
		waiting_susp_x, waiting_susp_y, proc_width, proc_height,
			0, 360*64);

	gdk_draw_arc(pixmap, cpu_ready_susp_gc, TRUE,
			ready_susp_x, ready_susp_y, proc_width, proc_height,
			0, 360*64);
	gdk_draw_arc(pixmap, widget->style->black_gc, FALSE,
			ready_susp_x, ready_susp_y, proc_width, proc_height,
			0, 360*64);
	gdk_draw_varrow(pixmap, widget->style->black_gc,
			waiting_x+proc_width/2, waiting_y,
			ready_x+proc_width/2, ready_y+proc_height);
	gdk_draw_varrow(pixmap, widget->style->black_gc,
			waiting_susp_x+proc_width/2, waiting_susp_y,
			ready_susp_x+proc_width/2, ready_susp_y+proc_height);
	gdk_draw_harrow(pixmap, widget->style->black_gc,
			ready_susp_x+proc_width, ready_susp_y+proc_height/2,
			ready_x, ready_y+proc_height/2);
	gdk_draw_harrow(pixmap, widget->style->black_gc,
			ready_x, ready_y+proc_height/2,
			ready_susp_x+proc_width, ready_susp_y+proc_height/2);
	gdk_draw_harrow(pixmap, widget->style->black_gc,
		waiting_susp_x+proc_width, waiting_susp_y+proc_height/2,
		waiting_x, waiting_y+proc_height/2);
	gdk_draw_harrow(pixmap, widget->style->black_gc,
		waiting_x, waiting_y+proc_height/2,
		waiting_susp_x+proc_width, waiting_susp_y+proc_height/2);

	gdk_draw_arrow(pixmap, widget->style->black_gc,
			ready_x+proc_width/2, ready_y,
			running_x, running_y+proc_height/2,
			8, 0);
	gdk_draw_arrow(pixmap, widget->style->black_gc,
			running_x+proc_width/2-5, running_y+proc_height,
			ready_x+proc_width, ready_y+proc_height/2,
			0, 6);
	gdk_draw_arrow(pixmap, widget->style->black_gc,
			running_x+proc_width/2, running_y+proc_height,
			waiting_x+proc_width, waiting_y+proc_height/2,
			1, 5);
}
static void draw_CPU_state(GtkWidget *widget)
{
	gint x, y;

	draw_skeleton(widget);

	if (proc == NULL)
		return;

	switch (proc->nqueue){
		case CPU_CURRENT:
			x=running_x;
			y=running_y;
			break;
		case CPU_WAITING:
			x=waiting_x;
			y=waiting_y;
			break;
		case CPU_NO_QUEUE:
			return;
		default:
			x=ready_x;
			y=ready_y;
	}
	draw_CPU_proc(proc, widget, x+6, y, proc_width, proc_height);
	return;
}
static void selected_proc_event_handler(sys_event_t type, proc_t *new_proc)
{
	proc=new_proc;
}
static void redraw (GtkWidget *widget)
{
	draw_CPU_state(widget);
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
	GdkBitmap *mask;
	if (CPU_pixmap == NULL)
		CPU_pixmap = gdk_pixmap_create_from_xpm_d (widget->window,
				&mask,
				&widget->style->bg[GTK_STATE_NORMAL],
				CPU_xpm);
	if (pixmap)
		gdk_pixmap_unref(pixmap);

	pixmap = gdk_pixmap_new(widget->window,
				widget->allocation.width,
				widget->allocation.height,
				-1);
	gdk_draw_rectangle(pixmap, widget->style->white_gc, TRUE,
				0, 0,
				widget->allocation.width,
				widget->allocation.height);

	draw_skeleton(widget);
	draw_CPU_state(widget);

	return TRUE;
}
void init_CPU_drawing_state(GtkWidget *drawing)
{
	widget = gtk_drawing_area_new();
	gtk_drawing_area_size(GTK_DRAWING_AREA(widget),
				total_width, total_height);
	
	gtk_widget_set_events (widget, GDK_EXPOSURE_MASK
			| GDK_BUTTON_PRESS_MASK);

	gtk_signal_connect(GTK_OBJECT(widget), "expose_event",
			   (GtkSignalFunc) expose_event, NULL);
	gtk_signal_connect(GTK_OBJECT(widget), "configure_event",
			   (GtkSignalFunc) configure_event, NULL);
	drawing_style.widget=widget;
	register_drawing_style (drawing, &drawing_style);
	system_event_receive(SYS_EVENT_PROC_SELECT,
			(sys_event_callback *)selected_proc_event_handler);
}

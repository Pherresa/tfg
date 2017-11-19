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

#include <CLOCK/CLOCK.h>
#include <CPU/queues.h>
#include <CPU/info.h>
#include <drawing.h>
#include <gdk-helper.h>
#include <events.h>
#include <process.h>
#include <CPU/cpu_config.h>

static GdkPixmap *pixmap = NULL;
static GdkGC *default_gc = NULL;
static GdkGC *white_gc = NULL;

static void redraw (GtkWidget *widget);
void init_CPU_drawing_overlapped_bars(GtkWidget *drawing);

static drawing_style_t drawing_style= {
	NULL,
	N_("Overlapped Bars"),
	redraw,
	DRAWING_FIXED_SIZE
};

static gint expose_event(GtkWidget * widget, GdkEventExpose * event)
{
	gdk_draw_pixmap(widget->window,
			default_gc,
			pixmap,
			event->area.x, event->area.y,
			event->area.x, event->area.y,
			event->area.width, event->area.height);

	return FALSE;
}
		
static GtkWidget *widget = NULL;
static gint total_width=0, total_height=0;
static gint time_width=3, row_height=15;
static gint hspace=5, vspace=5;
static gint thin_line_width=2;
static gint max_pid=0;
static gint last_cleared_time=0;
static gint pid=0;

static gint time_start = 0;
static void ready_proc_handler(sys_event_t type, proc_t *proc)
{
	gint time = get_time() - time_start;
	gint to_clear;

	if(time * time_width > CPU_config->drawing.max_graph_history){
		time_start = get_time();
		last_cleared_time = 0;
		time = 0;
	}
	total_width = MAX(total_width, time_width*(time+1)+hspace*2);
	if (type == SYS_EVENT_PROC_CREATE){
		max_pid = proc->pid;
		total_height = row_height*max_pid+vspace*2;
		enlarge_gdk_pixmap(&pixmap, total_width, total_height,
					white_gc);
		/* draw line from the begining */
		gdk_draw_rectangle(pixmap, default_gc, TRUE,
				hspace,
				vspace+(max_pid-1)*row_height
					+ (row_height-vspace)/2
					- thin_line_width/2,
				hspace+time*time_width - vspace,
				thin_line_width);
	}
	pid=proc->pid;
	enlarge_gdk_pixmap(&pixmap, total_width+burst(proc)*time_width,
				total_height, white_gc);
	
	/* clear the old data */
	to_clear = time+burst(proc) - last_cleared_time +1;
	if (to_clear > 0){
		gdk_draw_rectangle(pixmap, white_gc, TRUE,
				   hspace+last_cleared_time*time_width,
				   0,
				   time_width*to_clear,
				   total_height);
		last_cleared_time += to_clear;
	}

	/*draw the new piece of bar */
	gdk_draw_rectangle(pixmap, proc->color_gc, TRUE,
			   hspace+time*time_width,
			   vspace+(pid-1)*row_height,
			   time_width*burst(proc),
			   row_height-vspace);
}
static gint draw_CPU_state(GtkWidget *widget)
{
	gint time = get_time() - time_start;
	static gint last_time=0;
	gint i;

	if(time * time_width > CPU_config->drawing.max_graph_history){
		time_start = get_time();
		time = 0;
		last_time = 0;
		last_cleared_time = 0;
	}
	
	if (max_pid == 0) /* there are no processes */
		return 0;

	total_width = MAX(total_width, time_width*(time+1)+hspace*2);

	gtk_drawing_area_size(GTK_DRAWING_AREA(widget),
			total_width, total_height);

	enlarge_gdk_pixmap(&pixmap, total_width, total_height, white_gc);

	/* draw a line on each process to guide the user */
	for (i=0; i<max_pid; ++i)
		gdk_draw_rectangle(pixmap, default_gc, TRUE,
				hspace+last_time*time_width,
				vspace+i*row_height+(row_height-vspace)/2
					-thin_line_width/2,
				(time-last_time)*time_width,
				thin_line_width);
	/* clear the old data infront */
	if(time > last_cleared_time){
		gdk_draw_rectangle(pixmap, white_gc, TRUE,
				   hspace+time*time_width,
				   0,
				   time_width,
				   total_height);
		last_cleared_time = time;
	}

	last_time=time;
	return 0;
}

static void redraw (GtkWidget *widget)
{
	draw_CPU_state(widget);
	if (GTK_WIDGET_VISIBLE(widget)){
		gint width, height;
		gdk_window_get_size(pixmap, &width, &height);
		gdk_draw_pixmap(widget->window,
				default_gc,
				pixmap,
				0, 0,
				0, 0,
				width,
				height);
	}
	return;
}
static void setup_colors (void)
{
	GdkGCValues values;
	GdkColormap *colormap=gdk_colormap_get_system();

	gdk_color_black (colormap, &values.foreground);
	default_gc = gdk_gc_new_with_values (pixmap, &values,
						GDK_GC_FOREGROUND);
	gdk_color_white(colormap, &values.foreground);
	white_gc = gdk_gc_new_with_values (pixmap, &values,
						GDK_GC_FOREGROUND);

	return;
}

static gint button_press_event (GtkWidget *widget, GdkEventButton *event)
{
	proc_t *proc = NULL;
	gint pid;

	if (event->button != 1 || event->y < vspace)
		return TRUE;

	pid = (event->y - vspace)/row_height +1;

	if ((proc = get_proc_by_pid(pid)) != NULL)
		select_process (proc);

	return TRUE;
} 

static void realize (GtkWidget *widget)
{
	GdkGCValues values;
	
	gdk_gc_get_values(white_gc, &values);
	gdk_window_set_background(widget->window, &values.foreground);
}
void init_CPU_drawing_overlapped_bars(GtkWidget *drawing)
{
	widget = gtk_drawing_area_new();
	gtk_drawing_area_size(GTK_DRAWING_AREA(widget), 0, 0);
	
	pixmap = gdk_pixmap_new(NULL, 1, 1,
			gdk_visual_get_system()->depth);

	setup_colors();
	
	gdk_draw_point(pixmap, white_gc, 0, 0);

	gtk_widget_set_events (widget, GDK_EXPOSURE_MASK
			| GDK_BUTTON_PRESS_MASK);

	gtk_signal_connect(GTK_OBJECT(widget), "realize",
			   (GtkSignalFunc) realize, NULL);
	gtk_signal_connect(GTK_OBJECT(widget), "expose_event",
			   (GtkSignalFunc) expose_event, NULL);
	gtk_signal_connect (GTK_OBJECT (widget), "button_press_event",
			   (GtkSignalFunc) button_press_event, NULL);
	drawing_style.widget=widget;
	register_drawing_style (drawing, &drawing_style);
	system_event_receive(SYS_EVENT_PROC_CREATE,
			(sys_event_callback *)ready_proc_handler);
	system_event_receive(SYS_EVENT_PROC_READY,
			(sys_event_callback *)ready_proc_handler);
}

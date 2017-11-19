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
#include <gnome.h>
#include <stdio.h>

#include <CLOCK/CLOCK.h>
#include <CPU/queues.h>
#include <CPU/info.h>
#include <drawing.h>
#include <gdk-helper.h>
#include <events.h>
#include <process.h>
#include <CPU/cpu_config.h>

#include "shared.h"

static GdkPixmap *pixmap = NULL;
static GdkGC *default_gc = NULL;
static GdkGC *white_gc = NULL;
static GdkGC *proc_color_gc = NULL;

void init_CPU_drawing_bars(GtkWidget *drawing);
static void redraw (GtkWidget *widget);

static drawing_style_t drawing_style= {
	NULL,
	N_("Bars"),
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

static gint time_start=0;

static void new_proc_handler(sys_event_t type, proc_t *proc)
{
	gint time = get_time() - time_start;

	if(time * time_width > CPU_config->drawing.max_graph_history){
		time_start = get_time();
		time = 0;
	}
	max_pid = proc->pid;
	total_height = row_height*(max_pid+1)+vspace*2;
	/* draw line from the begining */
	gdk_draw_rectangle(pixmap, default_gc, TRUE,
			hspace,
			vspace+max_pid*row_height + (row_height-vspace)/2
				-thin_line_width/2,
			hspace+time*time_width - vspace,
			thin_line_width);
}
static gint pid=0;

static GdkGC **state_gc = NULL;
static void all_proc_handler(sys_event_t type, proc_t *proc)
{
	switch (type){
		case SYS_EVENT_PROC_CREATE:
			state_gc = g_renew(GdkGC *, state_gc, proc->pid+1);
			state_gc[proc->pid] = cpu_ready_gc;
			break;
		case SYS_EVENT_PROC_DESTROY:
			state_gc[proc->pid] = default_gc;
			break;
		case SYS_EVENT_PROC_READY:
		case SYS_EVENT_PROC_QUEUED:
			state_gc[proc->pid] = cpu_ready_gc;
			break;
		case SYS_EVENT_PROC_RUNNING:
			state_gc[proc->pid] = cpu_running_gc;
			break;
		case SYS_EVENT_PROC_WAITING:
			state_gc[proc->pid] = cpu_waiting_gc;
			break;
		case SYS_EVENT_PROC_SELECT:
			break;
		case SYS_EVENT_FRAME_SELECT:
		case SYS_EVENT_QUITTING:
			break;
	}
}

static void running_proc_handler(sys_event_t type, proc_t *proc)
{
	pid=proc->pid;
	gdk_gc_ref(proc->color_gc);
	proc_color_gc=proc->color_gc;
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
	}
	
	total_width = MAX(total_width, time_width*(time+1)+hspace*2);

	gtk_drawing_area_size(GTK_DRAWING_AREA(widget),
			total_width, total_height);

	enlarge_gdk_pixmap(&pixmap, total_width, total_height, white_gc);

	/* draw a line on each process to guide the user */
	for (i=0; i<=max_pid; ++i)
		gdk_draw_rectangle(pixmap, state_gc[i], TRUE,
				hspace+last_time*time_width,
				vspace+i*row_height+(row_height-vspace)/2 
					-thin_line_width/2,
				(time-last_time)*time_width,
				thin_line_width);
	/* clear the old data infront */
	gdk_draw_rectangle(pixmap, white_gc, TRUE,
			   hspace+time*time_width,
			   0,
			   time_width,
			   total_height);
	if (pid > 0){
		/*draw the new piece of bar */
		gdk_draw_rectangle(pixmap, proc_color_gc, TRUE,
				   hspace+last_time*time_width,
				   vspace+pid*row_height,
				   time_width*(time-last_time),
				   row_height-vspace);
		/*draw the new piece of bar on top*/
		gdk_draw_rectangle(pixmap, proc_color_gc, TRUE,
				   hspace+last_time*time_width,
				   vspace,
				   time_width*(time-last_time),
				   row_height-vspace);
	}
	last_time=time;
	return 0;
}

static void out_proc_handler(sys_event_t type, proc_t *proc)
{
	draw_CPU_state(widget);
	if (proc->nqueue == CPU_CURRENT)
		gdk_gc_unref(proc->color_gc);
	if (pid == proc->pid)
		pid=0;
}

static void redraw (GtkWidget *widget)
{
	if (!GTK_WIDGET_VISIBLE(widget))
		return;

	draw_CPU_state(widget);
	gdk_draw_pixmap(widget->window,
			default_gc,
			pixmap,
			0, 0,
			0, 0,
			total_width,
			total_height);
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

	state_gc = g_new(GdkGC *, 1);
	state_gc[0] = default_gc;

	return;
}

static gint size_allocate (GtkWidget * widget)
{
	gint width, height;
	GdkPixmap *new_pixmap;

	if (!GTK_WIDGET_REALIZED(widget))
		return TRUE;

	new_pixmap = gdk_pixmap_new(NULL,
				widget->allocation.width,
				widget->allocation.height,
				gdk_visual_get_system()->depth);
	/*clear the new pixmap*/
	gdk_draw_rectangle(new_pixmap, white_gc, TRUE,
			   0, 0,
			   widget->allocation.width,
			   widget->allocation.height);
	gdk_window_get_size(pixmap, &width, &height);
	/* copy old image */
	gdk_window_copy_area (new_pixmap,
			default_gc,
			0, 0, pixmap,
			0, 0,
			width, height);
	gdk_pixmap_unref(pixmap);

	pixmap = new_pixmap;

	return TRUE;
}

static gint button_press_event (GtkWidget *widget, GdkEventButton *event)
{
	proc_t *proc = NULL;
	gint pid;

	if (event->button != 1 || event->y < vspace + row_height)
		return TRUE;

	pid = (event->y - vspace - row_height)/row_height +1;

	if ((proc = get_proc_by_pid(pid)) != NULL)
		select_process (proc);

	return TRUE;
} 

void init_CPU_drawing_bars(GtkWidget *drawing)
{
	widget = gtk_drawing_area_new();
	gtk_drawing_area_size(GTK_DRAWING_AREA(widget), 0, 0);
	
	pixmap = gdk_pixmap_new(NULL, 1, 1,
			gdk_visual_get_system()->depth);

	setup_colors();
	total_height = row_height+vspace*2;

	gdk_draw_point(pixmap, white_gc, 0, 0);

	gtk_widget_set_events (widget, GDK_EXPOSURE_MASK
			| GDK_BUTTON_PRESS_MASK);

	gtk_signal_connect(GTK_OBJECT(widget), "expose_event",
			   (GtkSignalFunc) expose_event, NULL);
	gtk_signal_connect(GTK_OBJECT(widget), "size_allocate",
			   (GtkSignalFunc) size_allocate, NULL);
	gtk_signal_connect (GTK_OBJECT (widget), "button_press_event",
			   (GtkSignalFunc) button_press_event, NULL);
	drawing_style.widget=widget;
	register_drawing_style (drawing, &drawing_style);
	system_event_receive(SYS_EVENT_PROC_CREATE,
			(sys_event_callback *)all_proc_handler);
	system_event_receive(SYS_EVENT_PROC_DESTROY,
			(sys_event_callback *)all_proc_handler);
	system_event_receive(SYS_EVENT_PROC_READY,
			(sys_event_callback *)all_proc_handler);
	system_event_receive(SYS_EVENT_PROC_QUEUED,
			(sys_event_callback *)all_proc_handler);
	system_event_receive(SYS_EVENT_PROC_RUNNING,
			(sys_event_callback *)all_proc_handler);
	system_event_receive(SYS_EVENT_PROC_WAITING,
			(sys_event_callback *)all_proc_handler);
	
	system_event_receive(SYS_EVENT_PROC_CREATE,
			(sys_event_callback *)new_proc_handler);
	system_event_receive(SYS_EVENT_PROC_RUNNING,
			(sys_event_callback *)running_proc_handler);
	system_event_receive(SYS_EVENT_PROC_QUEUED,
			(sys_event_callback *)out_proc_handler);
	system_event_receive(SYS_EVENT_PROC_WAITING,
			(sys_event_callback *)out_proc_handler);
	system_event_receive(SYS_EVENT_PROC_DESTROY,
			(sys_event_callback *)out_proc_handler);

}

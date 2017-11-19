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

#include "shared.h"
#include "CPU.xpm"

void init_CPU_drawing_new4(GtkWidget *drawing);

static GdkPixmap *pixmap = NULL;
static GdkPixmap *CPU_pixmap = NULL;

static void redraw (GtkWidget *widget);

static drawing_style_t drawing_style= {
	NULL,
	N_("Queues"),
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
				"-*-*-bold-*-*-*-14-*-*-*-*-*-iso8859-1");
		if (pid_font == NULL)
			pid_font = gdk_font_load (
				"-*-*-*-*-*-*-14-*-*-*-*-*-iso8859-1");
		pid_height = gdk_string_height (pid_font, "0123456789");
		burst_font = gdk_font_load (
				"-*-*-normal-i-*-*-10-*-*-*-*-*-iso8859-1");
		if (burst_font == NULL)
			burst_font = gdk_font_load (
				"-*-*-*-*-*-*-10-*-*-*-*-*-iso8859-1");

		burst_height = gdk_string_height (burst_font, "0123456789");
	}
	/*draw the process box*/
	gdk_draw_rectangle(pixmap, 
		drawing->style->black_gc, FALSE, 
		x, y, 
		width, height);
	if(proc==NULL)
		return;
	/*write the process' pid*/
	y += margin + pid_height;
	sprintf(buff,"%d",proc->pid);
	gdk_draw_string(pixmap,
		pid_font, drawing->style->black_gc,
		x + width/4, y,
		buff);
	/*write the process' burst*/
	y += margin + burst_height;
	sprintf(buff,"%d",proc->next_event.time- proc->time);
	gdk_draw_string(pixmap,
		burst_font, drawing->style->black_gc,
		x + width/4, y ,
		buff);
}
		
static gint total_width, total_height;
static gint col_width=28, row_height=30;
static gint hspace=20, vspace=5;
static gint cpu_width=52, cpu_height=49;

static gint draw_CPU_queue(proc_queue_t queue, GtkWidget *drawing,
		gint y,
		GdkGC *gc)
{
	gint j = 0;
	gint len = proc_queue_len (queue);

	gdk_draw_rectangle(pixmap, drawing->style->black_gc, TRUE,
			hspace+3, y+3,
			col_width*len, row_height - vspace);
	gdk_draw_rectangle(pixmap, gc, TRUE,
			   hspace, y,
			   col_width*len, row_height - vspace);

	while (!proc_queue_end(queue)){

		draw_CPU_proc(proc_data(queue), drawing,
			j*col_width + hspace, y,
			col_width, row_height - vspace);
		queue = proc_queue_next(queue);
		++j;
	}
	return 0;
}

static void gdk_draw_arrow (GdkDrawable  *drawable, GdkGC *gc,
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

static gint draw_CPU_state(GtkWidget *widget)
{
	const proc_queues_t *queues = get_CPU_queues();
	gint len, max_len = 0;
	static gint old_width=0, old_height=0;
	gint cpu_xoffset = (cpu_width-col_width)/2 - 2;
	gint cpu_yoffset = (cpu_height-row_height)/2 +1;
	gint i;

	/* calculating maximum queue lenght */
	for (i=0; i < queues->nqueues; i++){
		len=proc_queue_len(queues->queue[i]);
		if ( len > max_len)
			max_len = len;
	}
	len = proc_queue_len(queues->wait);
	if ( len > max_len)
		max_len = len;
	/* we calculate some parameters for eficiency and to make the
	 * later code clearer */
	total_width = col_width*max_len + hspace*2;
	total_height = cpu_height + row_height*(queues->nqueues+1) + vspace*2;

	/* if the space need is diffrent from the last time set the new size
	 * or the drawing
	 * NOTE: if we set the size unconditionaly we get an infinite loop
	 * 	as the widget may get bigger than requested */
	
	if (old_width != total_width
			|| old_height != total_height){
		old_width = total_width;
		old_height = total_height;
		gtk_widget_set_usize(GTK_WIDGET(widget),
				total_width, total_height);
	}
	/*clear the pixmap*/
	gdk_draw_rectangle(pixmap, widget->style->white_gc, TRUE,
			   0, 0,
			   widget->allocation.width,
			   widget->allocation.height);
	/*draw the "CPU"*/
	gdk_window_copy_area (pixmap,
			widget->style->fg_gc[GTK_WIDGET_STATE(widget)],
			hspace, vspace, CPU_pixmap,
			0, 0, cpu_width, cpu_height);
	gdk_draw_rectangle(pixmap, cpu_running_gc, TRUE,
			   hspace+cpu_xoffset, vspace+cpu_yoffset,
			   col_width, row_height - vspace);
	gdk_draw_arrow(pixmap, widget->style->black_gc,
			3, vspace + cpu_height/2,
			hspace, vspace + cpu_height/2);
	draw_CPU_proc(queues->current, widget,
		hspace+cpu_xoffset, vspace+cpu_yoffset,
		col_width, row_height-vspace);
	/*draw all queues */
	if (max_len > 0 || !proc_queue_empty(queues->wait) )
	for (i=0; i < queues->nqueues; i++) {
		gint line_pos = cpu_height + i*row_height + row_height/2;
		gint len = proc_queue_len(queues->queue[i]);

		draw_CPU_queue(queues->queue[i], widget,
			cpu_height + i*row_height + vspace, 
			cpu_ready_gc);
		gdk_draw_line(pixmap, widget->style->black_gc,
				hspace, line_pos,
				3, line_pos);
		gdk_draw_line(pixmap, widget->style->black_gc,
				3, cpu_height + i*row_height + row_height/2,
				3, vspace + cpu_height/2);
		if (proc_queue_empty(queues->wait))
			continue;
		gdk_draw_arrow(pixmap, widget->style->black_gc,
				total_width -5, line_pos,
				len*col_width + hspace, line_pos);
	}

	/* go on drawing the single wait queue */
	if (!proc_queue_empty(queues->wait)) {
		gint line_pos = cpu_height + i*row_height + row_height/2;
		gint len = proc_queue_len(queues->wait);

		draw_CPU_queue(queues->wait, widget,
			cpu_height + i*row_height + vspace, 
			cpu_waiting_gc);
		gdk_draw_line(pixmap, widget->style->black_gc,
				len*col_width + hspace, line_pos,
				total_width -5, line_pos);
		gdk_draw_line(pixmap, widget->style->black_gc,
				total_width -5,
				cpu_height + i*row_height + row_height/2,
				total_width -5, cpu_height + row_height/2);
	}
	return 0;
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

	draw_CPU_state(widget);

	return TRUE;
}
static gint button_press_event (GtkWidget *widget, GdkEventButton *event)
{
	proc_queue_t queue = NULL;
	gint nqueues;
	gint clicked_queue, clicked_proc;
	proc_t *proc = NULL;

	if (event->button != 1 || event->x < hspace)
		return TRUE;

	clicked_queue = (event->y - (vspace + cpu_height))/row_height;
	clicked_proc = (event->x - hspace)/col_width;
	nqueues = get_CPU_queues()->nqueues;

	if (event->y < vspace + cpu_height){
		if (event->x < hspace + cpu_width)
			proc=get_CPU_current_proc();
	} else if (clicked_queue+1 <= nqueues )
		queue = get_CPU_queue(clicked_queue);
	else if (clicked_queue == nqueues )
		queue = get_CPU_wait_queue();
	if (queue != NULL)
		proc = proc_queue_nth (queue, clicked_proc);
	if (proc != NULL)
		select_process (proc);
	return TRUE;
} 

void init_CPU_drawing_new4(GtkWidget *drawing)
{
	GtkWidget *widget;
	widget = gtk_drawing_area_new();
	gtk_drawing_area_size(GTK_DRAWING_AREA(widget), 0, 0);
	
	gtk_widget_set_events (widget, GDK_EXPOSURE_MASK
			| GDK_BUTTON_PRESS_MASK);

	gtk_signal_connect(GTK_OBJECT(widget), "expose_event",
			   (GtkSignalFunc) expose_event, NULL);
	gtk_signal_connect(GTK_OBJECT(widget), "configure_event",
			   (GtkSignalFunc) configure_event, NULL);
	gtk_signal_connect (GTK_OBJECT (widget), "button_press_event",
			   (GtkSignalFunc) button_press_event, NULL);
	drawing_style.widget=widget;
	register_drawing_style (drawing, &drawing_style);
}

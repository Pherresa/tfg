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

void init_CPU_drawing_original(GtkWidget *drawing);
	
static GdkPixmap *pixmap = NULL;

static void redraw (GtkWidget *widget);

static drawing_style_t drawing_style= {
	NULL,
	N_("original"),
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
	char buff[10];
	gint len;
	/*draw the process box*/
	gdk_draw_rectangle(pixmap, 
		drawing->style->black_gc, FALSE, 
		x, y, 
		width, height);
	if(proc==NULL)
		return;
	/*write the process' pid*/
	len = sprintf(buff,"%d",proc->pid);
	gdk_draw_text(pixmap,
		drawing->style->font, drawing->style->black_gc,
		x + width/4, y + height/2,
		buff, len);
	/*write the process' burst*/
	len = sprintf(buff,"%d",proc->next_event.time- proc->time);
	gdk_draw_text(pixmap,
		drawing->style->font, drawing->style->black_gc,
		x + width/4, y + height - 2,
		buff, len);
}
		
static gint draw_CPU_queue(proc_queue_t queue, GtkWidget *drawing,
		gint y,
		gint hspace, gint vspace,
		gint col_width, gint row_height)
{
	int j=0;
	while (!proc_queue_end(queue)){

		draw_CPU_proc(proc_data(queue), drawing,
			j*col_width + hspace, y,
			col_width, row_height - vspace);
		queue = proc_queue_next(queue);
		++j;
	}
	return 0;
}

static gint draw_CPU_drawing(GtkWidget *widget)
{
	const proc_queues_t *queues = get_CPU_queues();
	gint len, max_len = 0;
	gint total_width, total_height;
	static gint old_width=0, old_height=0;
	gint col_width=30, row_height=30;
	gint hspace=5, vspace=5;
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
	total_height = row_height*(queues->nqueues+2) + vspace*2;

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
	draw_CPU_proc(queues->current, widget,
		hspace, vspace,
		col_width, row_height-vspace);
	/*draw all queues */
	for (i=0; i < queues->nqueues; i++)
		draw_CPU_queue(queues->queue[i], widget,
			(i+1)*row_height + vspace, 
			hspace, vspace,
			col_width, row_height);
	/* go on drawing the single wait queue */
	if (!proc_queue_empty(queues->wait))
		draw_CPU_queue(queues->wait, widget,
			(i+1)*row_height + vspace, 
			hspace, vspace,
			col_width, row_height);
	return 0;
}

static void redraw (GtkWidget *widget)
{
	draw_CPU_drawing(widget);
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
	if (pixmap)
		gdk_pixmap_unref(pixmap);

	pixmap = gdk_pixmap_new(widget->window,
				widget->allocation.width,
				widget->allocation.height,
				-1);

	draw_CPU_drawing(widget);

	return TRUE;
}

void init_CPU_drawing_original(GtkWidget *drawing)
{
	GtkWidget *widget;
	widget = gtk_drawing_area_new();
	gtk_drawing_area_size(GTK_DRAWING_AREA(widget), 0, 0);

	gtk_signal_connect(GTK_OBJECT(widget), "expose_event",
			   (GtkSignalFunc) expose_event, NULL);
	gtk_signal_connect(GTK_OBJECT(widget), "configure_event",
			   (GtkSignalFunc) configure_event, NULL);
	drawing_style.widget=widget;
	register_drawing_style (drawing, &drawing_style);
}

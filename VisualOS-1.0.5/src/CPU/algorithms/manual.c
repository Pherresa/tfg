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

#include <CPU/cpu_config.h>
#include <CLOCK/CLOCK.h>

#include "algorithm_api.h"
#include "manual.h"

static gint manual_select (void);
static gint manual_unselect (void);
static gint manual_clock (void);
static gint manual_select_proc (proc_t *proc);
static gint manual_init_proc (proc_t *proc);
static gint manual_end_proc (proc_t *proc);
static gint manual_event (proc_t *proc);
static gint manual_next (proc_t *proc);

static cpu_algorithm_t manual_algorithm = {
	N_("Manual"),
	manual_select,
	manual_unselect,
	manual_clock, 
	manual_select_proc,
	NULL,/*properties WidGet*/
	NULL,/*process_properties WidGet*/
	manual_init_proc,
	manual_end_proc,
	manual_event,
	manual_next
};
static const gint num_algorithm_params = 2;
static property_t algorithm_params[] = {
	{N_("Number of queues:"), 1, 10, 1},
	{N_("HeartBeat:"), 1, 99, 1}
};
#define NQUEUES 0
#define HEARTBEAT 1
static const gfloat default_params[]={
	1,	/* Number of queues */
	4,	/* Heartbeat */
};
static gint nqueues;
static properties_t *properties;

static void button_callback(GtkButton *button, gpointer data)
{
	gint queue=GPOINTER_TO_INT(data);
	proc_t *proc = get_CPU_selected_proc();

	g_return_if_fail(proc->nqueue != CPU_WAITING);

	if (queue==CPU_CURRENT){
		if (get_CPU_current_proc() != NULL)
			g_warning ("manual: Process %d not moved (CPU busy)\n",
				proc->pid);
		else
			move_proc_to_CPU(proc);
	} else
		move_proc_to_queue (proc,queue);
}

static gint fill_with_to_queue_buttons (GtkWidget *box, gint num)
{
	gint i;
	gchar label[40];
	GtkWidget *Gtk_aux;
	
	gtk_container_foreach (GTK_CONTAINER (box),
			(GtkCallback) gtk_widget_destroy, NULL);

	sprintf (label,_("To CPU"));
	Gtk_aux=gtk_button_new_with_label (label);
	gtk_box_pack_start (
			GTK_BOX(box),
			Gtk_aux, TRUE, TRUE, 0);
	gtk_widget_show(GTK_WIDGET(Gtk_aux));
	gtk_signal_connect(GTK_OBJECT(Gtk_aux), "clicked", 
			GTK_SIGNAL_FUNC(button_callback), 
			GINT_TO_POINTER(CPU_CURRENT));
	for (i=0; i < num; ++i){
		sprintf (label,_("To Queue %d"), i);
		Gtk_aux=gtk_button_new_with_label (label);
		gtk_box_pack_start (
				GTK_BOX(box),
				Gtk_aux, TRUE, TRUE, 0);
		gtk_widget_show(GTK_WIDGET(Gtk_aux));
		gtk_signal_connect(GTK_OBJECT(Gtk_aux), "clicked", 
				GTK_SIGNAL_FUNC(button_callback), 
				GINT_TO_POINTER(i));
	}
	return 0;
}

static void manual_property_change_notify(void)
{
	gfloat *value;

	value = properties_get_values(properties);
	nqueues = value[NQUEUES];
	request_nqueues(nqueues);
	set_CPU_heart_beat(value[HEARTBEAT]);
	fill_with_to_queue_buttons (manual_algorithm.process_properties,
			nqueues);
}
	
gint manual_init (void)
{
	register_CPU_algorithm (&manual_algorithm);
	return 0;
}
static gint manual_clock (void)
{ 
#ifdef DEBUG
	g_print ("manual_clock: Clock interrupt detected\n");
#endif
	return 0;
}
static gint manual_select_proc (proc_t *proc)
{
	return 0;
}
static gint manual_select (void)
{
	set_CPU_heart_beat(default_params[HEARTBEAT]);
	manual_algorithm.process_properties = gtk_vbox_new(FALSE, 0);

	properties = properties_create (algorithm_params,
			num_algorithm_params,
			manual_property_change_notify);
	properties_set_values(properties, default_params);
	nqueues = default_params[NQUEUES];

	manual_algorithm.properties = properties_get_widget(properties);

	fill_with_to_queue_buttons(manual_algorithm.process_properties,
			nqueues);

	request_nqueues (nqueues);
	return 0;
}
static gint manual_unselect (void)
{
	properties_destroy (properties);
	manual_algorithm.properties=NULL;
	gtk_object_destroy(GTK_OBJECT(manual_algorithm.process_properties));
	manual_algorithm.process_properties=NULL;
	return 0;
}
static gint manual_init_proc (proc_t *proc)
{
	static gint nqueue=0;
	if (get_CPU_current_proc() == NULL){
		move_proc_to_CPU(proc);
	} else {
		move_proc_to_queue(proc, nqueue);
		nqueue = (nqueue+1) % nqueues;
	}
	return 0;
}
static gint manual_end_proc (proc_t *proc)
{
#ifdef DEBUG
	g_print ("manual_end_proc: process %d freed\n",proc->pid);
#endif
	return 0;
}
static gint manual_event (proc_t *proc)
{
#ifdef DEBUG
	g_print ("manual_event: event for process %d\n",proc->pid);
#endif
	move_proc_to_queue(proc, 0);
	if(get_CPU_current_proc()==NULL && CPU_config->stop_clock)
		CLOCK_stop();
	return 0;
}
static gint manual_next (proc_t *proc)
{
#ifdef ALGORITHM_MANUAL_RUN_SELECTED_PROC
	proc_t *new_proc = get_CPU_selected_proc();
	
	if (new_proc != NULL && new_proc->nqueue >=0)
		move_proc_to_CPU(new_proc); 
#else
	if(CPU_config->stop_clock)
		CLOCK_stop();
#endif
#ifdef DEBUG
	g_print ("manual_event: process %d waiting\n",proc->pid);
#endif
	reset_CPU_timer();
	return 0;
}

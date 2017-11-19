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

#include "algorithm_api.h"
#include "test.h"

static gint test_select (void);
static gint test_unselect (void);
static gint test_clock (void);
static gint test_select_proc (proc_t *proc);
static gint test_init_proc (proc_t *proc);
static gint test_end_proc (proc_t *proc);
static gint test_event (proc_t *proc);
static gint test_next (proc_t *proc);

static cpu_algorithm_t test_algorithm = {
	"Test",
	test_select,
	test_unselect,
	test_clock, 
	test_select_proc,
	NULL,/*properties WidGet*/
	NULL,/*process_properties WidGet*/
	test_init_proc,
	test_end_proc,
	test_event,
	test_next
};
static const gint num_algorithm_params = 5;
static property_t algorithm_params[] = {
	{"Number of queues:", 1, 10, 1},
	{"HeartBeat:", 1, 99, 1},
	{"Property 2:", 0, 200, 2.3},
	{"Property 3:", 0, 300, 1.5},
	{"Property 4:", 0, 100, 1.7}
};
#define NQUEUES 0
#define HEARTBEAT 1
static const gfloat default_params[]={
	4,	/* Number of queues */
	4,	/* Heartbeat */
	4.6,	/* Property 2 */
	4.5,	/* Property 3 */
	3.4	/* Property 4 */
};
static gint nqueues;
static properties_t *properties;
	
/* this is not right, the second argument is a pointer not an integer
 * an this may not be portable, using a pointer variable a integer */
static void button_callback(GtkButton *button, gpointer queue)
{
	proc_t *proc = get_CPU_selected_proc();
#ifdef DEBUG
	g_print("test: Moving process %d to queue %d\n", proc->pid,
			(int)queue);
#endif
	if ((int)queue == CPU_CURRENT)
		move_proc_to_CPU(proc);
	else
		move_proc_to_queue(proc,(int)queue);
#ifdef DEBUG
	g_print("test: Process %d moved to queue %d\n", proc->pid,
			(int)queue);
#endif
}

static gint fill_with_to_queue_buttons (GtkWidget *box, gint num)
{
	gint i;
	gchar label[40];
	GtkWidget *button;
	
	
	gtk_container_foreach (GTK_CONTAINER (box),
			(GtkCallback) gtk_widget_destroy, NULL);

	sprintf (label,"To CPU");
	button=gtk_button_new_with_label (label);
	gtk_box_pack_start (
			GTK_BOX(box),
			button, TRUE, TRUE, 0);
	gtk_widget_show(GTK_WIDGET(button));
	gtk_signal_connect(GTK_OBJECT(button), "clicked", 
			GTK_SIGNAL_FUNC(button_callback), 
			(gpointer)CPU_CURRENT);
	for (i=0; i < num; ++i){
		sprintf (label,"To Queue %d", i);
		button=gtk_button_new_with_label (label);
		gtk_box_pack_start (
				GTK_BOX(box),
				button, TRUE, TRUE, 0);
		gtk_widget_show(GTK_WIDGET(button));
		gtk_signal_connect(GTK_OBJECT(button), "clicked", 
				GTK_SIGNAL_FUNC(button_callback), 
				(gpointer)i);
	}
	return 0;
}

static void test_property_change_notify(void)
{
	gint i;
	gfloat *value;

	value = properties_get_values(properties);
	nqueues = value[NQUEUES];
	request_nqueues(nqueues);
	set_CPU_heart_beat(value[HEARTBEAT]);
	g_print("test: current param values are:\n");
	for (i=0; i < num_algorithm_params; ++i)
			g_print("\t %s %f\n",algorithm_params[i].label,
					value[i]);
	fill_with_to_queue_buttons (test_algorithm.process_properties,
			nqueues);
}

gint test_init (void)
{
	register_CPU_algorithm (&test_algorithm);
	return 0;
}
static gint test_clock (void)
{ 
#ifdef DEBUG
	g_print ("test_clock: Clock interrupt detected\n");
#endif
	return 0;
}
static gint test_select_proc (proc_t *proc)
{
#ifdef DEBUG
	g_print ("Selected process %d\n",proc->pid);
#endif
	return 0;
}
static gint test_select (void)
{
	set_CPU_heart_beat(default_params[HEARTBEAT]);
	test_algorithm.process_properties = gtk_vbox_new(FALSE, 0);

	properties = properties_create (algorithm_params,
			num_algorithm_params,
			test_property_change_notify);
	properties_set_values(properties, default_params);
	nqueues = default_params[NQUEUES];

	test_algorithm.properties = properties_get_widget(properties);

	test_algorithm.process_properties = gtk_vbox_new(FALSE, 0);
	fill_with_to_queue_buttons(test_algorithm.process_properties, nqueues);

	request_nqueues (nqueues);
	return 0;
}
static gint test_unselect (void)
{
	properties_destroy (properties);
	test_algorithm.properties = NULL;
	gtk_widget_destroy(GTK_WIDGET(test_algorithm.process_properties));
	test_algorithm.process_properties = NULL;
	return 0;
}
static gint test_init_proc (proc_t *proc)
{
	static gboolean first=TRUE;
	static gint nqueue=0;
	if (first){
		move_proc_to_CPU(proc);
		first=FALSE;
	} else {
		move_proc_to_queue(proc, nqueue);
		nqueue = (nqueue+1) % nqueues;
	}
	return 0;
}
static gint test_end_proc (proc_t *proc)
{
#ifdef DEBUG
	g_print ("test_end_proc: process %d freed\n",proc->pid);
#endif
	return 0;
}
static gint test_event (proc_t *proc)
{
#ifdef DEBUG
	g_print ("test_event: event for process %d\n",proc->pid);
#endif
	return 0;
}
static gint test_next (proc_t *proc)
{
#ifdef DEBUG
	g_print ("test_event: event for process %d\n",proc->pid);
#endif
	reset_CPU_timer();
	return 0;
}

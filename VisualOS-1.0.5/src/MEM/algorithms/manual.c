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

#include <SCHED.h>
#include <events.h>
#include <CLOCK/CLOCK.h>
#include <MEM/mem_config.h>

#include "algorithm_api.h"
#include "manual.h"

void selected_frame_event_handler (sys_event_t type, gpointer frame_p);
static void algorithm_select (void);
static void algorithm_unselect (void);
static void page_access (gint pid, gint page);
static gint select_frame (void);

static mem_algorithm_t algorithm = {
	N_("Manual"),
	algorithm_select,
	algorithm_unselect,
	NULL,/*properties WidGet*/
	page_access,
	select_frame
};
static gchar *pending_faults_msg = N_("We are short on memory,"
				      " please select a frame to use");
static gchar *no_pending_faults_msg=N_("No pending page faults");
static gint pending_faults = 0;
static gint selected_frame = NO_FRAME;
static gboolean algorithm_selected = FALSE;
static gboolean system_is_quitting = FALSE;

void selected_frame_event_handler (sys_event_t type, gpointer frame_p)
{
	if (pending_faults == 0)
		return;

	g_return_if_fail(pending_faults > 0);
	g_return_if_fail(selected_frame == NO_FRAME);
	selected_frame = GPOINTER_TO_INT(frame_p);
#ifndef I__CAN__MAKE__GTK_MAIN_QUIT__WORK__WITH__THIS
	gtk_main_quit();
#endif
}
static void system_quitting(void)
{
	system_is_quitting = TRUE;
	if(pending_faults > 0)
		gtk_main_quit();
}

void mem_manual_init (void)
{
	system_event_receive(
		SYS_EVENT_QUITTING,
		(sys_event_callback *)system_quitting);
	system_event_receive(
		SYS_EVENT_FRAME_SELECT,
		(sys_event_callback *) selected_frame_event_handler);
	register_MEM_algorithm (&algorithm);
	return ;
}
static void algorithm_select (void)
{
	algorithm.properties = gtk_label_new(_(no_pending_faults_msg));
	gtk_label_set_line_wrap(GTK_LABEL(algorithm.properties), TRUE); 
	algorithm_selected = TRUE;
}
static void algorithm_unselect (void)
{
	gtk_widget_destroy(GTK_WIDGET(algorithm.properties));
	algorithm.properties = NULL;
	algorithm_selected = FALSE;
}
static void page_access(gint pid, gint page)
{
}
static gint select_frame (void)
{
	frame_info_t *frame = NULL;
	
	g_return_val_if_fail(system_is_quitting == FALSE, NO_FRAME);

	if(MEM_config->stop_clock)
		CLOCK_stop();
	pending_faults++;
	gtk_label_set_text(GTK_LABEL(algorithm.properties),
			   _(pending_faults_msg));
#ifdef I__CAN__MAKE__GTK_MAIN_QUIT__WORK__WITH__THIS
	do {
		while (selected_frame == NO_FRAME)
			gtk_main_iteration();
		frame = get_frame_info(selected_frame);
		selected_frame = NO_FRAME;
	} while (FRAME_LOCKED(frame));
#else
	do {
		gtk_main();
		if (system_is_quitting){
			if (pending_faults > 0)
				gtk_main_quit();
			return NO_FRAME;
		}
		g_return_val_if_fail(selected_frame != NO_FRAME, NO_FRAME);
		frame = get_frame_info(selected_frame);
		selected_frame = NO_FRAME;
	} while (FRAME_LOCKED(frame));
#endif
	if (--pending_faults == 0)
		gtk_label_set_text(GTK_LABEL(algorithm.properties),
				   _(no_pending_faults_msg));
	return frame->frame;
}

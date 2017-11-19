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

#include <gtk/gtk.h>
#include <glade/glade.h>

#include <CLOCK/CLOCK.h>
#include <process.h>
#include <events.h>
#include <interface.h>

void init_CPU_stats(void);

struct proc_stats_s {
	gint start_time;
	gint ready_time;
	gint last_ran;
	gboolean ran;
};
typedef struct proc_stats_s proc_stats_t;

static GtkEntry *creation_wid=NULL;
static GtkEntry *return_wid=NULL;
static GtkEntry *burst_wid=NULL;
static GtkEntry *response_wid=NULL;
static void all_proc_handler (sys_event_t type, proc_t *proc)
{
	static gint max_pid=1;
	static gint total_start_wait=0;
	static gint n_start_wait=0;
	static gint total_return=0;
	static gint n_return=0;
	gint time = get_time();
	proc_stats_t *data = proc->stats_data;
	GString *str = g_string_new(NULL);
	switch (type){
		case SYS_EVENT_PROC_CREATE:
			proc->stats_data = data = g_new(proc_stats_t, 1);
			data->ran = FALSE;
			data->start_time = time;
			max_pid = proc->pid;
			g_string_sprintf(str, "%d", time/max_pid);
			gtk_entry_set_text(creation_wid, str->str);
			break;
		case SYS_EVENT_PROC_READY:
			data->ready_time=time;
			break;
		case SYS_EVENT_PROC_WAITING:
			data->last_ran = time;
			break;
		case SYS_EVENT_PROC_QUEUED:
			data->last_ran = time;
			break;
		case SYS_EVENT_PROC_DESTROY:
			total_return += time - data->start_time;
			++n_return;
			g_string_sprintf(str, "%d", total_return/n_return);
			gtk_entry_set_text(return_wid, str->str);
			g_free(data);
			break;
		case SYS_EVENT_PROC_RUNNING:
			if(data->ran)
				break;
			data->ran = TRUE;
			total_start_wait += time - data->start_time;
			++n_start_wait;
			g_string_sprintf(str, "%d",
					total_start_wait/n_start_wait);
			gtk_entry_set_text(response_wid, str->str);
			break;
		case SYS_EVENT_PROC_SELECT:
		case SYS_EVENT_FRAME_SELECT:
		case SYS_EVENT_QUITTING:
			break;
	}
	g_string_free(str, TRUE);
}
void init_CPU_stats(void)
{
	GladeXML *xml;
	
	xml = glade_xml_new(get_xml_file(), "stats");
	glade_xml_signal_autoconnect(xml);
	creation_wid = GTK_ENTRY(glade_xml_get_widget(xml, "stats_creation"));
	return_wid = GTK_ENTRY(glade_xml_get_widget(xml, "stats_return"));
	burst_wid = GTK_ENTRY(glade_xml_get_widget(xml, "stats_burst"));
	response_wid = GTK_ENTRY(glade_xml_get_widget(xml, "stats_response"));
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
}

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

#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <gnome.h>

#include <messaging.h>
#include <events.h>
#include <CLOCK/CLOCK.h>

#include "CPU.h"
#include "status.h"
#include "cpu_config.h"

static proc_creat_callback_t proc_creat_callback = NULL;
static proc_finish_callback_t proc_finish_callback = NULL;

static void proc_creat(const message_t *m)
{
     	if (proc_creat_callback)
		proc_creat_callback(*(gint *)m->data);
}
static void proc_finish(const message_t *m)
{
	if (proc_finish_callback)
		proc_finish_callback(*(gint *)m->data);
}
/**
 * cpu_register_proc_creat:
 * @func: function to be called.
 *
 * Registers @func to be called when ever a new process gets created.
 */
void cpu_register_proc_creat(proc_creat_callback_t func)
{
	proc_creat_callback = func;
	mesg_callback_register(PROC_CREAT, proc_creat);
}
/**
 * cpu_register_proc_finish:
 * @func: function to be called.
 *
 * Registers @func to be called when ever a process is terminated.
 */

void cpu_register_proc_finish(proc_finish_callback_t func)
{
	proc_finish_callback = func;
	mesg_callback_register(PROC_FINISH, proc_finish);
}
static void sys_event_handler (sys_event_t type, proc_t *proc)
{
	switch(type){
	case SYS_EVENT_PROC_CREATE:
		mesg_send(MEM, PROC_CREAT, &proc->pid, sizeof(gint));
		break;
	case SYS_EVENT_PROC_DESTROY:
		mesg_send(MEM, PROC_FINISH, &proc->pid, sizeof(gint));
		break;
	case SYS_EVENT_PROC_READY:
	case SYS_EVENT_PROC_QUEUED:
	case SYS_EVENT_PROC_RUNNING:
	case SYS_EVENT_PROC_WAITING:
	case SYS_EVENT_PROC_SELECT:
	case SYS_EVENT_FRAME_SELECT:
	case SYS_EVENT_QUITTING:
		break;
	}
}
/**
 * CPU_terminate_proc:
 * @pid: Process to terminates.
 *
 * Terminate process @pid.
 */
void CPU_terminate_proc(gint pid)
{
	mesg_send(CPU, PROC_TERMINATE, &pid, sizeof(gint));
}
static void cpu_terminate_proc_server(const message_t *m)
{
	GString *str = g_string_new(NULL);
	gint pid = *(gint *)m->data;

	destroy_process(get_proc_by_pid(pid));

	if(CPU_config->stop_clock)
		CLOCK_stop();
	g_string_sprintf(str, _("Process %d was terminated by %s"), pid,
				mesg_subsystem_name(m->sender));
	CPU_status_set(str->str);
	g_string_free(str, TRUE);
}

void cpu_server_init(void)
{
	system_event_receive(SYS_EVENT_PROC_CREATE,
			     (sys_event_callback *)sys_event_handler);
	system_event_receive(SYS_EVENT_PROC_DESTROY,
			     (sys_event_callback *)sys_event_handler);
	mesg_callback_register(PROC_TERMINATE, cpu_terminate_proc_server);
}



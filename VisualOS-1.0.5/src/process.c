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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

#include <CPU/combos.h>
#include <CPU/queues.h>
#include <CPU/editor/editor.h>
#include <CPU/editor/parser.h>
#include <CPU/info.h>
#include <CPU/cpu_config.h>
#include <CLOCK/CLOCK.h>
#include <SCHED.h>
#include <gdk-helper.h>
#include <events.h>
#include <file-dialog.h>

#include "process.h"
/**
 * burst:
 * @proc: process involved.
 *
 * Calculates the current burst (time until next voluntary event) for @proc.
 */
/**
 * DECLARE_PROC_QUEUE:
 * @queue: name for the queue.
 *
 * Declares a new and empty process queue with name @queue.
 */
/**
 * proc_queue_empty:
 * @queue: process queue involved.
 *
 * Is @queue empty?
 */
/**
 * proc_data:
 * @element: process queue element involved.
 *
 * Retrives the process data from the queue @element.
 */
/**
 * proc_queue_next:
 * @element: process queue element involved.
 *
 * Gets the next element on the queue starting at @element.
 */
/**
 * proc_queue_find:
 * @queue: process queue involved.
 * @proc: process involved.
 *
 * Find the queue element for process @proc.
 */
/**
 * proc_queue_len:
 * @queue: process queue involved.
 *
 * Calculate the number of elements on @queue.
 */
/**
 * proc_queue_init:
 * @queue: process queue involved.
 *
 * Initializes process queue @queue.
 *
 * Note: applyed to a non empty queue will loose all its elements.
 */
/**
 * proc_queue_foreach:
 * @queue: process queue involved.
 * @func: function to be called.
 * @data: second argument to @func.
 *
 * Will call @func for every process on @queue using the process pointer
 * as the first arguemt and @data as the second.
 */
/**
 * proc_queue_concat:
 * @dest: Target queue.
 * @orig1: First source queue.
 * @orig2: Second source queue.
 *
 * Concatenates @orig1 and @orig2 into @dest.
 */
/**
 * proc_queue_remove:
 * @queue: process queue involved.
 * @proc: process to be removed.
 *
 * Remove @proc from @queue.
 */
/**
 * proc_queue_append:
 * @queue: process queue involved.
 * @proc: process to be appended.
 *
 * Append @proc to @queue.
 */
/**
 * proc_queue_nth:
 * @queue: queue involved.
 * @n: element index.
 *
 * Get element number @n from @queue.
 */
/**
 * proc_queue_end:
 * @element: process queue element involved.
 *
 * Is this element the end of the queue?
 */
static DECLARE_PROC_QUEUE(new_proc_list);
static DECLARE_PROC_QUEUE(proc_list);
static DECLARE_PROC_QUEUE(dead_proc_list);
static gint last_pid=0;

static void add_proc_to_gstring(proc_t *proc, GString *str)
{
		GString *proc_str=get_proc_in_gstring(proc);
		g_string_append(str, proc_str->str);
		g_string_free(proc_str, TRUE);
}
/**
 * save_processes_to_file:
 *
 * Asks the user for a filename and saves all processes on the current
 * session to a file, ready to be loaded in a new session.
 *
 * Note: Both terminated and not yet running processes will be written.
 */
void save_processes_to_file (void)
{
	const gchar *file_name = select_file();
	GString *str;
	int fd;

	if (!file_name)
		return;
	if((fd = creat (file_name, 0666))<0)
		return;
	
	g_print("Saving processes to %s\n", file_name);
	str = g_string_new(NULL);
	proc_queue_foreach(dead_proc_list, (GFunc)add_proc_to_gstring, str);
	proc_queue_foreach(proc_list, (GFunc)add_proc_to_gstring, str);
	proc_queue_foreach(new_proc_list, (GFunc)add_proc_to_gstring, str);

	write(fd, str->str, str->len);
	g_print("Done saving processes to %s\n", file_name);
	g_string_free(str, TRUE);
	close(fd);
		
}
/**
 * load_processes_from_file:
 *
 * Asks the user for a filename and loads all processes it can find it it.
 *
 * Note: Not all processes will be visible at once, they will be inserted
 * at the right time.
 */
void load_processes_from_file (void)
{
	const gchar *file_name = select_file();
	simul_data_t *data = g_new0(simul_data_t, 1);
	gchar *buff, *str;
	proc_t *proc;
	off_t size;
	int fd;

	if (!file_name)
		return;
	if ((fd = open (file_name, O_RDONLY))<0){
		perror("load_processes_from_file");
		return;
	}

	g_print("Loading processes from %s\n", file_name);
	size = lseek(fd, 0, SEEK_END);
	lseek (fd, 0, SEEK_SET);
	buff = (gchar *)g_malloc(size+1);
	str = buff;
	read (fd, buff, size);
	while ((str = get_simulation_from_string(data, str))){
		proc = new_process();
		proc->simul_data=data;
		fix_simulation_in_proc(proc);
		insert_process(proc);
		data = g_new0(simul_data_t, 1);
	}
	g_print("Done loading processes from %s\n", file_name);
	g_free(data);
	g_free(buff);
	close(fd);
}
/**
 * get_proc_list:
 *
 * returns: the list of all currently running processes.
 */
proc_queue_t get_proc_list(void)
{
	return proc_list;
}
/**
 * get_proc_by_pid:
 * @pid: process ID
 *
 * returns: the data of process @pid or NULL if there is no process
 * with PID @pid.
 */
proc_t *get_proc_by_pid(gint pid)
{
	proc_queue_t item=proc_list;

	while (!proc_queue_end(item)){
		if (proc_data(item)->pid == pid)
			return proc_data(item);
		item = proc_queue_next(item);
	}
	return NULL;
}

static proc_t *selected_proc=NULL;

static void set_color_in_proc(proc_t *proc)
{
	static guint8 color[3] = {80, 100, 100};
	static gint which = 0;
	color[which] += 50;

	proc->color_gc = new_gdk_GC_with_color(color[0], color[1], color[2]);

	which = ++which % 3;
}
static void insert_sched_proc(gint time, proc_t *proc)
{
	insert_process(proc);
}
/**
 * create_process:
 *
 * Do everything necesary to  have a new process in the system.
 *
 * returns: the newly created process.
 */
proc_t *create_process (void)
{
	proc_t *proc = new_process();
	gboolean canceled = FALSE;
	
	/*set process_properties*/
	if (CPU_config->auto_fill_procs)
		auto_fill_process_properties(proc);
	else
		canceled = edit_process_properties(proc);

	if (canceled){
		free_process(proc);
		return NULL;
	}

	insert_process(proc);
	return proc;
}
/**
 * new_process:
 *
 * Allocate data for new process.
 *
 * Note: the process will have to be inserted to have any efect.
 * 
 * returns: the newly allocate process data.
 */
proc_t *new_process (void)
{
	proc_t *proc;
	
	proc = g_new(proc_t, 1);
	proc->time = 0;
	proc->pid = -1;
	proc->nqueue = CPU_NO_QUEUE;
	proc->simul_data = NULL;
	proc->algorithm_data = NULL;
	set_color_in_proc(proc);
	init_CPU_simulation_in_proc(proc);
	return proc;
}
/**
 * insert_process:
 * @proc: the process involved.
 *
 * Inserts process @proc in the system.
 *
 * Note: @proc can be obtained with @new_process.
 */
void insert_process(proc_t *proc)
{
	gint time = get_time();

	proc_queue_remove (new_proc_list, proc);

	if (proc->next_event.type == EVENT_START){
		if(proc->next_event.time > time){
			proc_queue_append (new_proc_list, proc);
			sched_event(proc->next_event.time,
					(sched_callback_t)insert_sched_proc,
					proc, 0);
			return;
		} else
			next_CPU_simulation_in_proc(proc);
	}
	proc->pid = ++last_pid;
	proc_queue_append (proc_list, proc);
	system_event(SYS_EVENT_PROC_CREATE, proc);
	get_CPU_current_algorithm()->init_proc(proc);
	if (selected_proc == NULL)
		select_process(proc);
}
/**
 * select_process:
 * @proc: process involved.
 *
 * Makes @proc the selected process.
 *
 * Note: some code will do things to the selected process.
 */
void select_process (proc_t *proc)
{
	if (proc == selected_proc)
		return;
	selected_proc = proc;
	system_event(SYS_EVENT_PROC_SELECT, proc);
}
/**
 * get_CPU_selected_proc:
 *
 * Find out which is the currently selected process
 *
 * returns: the currently selected process.
 */
proc_t *get_CPU_selected_proc (void)
{
	return selected_proc;
}
/**
 * destroy_process:
 * @proc: process involved.
 * 
 * Start considering @proc a terminated process and remove it from the
 * system.
 *
 * Note: the data of @proc is not freed, and @proc will be saved by
 * @save_processes_to_file.
 *
 * returns: nothing important.
 */
gint destroy_process (proc_t *proc)
{
	cpu_algorithm_t *algorithm=get_CPU_current_algorithm();

	if(proc->nqueue == CPU_WAITING)
		wakeup_proc(proc);

	system_event(SYS_EVENT_PROC_DESTROY, proc);
	proc_queue_remove(proc_list, proc);
	proc_queue_append (dead_proc_list, proc);
	if (proc == selected_proc){ 
		if (!proc_queue_empty(proc_list))
			select_process(proc_data(proc_list));
		else 
			select_process(NULL);
	}
	algorithm->end_proc(proc);
	remove_proc_from_queue(proc);
	end_CPU_simulation_in_proc(proc);
#if 0
	gdk_gc_unref(proc->color_gc);
#endif
	return 0;
}
/**
 * free_process:
 * @proc: process involved.
 *
 * Definitely free all data related to @proc.
 *
 * Note: @proc will be gone for good.
 */
void free_process (proc_t *proc)
{
	proc_queue_remove(dead_proc_list, proc);
	free_CPU_proc_simulation_data(proc);
	g_free(proc);
}

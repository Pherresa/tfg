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

#include <string.h>
#include <gtk/gtk.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>


#include <IO/IO.h>
#include <CPU/simulation.h>
#include <file-dialog.h>

#include "file.h"
#include "parser.h"

void read_CPU_prop_file(proc_t *proc)
{
	const gchar *file = select_file();
	simul_data_t *old_data = proc->simul_data;
	simul_data_t *new_data;
	gchar *buff;
	off_t size;
	int fd;

	if (!file)
		return;

	fd = open (file, O_RDONLY);
	if(fd < 0){
		perror("read_CPU_prop_file");
		return;
	}

	size = lseek(fd, 0, SEEK_END);
	buff = (gchar *)g_malloc(size+1);

	g_print("Loading from file: %s\n", file);
	lseek (fd, 0, SEEK_SET);
	read (fd, buff, size);
	new_data = g_new0(simul_data_t, 1);
	if(get_simulation_from_string(new_data, buff) == NULL)
		g_free(new_data);
	else {
		free_CPU_simulation_data(old_data);
		proc->simul_data = new_data;
	}
	g_free(buff);
	close(fd);
}
void write_CPU_prop_file(proc_t *proc)
{
	const gchar *file = select_file();
	GString *string;
	int fd;
	if (!file)
		return;

	fd = creat (file, 0666);
	g_print("Saving to file: %s\n",  file);
	string = get_proc_in_gstring(proc);
	write(fd, string->str, string->len);
	g_string_free (string, TRUE);
	close(fd);
}

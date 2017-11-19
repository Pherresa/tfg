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
#define _BSD_SOURCE
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <gnome.h>
#include <glade/glade.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include <CLOCK/main.h>
#include <messaging.h>
#include <MEM/main.h>
#include <CPU/main.h>
#include <IO/main.h>
#include <IO/geometry.h>
#include <REQUESTOR/main.h>
#include <SCHED.h>
#include <events.h>

#include "interface.h"

void spawn (subsystem_t subsystem, void (*func)(void), 
	    int *argc, char **argv[]);


static gchar *xml_file=NULL;
gchar *get_xml_file(void)
{
	return xml_file;
}
static gchar *DISPLAY [N_SUBSYSTEMS];
struct poptOption options[] = {
	{"CPU-display", '\0', POPT_ARG_STRING, &DISPLAY[CPU], 0,
	 "X display to use for CPU", "DISPLAY"},
	{"MEM-display", '\0', POPT_ARG_STRING, &DISPLAY[MEM], 0,
	 "X display to use for MEM", "DISPLAY"},
	{"CLOCK-display", '\0', POPT_ARG_STRING, &DISPLAY[CLOCK], 0,
	 "X display to use for CLOCK", "DISPLAY"},
	{"REQUESTOR-display", '\0', POPT_ARG_STRING, &DISPLAY[REQUESTOR], 0,
	 "X display to use for REQUESTOR", "DISPLAY"},
	{"IO-display", '\0', POPT_ARG_STRING, &DISPLAY[IO], 0,
	 "X display to use for IO", "DISPLAY"},
	{NULL, '\0', 0, NULL, 0, NULL, NULL}
};
static void quit(void)
{
	system_event(SYS_EVENT_QUITTING, NULL);
	gtk_main_quit();
}
void spawn (subsystem_t subsystem, void (*func)(void), 
		int *argc, char **argv[])
{
	mesg_pre_setup(subsystem);

	if (fork() == 0) { /* child side */
		if(DISPLAY[subsystem])
			setenv("DISPLAY", DISPLAY[subsystem], TRUE);
		gnome_init_with_popt_table(PACKAGE, VERSION, *argc, *argv,
					    options, 0, NULL);
		glade_gnome_init();
		sched_init();
		mesg_callback_register(MISC_QUIT,
				       (receive_callback)quit);
		func();
		mesg_quit(subsystem);
		exit(0);
	} else { /* parent side */
		mesg_messanger_setup(subsystem);
	}
}
int main (int argc, char *argv[])
{
	GString *str = g_string_new(argv[0]);
	gchar *last_slash=rindex(argv[0], '/');

	/* Initialize the i18n stuff */
	bindtextdomain (PACKAGE, GNOMELOCALEDIR);
	textdomain (PACKAGE);

	gnomelib_init(PACKAGE, VERSION);
	gnomelib_register_popt_table (options, "General Options");
	gnomelib_parse_args (argc, argv, 0);

	if(last_slash != NULL)
		g_string_truncate(str, last_slash - argv[0]);
	else
		g_string_assign(str, ".");
	g_string_append(str, "/VisualOS.glade");
	
	if(access(str->str, R_OK) == -1)
		g_string_sprintf(str, PKGDATADIR "/VisualOS.glade");

	xml_file = str->str;
	g_string_free(str, FALSE);
	init_IO_geometry();
	do {
		spawn (IO, IO_main, &argc, &argv);
		spawn (MEM, MEM_main, &argc, &argv);
		spawn (CPU, CPU_main, &argc, &argv);
		spawn (CLOCK, CLOCK_main, &argc, &argv);
		spawn (REQUESTOR, REQUESTOR_main, &argc, &argv);
	} while (mesg_loop()==TRUE);
	return 0;
}






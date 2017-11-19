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
#include <gnome.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <signals.h>
#include <messaging.h>

RETSIGTYPE mesg_sigio_handler (int sig);
RETSIGTYPE subsystem_sigio_handler (int sig);

gint mesg_init(void);

/* this struct stores a callback for a given message type */
typedef struct mesg_callback_s {
	mesg_type_t type;
	receive_callback func;
} mesg_callback_t;
/* main_messaging_system variables */
gboolean reset_system = FALSE;
int messaging_socket [N_SUBSYSTEMS];
/* subsystem_names[] should be in sync with enum subsystem_e in messaging.h */
static gchar *subsystem_names[N_SUBSYSTEMS] = {
	N_("MESG"),
	N_("CPU"),
	N_("MEM"),
	N_("IO"),
	N_("CLOCK"),
	N_("REQUESTOR")};
fd_set all_socket_set;
gint running_subsystems=0;
/* subsystem variables */
int subsystem_socket;
mesg_callback_t *callback_list=NULL;
subsystem_t local_subsystem;
/* common variables */
int tmp_socket[2];
/* enoght */
/**
 * mesg_who_am_i:
 *
 * This is usefull so the code which is usable by all subsystems can find out
 * where it is executing.
 *
 * This is specialy usefull in combination with @mesg_subsystem_name.
 * 
 * returns: the identification number of the current subsystem.
 */
subsystem_t mesg_who_am_i(void)
{
	return local_subsystem;
}
/**
 * mesg_subsystem_name:
 * @subsystem: the identification number of a subsystem.
 *
 * The string returned should not be modified of freed.
 *
 * returns: a string with the name of the subsystem.
 */
gchar *mesg_subsystem_name(subsystem_t subsystem)
{
	if (subsystem >= N_SUBSYSTEMS){
		g_warning("mesg_subsystem_name: unknown subsystem\n");
		return _("unknown");
	}
	return _(subsystem_names[subsystem]);
}
/**
 * call_func:
 * @callback: list of callback functions.
 * @m: a message.
 * 
 * Calls all functions asocieted to the type of @m in @callback.
 * 
 * returns: the number or called functions.
 */
static gint call_func(mesg_callback_t callback[], message_t *m)
{
	gint i=0;
	gint j=0;

	if (callback==NULL)
		return j;
	for (i=0; callback[i].func != NULL; ++i){
		if (callback[i].type == m->type){
			callback[i].func(m);
			++j;
		}
	}
	return j;
}
/**
 * full_recv:
 * @fd: file descriptor.
 * @buff: data pointer.
 * @size: size of the data.
 * @flags: flags for function recv.
 * 
 * Does not return until it reads size bytes from fd, an error occurs
 * or we don't get any data on "retries" retries, this is because in IRIX
 * a disconnected socket is select(ed) as having data to be read so if we can't
 * manage to read any dada and buggy-select is enabled we asum the "bug" is
 * trigerd and return with an error
 */
static gint full_recv (int fd, void* buff, int size, int flags)
{
	const gint retries = 5;
	gint count=0;
	gint rest=size;
	gint ret;
	gint i=0;
	gboolean warned = FALSE;

	while (count < size){
		ret = recv(fd, (gint8 *)buff+count, rest, flags);
		if (ret < 0){
			perror("full_recv");
			return -1;
		}
		count += ret;
		rest = size - count;

		if ((i++ > retries)&&(count==0)&&(!warned)){
			g_warning("%s: could not get any data in %d retries",
					mesg_subsystem_name(local_subsystem),
					retries);
#ifndef BUGGY_SELECT
			warned = TRUE;
			g_warning("%s: I will keep trying.",
				mesg_subsystem_name(local_subsystem));
			g_warning("If I am not working try activating "
				"buggy_select workaround");
#else
			return -1;
#endif
		}
	}
	return count;
}
/**
 * messanger_send:
 * @m: message to be send.
 * @flags: flags for function send.
 *
 * Sends @m from the messager to the apropriate subsystem.
 *
 * returns: whatever send returns.
 */
static gint messanger_send(message_t *m, unsigned int flags)
{
#ifdef MESG_DEBUG
	g_print("sigio_handler: sending message from %s to %s\n",
			mesg_subsystem_name(m->sender),
			mesg_subsystem_name(m->dest));
#endif
	if (messaging_socket[m->dest] == NOSUBSYSTEM){
		g_print("sigio_handler: message send to a "
				"nonexistent subsystem %s\n",
				mesg_subsystem_name(m->dest));
		return -1;
	}
	return send(messaging_socket[m->dest], m, sizeof(*m), 0);

}
/**
 * messanger_broadcast:
 * @m: message to be sent.
 * @flags: flags for function send.
 *
 * Sends @m from the messager to all subsystems except to the one that
 * originaly sent it.
 *
 * returns: nothing important.
 */
static gint messanger_broadcast(message_t *m, unsigned int flags)
{
	gint i;
	
	for (i=0; i < N_SUBSYSTEMS; ++i){
		if ((messaging_socket[i] != NOSUBSYSTEM)&&(i != m->sender)){
			m->dest = i;
			messanger_send(m, flags);
		}
	}
	return 0;
}
/** 
 * process_message:
 * @m: the message.
 * 
 * Process a message sent to the messanger.
 */
static void process_message(message_t *m)
{
	switch (m->type){ 
	case MESG_QUIT: 
		FD_CLR(messaging_socket [m->sender], &all_socket_set);
		close (messaging_socket [m->sender]);
		messaging_socket[m->sender] = NOSUBSYSTEM;
		--running_subsystems;
		break; 
	case MESG_RESET_SYSTEM:
		reset_system = TRUE;
		m->sender = MESG;
		m->dest = ALL;
		m->type = MISC_QUIT;
		messanger_broadcast(m, 0);
		break;
		default: 
			g_warning("%s: don't know what to do with "
					"message type %d",
					mesg_subsystem_name(local_subsystem),
					m->type);
	}
}
/**
 * mesg_sigio_handler:
 * @sig: signal number.
 * 
 * Funcion to register for SIGIO signal in the messanger side
 * will read messages on all sockets until not available 
 */
RETSIGTYPE mesg_sigio_handler (int sig)
{
	message_t m;
	int i;
	fd_set socket_set = all_socket_set;
	struct timeval timeout ={0,0};

	while(select (FD_SETSIZE, &socket_set, NULL, NULL, &timeout) > 0){
		for (i=0; i < N_SUBSYSTEMS; ++i){
			if (messaging_socket[i]==NOSUBSYSTEM) continue;
			if (FD_ISSET(messaging_socket[i], &socket_set)){
				int ret;
				ret = full_recv(messaging_socket[i],
						&m, sizeof(m), 0);
				if(ret < 0) return;
				break;
			}
		}
	
		if (m.dest==local_subsystem)
			process_message(&m);
		else if (m.dest == ALL)
			messanger_broadcast(&m, 0);
		else
			messanger_send(&m, 0);

		socket_set = all_socket_set;
	}
	return;
}
/**
 * subsystem_sigio_handler:
 * @sig: signal number.
 * 
 * Funcion to register for SIGIO signal in the subsystems side, 
 * will read messages until not available 
 */
RETSIGTYPE subsystem_sigio_handler (int sig)
{
	fd_set fdset, fdset_orig;
	message_t m;
	struct timeval timeout ={0,0};

	FD_ZERO(&fdset_orig);
	FD_SET(subsystem_socket, &fdset_orig);
	fdset=fdset_orig;
	
	while (select (FD_SETSIZE, &fdset, NULL, NULL, &timeout) > 0){

		if (full_recv(subsystem_socket, &m, sizeof(m), 0) < 0)
			return;
		if (call_func(callback_list, &m) <= 0){
#ifdef MESG_DEBUG
			g_print("mesg: message type %d not "
					"registered for subsystem %s\n",
					m.type,
					mesg_subsystem_name(local_subsystem));
#endif
		}
		fdset=fdset_orig;
	}

}
/**
 * mesg_init:
 *
 * Initializes messaging code. Should be called only one before any forking
 * happens.
 *
 * returns: nothing important.
 */
gint mesg_init(void)
{
	gint i;
#ifdef MESG_DEBUG
	g_print("mesg_init: called\n");
#endif
	FD_ZERO(&all_socket_set);
	for (i=0; i<N_SUBSYSTEMS; ++i)
		messaging_socket[i]=NOSUBSYSTEM;
	local_subsystem = MESG;
	return 0;
}
/**
 * mesg_pre_setup:
 * @subsystem: the subsystem ID.
 * 
 * Should be called before the fork.
 * It registers the subsystem and gets ready for comunication. 
 *
 * returns: nothing important.
 */
gint mesg_pre_setup(subsystem_t subsystem)
{
	static gint init=FALSE;
	if (!init){
		mesg_init();
		init = TRUE;
	}
	++running_subsystems;
#ifdef MESG_DEBUG
	g_print("mesg_pre_setup: called for subsystem %s\n",
			mesg_subsystem_name(subsystem));
#endif
	if (socketpair (AF_UNIX, SOCK_STREAM, 0, tmp_socket) != 0)
		perror("mesg_pre_setup");
	subsystem_socket = tmp_socket[0];
	messaging_socket[subsystem] = tmp_socket[1];
#ifdef MESG_DEBUG
	g_print("mesg_pre_setup: sokets: subsystem:%s mesg:%d other:%d\n",
			mesg_subsystem_name(subsystem),
			messaging_socket[subsystem], subsystem_socket);
#endif
	FD_SET (tmp_socket[1], &all_socket_set);
	return 0;
}

/**
 * mesg_messanger_setup:
 * @subsystem: the subsystem ID.
 *
 * Should be called after the fork on the parent side.
 * Finishes setting up comunications.
 *
 * returns: nothing important.
 */
gint mesg_messanger_setup(subsystem_t subsystem)
{
#ifdef MESG_DEBUG
	g_print("mesg_messanger_setup: called for subsystem %s\n",
			mesg_subsystem_name(subsystem));
#endif
	if(close(subsystem_socket) < 0){
		fprintf(stderr, "(%d:%d)", getpid(),subsystem_socket);
		perror("socket close");
	}
	setup_async_io (messaging_socket[subsystem], mesg_sigio_handler);
	return 0;
}
/**
 * mesg_callback_register:
 * @type: message type.
 * @func: function to be called.
 * 
 * Registers the function @func to be called when messages of
 * type @type are received.
 *
 * returns: nothing important.
 */
gint mesg_callback_register(mesg_type_t type, receive_callback func)
{
	static gint n_callbacks=0;
	
	callback_list = g_renew(mesg_callback_t, callback_list,
				++n_callbacks+1);
	callback_list[n_callbacks-1].type = type;
	callback_list[n_callbacks-1].func = func;
	callback_list[n_callbacks].type = 0;
	callback_list[n_callbacks].func = NULL;
	return 0;
}
/**
 * mesg_subsystem_setup:
 * @subsystem: the subsystem ID.
 * @flags: could be MESG_WITH_GTK or 0.
 *
 * Should be called after the fork on the child side.
 * Finishes setting up comunications.
 * 
 * If @flags is MESG_WITH_GTK GTK I/O monitoring facilities will be used.
 *
 * NOTE: GTK+ I/O monitoring facilities can only be used if the subsystem
 * uses GTK+ and calls @gtk_main.
 *
 * returns: nothing important.
 */
gint mesg_subsystem_setup(subsystem_t subsystem, gint flags)
{
	gint i;
#ifdef DEBUG
	g_print("mesg_subsystem_setup: called from subsystem %s (%d)\n",
			mesg_subsystem_name(subsystem), getpid());
#endif
	local_subsystem = subsystem;
	for (i=0; i<N_SUBSYSTEMS; ++i)
		if (messaging_socket[i] != NOSUBSYSTEM){
			if (close(messaging_socket[i]) < 0){
				fprintf(stderr, "(%d:%d)", 
						getpid(),messaging_socket[i]);
				perror("socket close");
			}
		}
	/* use GTK I/O monitoring facilities in subsystems which use GTK
	 * and signal driven I/O in the rest */
	if (flags & MESG_WITH_GTK){
		gdk_input_add(subsystem_socket,
				GDK_INPUT_READ,
				(GdkInputFunction) subsystem_sigio_handler,
				NULL);
#ifdef MESG_DEBUG
		g_print("mesg_subsystem_setup: subsystem %s using GTK "
				"facilities\n",
				mesg_subsystem_name(subsystem));
#endif
	} else {
		setup_async_io (subsystem_socket,
				subsystem_sigio_handler);
#ifdef MESG_DEBUG
		g_print("mesg_subsystem_setup: subsystem %s using signal driven"
				" IO\n",
				mesg_subsystem_name(subsystem));
#endif
	}
	return 0;
}
/**
 * mesg_broadcast:
 * @type: type for the message.
 * @data: data for the message.
 * @size: size of @data.
 *
 * Send a message from one subsystem to all the other subsystems.
 *
 * returns: nothing important.
 */
gint mesg_broadcast (mesg_type_t type, const gpointer data, gint size)
{
	if (size > MESSAGE_MAXSIZE){
		g_print("mesg_broadcast: message size(%d) "
				"bigger than maximum(%d)\n", 
				size, MESSAGE_MAXSIZE);
		g_print("mesg_broadcast: message not sent\n");
		return -1;
	} else {
		message_t m;

		m.sender = local_subsystem;
		m.dest = ALL;
		m.data_size = size;
		m.type = type;
		if (size > 0)
			memcpy (m.data, data, size);
#ifdef MESG_DEBUG
		g_print("%s: sending message to ALL\n",
				mesg_subsystem_name(local_subsystem));
#endif
		send(subsystem_socket, &m, sizeof(m), 0); 
	}
	return 0;
}
/**
 * mesg_send:
 * @dest: target subsystem ID.
 * @type: type for the message.
 * @data: data for the message.
 * @size: size of @data.
 *
 * Send a message from one subsystem to another.
 *
 * returns: nothing important.
 */
gint mesg_send(subsystem_t dest, mesg_type_t type, const gpointer data,
		gint size)
{
	if (size > MESSAGE_MAXSIZE){
		g_print("mesg_send: message size(%d) "
				"bigger than maximum(%d)\n", 
				size, MESSAGE_MAXSIZE);
		g_print("mesg_send: message not sent\n");
		return -1;
	} else {
		message_t m;

		m.sender = local_subsystem;
		m.dest = dest;
		m.data_size = size;
		m.type = type;
		if (size > 0)
			memcpy (m.data, data, size);
#ifdef MESG_DEBUG
		g_print("%s: sending message to %s\n",
				mesg_subsystem_name(local_subsystem),
				mesg_subsystem_name(dest));
#endif
		send(subsystem_socket, &m, sizeof(m), 0); 
	}
	return 0;
}
/**
 * mesg_loop:
 *
 * Function to be called last on the messanger process, it will handle messages
 * between the different subsystems and won't return until the system is reset
 * or terminated.
 *
 * returns: TRUE if the system should be reset insted of terminated.
 */
gboolean mesg_loop(void)
{
	reset_system = FALSE;
	signal(SIGCHLD, SIG_IGN);
	while (running_subsystems > 0)
		sleep(3);
	return reset_system;
}
/**
 * mesg_quit:
 * @subsystem: ID of the subsystem.
 * 
 * Should be called by all subsystems before quitting.
 *
 * returns: nothing important.
 */
gint mesg_quit (subsystem_t subsystem)
{
	return mesg_send(MESG, MESG_QUIT, NULL, 0);
}
/**
 * mesg_block:
 * 
 * In case GTK I/O monitoring facilities are not been used, this function
 * can be used to protect critical sections of code from been interrupted
 * by I/O signals.
 * 
 * returns: nothing important.
 */
gint mesg_block (void)
{
	block_io_signals();
	return 0;
}
/**
 * mesg_unblock:
 *
 * Restarts normal messaging behaviour after calling @mesg_block.
 *
 * returns: nothing important.
 */
gint mesg_unblock (void)
{
	unblock_io_signals();
	return 0;
}

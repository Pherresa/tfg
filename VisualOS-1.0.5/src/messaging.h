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

#ifndef MESSAGING_H
#define MESSAGING_H
#include <glib.h>

/* enum subsystem_e should be in sync with names[] in messaging.c */
typedef enum {		/* identification for the subsystems will also be used
			   as the higher byte for message types */
	ALL=-2,		/* broadcast messages */
	NOSUBSYSTEM=-1,	/* used within the messaging code to indicate that
			   there is no subsystem */
	MESG=0,		/* messages to be handled by the messager */
	CPU,		/* CPU subsystem */
	MEM,		/* Memory subsystem */
	IO,		/* I/O subsystem */
	CLOCK,		/* Clock subsystem */
	REQUESTOR,	/* Requestor subsystem */
	N_SUBSYSTEMS,	/* used within the messaging code as the total number
			   of subsystems */
	MISC_MESSAGES	/* messages that don't belong to any subsystem in 
			   particular */
} subsystem_t;
/* message types for MESG subsystem */
typedef enum {			/* message types to be handled by the
				   messager */
	MESG_QUIT=MESG<<8,	/* each subsystem should send this message
				   to the messenger before quiting */
	MESG_RESET_SYSTEM	/* send by the CPU tells the messager to
				   terminate all subsystems and restart
				   the system */
} mesg_message_type_t;
typedef enum {				/* message types not belonging to a
					   particular subsystem */
	MISC_SHOW=MISC_MESSAGES<<8,	/* tells a subsystem to show it's
					   main window */
	MISC_QUIT			/* tells a subsystem to quit */
} misc_message_type_t;
typedef gint32 mesg_type_t;
#define MESSAGE_MAXSIZE 8	/* maximun size of the data to send in a message */
typedef struct {			/* struct describing a message */
	subsystem_t sender;		/* sender of the message */
	subsystem_t dest;		/* target of the message */
	mesg_type_t type;		/* type of message */
	gint data_size;			/* size of the data */
	gint8 data[MESSAGE_MAXSIZE];	/* data of the message */
} message_t;

/**
 * receive_callback:
 * @m: message.
 *
 * Function pointer type to be used in mesg_callback_register.
 */
typedef void (*receive_callback)(const message_t *m);

gint mesg_pre_setup (subsystem_t subsystem);
gint mesg_messanger_setup (subsystem_t subsystem);
gint mesg_subsystem_setup (subsystem_t subsystem, gint flags);
enum mesg_subsystem_setup_flags {MESG_WITH_GTK=1};
gint mesg_quit (subsystem_t subsystem);
subsystem_t mesg_who_am_i(void);
/* returns a symbolic string corresponting to the subsystem specified */
gchar *mesg_subsystem_name(subsystem_t subsystem);
gint mesg_send (subsystem_t dest, mesg_type_t type,
		const gpointer data, gint size);
gint mesg_broadcast (mesg_type_t type, const gpointer data, gint size);
gboolean mesg_loop (void);
gint mesg_callback_register(mesg_type_t type, receive_callback func);
gint mesg_block (void);
gint mesg_unblock (void);

#endif

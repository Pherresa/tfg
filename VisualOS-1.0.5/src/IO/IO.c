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
#include <stdlib.h>
#include <gnome.h>

#include <messaging.h>
#include <CLOCK/CLOCK.h>

#include "algorithms/algorithms.h"
#include "simulation.h"
#include "io_config.h"
#include "status.h"
#include "geometry.h"
#include "info.h"
#include "IO.h"


static block_ready_callback_t io_block_ready_callback = NULL;

struct io_request_message {
	gint block;
	gint flags;
};

static void io_block_ready(const message_t *m)
{
	g_return_if_fail(io_block_ready_callback != NULL);

	io_block_ready_callback(*(gint *)(m->data));
}
/**
 * io_register_block_ready:
 * @func: function to be called when a requested block access is finished.
 *
 * Instructs the IO subsystem to call @func when a block access is finished.
 *
 * returns: nothing important.
 */
gint io_register_block_ready(block_ready_callback_t func)
{
	io_block_ready_callback = func;
	return mesg_callback_register(BLOCK_READY, io_block_ready);
}
static void io_request_block_with_flags(gint block, guint flags)
{
	struct io_request_message message;
	message.block = block;
	message.flags = flags;
	
	mesg_send(IO, REQUEST_BLOCK, &message, sizeof(message));
}
/**
 * io_request_block:
 * @block: data block to be accessed.
 *
 * Instructs the IO subsystem to accesses @block from the data area.
 */
void io_request_block(gint block)
{
	io_request_block_with_flags(block, 0);
}
/**
 * io_request_swap_block:
 * @block: swap block to be accessed.
 *
 * Instructs the IO subsystem to accesses @block from the swap area.
 */
void io_request_swap_block(gint block)
{
	io_request_block_with_flags(block, IO_REQUEST_FLAGS_SWAP);
}
GString *str = NULL;

static void io_request_block_server(const message_t *m)
{
	io_request_t *request = g_new(io_request_t, 1);
	struct io_request_message *message=(struct io_request_message *)m->data;

	request->client= m->sender;
	request->flags= message->flags;
	request->block= message->block;
	request->track= IO_request_track(request);

	if(IO_config->disabled){
		io_block_ready_server(request);
		return;
	}

	if(IO_config->stop_clock)
		CLOCK_stop();
	g_string_sprintf(str, _("Block %d (Track %d) was requested"),
			 request->block, request->track);
	IO_status_set(str->str);

	IO_algorithm_event(request);
	return;
}
/**
 * io_server_init:
 *
 * Called from the subsystem's code to initialize the interface code.
 *
 * returns: nothing important.
 */
gint io_server_init(void)
{
	str = g_string_new(NULL); 
	return mesg_callback_register(REQUEST_BLOCK, io_request_block_server);
}
/**
 * io_block_ready_server:
 * @io_request: the fulfilled request.
 *
 * Called from the subsystem's code to tell the client about a fulfilled 
 * request.
 *
 * returns: nothing important.
 */
gint io_block_ready_server(io_request_t *io_request)
{
	if(IO_config->stop_clock)
		CLOCK_stop();
	g_string_sprintf(str, _("Read block %d (Track %d)"), io_request->block,
			 io_request->track);
	IO_status_set(str->str);
	return mesg_send(io_request->client, BLOCK_READY,
			&(io_request->block), sizeof(io_request->block));
}

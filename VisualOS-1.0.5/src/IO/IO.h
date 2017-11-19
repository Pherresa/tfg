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

#ifndef IO_H
#define IO_H
#include <messaging.h>
enum {REQUEST_BLOCK=IO<<8, BLOCK_READY};
enum {IO_REQUEST_FLAGS_SWAP=1};

typedef struct {
	subsystem_t client;
	guint flags;
	gint track;
	gint block;
} io_request_t;

#include <IO/geometry.h>
/**
 * block_ready_callback_t:
 * @block: block number of the fulfilled IO access.
 *
 * Function pointer type for the callback used on @io_register_block_ready.
 */
typedef void (*block_ready_callback_t)(gint block);
gint io_register_block_ready(block_ready_callback_t func); 
void io_request_block(gint block);
void io_request_swap_block(gint block);
gint io_server_init(void);
gint io_block_ready_server(io_request_t *io_request);
#endif

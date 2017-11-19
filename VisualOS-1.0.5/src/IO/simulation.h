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

#ifndef IO_SIMULATION_H
#define IO_SIMULATION_H
#include <glib.h>

#include <messaging.h>

#include "queues.h"
#include "IO.h"

gint init_IO_simulation (void);
gint IO_algorithm_event(io_request_t *request);
gint get_IO_head_pos(void);
io_queue_t get_IO_reading_queue(void);
void set_IO_reading_queue(io_queue_t new_reading);
io_queue_t get_IO_requested_queue(void);
#endif

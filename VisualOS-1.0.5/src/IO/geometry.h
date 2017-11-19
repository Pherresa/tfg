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

#ifndef IO_GEOMETRY
#define IO_GEOMETRY
#include <IO/IO.h>
gint get_IO_blocks_per_track(void);
gint get_IO_max_data_block(void);
gint get_IO_max_swap_block(void);
gint get_IO_ntracks(void);
gint get_IO_last_data_track(void);
gint IO_request_track(io_request_t *request);
void init_IO_geometry(void);
#endif

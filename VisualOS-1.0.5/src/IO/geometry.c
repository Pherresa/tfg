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

#include <IO/IO.h>

#include "geometry.h"

/* number of tracks in the disk */
static gint num_tracks=50;
static gint blocks_per_track=20;
static gint last_data_track=25;
static gint max_data_block;
static gint max_swap_block;
/**
 * init_IO_geometry:
 *
 * Initialices the geometry calculation code.
 */
void init_IO_geometry(void)
{
	max_data_block = last_data_track*blocks_per_track;
	max_swap_block = (num_tracks-last_data_track)*blocks_per_track;
}
/**
 * get_IO_max_data_block:
 *
 * returns: the maximun data block number.
 */
gint get_IO_max_data_block(void)
{
	return max_data_block;
}

/**
 * get_IO_max_swap_block:
 *
 * returns: the maximun swap block number.
 */
gint get_IO_max_swap_block(void)
{
	return max_swap_block;
}
/**
 * get_IO_blocks_per_track:
 *
 * returns: the number of blocks per track.
 */
gint get_IO_blocks_per_track(void)
{
        return blocks_per_track;
}
/**
 * get_IO_ntracks:
 *
 * returns: the number of tracks on the disk.
 */
gint get_IO_ntracks(void)
{
        return num_tracks;
}
/**
 * get_IO_last_data_track:
 *
 * returns: the last data track number.
 */
gint get_IO_last_data_track(void)
{
	return last_data_track;
}
/**
 * IO_request_track:
 * @request: IO request involved.
 *
 * Calculates the track of a certain @request.
 *
 * returns: the track number corespoinding to @request.
 */
gint IO_request_track(io_request_t *request)
{
        if (request->flags & IO_REQUEST_FLAGS_SWAP)
                return request->block / blocks_per_track + last_data_track +1;
        else
                return request->block / blocks_per_track;
}


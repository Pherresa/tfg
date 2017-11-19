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

/* this functions are inspired in the linux kernel include/asm-generic/bitops.h
 * */

#include <glib.h>

#include "bitops.h"
/**
 * set_bit:
 * @nr: index of the bit to be set to 1.
 * @addr: address of the bitmap.
 *
 * Set bit number @nr in the bitmap pointed by @addr.
 */
void set_bit(gint nr, guint32 * addr)
{
	guint32	mask;

	mask = 1 << nr;
	*addr |= mask;
}
/**
 * clear_bit:
 * @nr: index of the bit to be cleard to 0.
 * @addr: address of the bitmap.
 *
 * Clear bit number @nr in the bitmap pointed by @addr.
 */
void clear_bit(gint nr, guint32 * addr)
{
	guint32	mask;

	mask = 1 << nr;
	*addr &= ~mask;
}
/**
 * test_bit:
 * @nr: index of the bit to be tested.
 * @addr: address of the bitmap.
 *
 * Test if bit number @nr of the bitmap pointed by @addr is 1 or 0.
 *
 * returns: TRUE if the bit is 1 and FALSE otherwise.
 */
gboolean test_bit(gint nr, guint32 * addr)
{
	guint32	mask;

	mask = 1 << nr;
	return ((mask & *addr) != 0);
}

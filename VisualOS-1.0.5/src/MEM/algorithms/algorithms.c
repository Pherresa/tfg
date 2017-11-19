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
#include <stdio.h>

#include "algorithms.h"
#include "fifo.h"
#include "second_chance.h"
#include "clock.h"
#include "manual.h"

static GSList *algorithms = NULL;

/**
 * init_MEM_algorithms:
 * 
 * Initializes the MEM algorithms code.
 * 
 * Mainly will call init functions for each algorithm.
 *
 * returns: a pointer to the algorithm structs linked list.
 */
/* will call init functions for each algorithm and return a pointer
 * to the algorithm structs linked list */
GSList * init_MEM_algorithms(void)
{
	/* just in case this function gets called more than once only
	 * the first call will initialize the algorithms */
	if (algorithms==NULL){
		mem_fifo_init();
		mem_second_chance_init();
		mem_clock_init();
		mem_manual_init();
	}
	return algorithms;
}

/**
 * register_MEM_algorithm:
 * @algorithm: algorithm struct to register.
 *
 * Each algorithm should call this function in it's initialization function
 * to register its algorithm struct.
 *
 * returns: nothing important.
 */
void register_MEM_algorithm (mem_algorithm_t * algorithm)
{
	algorithms = g_slist_append(algorithms, algorithm);
}

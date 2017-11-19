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
#include "fcfs.h"
#include "sstf.h"
#include "scan.h"
#include "nsscan.h"
#include "manual.h"
#include "test.h"

static GSList *IO_algorithms = NULL;

/**
 * init_IO_algorithms:
 * 
 * Initializes the IO algorithms code.
 * 
 * Mainly will call init functions for each algorithm.
 *
 * returns: a pointer to the algorithm structs linked list.
 */
GSList * init_IO_algorithms(void)
{
	/* just in case this function gets called more than once only
	 * the first call will initialize the algorithms */
	if (IO_algorithms==NULL){
		io_fcfs_init();
		io_sstf_init();
		io_scan_init();
		io_nsscan_init();
		io_manual_init();
		io_test_init();
	}
	return IO_algorithms;
}

/**
 * register_IO_algorithm:
 * @algorithm: algorithm struct to register.
 *
 * Each algorithm should call this function in it's initialization function
 * to register its algorithm struct.
 *
 * returns: nothing important.
 */
gint register_IO_algorithm (io_algorithm_t * algorithm)
{
	IO_algorithms = g_slist_append(IO_algorithms, algorithm);
	return 0;
}

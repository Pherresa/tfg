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

#ifndef _CPU_H
#define _CPU_H
#include <glib.h>

#include <messaging.h>
#include <process.h>

enum {PROC_CREAT=CPU<<8, PROC_FINISH, PROC_TERMINATE};

/** proc_creat_callback_t:
 * @pid: Process ID of the created process.
 *
 * Function pointer type for the callback used on @cpu_register_proc_creat.
 *
 */

typedef void (*proc_creat_callback_t)(gint pid);

/** proc_creat_callback_t:
 * @pid: Process ID of the terminated process.
 *
 * Function pointer type for the callback used on @cpu_register_proc_finish.
 *
 */

typedef void (*proc_finish_callback_t)(gint pid);

void cpu_register_proc_creat(proc_creat_callback_t func); 
void cpu_register_proc_finish(proc_finish_callback_t func); 
void CPU_terminate_proc(gint pid);

void cpu_server_init(void);
#endif


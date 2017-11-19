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
#define _POSIX_SOURCE
#define _BSD_SOURCE
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <fcntl.h>
#include <glib.h>
#include <errno.h>
#include <signal.h>

#include <signals.h>

/* sets up SIGIO and SIGURG to be received by the calling process
 * refering to the specified file descriptor (fd) calling handler
 * when SIGIO is receiced */
gint setup_async_io (int fd, void (*handler)(int))
{
	int flags;
	struct sigaction act;


	act.sa_handler = handler;
	sigemptyset(&act.sa_mask);
	sigaddset(&act.sa_mask, SIGURG);
	sigaddset(&act.sa_mask, SIGIO);
	act.sa_flags = 0;
	sigaction( SIGIO, &act, NULL);

	if (fcntl (fd, F_SETOWN, getpid()) == -1)
		perror("F_SETOWN");

	flags = fcntl (fd, F_GETFL);
	if (flags == -1)
		perror("F_GETFL");
	flags |= O_ASYNC;
	if (fcntl (fd, F_SETFL, flags) == -1)
		perror("F_SETFL");
	return 0;
}
/* blocks IO signals */
void block_io_signals (void)
{
	sigset_t sigset;

	sigemptyset(&sigset);
	sigaddset(&sigset, SIGURG);
	sigaddset(&sigset, SIGIO);

	while ((sigprocmask(SIG_BLOCK, &sigset, NULL)==-1) && (errno == EINTR));
}
/* unblocks IO signals */
void unblock_io_signals (void)
{
	sigset_t sigset;

	sigemptyset(&sigset);
	sigaddset(&sigset, SIGURG);
	sigaddset(&sigset, SIGIO);

	while ((sigprocmask(SIG_UNBLOCK, &sigset, NULL)==-1) 
			&& (errno == EINTR));
}

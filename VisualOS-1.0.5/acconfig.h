/* VisualOS is and educational visual simulator of an operating system.   
   Copyright (C) 2000,2003 Manuel Estrada Sainz <ranty@debian.org>
   
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

/* included by automake */
#undef PACKAGE
#undef VERSION
#undef ENABLE_NLS
#undef HAVE_CATGETS
#undef HAVE_GETTEXT
#undef HAVE_LC_MESSAGES
#undef HAVE_STPCPY
/* print general debuging messages */
#undef DEBUG
/* print messaging debuging messages */
#undef MESG_DEBUG
/* use reentrant versions of funtions on C library */
#undef _REENTRANT
/* O_ASYNC fcntl flags many not exist */
#undef O_ASYNC
/* select function on some IRIX (at least mips-sgi-irix6.2) reports a just
 * disconnected socket as having pending data to be read */
#undef BUGGY_SELECT

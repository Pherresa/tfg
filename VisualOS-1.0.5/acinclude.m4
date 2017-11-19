#
# Check to make sure that we have O_ASYNC flags in fcntl.h and other wise
# try to use FASYNC.
#

AC_DEFUN(CHECK_O_ASYNC_IN_FCNTL_H,
[
AC_MSG_CHECKING([whether fcntl.h defines O_ASYNC flag])
AC_EGREP_CPP(yes,
[#include <fcntl.h>
 #ifdef O_ASYNC
  yes
 #endif
],AC_MSG_RESULT(yes)
have_O_ASYNC=yes,
AC_MSG_RESULT(no)
have_O_ASYNC=no)
if test "x$have_O_ASYNC" = "xno"; then
	AC_MSG_CHECKING([whether fcntl.h defines FASYNC flag])
	AC_EGREP_CPP(yes,
	[#include <fcntl.h>
	 #ifdef FASYNC
	  yes
	 #endif
	],AC_MSG_RESULT(yes)
	AC_DEFINE(O_ASYNC, FASYNC),
	AC_MSG_RESULT(no)
	echo "***** the program will not compile on this system ******"
	exit 1)
fi
])

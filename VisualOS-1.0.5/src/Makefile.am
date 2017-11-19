include $(top_srcdir)/src/Make.defaults
EXTRA_DIST = VisualOS.glade \
	io_editor_disk.xpm \
	libglade_strings.h \
	Make.defaults

SUBDIRS = CPU CLOCK IO MEM REQUESTOR

pkgdata_DATA = \
	VisualOS.glade \
	io_editor_disk.xpm

bin_PROGRAMS = VisualOS
VisualOS_LDADD = \
	CPU/libcpu.a\
	CPU/editor/libsimulation.a\
	CPU/algorithms/libalgorithms.a\
	CPU/drawings/libdrawings.a\
	IO/libio.a\
	IO/algorithms/libalgorithms.a\
	IO/drawings/libdrawings.a\
	CLOCK/libclock.a\
	MEM/libmem.a\
	MEM/drawings/libdrawings.a\
	MEM/algorithms/libalgorithms.a\
	REQUESTOR/librequestor.a
VisualOS_SOURCES = \
	bitops.h\
	bitops.c\
	SCHED.c \
	SCHED.h\
	VisualOS.c \
	messaging.c \
	messaging.h\
	process.c \
	process.h\
	property_widgets.c \
	property_widgets.h\
	signals.c\
	signals.h\
	drawing.c\
	drawing.h\
	util.c\
	util.h\
	gdk-helper.h\
	gdk-helper.c\
	events.h\
	events.c\
	interface.h\
	file-dialog.h\
	file-dialog.c

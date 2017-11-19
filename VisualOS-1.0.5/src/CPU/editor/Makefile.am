include $(top_srcdir)/src/Make.defaults
noinst_LIBRARIES = libsimulation.a
libsimulation_a_SOURCES = \
	util.h\
	editor.c\
	editor.h\
	io.c\
	io.h\
	general.c\
	general.h\
	advanced.c\
	advanced.h\
	memory.c\
	memory.h\
	parser.tab.c\
	dump.c\
	parser.h\
	file.c\
	file.h

EXTRA_DIST=parser.y
parser.tab.c: parser.y
	bison parser.y

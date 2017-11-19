EXTRA_DIST = project.tex
CLEANFILES = project.ps project.dvi project.aux project.log
all: project.ps project.dvi
project.ps: project.dvi
	dvips $^ -o $@
project.dvi: project.tex
	latex $^ 

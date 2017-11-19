## Process this file with automake to produce Makefile.in

SUBDIRS=screen_shots

DOCBOOK2X_OPTS=-e no-idref

%.ps: %.sgml
	$(MAKE) eps -C screen_shots
#	db2ps $(DOCBOOK2X_OPTS) $<
	SP_ENCODING=$(SP_ENCODING) \
	jw -f docbook -b ps $(DOCBOOK2X_OPTS) \
		-d print-eps.dsl \
			$<
#	rm -f `expr $^ : '\(.*\)\.sgml'`.{aux,dvi,log,tex}
%.pdf: %.sgml #%.ps #%.sgml
#	ps2pdf $^ $@
	$(MAKE) png -C screen_shots
	SP_ENCODING=$(SP_ENCODING) \
	jw -f docbook -b pdf $(DOCBOOK2X_OPTS) \
		-d print-png.dsl \
			$<
#	rm -f `expr $^ : '\(.*\)\.sgml'`.{aux,dvi,log,tex}

%.txt: %.sgml
	sgmltools -b txt $<
#	jw -f docbook -b txt $(DOCBOOK2X_OPTS) \
#		-d html-png.dsl \
#			$<

%: %.sgml
	$(MAKE) png -C screen_shots
#	db2html $(DOCBOOK2X_OPTS) $^
	SP_ENCODING=$(SP_ENCODING) \
	jw -f docbook -b html $(DOCBOOK2X_OPTS) \
		-d html-png.dsl \
		-o $@ \
			$<
	ln -fs ../screen_shots $@

%.eps: %.dia
	dia $^ -e $@
%.png: %.dia
	dia $^ -e $@

man_MANS=VisualOS.1
man: 
	help2man -N -i VisualOS.man-include  VisualOS | \
	sed -e'/^\.TH/s/^[^"]*"."[^"]*"\([^"]*\)".*$$/.TH VisualOS "1" "\1" "VisualOS 1.0.4"/'  \
	> VisualOS.1
VisualOS.1: man

Diseño.pdf Diseño: Block-Diagram-real.png Block-Diagram.png
Diseño.ps: Block-Diagram-real.eps Block-Diagram.eps
Diseño: Diseño/Block-Diagram-real.png Diseño/Block-Diagram.png

Diseño/Block-Diagram-real.png:
	mkdir -p Diseño
	ln -fs ../Block-Diagram-real.png Diseño
Diseño/Block-Diagram.png:
	mkdir -p Diseño
	ln -fs ../Block-Diagram.png Diseño

#Should not require the user to have gtk-doc-tools installed
#VisualOS-docs $(addprefix VisualOS-docs., ps pdf txt): sgml
Manual-cs.ps Manual-cs Manual-cs.pdf: SP_ENCODING=ISO-8859-2


html-doc: Manual-es html Descripcion Bibliografia Lista_de_cambios \
	Especificacion_de_requisitos Diseño Resumen Manual-en Manual-cs

pdf-doc: Manual-es.pdf VisualOS-docs.pdf Descripcion.pdf Bibliografia.pdf \
	Lista_de_cambios.pdf Manual-en.pdf Manual-cs.pdf\
	Especificacion_de_requisitos.pdf Diseño.pdf Resumen.pdf 

ps-doc: Manual-es.ps VisualOS-docs.ps Descripcion.ps Bibliografia.ps \
	Lista_de_cambios.ps Manual-en.ps Manual-cs.ps\
	Especificacion_de_requisitos.ps Diseño.ps Resumen.ps

text-doc: VisualOS-docs.txt Descripcion.txt Bibliografia.txt \
	Lista_de_cambios.txt \
	Especificacion_de_requisitos.txt Diseño.txt Resumen.txt

all-doc: text-doc ps-doc pdf-doc html-doc

EXTRA_DIST= \
	html-png.dsl print-eps.dsl print-png.dsl \
	VisualOS-docs.sgml sgml tmpl VisualOS-docs.txt \
	Manual-es.sgml Manual-en.sgml Manual-cs.sgml \
	Descripcion.sgml Descripcion.txt \
	Bibliografia.sgml Bibliografia.txt \
	Especificacion_de_requisitos.sgml Especificacion_de_requisitos.txt \
	Lista_de_cambios.sgml Lista_de_cambios.txt \
	Diseño.sgml Diseño.txt \
	Resumen.sgml Resumen.txt \
	VisualOS.1 VisualOS.man-include \
	Block-Diagram-real.dia Block-Diagram.dia \
	Block-Diagram-real.png Block-Diagram.png \
	Block-Diagram-real.eps Block-Diagram.eps

CLEANFILES=Manual-es.pdf VisualOS-docs.pdf Descripcion.pdf Bibliografia.pdf \
	Lista_de_cambios.pdf Manual-en.pdf Manual-cs.pdf\
	Especificacion_de_requisitos.pdf Diseño.pdf Resumen.pdf \
	Manual-es.ps VisualOS-docs.ps Descripcion.ps Bibliografia.ps \
	Lista_de_cambios.ps Manual-en.ps Manual-cs.ps\
	Especificacion_de_requisitos.ps Diseño.ps Resumen.ps \
	VisualOS-docs.txt Descripcion.txt Bibliografia.txt \
	Lista_de_cambios.txt \
	Especificacion_de_requisitos.txt Diseño.txt Resumen.txt \
	*~ *.bak *.hierarchy *.signals *.args *-unused.txt *.out *.log *.aux \
	*.stamp


MAINTAINERCLEANFILES= \
	Block-Diagram-real.eps Block-Diagram-real.png \
	Block-Diagram.eps Block-Diagram.png



# The name of the module.
DOC_MODULE=VisualOS

# The top-level SGML file.
DOC_MAIN_SGML_FILE=VisualOS-docs.sgml

# The directory containing the source code (if it contains documentation).
DOC_SOURCE_DIR=../src


TARGET_DIR=$(HTML_DIR)/$(DOC_MODULE)

scan:
	gtkdoc-scan --module=$(DOC_MODULE) --source-dir=$(DOC_SOURCE_DIR) # --ignore-headers=""

templates: scan
	gtkdoc-mktmpl --module=$(DOC_MODULE)

sgml: scan
	gtkdoc-mkdb --module=$(DOC_MODULE) --source-dir=$(DOC_SOURCE_DIR)

html:
	if ! test -d html ; then mkdir html ; fi
	-cd html && gtkdoc-mkhtml $(DOC_MODULE) ../$(DOC_MAIN_SGML_FILE)

clean-local:
	rm -rf Manual-es html Descripcion Bibliografia Lista_de_cambios \
		Especificacion_de_requisitos Diseño Resumen Manual-en Manual-cs

maintainer-clean-local:
	rm -rf sgml html $(DOC_MODULE)-decl-list.txt $(DOC_MODULE)-decl.txt

#install-data-local:
#	install -d -m 0755 $(TARGET_DIR)
#	install -m 0644 html/*.html $(TARGET_DIR)
#	install -m 0644 html/index.sgml $(TARGET_DIR)
#	gtkdoc-fixxref --module=$(DOC_MODULE) --html-dir=$(HTML_DIR)

.PHONY : html sgml templates scan

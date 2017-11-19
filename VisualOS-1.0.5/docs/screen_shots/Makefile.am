EXTRA_DIST= *-clean.gif *.fig
BASE_NAMES=	\
	VisualOS-0.0.0\
	VisualOS-0.9.0\
	CLOCK\
	CPU-Drawing-Bars\
	CPU-Drawing-Overlapped\
	CPU-Drawing-Queues\
	CPU-Drawing-State\
	CPU-Menu-File\
	CPU-Menu-Settings\
	CPU-Menu-View\
	CPU-Preferences-Drawing-Styles\
	CPU-Preferences-Process-Auto-Filling\
	CPU-Process-Properties-Advanced\
	CPU-Process-Properties-General\
	CPU-Process-Properties-IO\
	CPU-Process-Properties-Memory\
	CPU\
	IO-Drawing-Round\
	IO-Drawing-Route\
	IO-Menu\
	IO\
	MEM-Menu\
	MEM-Physical\
	MEM-Virtual\
	MEM\
	REQUESTOR-IO\
	REQUESTOR-MEM

PNGS= $(addsuffix .png,$(BASE_NAMES))
GIFS= $(addsuffix .gif,$(BASE_NAMES))
EPSS= $(addsuffix .eps,$(BASE_NAMES))
PDFS= $(addsuffix .pdf,$(BASE_NAMES))
	
%.png: %.fig
	fig2dev -L png $^ > $@
%.gif: %.fig
	fig2dev -L gif $^ > $@
%.eps: %.fig
	fig2dev -L eps $^ > $@
%.pdf: %.fig
	fig2dev -L pdf $^ > $@

all:
screen_shots: eps png #gif


VisualOS-0_0_0.png: VisualOS-0.0.0.png
	@echo "*********************************************************" >&2
	@echo "* JADETEX DOESN'T HANDLE DOTS IN PICTURE NAMES PROPERLY *" >&2
	@echo "*********************************************************" >&2
	ln -s $< $@
VisualOS-0_9_0.png: VisualOS-0.9.0.png
	@echo "*********************************************************" >&2
	@echo "* JADETEX DOESN'T HANDLE DOTS IN PICTURE NAMES PROPERLY *" >&2
	@echo "*********************************************************" >&2
	ln -s $< $@
VisualOS-0_0_0.eps: VisualOS-0.0.0.eps
	@echo "*********************************************************" >&2
	@echo "* JADETEX DOESN'T HANDLE DOTS IN PICTURE NAMES PROPERLY *" >&2
	@echo "*********************************************************" >&2
	ln -s $< $@
VisualOS-0_9_0.eps: VisualOS-0.9.0.eps
	@echo "*********************************************************" >&2
	@echo "* JADETEX DOESN'T HANDLE DOTS IN PICTURE NAMES PROPERLY *" >&2
	@echo "*********************************************************" >&2
	ln -s $< $@
VisualOS-0_0_0.pdf: VisualOS-0.0.0.pdf
	@echo "*********************************************************" >&2
	@echo "* JADETEX DOESN'T HANDLE DOTS IN PICTURE NAMES PROPERLY *" >&2
	@echo "*********************************************************" >&2
	ln -s $< $@
VisualOS-0_9_0.pdf: VisualOS-0.9.0.pdf
	@echo "*********************************************************" >&2
	@echo "* JADETEX DOESN'T HANDLE DOTS IN PICTURE NAMES PROPERLY *" >&2
	@echo "*********************************************************" >&2
	ln -s $< $@

gif: $(GIFS) 
pdf: $(PDFS) VisualOS-0_0_0.pdf VisualOS-0_9_0.pdf
eps: $(EPSS) VisualOS-0_0_0.eps VisualOS-0_9_0.eps
png: $(PNGS) VisualOS-0_0_0.png VisualOS-0_9_0.png

CLEANFILES= $(GIFS) $(EPSS) $(PNGS) \
		VisualOS-0_0_0.png VisualOS-0_9_0.png \
		VisualOS-0_0_0.eps VisualOS-0_9_0.eps

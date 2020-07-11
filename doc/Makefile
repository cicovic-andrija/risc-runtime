# System software - Documentation
# Makefile for building PDF doc. files from LaTeX files
#

MAIN=doc.tex
PDFDIR=pdf

all: $(PDFDIR)
	pdflatex -output-directory $(PDFDIR) $(MAIN)

$(PDFDIR):
	mkdir -p $(PDFDIR)

clean:
	rm -rf $(PDFDIR)

.PHONY: all clean


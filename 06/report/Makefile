.PHONY: all
all:
	pdflatex -jobname report head.tex > /dev/null
	pdflatex -jobname report head.tex > /dev/null
	open report.pdf

.PHONY: clean
clean:
	rm *.aux *.toc *.log *.ind *.ilg

.PHONY: err
err:
	pdflatex -jobname report head.tex 

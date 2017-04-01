################################################################################
#
#  Copyright (C) 2015, Pluess Jonas
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
#  MA 02110-1301, USA.
#
################################################################################

NAME	= pflichtenheft
SRCS	= 00_fhnwreport.tex
# Attention: make clean removes all ${JOB}.* files
JOB	= zzz

build: ${SRCS}
	@-pdflatex -jobname ${JOB} -synctex=1 -interaction=nonstopmode ${SRCS}
	@-bibtex ${JOB}.aux
	@-pdflatex -jobname ${JOB} -synctex=1 -interaction=nonstopmode ${SRCS}
	@-pdflatex -jobname ${JOB} -synctex=1 -interaction=nonstopmode ${SRCS}
	@-mv ${JOB}.pdf ${NAME}.pdf
#	@-cp ${NAME}.pdf ../${NAME}.pdf
	@echo ""
	@echo "################################################################################"
	@echo "Warnings"
	@echo "--------------------------------------------------------------------------------"
	@-grep -A0 ".*Warning.*" ${JOB}.log | sed 's/Warning/Warning/'
	@-grep -A0 ".*Overfull.*" ${JOB}.log | sed 's/Overfull/Overfull/'
	@-grep -A0 ".*Underfull.*" ${JOB}.log | sed 's/Underfull/Underfull/'
	@echo ""
	@echo "################################################################################"
	@echo "Errors"
	@echo "--------------------------------------------------------------------------------"
	@-grep -A0 -E "(\!|^l\.[0-9].*)" ${JOB}.log | sed 's/l\./Line /'
	@echo "################################################################################"

complete: build clean tar

clean:
	rm -rf ${JOB}.* *.tar.gz

allclean:
	rm -rf ${JOB}.* ${NAME}.pdf *.tar.gz *.aux *.lof *.log *.lot *.out *.toc *.bbl *.blg *.synctex.gz *${NAME}.pdf

tar:
	tar --exclude='${JOB}.*' --exclude='*~' --exclude=*.tar.gz -cvzf ${NAME}.tar.gz *

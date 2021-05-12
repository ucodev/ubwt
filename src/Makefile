CC=`cat ../.compiler`
ECFLAGS=`cat ../.ecflags`
CCFLAGS=-O2
LDFLAGS=
ELFLAGS=`cat ../.elflags`
ARCHFLAGS=`cat ../.archflags`
INCLUDEDIRS=-I../include
OBJS=bitops config conn current datetime debug error main net process report runtime stage talk usage worker
TARGET=ubwt
SYSSBINDIR=`cat ../.dirsbin`

all:
	@echo
	@for obj in ${OBJS}; do \
		echo " * [CC] $${obj}"; \
		${CC} ${ECFLAGS} ${CCFLAGS} ${ARCHFLAGS} ${INCLUDEDIRS} -c $${obj}.c; \
	done
	@echo
	@echo " * [LD] ${TARGET}"
	@${CC} -o ${TARGET} `for obj in ${OBJS}; do printf -- " $${obj}.o"; done` ${LDFLAGS} ${ELFLAGS}
	@echo

install:
	@echo
	@echo " * [CP] ${TARGET}"
	@cp ${TARGET} ${SYSSBINDIR}/
	@echo

uninstall:
	@echo
	@echo " * [UL] ${TARGET}"
	@rm -f ${SYSSBINDIR}/${TARGET}
	@echo

clean:
	@echo
	@for obj in ${OBJS}; do \
		echo " * [UL] $${obj}"; \
		rm -f $${obj}.o; \
	done
	@echo
	@echo " * [UL] ${TARGET}"
	@rm -f ${TARGET}
	@echo


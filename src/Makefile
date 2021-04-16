CC=`cat ../.compiler`
ECFLAGS=`cat ../.ecflags`
CCFLAGS=-O2
LDFLAGS=
ELFLAGS=`cat ../.elflags`
ARCHFLAGS=`cat ../.archflags`
INCLUDEDIRS=-I../include
OBJS=bitops.o config.o current.o datetime.o debug.o error.o main.o net.o process.o report.o runtime.o stage.o talk.o usage.o
TARGET=ubwt
SYSSBINDIR=`cat ../.dirsbin`

all:
	${CC} ${ECFLAGS} ${CCFLAGS} ${ARCHFLAGS} ${INCLUDEDIRS} -c bitops.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${ARCHFLAGS} ${INCLUDEDIRS} -c config.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${ARCHFLAGS} ${INCLUDEDIRS} -c current.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${ARCHFLAGS} ${INCLUDEDIRS} -c datetime.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${ARCHFLAGS} ${INCLUDEDIRS} -c debug.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${ARCHFLAGS} ${INCLUDEDIRS} -c error.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${ARCHFLAGS} ${INCLUDEDIRS} -c main.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${ARCHFLAGS} ${INCLUDEDIRS} -c net.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${ARCHFLAGS} ${INCLUDEDIRS} -c process.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${ARCHFLAGS} ${INCLUDEDIRS} -c report.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${ARCHFLAGS} ${INCLUDEDIRS} -c runtime.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${ARCHFLAGS} ${INCLUDEDIRS} -c stage.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${ARCHFLAGS} ${INCLUDEDIRS} -c talk.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${ARCHFLAGS} ${INCLUDEDIRS} -c usage.c
	${CC} -o ${TARGET} ${OBJS} ${LDFLAGS} ${ELFLAGS}

install:
	cp ${TARGET} ${SYSSBINDIR}/

uninstall:
	rm ${SYSSBINDIR}/${TARGET}

clean:
	rm -f ${TARGET} ${OBJS}

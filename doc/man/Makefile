SYSMANDIR=`cat ../../.dirman`

all:
	gzip man8/*.8 2> /dev/null || true

install:
	cp man8/* ${SYSMANDIR}/man8/

clean:
	gunzip man8/*.gz 2> /dev/null || true


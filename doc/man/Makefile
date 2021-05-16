SYSMANDIR=`cat ../../.dirman`

all:
	@echo
	@echo " * [GZ] ubwt.8"
	@gzip man8/ubwt.8 2> /dev/null || true
	@echo


install:
	@echo
	@echo " * [CP] ubwt.8"
	@cp man8/ubwt.8.gz ${SYSMANDIR}/man8/
	@echo

clean:
	@echo
	@echo " * [UZ] ubwt.8"
	@gunzip man8/ubwt.8.gz 2> /dev/null || true
	@echo


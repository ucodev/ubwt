all:
	@if [ ! -e ".compiler" ]; then echo; echo "Please read the doc/text/INSTALL file for instructions."; echo; exit 1; fi
	cd src && make && cd ..

clean:
	cd src && make clean && cd ..

install:
	cd src && make install && cd ..

uninstall:
	cd src && make uninstall && cd ..


all:
	@if [ ! -e ".compiler" ]; then echo; echo "Please read the doc/text/INSTALL file for instructions."; echo; exit 1; fi
	cd src && make && cd ..

test:
	@if [ ! -e .done ]; then echo; echo "Cannot run tests without building the project first."; echo; exit 1; fi
	cd tests && make && cd ..

clean:
	cd src && make clean && cd ..
	cd doc && make clean && cd ..

install_all:
	cd src && make install && cd ..

install_doc:
	cd doc && make && make install && cd ..


uninstall:
	cd src && make uninstall && cd ..


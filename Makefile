TARGET=ubwt

all:
	make -C src/

clean:
	make -C src/ clean

install:
	cp src/${TARGET} /usr/local/bin/

uninstall:
	rm /usr/local/bin/${TARGET}

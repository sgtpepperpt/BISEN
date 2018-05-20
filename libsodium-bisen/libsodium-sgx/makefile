.PHONY : clean

all: compile-libsodium libsodium-sgx.a libsodium-sgx-renamed.a

libsodium-sgx-renamed.a: libsodium-sgx.a
	cp libsodium-sgx.a libsodium-sgx-renamed.a
	nm libsodium-sgx-renamed.a | grep " T " | awk '{print $$3" sodium_"$$3}' | grep -v "main" > libsodium_symbols.txt
	objcopy --redefine-syms=libsodium_symbols.txt libsodium-sgx-renamed.a
	rm libsodium_symbols.txt

libsodium-sgx.a: build/*.o
	gcc -Wall -c src/utils.c -o build/utils.o
	ar rcs libsodium-sgx.a build/*.o

build/*.o:
	./copyobjs.sh

compile-libsodium:
	(cd ../libsodium && ./configure --enable-static --disable-libtool-lock --without-pthreads --enable-minimal && make && make check)

install-libsodium-sgx.a:
	(cd ../libsodium && sudo make install)
	sudo cp libsodium-sgx.a /usr/local/lib/

install-libsodium-sgx-renamed.a:
	sudo cp libsodium-sgx-renamed.a /usr/local/lib/
	sudo cp sodium-sgx-renamed.h /usr/local/include/

clean:
	rm -fr build/*
	find . -name "*~" -exec rm {} \;

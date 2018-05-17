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
	mkdir -p build
	find ../libsodium -name *.o | grep crypto_scalarmult | while read fn ; do cp $$fn build/; done
	find ../libsodium -name *.o | grep sha | while read fn ; do cp $$fn build/; done
	find ../libsodium -name *.o | grep secretbox | while read fn ; do cp $$fn build/; done
	find ../libsodium -name *.o | grep crypto_stream | while read fn ; do cp $$fn build/; done
	find ../libsodium -name *.o | grep crypto_onetimeauth | while read fn ; do cp $$fn build/; done
	find ../libsodium -name *.o | grep crypto_core/hsalsa20 | while read fn ; do cp $$fn build/; done
	find ../libsodium -name *.o | grep crypto_verify | while read fn ; do cp $$fn build/; done
	find ../libsodium -name *.o | grep crypto_sign | while read fn ; do cp $$fn build/; done
	find ../libsodium -name *.o | grep crypto_core/curve25519 | while read fn ; do cp $$fn build/; done

compile-libsodium:
	(cd ../libsodium && ./configure --enable-static --disable-libtool-lock --without-pthreads --enable-minimal && make && make check)

install-libsodium-sgx.a:
	sudo cp libsodium-sgx.a /usr/local/lib/

install-libsodium-sgx-renamed.a:
	sudo cp libsodium-sgx-renamed.a /usr/local/lib/
	sudo cp sodium-sgx-renamed.h /usr/local/include/

clean:
	rm -fr build/
	find . -name "*~" -exec rm {} \;

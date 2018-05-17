.PHONY : clean

test1: test1.c ../libsodium-sgx.a
	gcc -Wall -o test1 test1.c ../libsodium-sgx.a

test1-renamed: test1-renamed.c ../libsodium-sgx-renamed.a
	gcc -Wall -o test1-renamed test1-renamed.c ../libsodium-sgx-renamed.a

clean:
	rm -f test1 test1-renamed
	find . -name "*~" -exec rm {} \;


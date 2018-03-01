.PHONY : clean test_bisen

test_bisen:
	cp ../../bin/bisen_benchmark build/bisen_benchmark
	$(MAKE) -C src/
	$(MAKE) -C test_bisen/f
	$(MAKE) -C test_bisen/enclave
	$(MAKE) -C test_bisen/

keypair: keypair.c
	$(CC) -Wall -o keypair keypair.c -lsodium-sgx

clean:
	rm -f keypair
	rm -rf build/*
	find . -name "*.o" -exec rm -rf {} \;
	find . -name "*~" -exec rm {} \;

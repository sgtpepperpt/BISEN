.PHONY : clean

##

main1: main.c example1.a
	gcc -Wall -g -o main1 main.c example1.a -lsodium-sgx

example1.a: example1/*.c
	cd example1 && gcc -Wall -g -I.. -c *.c
	ar rcs example1.a example1/*.o

##

main2: main.c example2.a
	gcc -Wall -g -o main2 main.c example2.a -lsodium-sgx

example2.a: example2/*.c
	cd example2 && gcc -Wall -g -I.. -c *.c
	ar rcs example2.a example2/*.o

##

clean:
	rm -f main1 storage.txt
	find . -name "*~" -exec rm {} \;
	find . -name "*.o" -exec rm {} \;
	find . -name "*.a" -exec rm {} \;


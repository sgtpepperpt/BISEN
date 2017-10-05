.PHONY : clean

##

main1: main.c iee.a
	gcc -Wall -g -o main main.c iee.a -lsodium-sgx

iee.a: src/*.c
	cd src && gcc -Wall -g -I.. -c *.c
	cd src/util && gcc -Wall -g -I../.. -c *.c
	ar rcs iee.a src/*.o src/util/*.o

##

clean:
	@rm -f main storage.txt
	@find . -name "*~" -exec rm {} \;
	@find . -name "*.o" -exec rm {} \;
	@find . -name "*.a" -exec rm {} \;


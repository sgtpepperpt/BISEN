INC = -I../Server/ -I../Client/ -I../Iee/ 

Test : Main.o
	g++ -o Test main.o -lcrypto -ldl -pthread

Debug : Main.debug
	g++ -o Test main.debug -lcrypto -ldl -pthread

###############################################################################

Main.o : main.cpp
	g++ $(INC) -std=c++11 -pthread -c main.cpp

###############################################################################

Test.debug : main.cpp
	g++ -std=c++11 -pthread -g -O0 -c main.cpp -o main.debug

clean :
	rm -f Test main.o main.debug
	
.PHONY: run clean

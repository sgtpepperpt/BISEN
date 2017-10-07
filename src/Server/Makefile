Server : Main.o SseServer.o
	g++ ../Common/Utils.cpp -pthread -g -O0 -o Server main.o SseServer.o -lcrypto -ldl -pthread -DVERBOSE

###############################################################################

Main.o : main.cpp
	g++ -std=c++11 -pthread -g -O0 -c main.cpp -o main.o -DVERBOSE

SseServer.o : SseServer.cpp
	g++ -std=c++11 -pthread -g -O0 -c SseServer.cpp -o SseServer.o -DVERBOSE

###############################################################################

clean :
	rm -f Server main.o SseServer.o
	
.PHONY: run clean

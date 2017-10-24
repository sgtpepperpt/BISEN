# -DVERBOSE
CONFIG_FLAGS=

Server : Main.o SseServer.o
	g++ ../Common/Utils.cpp -pthread -g -O0 -o Server main.o SseServer.o -lcrypto -ldl -pthread $(CONFIG_FLAGS)

###############################################################################

Main.o : main.cpp
	g++ -std=c++11 -pthread -g -O0 -c main.cpp -o main.o $(CONFIG_FLAGS)

SseServer.o : SseServer.cpp
	g++ -std=c++11 -pthread -g -O0 -c SseServer.cpp -o SseServer.o $(CONFIG_FLAGS)

###############################################################################

clean :
	rm -f Server main.o SseServer.o
	
.PHONY: run clean

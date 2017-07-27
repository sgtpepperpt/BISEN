Server : Main.o SseServer.o
	g++ ../Utils.cpp -o Server main.o SseServer.o -lcrypto -ldl -pthread

Debug : Main.debug SseServer.debug
	g++ -o Server main.debug SseServer.debug -lcrypto -ldl -pthread

###############################################################################

Main.o : main.cpp
	g++ -std=c++11 -pthread -c main.cpp

SseServer.o : SseServer.cpp
	g++ -std=c++11 -pthread -c SseServer.cpp

###############################################################################

Main.debug : main.cpp
	g++ -std=c++11 -pthread -g -O0 -c main.cpp -o main.debug

SseServer.debug : SseServer.cpp
	g++ -std=c++11 -pthread -g -O0 -c SseServer.cpp -o SseServer.debug

clean :
	rm -f Server main.o SseServer.o main.debug SseServer.debug
	
.PHONY: run clean

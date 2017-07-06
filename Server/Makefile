Server : Main.o SseServer.o ServerUtils.o
	g++ -o Server main.o SseServer.o ServerUtils.o -lcrypto -ldl -pthread

Debug : Main.debug SseServer.debug ServerUtils.debug
	g++ -o Server main.debug SseServer.debug ServerUtils.debug -lcrypto -ldl -pthread

Main.o : main.cpp
	g++ -std=c++11 -pthread -c main.cpp

SseServer.o : SseServer.cpp
	g++ -std=c++11 -pthread -c SseServer.cpp

ServerUtils.o : ServerUtils.cpp
	g++ -std=c++11 -pthread -c ServerUtils.cpp

Main.debug : main.cpp
	g++ -std=c++11 -pthread -g -O0 -c main.cpp -o main.debug

SseServer.debug : SseServer.cpp
	g++ -std=c++11 -pthread -g -O0 -c SseServer.cpp -o SseServer.debug

ServerUtils.debug : ServerUtils.cpp
	g++ -std=c++11 -pthread -g -O0 -c ServerUtils.cpp -o ServerUtils.debug
	
clean :
	rm -f Server main.o SseServer.o ServerUtils.o main.debug SseServer.debug ServerUtils.debug
	
.PHONY: run clean

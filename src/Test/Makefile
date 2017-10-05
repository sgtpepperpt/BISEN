.PHONY : main

INC = -I../Server/ -I../Client/ -I../Iee/ -I../

SRC = ../Server/SseServer.cpp ../Client/ClientCrypt.c ../Client/EnglishAnalyzer.cpp ../Client/QueryParser.cpp ../Client/SseClient.cpp ../Utils.cpp

LIBS = -lsodium -ldl -pthread

CFLAGS = -Wno-unused-variable -Wno-unused-function

main: main.cpp
	g++ -g -Wall $(CFLAGS) -DVERBOSE -std=c++11 $(INC) -o main main.cpp $(SRC) $(LIBS)

Debug : Main.debug
	g++ -o Test main.debug $(LIBS)

###############################################################################

Main.o : main.cpp
	g++ $(INC) -std=c++11 -pthread -c main.cpp

###############################################################################

Test.debug : main.cpp
	g++ -std=c++11 -pthread -g -O0 -c main.cpp -o main.debug

clean :
	rm -f main server main.o main.debug

.PHONY: run clean

.PHONY : main server

INC = -I../Server/ -I../Client/ -I../Iee/ -I../

SRC = ../Server/SseServer.cpp ../Client/ClientCrypt.cpp ../Client/EnglishAnalyzer.cpp ../Client/QueryParser.cpp ../Client/SseClient.cpp ../Iee/IeeCrypt.cpp ../Iee/QueryEvaluator.cpp ../Iee/SseIee.cpp ../Utils.cpp ../Iee/vec_int.c ../Iee/vec_token.c

LIBS = -lcrypto -ldl -pthread

CFLAGS = -Wno-unused-variable -Wno-unused-function -DVERBOSE

main: main.cpp
	g++ -g -Wall $(CFLAGS) -std=c++11 $(INC) -o main main.cpp $(SRC) ../Iee/SseIeeF.cpp $(LIBS)

server: server.cpp
	g++ -Ofast -Wall $(CFLAGS) -std=c++11 $(INC) -o server server.cpp $(SRC) $(LIBS)

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

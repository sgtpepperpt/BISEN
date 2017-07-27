
INC = -I../Server/ -I../Client/ -I../Iee/ -I../
SRC = ../Server/SseServer.cpp ../Client/ClientCrypt.cpp ../Client/EnglishAnalyzer.cpp ../Client/QueryParser.cpp ../Client/SseClient.cpp ../Iee/IeeCrypt.cpp ../Iee/QueryEvaluator.cpp ../Iee/SseIee.cpp ../Utils.cpp
LIBS = -lcrypto -ldl -pthread

main: main.cpp 
	g++ -std=c++11 -Wall $(INC) -o main main.cpp $(SRC) $(LIBS)

# 

Debug : Main.debug
	g++ -o Test main.debug -lcrypto -ldl -pthread

###############################################################################

Main.o : main.cpp
	g++ $(INC) -std=c++11 -pthread -c main.cpp

###############################################################################

Test.debug : main.cpp
	g++ -std=c++11 -pthread -g -O0 -c main.cpp -o main.debug

clean :
	rm -f main Test main.o main.debug
	
.PHONY: run clean

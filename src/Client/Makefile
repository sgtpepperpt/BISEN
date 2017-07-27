Client : Main.o SseClient.o ClientCrypt.o EnglishAnalyzer.o QueryParser.o PorterStemmer.o
	g++ ../Utils.cpp -o Client main.o SseClient.o ClientCrypt.o EnglishAnalyzer.o QueryParser.o PorterStemmer.o -lcrypto -ldl -pthread

Debug : Main.debug SseClient.debug ClientCrypt.debug EnglishAnalyzer.debug QueryParser.debug PorterStemmer.debug
	g++ -o Client main.debug SseClient.debug ClientCrypt.debug EnglishAnalyzer.debug QueryParser.debug PorterStemmer.debug -lcrypto -ldl -pthread

###############################################################################
Main.o : main.cpp
	g++ -std=c++11 -pthread -c main.cpp

SseClient.o : SseClient.cpp
	g++ -std=c++11 -pthread -c SseClient.cpp

ClientCrypt.o : ClientCrypt.cpp
	g++ -std=c++11 -pthread -c ClientCrypt.cpp

EnglishAnalyzer.o : EnglishAnalyzer.cpp
	g++ -std=c++11 -pthread -c EnglishAnalyzer.cpp

QueryParser.o : QueryParser.cpp
	g++ -std=c++11 -pthread -c QueryParser.cpp

PorterStemmer.o : PorterStemmer.c
	gcc -pthread -c PorterStemmer.c

###############################################################################
Main.debug : main.cpp
	g++ -std=c++11 -pthread -g -O0 -c main.cpp -o main.debug

SseClient.debug : SseClient.cpp
	g++ -std=c++11 -pthread -g -O0 -c SseClient.cpp -o SseClient.debug

ClientCrypt.debug : ClientCrypt.cpp
	g++ -std=c++11 -pthread -g -O0 -c ClientCrypt.cpp -o ClientCrypt.debug

EnglishAnalyzer.debug : EnglishAnalyzer.cpp
	g++ -std=c++11 -pthread -g -O0 -c EnglishAnalyzer.cpp -o EnglishAnalyzer.debug

QueryParser.debug : QueryParser.cpp
	g++ -std=c++11 -pthread -g -O0 -c QueryParser.cpp -o QueryParser.debug

PorterStemmer.debug : PorterStemmer.c
	gcc -pthread -g -O0 -c PorterStemmer.c -o PorterStemmer.debug
	
###############################################################################
clean :
	rm -f Client main.o SseClient.o ClientCrypt.o EnglishAnalyzer.o QueryParser.o PorterStemmer.o main.debug SseClient.debug ClientCrypt.debug EnglishAnalyzer.debug QueryParser.debug PorterStemmer.debug

.PHONY: run clean

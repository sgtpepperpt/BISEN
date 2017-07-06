Client : Main.o SseClient.o ClientCrypt.o EnglishAnalyzer.o ClientUtils.o PorterStemmer.o
	g++ -o Client Main.o SseClient.o ClientCrypt.o EnglishAnalyzer.o ClientUtils.o PorterStemmer.o -lcrypto -ldl -pthread

Debug : Main.debug SseClient.debug ClientCrypt.debug EnglishAnalyzer.debug ClientUtils.debug PorterStemmer.debug
	g++ -o Client Main.debug SseClient.debug ClientCrypt.debug EnglishAnalyzer.debug ClientUtils.debug PorterStemmer.debug -lcrypto -ldl -pthread

Main.o : main.cpp
	g++ -std=c++11 -pthread -c main.cpp

SseClient.o : SseClient.cpp
	g++ -std=c++11 -pthread -c SseClient.cpp

ClientCrypt.o : ClientCrypt.cpp
	g++ -std=c++11 -pthread -c ClientCrypt.cpp

EnglishAnalyzer.o : EnglishAnalyzer.cpp
	g++ -std=c++11 -pthread -c EnglishAnalyzer.cpp

ClientUtils.o : ClientUtils.cpp
	g++ -std=c++11 -pthread -c ClientUtils.cpp

PorterStemmer.o : PorterStemmer.c
	gcc -pthread -c PorterStemmer.c

Main.debug : main.cpp
	g++ -std=c++11 -pthread -g -O0 -c main.cpp -o Main.debug

SseClient.debug : SseClient.cpp
	g++ -std=c++11 -pthread -g -O0 -c SseClient.cpp -o SseClient.debug

ClientCrypt.debug : ClientCrypt.cpp
	g++ -std=c++11 -pthread -g -O0 -c ClientCrypt.cpp -o ClientCrypt.debug

EnglishAnalyzer.debug : EnglishAnalyzer.cpp
	g++ -std=c++11 -pthread -g -O0 -c EnglishAnalyzer.cpp -o EnglishAnalyzer.debug

ClientUtils.debug : ClientUtils.cpp
	g++ -std=c++11 -pthread -g -O0 -c ClientUtils.cpp -o ClientUtils.debug

PorterStemmer.debug : PorterStemmer.c
	gcc -pthread -g -O0 -c PorterStemmer.c -o PorterStemmer.debug

clean :
	rm Server Main.o MIEServer.o MIEServerMT.o SSEServer.o ServerUtil.o CashServer.o PaillierCashServer.o paillier.o core

.PHONY: run clean

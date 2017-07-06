##make run build accept arguments
#ifeq (run,$(firstword $(MAKECMDGOALS)))
#RUN_ARGS := $(wordlist 2,$(words $(MAKECMDGOALS)),$(MAKECMDGOALS))
#$(eval $(RUN_ARGS):;@:)
#endif

Client : Main.o SseClient.o ClientCrypt.o EnglishAnalyzer.o ClientUtils.o PorterStemmer.o
g++ -o Client Main.o SseClient.o ClientCrypt.o EnglishAnalyzer.o ClientUtils.o PorterStemmer.o -lcrypto -ldl -pthread

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

clean :
rm Server Main.o MIEServer.o MIEServerMT.o SSEServer.o ServerUtil.o CashServer.o PaillierCashServer.o paillier.o core



#run : Server
#LD_LIBRARY_PATH=/usr/local/opencv/opencv3.1/lib/:$$LD_LIBRARY_PATH ./Server $(RUN_ARGS) >teste.txt 2>&1 &

.PHONY: run clean

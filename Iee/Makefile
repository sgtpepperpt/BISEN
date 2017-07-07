Iee : Main.o SseIee.o IeeCrypt.o QueryEvaluator.o IeeUtils.o
	g++ -o Iee main.o SseIee.o IeeCrypt.o QueryEvaluator.o IeeUtils.o -lcrypto -ldl -pthread

Debug : Main.debug SseIee.debug IeeCrypt.debug QueryEvaluator.debug IeeUtils.debug
	g++ -o Iee main.debug SseIee.debug IeeCrypt.debug QueryEvaluator.debug IeeUtils.debug -lcrypto -ldl -pthread

###############################################################################

Main.o : main.cpp
	g++ -std=c++11 -pthread -c main.cpp

SseIee.o : SseIee.cpp
	g++ -std=c++11 -pthread -c SseIee.cpp

IeeCrypt.o : IeeCrypt.cpp
	g++ -std=c++11 -pthread -c IeeCrypt.cpp
	
QueryEvaluator.o : QueryEvaluator.cpp
	g++ -std=c++11 -pthread -c QueryEvaluator.cpp

IeeUtils.o : IeeUtils.cpp
	g++ -std=c++11 -pthread -c IeeUtils.cpp

###############################################################################

Main.debug : main.cpp
	g++ -std=c++11 -pthread -g -O0 -c main.cpp -o main.debug

SseIee.debug : SseIee.cpp
	g++ -std=c++11 -pthread -g -O0 -c SseIee.cpp -o SseIee.debug

IeeCrypt.debug : IeeCrypt.cpp
	g++ -std=c++11 -pthread -g -O0 -c IeeCrypt.cpp -o IeeCrypt.debug
	
QueryEvaluator.debug : QueryEvaluator.debug
	g++ -std=c++11 -pthread -g -O0 -c QueryEvaluator.cpp -o QueryEvaluator.debug
	
IeeUtils.debug : IeeUtils.cpp
	g++ -std=c++11 -pthread -g -O0 -c IeeUtils.cpp -o IeeUtils.debug

###############################################################################

clean :
	rm -f Iee main.o SseIee.o IeeCrypt.o QueryEvaluator.o IeeUtils.o main.debug SseIee.debug IeeCrypt.debug QueryEvaluator.debug IeeUtils.debug
	
.PHONY: run clean

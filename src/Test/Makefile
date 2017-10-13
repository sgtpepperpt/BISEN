IEE_DIR=../Iee
CLIENT_DIR=../Client
COMMON_DIR=../Common

C_FLAGS= -Wall -g -Wno-unused-function -Wno-unused-variable -fPIC -DVERBOSE
CPP_FLAGS= $(C_FLAGS) -std=c++11

LIBS= -lsodium-sgx

.PHONY : clean main

##

main: main.cpp common.a iee.a client.a
	g++ $(CPP_FLAGS) -o main main.cpp common.a iee.a client.a $(LIBS)

common.a: $(COMMON_DIR)/*.cpp
	cd $(COMMON_DIR) && g++ $(CPP_FLAGS) -c *.cpp
	ar rcs common.a $(COMMON_DIR)/*.o

iee.a: $(IEE_DIR)/*.c
	cd $(IEE_DIR) && gcc $(C_FLAGS) -c *.c
	cd $(IEE_DIR)/util && gcc $(C_FLAGS) -c *.c
	gcc $(C_FLAGS) -c commands.c
	ar rcs iee.a $(IEE_DIR)/*.o $(IEE_DIR)/util/*.o commands.o

client.a: $(CLIENT_DIR)/*.c $(CLIENT_DIR)/*.cpp
	cd $(CLIENT_DIR) && gcc $(C_FLAGS) -c *.c
	cd $(CLIENT_DIR) && g++ $(CPP_FLAGS) -c *.cpp
	ar rcs client.a $(CLIENT_DIR)/*.o

##

clean:
	@rm -f main storage.txt
	@find .. -name "*~" -exec rm {} \;
	@find .. -name "*.a" -exec rm {} \;
	@find .. -name "*.o" -exec rm {} \;

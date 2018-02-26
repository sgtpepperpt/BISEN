IEE_DIR=../Iee
CLIENT_DIR=../Client
COMMON_DIR=../Common

#DATASET_SIZE ?= 517402

# -DLOCALTEST -DVERBOSE
CONFIG_FLAGS= #-DDATASET_DIR=\"../Data/parsed/$(DATASET_SIZE)/\" # should now come from env
CONFIG_FLAGS+= -DVERBOSE

C_FLAGS= -Wall -g -Wno-unused-function -Wno-unused-variable -fPIC $(CONFIG_FLAGS)
CPP_FLAGS= $(C_FLAGS) -std=c++11

LIBS= -lsodium

.PHONY : clean main

##

main: main.cpp common.a iee.a client.a unstrusted.a
	g++ $(CPP_FLAGS) -o main main.cpp common.a iee.a client.a unstrusted.a $(LIBS)
	cp iee.a ../tsgx/test_bisen/f/

unstrusted.a: ../tsgx/src/untrusted/untrusted_free_bytes.c
	gcc $(C_FLAGS) -I../tsgx/include/ -c $<
	ar rcs $@ untrusted_free_bytes.o

common.a: $(COMMON_DIR)/*.cpp
	cd $(COMMON_DIR) && g++ $(CPP_FLAGS) -c *.cpp
	ar rcs common.a $(COMMON_DIR)/*.o

iee.a: $(IEE_DIR)/*.c
	cd $(IEE_DIR) && gcc $(C_FLAGS) -c *.c
	cd $(IEE_DIR)/util && gcc $(C_FLAGS) -c *.c
	ar rcs iee.a $(IEE_DIR)/*.o $(IEE_DIR)/util/*.o

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

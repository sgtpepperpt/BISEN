#
# 'make'        build executable file 'mycc'
# 'make clean'  removes all .o, executable and library files

# SGX SDK Settings
SGX_SDK ?= /opt/intel/sgxsdk

SGX_MODE ?= HW

USE_SGX = 1

ifeq ($(USE_SGX), 1)
	SGX_FLAG :=
	SGX_OBJ  := enclave/Enclave/Enclave_u.o
	FSRCS    :=
	SGX_COMMON_CFLAGS := -m64
	SGX_LIBRARY_PATH := $(SGX_SDK)/lib64
	Urts_Library_Name := sgx_urts
	SGX_Link_Flags := $(SGX_COMMON_CFLAGS) -L$(SGX_LIBRARY_PATH) -l$(Urts_Library_Name) -lsgx_uae_service

	ifneq ($(SGX_MODE), HW)
		Urts_Library_Name := sgx_urts_sim
		SGX_Link_Flags := $(SGX_COMMON_CFLAGS) -L$(SGX_LIBRARY_PATH) -l$(Urts_Library_Name) -lsgx_uae_service_sim

	else
		Urts_Library_Name := sgx_urts
		SGX_Link_Flags := $(SGX_COMMON_CFLAGS) -L$(SGX_LIBRARY_PATH) -l$(Urts_Library_Name) -lsgx_uae_service

	endif

else
	SGX_FLAG := -DSGX_MPC_OUTSIDE
	SGX_OBJ  :=
	FSRCS    :=
	SGX_COMMON_CFLAGS :=
	SGX_LIBRARY_PATH :=
	Urts_Library_Name :=
	SGX_Link_Flags :=
endif

# define the C compiler to use
CC = gcc
CXX = g++

# define any compile-time flags
# -DSGX_MPC_OUTSIDE
CFLAGS = -c -Wall -Werror -Wno-error=unused-variable -fPIC -DSGX_MPC_TEST  $(SGX_FLAG)

################################################################################

LINKFLAGS =  -L../build -lsgx_mpc -lremote -lf -lsodium-sgx /home/guilherme/BISEN/bin/libCommon.a
INCLUDES = -I../include -I$(SGX_SDK)/include
SRCS = remote_mpc_client.c

SGX_COMMON_CFLAGS := -m64
SGX_LIBRARY_PATH := $(SGX_SDK)/lib64

# define the C object files
OBJS = $(SRCS:.c=.o)

# define the executable file
MAIN = ../build/mpc_client

# The following part of the makefile is generic; it can be used to
# build any executable just by changing the definitions above and by
# deleting dependencies appended to the file from 'make depend'

.PHONY: clean

all: $(MAIN)
		@echo  Sources compiled

$(MAIN): $(OBJS)
		$(CC) $(INCLUDES) -o $(MAIN) $(OBJS) $(SGX_OBJ) $(LINKFLAGS) $(SGX_Link_Flags) $(GENLIBS)


# this is a suffix replacement rule for building .o's from .c's
# it uses automatic variables $<: the name of the prerequisite of
# the rule(a .c file) and $@: the name of the target of the rule (a .o file)
# (see the gnu make manual section about automatic variables)

%.o: %.c
		$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
		$(RM) *.o *~ $(MAIN)

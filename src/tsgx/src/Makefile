#
# 'make'        build executable file 'mycc'
# 'make clean'  removes all .o, executable and library files

# SGX SDK Settings
SGX_SDK ?= /opt/intel/sgxsdk

# define the C compiler to use
CC = gcc

USE_SGX = 1

ifeq ($(USE_SGX), 1)
	SGX_FLAG := 
else
	SGX_FLAG := -DSGX_MPC_OUTSIDE
endif

# define any compile-time flags -DSGX_MPC_OUTSIDE 
CFLAGS = -c -Wall -Werror -fPIC -DSGX_MPC_TEST $(SGX_FLAG)

# define the AR tool
AR = ar rcs

# define any directories containing header files other than /usr/include
INCLUDES = -I./include -I../include -I$(SGX_SDK)/include

# define the C source files
PROGSRCS = ae/ae.c attke/attke.c attke/attke_remote.c attke/attke_par.c box/box.c lac/lac_attest.c lac/lac_verify.c mach/mach.c mach/mach_iee.c mpc/mpc_process.c mpc/mpc_program.c mpc/mpc_setup.c untrusted/untrusted_malloc_bytes.c

# define the C object files 
PRPGOBJS =  $(PROGSRCS:.c=.o)

#define build folder
BUILD_PATH = ../build

#define the output library files
LIBLOCAL = $(BUILD_PATH)/libsgx_mpc.a
LOCALOBJS = ae/ae.o attke/attke.o lac/lac_attest.o lac/lac_verify.o mach/mach.o mpc/mpc_process.o

LIBREMOTE =  $(BUILD_PATH)/libremote.a
REMOTEOBJS = ae/ae.o attke/attke_remote.o attke/attke_par.o box/box.o mach/mach_iee.o mpc/mpc_program.o untrusted/untrusted_malloc_bytes.o

# The following part of the makefile is generic; it can be used to 
# build any executable just by changing the definitions above and by
# deleting dependencies appended to the file from 'make depend'
.PHONY: clean

all: $(BUILD_PATH) $(LIBLOCAL) $(LIBREMOTE)
		@echo Sources compiled

$(BUILD_PATH):
		mkdir -p $(BUILD_PATH)

$(LIBLOCAL): $(LOCALOBJS)
		$(AR) $(LIBLOCAL) $(LOCALOBJS)

$(LIBREMOTE): $(REMOTEOBJS)
		$(AR) $(LIBREMOTE) $(REMOTEOBJS)

# this is a suffix replacement rule for building .o's from .c's
# it uses automatic variables $<: the name of the prerequisite of
# the rule(a .c file) and $@: the name of the target of the rule (a .o file) 
# (see the gnu make manual section about automatic variables)

%.o: %.c
		$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
		$(RM) ae/*.o attke/*.o box/*.o lac/*.o mach/*.o mpc/*.o *~ $(MAIN) $(LIBLOCAL) $(LIBREMOTE)



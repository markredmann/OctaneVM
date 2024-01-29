##########################################
###  ROSETTA HSI - Copyright (c) 2023  ###
### Generic Platform Agnostic Makefile ###
##########################################

### Compilers ###
CC_GEN=c++
CC_MAC=clang++
CC_LIN=clang++
CC_WIN=$(CC_GEN)

### Project Specific ###
BIN_NAME_GEN=OctaneTesting
BIN_NAME_WIN=OctaneTesting.exe
## If the Project is, or contains: a framework
LIB_NAME_GEN=libOctaneVM.so
LIB_NAME_WIN=libOctaneVM.dll

BIN_INSTALL_GEN=/usr/local/bin
BIN_INSTALL_WIN=/Unknown
## If the Project is, or contains: a framework
LIB_INSTALL_GEN=/usr/local/lib
LIB_HEADERS_GEN=/usr/local/include
LIB_INSTALL_WIN=/Unknown
LIB_HEADERS_WIN=/Unknown

### Agnostic Defaults ###
CC=$(CC_GEN)
BIN_NAME=$(BIN_NAME_GEN)
LIB_NAME=$(LIB_NAME_GEN)
BIN_INSTALL=$(BIN_INSTALL_GEN)
LIB_INSTALL=$(LIB_INSTALL_GEN)
LIB_HEADERS=$(LIB_HEADERS_GEN)

### Compile Defaults ###
SRCS_FOLDER=Source
SRCS_HEADERS=Source/Headers
SRCS_EXT=.cpp
SRCS=$(SRCS_FOLDER)/*$(SRCS_EXT)
BINS_FOLDER=Bins.nosync
BINS=$(BINS_FOLDER)/*.o
ENTRYPOINT_FILE=$(SRCS_FOLDER)/TESTING.cc
## Flags
FLAGS_STRIP_GEN=
FLAGS_STRIP_MAC=-S
FLAGS_STRIP_LIN=--strip-all

FLAGS_STRIP=$(FLAGS_STRIP_GEN)
FLAGS_MAIN=-std=c++17
FLAGS_OBJ=-std=c++17 -fPIC
FLAGS_SHARED=-shared
FLAGS_WARN=-Wall -Wextra -Winline -pedantic -Wpedantic \
		   -Wdisabled-optimization -Wnewline-eof -Wno-strict-aliasing

### OS Detection ###
ifeq ($(OS), Windows_NT) 
	RUNNING_OS := NT
	BIN_INSTALL=$(BIN_INSTALL_WIN)
	LIB_INSTALL=$(LIB_INSTALL_WIN)
	LIB_HEADERS=$(LIB_HEADERS_WIN)
	LIB_NAME=$(LIB_NAME_WIN)
else
    RUNNING_OS := $(shell sh -c 'uname 2>/dev/null || echo Unknown')
endif

### OS Specific Assignments ###
ifeq ($(RUNNING_OS), Darwin)
	CC = $(CC_MAC)
	FLAGS_STRIP = $(FLAGS_STRIP_MAC)
endif
ifeq ($(RUNNING_OS), Linux)
	CC = $(CC_LIN)
	FLAGS_STRIP = $(FLAGS_STRIP_LIN)
endif

### Platspec Defaults ###

### Cases ###


all: $(BINS) shared example

example:
	$(CC) $(FLAGS_MAIN) $(FLAGS_WARN) $(ENTRYPOINT_FILE) $(BINS) -o $(BIN_NAME)

$(BINS_FOLDER)/%.o: $(SRCS_FOLDER)/%$(SRCS_EXT)
	$(CC) $(FLAGS_OBJ) $(FLAGS_WARN) -c $^
	mv -f *.o $(BINS_FOLDER)

shared: $(BINS)
	$(CC) $(FLAGS_MAIN) $(FLAGS_WARN) $(FLAGS_SHARED) $(BINS) -o $(LIB_NAME)
	strip $(LIB_NAME) $(FLAGS_STRIP)

clear:
	rm -f $(BINS) $(BIN_NAME) $(LIB_NAME)
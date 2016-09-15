include config.mk

BIN_DIR := ./bin
SRC_DIR := ./src

CFLAGS 	:= -g -std=c++0x -fPIC -O3

INCLUDES:= -I./include  \
	   -I$(BOOST_DIR)/include \
	   -I$(GLOG_DIR)/include \
	   -I$(GFLAGS_DIR)/include
	
LIBS	:= -L$(BOOST_DIR)/lib \
	   	-lboost_system \
		-lboost_thread \
		-lboost_iostreams \
		-lboost_filesystem \
		-lboost_regex \
		-lboost_program_options \
	   -L$(GLOG_DIR)/lib -lglog \
	   -L$(GFLAGS_DIR)/lib -lgflags \
	   -lpthread -lm -ldl -lz -lrt

ifneq ($(NDEBUG),)
CFLAGS   := $(CFLAGS) -DNDEBUG
endif

OBJS	 := $(SRC_DIR)/main.o \
	    $(SRC_DIR)/worker-align.o \
	    $(SRC_DIR)/worker-markdup.o \
	    $(SRC_DIR)/common.o \
	    $(SRC_DIR)/config.o 

PROG	 := ./$(BIN_DIR)/fcs-genome

all:	$(PROG)

$(PROG): $(OBJS)
	$(PP) $(LIBS) $(OBJS) -o $@

$(SRC_DIR)/%.o:	$(SRC_DIR)/%.cpp
	$(CC) -c $(CFLAGS) $(INCLUDES) $< -o $@

clean:
	rm -f $(OBJS)
	rm -f $(PROG)  

.PHONY: all clean

include config.mk

BIN_DIR := ./bin
SRC_DIR := ./src

CFLAGS 	:= -g -std=c++0x -fPIC -O3

INCLUDES:= -I./include  \
	   -I$(BOOST_DIR)/include \
	   -I$(GLOG_DIR)/include \
	   -I$(GFLAGS_DIR)/include
	
LINK	:= -L$(BOOST_DIR)/lib \
	   	-lboost_system \
		-lboost_thread \
		-lboost_iostreams \
		-lboost_filesystem \
		-lboost_regex \
		-lboost_program_options \
	   -L$(GLOG_DIR)/lib -lglog \
	   -L$(GFLAGS_DIR)/lib -lgflags \
	   -lpthread -lm -ldl -lz -lrt

ifeq ($(RELEASE),)
ifeq ($(NDEBUG),)
CFLAGS   	:= $(CFLAGS) -g
else
CFLAGS   	:= $(CFLAGS) -O2 -DNDEBUG
endif
else
# check FLMDIR
ifeq ($(FLMDIR),)
$(error FLMDIR not set properly in Makefile.config)
endif
# add support for flex license manage
FLMLIB 		:= -llmgr_trl -lcrvs -lsb -lnoact -llmgr_dongle_stub

CFLAGS   	:= $(CFLAGS) -DNDEBUG -DUSELICENSE
INCLUDES 	:= $(INCLUDES) -I$(FLMDIR)
LINK 	 	:= $(LINK) -L$(FLMDIR) $(FLMLIB) 
LMDEPS 	 	:= $(FLMDIR)/license.o \
		   $(FLMDIR)/lm_new.o 
endif

OBJS	 := $(SRC_DIR)/main.o \
	    $(SRC_DIR)/common.o \
	    $(SRC_DIR)/config.o \
	    $(SRC_DIR)/Executor.o \
	    $(SRC_DIR)/worker-align.o \
	    $(SRC_DIR)/worker-markdup.o \
	    $(SRC_DIR)/worker-bqsr.o \
	    $(SRC_DIR)/worker-htc.o \
	    $(SRC_DIR)/worker-concat.o \
	    $(SRC_DIR)/workers/BQSRWorker.o \
	    $(SRC_DIR)/workers/BWAWorker.o \
	    $(SRC_DIR)/workers/HTCWorker.o \
	    $(SRC_DIR)/workers/MarkdupWorker.o \
	    $(SRC_DIR)/workers/PRWorker.o \
	    $(SRC_DIR)/workers/VCFConcatWorker.o

PROG	 := ./$(BIN_DIR)/fcs-genome

all:	$(PROG)

release:
	$(MAKE) RELEASE=1

$(PROG): $(OBJS) $(LMDEPS)
	$(PP) $(OBJS) $(LMDEPS) -o $@ $(LINK)

$(SRC_DIR)/%.o:	$(SRC_DIR)/%.cpp
	$(CC) -c $(CFLAGS) $(INCLUDES) $< -o $@

clean:
	rm -f $(OBJS)
	rm -f $(PROG)  

.PHONY: all clean

BIN_DIR 	:= ./bin
SRC_DIR 	:= ./src
TOOLS_DIR 	:= ./tools
TEST_DIR 	:= ./test

GLOG_DIR	:= ./deps/glog-0.3.3
GTEST_DIR	:= ./deps/googletest
OPENMPI_DIR	:= ./deps/openmpi-1.10.2
HTSLIB_DIR	:= ./deps/htslib-1.3.1
JSONCPP_DIR	:= ./deps/jsoncpp-1.7.7
FLMDIR		:= ./deps/falcon-lm

include config.mk

CFLAGS 		:= -std=c++0x -fPIC -O3

INCLUDES	:= -I./include  \
		   -I$(GLOG_DIR)/include \
		   -I$(HTSLIB_DIR) \
		   -I$(JSONCPP_DIR)/include \
		   -I$(GTEST_DIR)/include
	
LINK	:= -lboost_system \
	   -lboost_thread \
	   -lboost_iostreams \
	   -lboost_filesystem \
	   -lboost_regex \
	   -lboost_program_options \
	   -L$(GLOG_DIR)/lib -lglog \
	   -L$(GTEST_DIR)/lib -lgtest \
	   -L$(HTSLIB_DIR) -lhts \
	   -L$(JSONCPP_DIR)/lib -ljsoncpp \
	   -lpthread -lm -ldl -lz -lrt

GIT_VERSION := $(shell git describe --tags | sed 's/\(.*\)-.*/\1/')

ifeq ($(RELEASE),)
CFLAGS   	:= $(CFLAGS) -g
GIT_VERSION	:= $(GIT_VERSION)-dev
else
# check FLMDIR
ifneq ($(FLMDIR),)
# add support for flex license manage
FLMLIB 		:= -llmgr_trl -lcrvs -lsb -lnoact -llmgr_dongle_stub

CFLAGS   	:= $(CFLAGS) -DNDEBUG -DUSELICENSE
INCLUDES 	:= $(INCLUDES) -I$(FLMDIR)
LINK 	 	:= $(LINK) -L$(FLMDIR) $(FLMLIB) 
LMDEPS 	 	:= $(FLMDIR)/license.o \
		   $(FLMDIR)/lm_new.o 
else
CFLAGS   	:= $(CFLAGS) -O2 -DNDEBUG
endif
endif

CFLAGS	:= $(CFLAGS) -DVERSION=\"$(GIT_VERSION)\"

OBJS	 := $(SRC_DIR)/common.o \
	    $(SRC_DIR)/config.o \
	    $(SRC_DIR)/Executor.o \
	    $(SRC_DIR)/worker-align.o \
	    $(SRC_DIR)/worker-bqsr.o \
	    $(SRC_DIR)/worker-concat.o \
	    $(SRC_DIR)/worker-gatk.o \
	    $(SRC_DIR)/worker-htc.o \
	    $(SRC_DIR)/worker-indel.o \
	    $(SRC_DIR)/worker-joint.o \
	    $(SRC_DIR)/worker-markdup.o \
	    $(SRC_DIR)/worker-ug.o \
	    $(SRC_DIR)/workers/BQSRWorker.o \
	    $(SRC_DIR)/workers/BWAWorker.o \
	    $(SRC_DIR)/workers/CombineGVCFsWorker.o \
	    $(SRC_DIR)/workers/GenotypeGVCFsWorker.o \
	    $(SRC_DIR)/workers/HTCWorker.o \
	    $(SRC_DIR)/workers/IndelWorker.o \
	    $(SRC_DIR)/workers/MarkdupWorker.o \
	    $(SRC_DIR)/workers/VCFUtilsWorker.o \
	    $(SRC_DIR)/workers/UGWorker.o

TEST_OBJS := $(TEST_DIR)/TestConfig.o

PROG	 := ./$(BIN_DIR)/fcs-genome
TEST	 := ./$(BIN_DIR)/fcs-genome-test
DEPS	 := ./deps/.ready

all:	$(PROG)

install:
	mkdir -p $(PREFIX)/bin; \
	cp $(PROG) $(PREFIX)/bin; \
	cp setup.sh $(PREFIX)

dist: 	$(PROG)
	aws s3 cp $(PROG) $(AWS_REPO)/fcs-genome-$(GIT_VERSION)

test:   $(TEST)

runtest: test
	$(TEST)

$(DEPS): ./deps/get-all.sh
	./deps/get-all.sh

$(PROG): $(OBJS) $(LMDEPS) $(SRC_DIR)/main.o
	$(PP) $(SRC_DIR)/main.o $(OBJS) $(LMDEPS) -o $@ $(LINK)

$(TEST): $(OBJS) $(TEST_OBJS) $(TEST_DIR)/main.o
	$(PP) $(TEST_DIR)/main.o $(OBJS) $(TEST_OBJS) -o $@ $(LINK)

$(SRC_DIR)/%.o:	$(SRC_DIR)/%.cpp $(DEPS)
	$(CC) -c $(CFLAGS) $(INCLUDES) $< -o $@

$(TEST_DIR)/%.o: $(TEST_DIR)/%.cpp $(DEPS)
	$(CC) -c $(CFLAGS) $(INCLUDES) $< -o $@

clean:
	rm -f $(OBJS)
	rm -f $(PROG)  

.PHONY: all clean install dist test runtest

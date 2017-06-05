include config.mk

BIN_DIR := ./bin
SRC_DIR := ./src
TOOLS_DIR := ./tools

CFLAGS 	:= -g -std=c++0x -fPIC -O3

INCLUDES:= -I./include  \
	   -I$(GLOG_DIR)/include \
	   -I$(HTSLIB_DIR) \
	   -I$(JSONCPP_DIR)/install/include

ifeq ($(PREFIX),)
PREFIX  := ./install
endif

ifneq ($(BOOST_DIR),)
INCLUDES:= $(INCLUDES) -I$(BOOST_DIR)/include
endif
ifneq ($(GFLAGS_DIR),)
INCLUDES:= $(INCLUDES) -I$(GFLAGS_DIR)/include
endif
	
LINK	:= -L$(BOOST_DIR)/lib \
	   	-lboost_system \
		-lboost_thread \
		-lboost_iostreams \
		-lboost_filesystem \
		-lboost_regex \
		-lboost_program_options \
	   -L$(GLOG_DIR)/lib -lglog \
	   -L$(HTSLIB_DIR) -lhts \
	   -L$(JSONCPP_DIR) -ljsoncpp \
	   -lpthread -lm -ldl -lz -lrt

ifeq ($(RELEASE),)
CFLAGS   	:= $(CFLAGS) -g
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

OBJS	 := $(SRC_DIR)/main.o \
	    $(SRC_DIR)/common.o \
	    $(SRC_DIR)/config.o \
	    $(SRC_DIR)/Executor.o \
	    $(SRC_DIR)/worker-align.o \
	    $(SRC_DIR)/worker-bqsr.o \
	    $(SRC_DIR)/worker-concat.o \
	    $(SRC_DIR)/worker-gatk.o \
	    $(SRC_DIR)/worker-hist.o \
	    $(SRC_DIR)/worker-htc.o \
	    $(SRC_DIR)/worker-doc.o \
	    $(SRC_DIR)/FCSJson.o \
	    $(SRC_DIR)/worker-json.o \
	    $(SRC_DIR)/worker-indel.o \
	    $(SRC_DIR)/worker-joint.o \
	    $(SRC_DIR)/worker-markdup.o \
	    $(SRC_DIR)/worker-ug.o \
	    $(SRC_DIR)/workers/BQSRWorker.o \
	    $(SRC_DIR)/workers/BWAWorker.o \
	    $(SRC_DIR)/workers/CombineGVCFsWorker.o \
	    $(SRC_DIR)/workers/GenotypeGVCFsWorker.o \
	    $(SRC_DIR)/workers/HTCWorker.o \
	    $(SRC_DIR)/workers/DOCWorker.o \
	    $(SRC_DIR)/workers/IndelWorker.o \
	    $(SRC_DIR)/workers/MarkdupWorker.o \
	    $(SRC_DIR)/workers/VCFUtilsWorker.o \
	    $(SRC_DIR)/workers/UGWorker.o

PROG	 := ./$(BIN_DIR)/fcs-genome

all:	$(PROG)

install:
	mkdir -p $(PREFIX)/bin; \
	cp $(PROG) $(PREFIX)/bin; \
	cp setup.sh $(PREFIX)

release:
	$(MAKE) RELEASE=1

$(PROG): $(OBJS) $(LMDEPS)
	$(PP) $(OBJS) $(LMDEPS) -o $@ $(LINK)

$(SRC_DIR)/%.o:	$(SRC_DIR)/%.cpp
	$(CC) -c $(CFLAGS) $(INCLUDES) $< -o $@

clean:
	rm -f $(OBJS)
	rm -f $(PROG)  

.PHONY: all clean install release

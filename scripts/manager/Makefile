include config.mk

CFLAGS 	:= -g -std=c++0x -fPIC -O1 -DNDEBUG\
	   -Wno-deprecated-declarations

INCLUDES:= -I. \
	   -I$(BOOST_DIR)/include \
	   -I$(GLOG_DIR)/include \
	   -I$(GFLAGS_DIR)/include

LIBS	:= -L$(BOOST_DIR)/lib \
	   -lboost_thread \
	   -lboost_iostreams \
	   -lboost_filesystem \
	   -lboost_regex \
	   -lboost_system \
	   -L$(GLOG_DIR)/lib -lglog \
	   -L$(GFLAGS_DIR)/lib -lgflags \
	   -lpthread -lm -ldl -lz -lrt

all: manager client

manager: manager.o
	$(PP) $(LIBS) $< -o $@

client: client.o
	$(PP) $(LIBS) $< -o $@

%.o: %.cpp
	$(PP) -c $(CFLAGS) $(INCLUDES) $< -o $@

clean:
	rm -rf *.o
	rm -rf manager

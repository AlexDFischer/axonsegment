CPP = g++
CC = gcc
CPP_FLAGS = -w -lm
#CPPFLAGS = -g -Wall
#BOOST_FLAGS = -lboost_thread -lboost_system -lboost_filesystem
BOOST_FLAGS = -lboost_thread -lboost_system -lboost_filesystem -I/home/afis/lib/boost/include -L/home/afis/lib/boost/lib
LIBTIFF_LDFLAGS = -L/home/afis/lib/libtiff/lib -ltiff
LIBTIFF_HEADERS = -I/home/afis/lib/libtiff/include
BUILD_DIR = bin
HEADERS = project.h volume.h

axonsegment: $(shell find maxflow/*.cpp maxflow/*.h)
	$(CPP) $(CPP_FLAGS) maxflow/*.cpp -o $(BUILD_DIR)/$@ $(BOOST_FLAGS) $(LIBTIFF_LDFLAGS) $(LIBTIFF_HEADERS)

histogram: $(shell find histogram/*.cpp histogram/*.h)
	$(CPP) $(CPP_FLAGS) histogram/*.cpp -o $(BUILD_DIR)/$@ $(LIBTIFF_LDFLAGS) $(LIBTIFF_HEADERS)

clean:
	rm -f $(BUILD_DIR)/*

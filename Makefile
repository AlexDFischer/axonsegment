CPP = g++
CPP_FLAGS = -w -lm
#CPPFLAGS = -g -Wall
BOOST_FLAGS = -lboost_thread -lboost_system -lboost_filesystem
LIBTIFF_LDFLAGS = -L/home/afis/lib/libtiff/lib -ltiff
LIBTIFF_HEADERS = -I/home/afis/lib/libtiff/include
BUILD_DIR = bin
HEADERS = project.h volume.h

axonsegment: *.cpp $(HEADERS)
	$(CPP) $(CPP_FLAGS) *.cpp -o $(BUILD_DIR)/$@ $(BOOST_FLAGS) $(LIBTIFF_LDFLAGS) $(LIBTIFF_HEADERS)

clean:
	rm -f $(BUILD_DIR)/*

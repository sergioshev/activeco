GCC=g++
CC_FLAGS=-std=gnu++11 -I/usr/include/opencv -g
#LD_FLAGS=-ldl -lopencv_imgcodecs -lopencv_highgui -lopencv_videoio -lopencv_core -lpthread -ljpeg 
LD_FLAGS=-ldl -lboost_program_options -lopencv_imgcodecs -lopencv_imgproc -lopencv_highgui -lopencv_core -lpthread -ljpeg -lpqxx -lavcodec -lavutil -lavformat -lswscale
LD_FLAGS_LOG=-lpthread

TARGET=activeco
TESTCV=testcv2
TESTLOG=testlog

HEADERS=vpmr videoStreamReader dumper observers iniReader logger
HEADERSLOG=

OBJECTS=$(TARGET).o $(HEADERS:%=%.o) 
OBJECTSCV=$(TESTCV).o $(HEADERS:%=%.o) 
OBJECTLOG=$(TESTLOG).o $(HEADERSLOG:%=%.o)

FAKE_SO=libfakevpar.so
FAKE_TARGETS=libfakevpar.cpp libfakevpar.h

.phony: all
all: $(TARGET)

frameReader: frameReader.cpp
	g++ -Wsign-compare -Wall -g -fno-inline -std=c++11 -lavcodec -lavutil -lavformat -lswscale -lopencv_imgcodecs -lopencv_highgui -lopencv_videoio -lopencv_core frameReader.cpp -o frameReader

testini: testini.cpp
	g++ -std=gnu++11 -c iniReader.cpp -o iniReader.o
	g++ -std=gnu++11 -lboost_program_options testini.cpp iniReader.o -o testini

$(TARGET): $(OBJECTS)
	$(GCC) $(LD_FLAGS) $? -o $(TARGET)

$(TESTCV): $(TESTCV).o $(OBJECTSCV) 
	$(GCC) $(LD_FLAGS) $? -o $(TESTCV)

$(TESTLOG): $(TESTLOG).o $(OBJECTLOG) 
	$(GCC) $(LD_FLAGS_LOG) $? -o $(TESTLOG)

%.o : %.cpp
	$(GCC) -c $(CC_FLAGS) $< -o $@

.phony: fakelib
fakelib: $(FAKE_TARGETS)
	$(GCC) -fPIC -shared libfakevpar.cpp -o $(FAKE_SO)
	install $(FAKE_SO) /usr/lib

.phony: clean
clean:
	rm -f *.o
	rm -f $(TARGET)
	rm -f testini frameReader



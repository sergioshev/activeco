GCC=g++
CC_FLAGS=-Wall -std=gnu++11 -I/usr/include/opencv -g

#CC_FLAGS=-M -Wsign-compare -Wall -fno-inline -std=gnu++11 -I/usr/include/opencv -g

#LD_FLAGS=-ldl -lopencv_imgcodecs -lopencv_highgui -lopencv_videoio -lopencv_core -lpthread -ljpeg 

#LD_FLAGS=-ldl -lboost_program_options -lopencv_imgcodecs -lopencv_imgproc -lopencv_highgui -lopencv_core -lpthread -ljpeg -lpqxx -lavcodec -lavutil -lavformat -lswscale

LD_FLAGS=-ldl -lboost_program_options -lopencv_imgproc -lopencv_highgui -lopencv_core -lpthread -ljpeg -lpqxx -lavcodec -lavutil -lavformat -lswscale

TARGET=activeco
HEADERS=vpmr videoStreamReader dumper observers iniReader logger

OBJECTS=$(TARGET).o $(HEADERS:%=%.o) 

FAKE_SO=libfakevpar.so
FAKE_TARGETS=libfakevpar.cpp libfakevpar.h

DEPDIR=.d

$(shell mkdir -p $(DEPDIR) > /dev/null)

DEPFLAGS=-MT $@ -MMD -MP -MF $(DEPDIR)/$*.Td

#deshabilito la regla implicita de make
%.o: %.cpp

#defino la regla propia
%.o : %.cpp $(DEPDIR)/%.d
	$(GCC) $(DEPFLAGS) -c $(CC_FLAGS) $< -o $@
	mv -f $(DEPDIR)/$*.Td $(DEPDIR)/$*.d

.phony: fakelib
fakelib: $(FAKE_TARGETS)
	$(GCC) -fPIC -shared libfakevpar.cpp -o $(FAKE_SO)
	install $(FAKE_SO) /usr/lib

.phony: all
all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(GCC) $(LD_FLAGS) $^ -o $(TARGET)

.phony: clean
clean:
	rm -f *.o
	rm -f $(TARGET)
	rm -r .d

$(DEPDIR)/%.d: ;

.PRECIOUS: $(DEPDIR)/%.d

#@echo $(wildcard $(patsubst %,$(DEPDIR)/%.d,$(basename $(SRCS))))
include $(wildcard $(DEPDIR)/*.d)

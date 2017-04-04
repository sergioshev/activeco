GCC=g++
CC_FLAGS=-Wall -std=gnu++11 -I/usr/include/opencv -g

#CC_FLAGS=-M -Wsign-compare -Wall -fno-inline -std=gnu++11 -I/usr/include/opencv -g

#LD_FLAGS=-ldl -lopencv_imgcodecs -lopencv_highgui -lopencv_videoio -lopencv_core -lpthread -ljpeg 
LD_FLAGS=-ldl -lboost_program_options -lopencv_imgcodecs -lopencv_imgproc -lopencv_highgui -lopencv_core -lpthread -ljpeg -lpqxx -lavcodec -lavutil -lavformat -lswscale

TARGET=activeco
HEADERS=vpmr videoStreamReader dumper observers iniReader logger

OBJECTS=$(TARGET).o $(HEADERS:%=%.o) 

DEPDIR=.d

$(shell mkdir -p $(DEPDIR) > /dev/null)

DEPFLAGS=-MT $@ -MMD -MP -MF $(DEPDIR)/$*.Td

.phony: all
all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(GCC) $(LD_FLAGS) $^ -o $(TARGET)

%.o : %.cpp $(DEPDIR)/%.d
	$(GCC) $(DEPFLAGS) -c $(CC_FLAGS) $< -o $@
	mv -f $(DEPDIR)/$*.Td $(DEPDIR)/$*.d

.phony: clean
clean:
	rm -f *.o
	rm -f $(TARGET)

$(DEPDIR)/%.d: ;

#.PRECIOUS: $(DEPDIR)/%.d
#@echo $(wildcard $(patsubst %,$(DEPDIR)/%.d,$(basename $(SRCS))))

include $(wildcard $(DEPDIR)/*.d)

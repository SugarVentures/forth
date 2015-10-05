CXX = g++
RM = rm -f

INCDIR = -I$(rootDir)/../include -I$(rootDir)/../thread

CFLAGS = -Wall -g

LDFLAGS = -L$(rootDir)/../../libs
LIBS = -lthreadengine

FRAMEWORKS = -framework AudioToolbox -framework CoreFoundation -framework AudioUnit -framework CoreAudio

OBJLIBS = libaudioengine.so

SRCS = $(wildcard *.cpp)
SRCS += $(wildcard mac_utility/*.cpp)
OBJS = $(subst .cpp,.o,$(SRCS))


rootDir = .

all: $(OBJLIBS) install

$(OBJLIBS): $(OBJS)
	$(CXX) $(INCDIR) $(LDFLAGS) -fgnu-runtime -shared -o $(OBJLIBS) $(OBJS) $(LIBS) $(FRAMEWORKS)


%.o: %.cpp
	$(CXX) $(INCDIR) $(CFLAGS) -c -o $@ $<	

install:
	cp *.so $(rootDir)/../../libs

clean:
	$(RM) *.o
	$(RM) *.so

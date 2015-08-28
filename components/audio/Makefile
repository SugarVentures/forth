CXX = g++
RM = rm -f

INCDIR = -I$(rootDir)/../include

CFLAGS = -Wall -g

LDFLAGS = -L$(rootDir)/../../libs
LIBS = 

OBJLIBS = libaudioengine.so

SRCS = $(wildcard *.cpp)
OBJS = $(subst .cpp,.o,$(SRCS))


rootDir = .

all: $(OBJLIBS) install

$(OBJLIBS): $(OBJS)
	$(CXX) $(INCDIR) $(LDFLAGS) -fgnu-runtime -shared -o $(OBJLIBS) $(OBJS) $(LIBS)


%.o: %.cpp
	$(CXX) $(INCDIR) $(CFLAGS) -c -o $@ $<	

install:
	cp *.so $(rootDir)/../../libs

clean:
	$(RM) *.o
	$(RM) *.so

CC = gcc
CXX = g++

INCDIR = -I$(rootDir)/include

CFLAGS = -Wall -g -fPIC

LDFLAGS = 
LIBS = 


OBJLIBS = libyuv.so

SRCS = $(wildcard source/*.cc)
OBJS = $(subst .cpp,.o,$(SRCS))

rootDir = .

all: $(OBJLIBS) install

$(OBJLIBS): $(OBJS)
	$(CXX) $(INCDIR) $(LDFLAGS) -fgnu-runtime -shared -o $(OBJLIBS) $(OBJS) $(LIBS)


%.o: %.cpp 
	$(CXX) $(INCDIR) $(CFLAGS)  -c -o $@ $<	


install:
	cp *.so $(rootDir)/../../libs

clean:
	$(RM) *.o
	$(RM) *.so


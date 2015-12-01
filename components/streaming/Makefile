include ../../Makefile.inc


INCDIR = -I$(rootDir)/../include -I$(rootDir)/../interface  -I$(rootDir)/../error  \
		 -I$(rootDir)/../../libs/libsrtp/include -I$(rootDir)/../../libs/libsrtp/crypto/include  \
		 -I$(rootDir)/../thread -I$(rootDir)/../misc \
		 -I$(rootDir)/../../libs/libyuv/include -I$(rootDir)/../encoding

INCDIR += -I$(rootDir)/../network 
INCDIR += -I$(rootDir)/../audio

INCDIR += -I$(INCLUDE_PATHS) -I$(LIB_PATHS)/glib-2.0/include -I$(INCLUDE_PATHS)/glib-2.0

CFLAGS = -Wall -g -std=c++11


LDFLAGS = -L$(rootDir)/../../libs

LIBS = -lnetworkengine -lthreadengine -lmisc -lencodingengine

OBJLIBS = libstreaming.so

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

 
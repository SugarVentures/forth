include ../../Makefile.inc


INCDIR = -I$(rootDir)/../include -I$(rootDir)/../interface  -I$(rootDir)/../error  \
		 -I$(rootDir)/../misc \
		 -I$(rootDir)/../thread

INCDIR += -I$(INCLUDE_PATHS) -I$(LIB_PATHS)/glib-2.0/include -I$(INCLUDE_PATHS)/glib-2.0


NICELDFLAGS = -L$(LIB_PATHS)
NICELIBS = -lgio-2.0 -lgobject-2.0 -lgthread-2.0 -lglib-2.0 -lintl -lnice


CFLAGS = -Wall -g -fPIC -std=c++11

OS := $(shell uname)


#IP6 support in OSX
ifeq "$(OS)" "Darwin"
CFLAGS +=  -D__APPLE_USE_RFC_3542
endif

LDFLAGS = -L$(rootDir)/../../libs
LIBS = -lsrtp -lmisc -lssl -lcrypto -lmisccpp -lthreadengine

OBJLIBS = libnetworkengine.so

SRCS = $(wildcard *.cpp)
OBJS = $(subst .cpp,.o,$(SRCS))


rootDir = .

all: $(OBJLIBS) install

$(OBJLIBS): $(OBJS)
	$(CXX) $(INCDIR) $(LDFLAGS) $(NICELDFLAGS) -fgnu-runtime -shared -o $(OBJLIBS) $(OBJS) $(LIBS) $(NICELIBS)


%.o: %.cpp
	$(CXX) $(INCDIR) $(CFLAGS)  -c -o $@ $<	

install:
	cp *.so $(rootDir)/../../libs

clean:
	$(RM) *.o
	$(RM) *.so

 
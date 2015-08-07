
CXX = g++
RM = rm -f

INCDIR = -I$(rootDir)/../include -I$(rootDir)/../interface  -I$(rootDir)/../error  \
		 -I$(rootDir)/../../libs/libsrtp/include -I$(rootDir)/../../libs/libsrtp/crypto/include   \
		 -I$(rootDir)/../misc

INCDIR += -I$(rootDir)/../../libs/libnice -I$(rootDir)/../../libs/libnice/agent \
		  -I/usr/local/include/glib-2.0  \
		  -I$(rootDir)/../../libs/libnice/include \
		  -I$(rootDir)/../thread


NICELDFLAGS = -L/user/local/lib
NICELIBS = -lgio-2.0 -lgobject-2.0 -lgthread-2.0 -lglib-2.0 -lintl -lnice.10


CFLAGS = -Wall -g -fPIC -std=c++11

OS := $(shell uname)


#IP6 support in OSX
ifeq "$(OS)" "Darwin"
CFLAGS +=  -D__APPLE_USE_RFC_3542
endif

LDFLAGS = -L$(rootDir)/../../libs/libsrtp -L$(rootDir)/../../libs
LIBS = -lsrtp -lmisc -lssl -lcrypto -lmisccpp -lthreadengine

OBJLIBS = libnetworkengine.so

SRCS = $(wildcard *.cpp)
OBJS = $(subst .cpp,.o,$(SRCS))



rootDir = .

all: $(OBJLIBS) install

$(OBJLIBS): $(OBJS)
	$(CXX) $(INCDIR) $(LDFLAGS) -fgnu-runtime -shared -o $(OBJLIBS) $(OBJS) $(LIBS) $(NICELIBS)


%.o: %.cpp
	$(CXX) $(INCDIR) $(CFLAGS)  -c -o $@ $<	

install:
	cp *.so $(rootDir)/../../libs

clean:
	$(RM) *.o
	$(RM) *.so

 

CXX = g++
RM = rm -f

INCDIR = -I$(rootDir)/../include -I$(rootDir)/../interface  -I$(rootDir)/../error  \
		 -I$(rootDir)/../../libs/libsrtp/include -I$(rootDir)/../../libs/libsrtp/crypto/include  

CFLAGS = -Wall -g

#IP6 support in OSX
CFLAGS +=  -D__APPLE_USE_RFC_3542

LDFLAGS = -L$(rootDir)/../../libs/libsrtp -L$(rootDir)/../../libs
LIBS = -lsrtp -lmisc

OBJLIBS = libnetworkengine.so

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

 

CXX = g++
RM = rm -f

INCDIR = -I$(rootDir)/../include -I$(rootDir)/../interface  -I$(rootDir)/../error  \
		 -I$(rootDir)/../../libs/libsrtp/include -I$(rootDir)/../../libs/libsrtp/crypto/include  \
		 -I$(rootDir)/../thread -I$(rootDir)/../misc \
		 -I$(rootDir)/../../libs/libyuv/include -I$(rootDir)/../encoding \
		 -I$(rootDir)/../../libs/libvpx

INCDIR += -I$(rootDir)/../network -I$(rootDir)/../../libs/libnice -I$(rootDir)/../../libs/libnice/agent \
		  -I/usr/local/include/glib-2.0 \
		  -I$(rootDir)/../../libs/libnice/include

CFLAGS = -Wall -g -std=c++11


LDFLAGS = -L$(rootDir)/../../libs

LIBS = -lsrtp -lnetworkengine -lthreadengine -lmisc -lencodingengine

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

 
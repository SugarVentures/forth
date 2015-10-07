
CXX = g++
CX = gcc
RM = rm -f

INCDIR = -I$(rootDir)/../include -I$(rootDir)/../interface  -I$(rootDir)/../error  \
		-I$(rootDir)/../../libs/libyuv/include \
		-I$(rootDir)/../../libs/libvpx \
		-I$(rootDir)/../../libs/libsrtp/crypto/include  \
		-I$(rootDir)/../misc \
		-I$(rootDir)/../../libs/libopus/include

CFLAGS = -Wall -g -std=c++11

LDFLAGS = -L$(rootDir)/../../libs
LIBS = -lyuv -lvpx -lsrtp -lmisc -lopus

OBJLIBS = libencodingengine.so

SRCS = $(wildcard *.cpp)
OBJS = $(subst .cpp,.o,$(SRCS))


rootDir = .

all: $(OBJLIBS) install

$(OBJLIBS): $(OBJS)
	$(CXX) $(INCDIR) $(LDFLAGS) -fgnu-runtime -shared -o $(OBJLIBS) $(OBJS) $(LIBS)


%.o: %.cpp 
	$(CXX) $(INCDIR) $(CFLAGS) $(resample_CPPFLAGS) -c -o $@ $<	


install:
	cp *.so $(rootDir)/../../libs

clean:
	$(RM) *.o
	$(RM) *.so

 
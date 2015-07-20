CC = gcc
CXX = g++

INCDIR = -I$(rootDir)/../include -I$(rootDir)/../../libs/libsrtp/include -I$(rootDir)/../../libs/libsrtp/crypto/include  

CFLAGS = -Wall -g

LDFLAGS = -L$(rootDir)/../../libs/libsrtp
LIBS = -lsrtp

rootDir = .

OBJLIBS = libmisc.so
OBJLIBS_CPP = libmisccpp.so

SRCS = md5.c random_unique32.c utility.c
SRCS_CPP = atomic_helper.cpp fasthash.cpp

OBJS = $(subst .c,.o,$(SRCS))
OBJS_CPP = $(subst .cpp,.o,$(SRCS_CPP))

all: $(OBJLIBS) $(OBJLIBS_CPP) md5driver.o install

$(OBJLIBS): $(OBJS)
	$(CC) $(INCDIR) $(LDFLAGS) $(CFLAGS) -fgnu-runtime -shared -o $(OBJLIBS) $(OBJS) $(LIBS)

%.o: %.c
	$(CC) $(INCDIR) $(CFLAGS) -c -o $@ $<	

$(OBJLIBS_CPP): $(OBJS_CPP)
	$(CXX) $(INCDIR) $(LDFLAGS) $(CFLAGS) -fgnu-runtime -shared -o $(OBJLIBS_CPP) $(OBJS_CPP) $(LIBS)

%.o: %.cpp
	$(CXX) $(INCDIR) $(CFLAGS) -c -o $@ $<


md5driver.o: md5main.c
	$(CC) $(CFLAGS)  -o $@ -L. -lmisc $<


install:
	cp *.so ./../../libs

clean:
	rm *.o
	rm *.so


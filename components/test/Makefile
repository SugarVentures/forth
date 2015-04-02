
CXX = g++
RM = rm -f

INCDIR = -I$(rootDir)/../include -I$(rootDir)/../interface  \
		-I$(rootDir)/../../libs/libsrtp/include -I$(rootDir)/../../libs/libsrtp/crypto/include  
CFLAGS	= -Wall -g

VIDEOLDFLAGS = -L$(rootDir)/../video
VIDEOLIBS = -lvideoengine

NETWORKLDFLAGS = -L$(rootDir)/../network -L$(rootDir)/../../libs/libsrtp
NETWORKLIBS = -lnetworkengine 

SRCS = testvideoengine.cpp
OBJS = $(subst .cpp,.o,$(SRCS))

rootDir = .

all: testvideoengine proclib

testvideoengine: $(OBJS)
	$(CXX) $(INCDIR) $(CFLAGS) $(VIDEOLDFLAGS) $(VIDEOLIBS) $(OBJS) -o testvideoengine


$(OBJS): $(SRCS)
	$(CXX) $(INCDIR) $(CFLAGS) $(VIDEOLDFLAGS) $(VIDEOLIBS)  -o $@ -c $<

testnetworkengine: testnetworkengine.o
	$(CXX)  $(INCDIR) $(CFLAGS) $(NETWORKLDFLAGS) $< $(NETWORKLIBS) -o $@ 

testnetworkengine.o: testnetworkengine.cpp
	$(CXX) $(INCDIR) $(CFLAGS)  -o $@ -c $< 

proclib:
	cp $(rootDir)/../video/*.so $(rootDir)
	cp $(rootDir)/../network/*.so $(rootDir)
	cp $(rootDir)/../*.so $(rootDir)
	cp $(rootDir)/../../libs/libsrtp/*.a $(rootDir)

clean:
	$(RM) *.o


 
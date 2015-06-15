
CXX = g++
RM = rm -f

INCDIR = -I$(rootDir)/../include -I$(rootDir)/../interface -I$(rootDir)/../thread -I$(rootDir)/../misc \
		-I$(rootDir)/../../libs/libsrtp/include -I$(rootDir)/../../libs/libsrtp/crypto/include  \
		-I$(rootDir)/../../libs/libyuv/include -I$(rootDir)/../encoding \
		-I$(rootDir)/../../libs/libvpx

CFLAGS	= -Wall -g -std=c++11

VIDEOLDFLAGS = -L$(rootDir)/../../libs
VIDEOLIBS = -lvideoengine -lstreaming -lyuv

NETWORKLDFLAGS = -L$(rootDir)/../../libs
NETWORKLIBS = -lnetworkengine -lstreaming -lyuv -lencodingengine

ENCODINGFLAGS = -L$(rootDir)/../../libs
ENCODINGLIBS = -lyuv -lvpx

SRCS = testvideoengine.cpp
OBJS = $(subst .cpp,.o,$(SRCS))

rootDir = .

all: testvideoengine testnetworkengine teststream testvp proclib

testvideoengine: $(OBJS)
	$(CXX) $(INCDIR) $(CFLAGS) $(VIDEOLDFLAGS) $(VIDEOLIBS) $(OBJS) -o testvideoengine


$(OBJS): $(SRCS)
	$(CXX) $(INCDIR) $(CFLAGS) $(VIDEOLDFLAGS) $(VIDEOLIBS)  -o $@ -c $<

testnetworkengine: testnetworkengine.o
	$(CXX)  $(INCDIR) $(CFLAGS) $(NETWORKLDFLAGS) $< $(NETWORKLIBS) -o $@ 

testnetworkengine.o: testnetworkengine.cpp
	$(CXX) $(INCDIR) $(CFLAGS)  -o $@ -c $< 

teststream: teststream.o
	$(CXX)  $(INCDIR) $(CFLAGS) $(NETWORKLDFLAGS) $< $(NETWORKLIBS) -o $@ 

teststream.o: teststream.cpp
	$(CXX) $(INCDIR) $(CFLAGS)  -o $@ -c $< 

testvp: testvp.o
	$(CXX) $(INCDIR) $(CFLAGS) $(ENCODINGFLAGS) $< $(ENCODINGLIBS) -o $@

testvp.o: testvp.cpp
	$(CXX) $(INCDIR) $(CFLAGS) -o $@ -c $<

proclib:
	cp $(rootDir)/../video/*.so $(rootDir)
	cp $(rootDir)/../network/*.so $(rootDir)
	cp $(rootDir)/../thread/*.so $(rootDir)
	cp $(rootDir)/../misc/*.so $(rootDir)
	cp $(rootDir)/../streaming/*.so $(rootDir)
	cp $(rootDir)/../*.so $(rootDir)
	cp $(rootDir)/../../libs/libsrtp/*.a $(rootDir)
	cp $(rootDir)/../../libs/*.a $(rootDir)
	cp $(rootDir)/../encoding/*.so $(rootDir)
clean:
	$(RM) *.o


 
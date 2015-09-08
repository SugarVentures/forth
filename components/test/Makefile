
CXX = g++
RM = rm -f

INCDIR = -I$(rootDir)/../include -I$(rootDir)/../interface -I$(rootDir)/../thread -I$(rootDir)/../misc \
		-I$(rootDir)/../../libs/libsrtp/include -I$(rootDir)/../../libs/libsrtp/crypto/include  \
		-I$(rootDir)/../../libs/libyuv/include -I$(rootDir)/../encoding \
		-I$(rootDir)/../../libs/libvpx -I$(rootDir)/../error \
		-I$(rootDir)/../network \
		-I$(rootDir)/../streaming \
		-I$(rootDir)/../server -I$(rootDir)/../client  \
		-I$(rootDir)/../audio

INCDIR += -I$(rootDir)/../../libs/libnice -I$(rootDir)/../../libs/libnice/agent \
		  -I/usr/local/include/glib-2.0 \
		  -I$(rootDir)/../../libs/libnice/include

NICELDFLAGS = -L$(rootDir)/../../libs
NICELIBS = -lnetworkengine -lstreaming

CFLAGS	= -Wall -g -fPIC -std=c++11

VIDEOLDFLAGS = -L$(rootDir)/../../libs
VIDEOLIBS = -lvideoengine -lstreaming -lyuv

NETWORKLDFLAGS = -L$(rootDir)/../../libs
#NETWORKLIBS = -lnetworkengine -lstreaming -lyuv -lencodingengine -lserver -lclient
NETWORKLIBS = -lnetworkengine -lserver -lclient

ENCODINGFLAGS = -L$(rootDir)/../../libs -L/Users/caominhtrang/Downloads/libvpx
ENCODINGLIBS = -lyuv -lvpx

SRCS = testvideoengine.cpp
OBJS = $(subst .cpp,.o,$(SRCS))

AUDIOLDFLAGS = -L$(rootDir)/../../libs
AUDIOLIBS = -laudioengine

rootDir = .

#all: testvideoengine testnetworkengine teststream testvp teststun testbuffer testdatastream proclib
all: testaudio proclib

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

teststun: teststun.o
	$(CXX)  $(INCDIR) $(CFLAGS) $(NETWORKLDFLAGS) $< $(NETWORKLIBS) -o $@ 

teststun.o: teststun.cpp
	$(CXX) $(INCDIR) $(CFLAGS) -o $@ -c $<

testbuffer: testbuffer.o
	$(CXX)  $(INCDIR) $(CFLAGS) $(NETWORKLDFLAGS) $< $(NETWORKLIBS) -o $@ 

testbuffer.o: testbuffer.cpp
	$(CXX) $(INCDIR) $(CFLAGS) -o $@ -c $<

testdatastream: testdatastream.o
	$(CXX)  $(INCDIR) $(CFLAGS) $(NETWORKLDFLAGS) $< $(NETWORKLIBS) -o $@ 

testdatastream.o: testdatastream.cpp
	$(CXX) $(INCDIR) $(CFLAGS) -o $@ -c $<

testice: testice.o
	$(CXX) $(INCDIR) $(CFLAGS) $(NICELDFLAGS) $< $(NICELIBS) -o $@

testice.o: testice.cpp
	$(CXX) $(INCDIR) $(CFLAGS) -o $@ -c $<

testaudio: testaudio.o
	$(CXX) $(INCDIR) $(CFLAGS) $(AUDIOLDFLAGS) $< $(AUDIOLIBS) -o $@

testaudio.o: testaudio.cpp
	$(CXX) $(INCDIR) $(CFLAGS) -o $@ -c $<

proclib:
	cp $(rootDir)/../video/*.so $(rootDir)
	cp $(rootDir)/../network/*.so $(rootDir)
	cp $(rootDir)/../thread/*.so $(rootDir)
	cp $(rootDir)/../misc/*.so $(rootDir)
	cp $(rootDir)/../streaming/*.so $(rootDir)
	cp $(rootDir)/../../libs/libsrtp/*.a $(rootDir)
	cp $(rootDir)/../../libs/*.a $(rootDir)
	cp $(rootDir)/../encoding/*.so $(rootDir)
	cp $(rootDir)/../client/*.so $(rootDir)
	
clean:
	$(RM) *.o


 

CXX = g++
CC = gcc
RM = rm -f
CP = cp

INCDIR = -I$(rootDir)/../include -I$(rootDir)/../interface  -I$(rootDir)/../error \
		 -I$(rootDir)/../encoding -I$(rootDir)/../../libs/libyuv/include \
		 -I$(rootDir)/../../libs/libvpx

CFLAGS = -Wall -g
FRAMEWORKS = -framework Foundation -framework AVFoundation -framework CoreGraphics -framework CoreFoundation -framework CoreVideo \
 -framework CoreMedia -framework ApplicationServices -framework QuartzCore \
 -framework IOSurface

LDFLAGS = -L$(rootDir)/../../libs
LIBS = 

OBJLIBS = libvideoengine.so

SRCS = mac_video_capture.cpp 
OBJS = $(subst .cpp,.o,$(SRCS))

OCSRCS = mac_video_source_info.mm mac_video_avfoundation_capture.mm
OCOBJS = $(subst .mm,.o,$(OCSRCS))

rootDir = .

all: $(OBJLIBS) install

$(OBJLIBS): $(OCOBJS) $(OBJS) 
	$(CXX) $(INCDIR) $(LDFLAGS) $(LIBS) $(FRAMEWORKS) -fgnu-runtime -shared -o $(OBJLIBS) $(OBJS) $(OCOBJS)


mac_video_source_info.o: mac_video_source_info.mm 
	$(CXX) $(INCDIR) $(CFLAGS) $(FRAMEWORKS) -c  -o $@ $<

mac_video_avfoundation_capture.o: mac_video_avfoundation_capture.mm
	$(CXX) $(INCDIR) $(CFLAGS) $(FRAMEWORKS) -c  -o $@ $<


$(OBJS): $(SRCS)
	$(CXX) $(INCDIR) $(CFLAGS) -c -o $@ $<	

install:
	$(CP) *.so $(rootDir)/../../libs

clean:
	$(RM) *.o
	$(RM) *.so

 
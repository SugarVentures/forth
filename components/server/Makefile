
CXX = g++
RM = rm -f

INCDIR = -I$(rootDir)/../include -I$(rootDir)/../interface  -I$(rootDir)/../error  \
		 -I$(rootDir)/../network -I$(rootDir)/../thread -I$(rootDir)/../misc

CFLAGS = -Wall -g -fPIC -std=c++11

LDFLAGS += -Wl -L$(rootDir)/../../libs/
LIBS = -lnetworkengine -lthreadengine

SIG_SRCS = signaling_server_subthread.cpp signaling_server_thread.cpp signaling_server.cpp signaling_server_main.cpp
SIG_OBJS = $(subst .cpp,.o,$(SIG_SRCS))

FORWD_SRCS = forwarding_server_subthread.cpp forwarding_server_thread.cpp forwarding_server.cpp forwarding_server_main.cpp
FORWD_OBJS = $(subst .cpp,.o,$(FORWD_SRCS))


rootDir = .

all: sig_server forwarding_server

sig_server: $(SIG_OBJS)
	mkdir -p bin
	$(CXX) $(INCDIR) $(LDFLAGS) -o bin/sig_server $(SIG_OBJS) $(LIBS)

forwarding_server: $(FORWD_OBJS)
	$(CXX) $(INCDIR) $(LDFLAGS)  $(FORWD_OBJS) $(LIBS) -o bin/forwarding_server

%.o: %.cpp
	$(CXX) $(INCDIR) $(CFLAGS) -c -o $@ $<	

clean:
	$(RM) bin/*

 
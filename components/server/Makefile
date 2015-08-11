
CXX = g++
RM = rm -f

INCDIR = -I$(rootDir)/../include -I$(rootDir)/../interface  -I$(rootDir)/../error  \
		 -I$(rootDir)/../network -I$(rootDir)/../thread -I$(rootDir)/../misc

CFLAGS = -Wall -g -fPIC -std=c++11

LDFLAGS = -L$(rootDir)/../../libs/
LIBS = -lnetworkengine -lthreadengine

SRCS = signaling_server_thread.cpp signaling_server.cpp signaling_server_main.cpp
OBJS = $(subst .cpp,.o,$(SRCS))



rootDir = .

all: sig_server

sig_server: $(OBJS)
	$(CXX) $(INCDIR) $(LDFLAGS) -o sig_server $(OBJS) $(LIBS)


%.o: %.cpp
	$(CXX) $(INCDIR) $(CFLAGS) -c -o $@ $<	

clean:
	$(RM) *.o

 
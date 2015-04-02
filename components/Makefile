
CXX = g++
RM = rm -f
CP = cp

INCDIR = -I$(rootdir)/include -I$(rootdir)/interface   \
	-I$(rootdir)/../libs/libsrtp/include -I$(rootdir)/../libs/libsrtp/crypto/include  
CFLAGS	= -Wall -g

NETWORKLDFLAGS = -L$(rootdir)/network -L$(rootdir)/../libs/libsrtp
NETWORKLIBS = -lnetworkengine -lsrtp


SRCS = utility.cpp stream.cpp
OBJS = $(subst .cpp,.o,$(SRCS))

rootdir = .

all: liboppvs.so install


liboppvs.so: $(OBJS)
	$(CXX) $(INCDIR) $(CFLAGS) $(NETWORKLDFLAGS) $(NETWORKLIBS) -shared -o $@ $(OBJS)


%.o: %.cpp
	$(CXX) $(INCDIR) $(CFLAGS) $(NETWORKLDFLAGS) $(NETWORKLIBS) -c -o $@ $<

install:
	$(CP) *.so $(rootdir)/../libs

clean:
	$(RM) *.o
	$(RM) *.so


 
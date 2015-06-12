CXX = gcc
RM = rm -f

INCDIR = -I$(rootDir)/../include

CFLAGS = -Wall -g

OBJLIBS = libyuv.a

SRCS = $(wildcard *.cc)
OBJS = $(subst .cc,.o,$(SRCS))

rootDir = .

all: $(OBJLIBS) install

$(OBJLIBS): $(OBJS)
	$(CXX) $(INCDIR)  -fgnu-runtime -shared -o $(OBJLIBS) $(OBJS) $(LIBS)


%.o: %.cc
	$(CXX) $(INCDIR) $(CFLAGS) -c -o $@ $<	

install:
	cp *.a $(rootDir)/../../

clean:
	$(RM) *.o
	$(RM) *.a




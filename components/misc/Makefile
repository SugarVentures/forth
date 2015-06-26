CC = gcc

INCDIR = -I$(rootDir)/../../libs/libsrtp/include -I$(rootDir)/../../libs/libsrtp/crypto/include  

CFLAGS = -Wall -g

LDFLAGS = -L$(rootDir)/../../libs/libsrtp
LIBS = -lsrtp

rootDir = .

OBJLIBS = libmisc.so

SRCS = md5.c random_unique32.c utility.c
OBJS = $(subst .c,.o,$(SRCS))

all: $(OBJLIBS) md5driver.o install

$(OBJLIBS): $(OBJS)
	$(CC) $(INCDIR) $(LDFLAGS) $(CFLAGS) -fgnu-runtime -shared -o $(OBJLIBS) $(OBJS) $(LIBS)

%.o: %.c
	$(CC) $(INCDIR) $(CFLAGS) -c -o $@ $<	

md5driver.o: md5main.c
	$(CC) $(CFLAGS)  -o $@ -L. -lmisc $<


install:
	cp *.so ./../../libs

clean:
	rm *.o
	rm *.so


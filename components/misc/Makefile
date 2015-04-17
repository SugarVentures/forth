CC = gcc
CFLAGS = -Wall -g 

OBJLIBS = libmisc.so

SRCS = md5.c random_unique32.c
OBJS = $(subst .c,.o,$(SRCS))

all: $(OBJLIBS) md5driver.o

$(OBJLIBS): $(OBJS)
	$(CC) $(CFLAGS) -fgnu-runtime -shared -o $(OBJLIBS) $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<	

md5driver.o: md5main.c
	$(CC) $(CFLAGS)  -o $@ -L. -lmisc $<

clean:
	rm *.o
	rm *.so


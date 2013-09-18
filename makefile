CC = gcc
CFLAGS=-g 
LIBS=-lpthread 
FLIBS=-lfl

CCFILES = myshell.c\
	lex.yy.c

OFILES = $(CCFILES:.c=.o)

all: link

.cc.o:	 $(CCFILES) $(HFILES)
	$(CC) $(CFLAGS) -c $*.cc

link:	$(OFILES)
	$(CC) $(CFLAGS) $(LIBS)  -o run $(OFILES) $(FLIBS)
clean:
	rm -f *.o *.*~ run

# CC = gcc
# CFLAGS=-std=c99 -Wall -g
# LIBS=-lpthread
# FLIBS=-lfl

# CFILES = myshell.c\
# 	lex.yy.c

# OFILES = $(CFILES:.c=.o)

# all: lab

# lab: $(CFILES) $(HFILES)
# 	$(CC) $(CFLAGS) -c $*.c

# link: $(OFILES)
# 	$(CC) $(CFLAGS) $(LIBS)  -o run $(OFILES) $(FLIBS)

# clean:
# 	rm -f *.o *.*~ run
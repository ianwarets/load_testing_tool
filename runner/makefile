# web request
# MAKEFILE

CC=gcc
CFLAGS=-Wall
ifeq ($(mode), debug)
	CFLAGS+=-g3 -O0 -DDEBUG
endif

CURL=C:\Programming\C\libraries\curl-7.61.1
ZLIB=C:\Programming\C\libraries\zlib-1.2.11
BROTLI=C:\Programming\C\libraries\libbrotli-master
ZLOG=C:\Programming\C\libraries\WinZlog\Release
UJSON4C=C:\Programming\C\libraries\ujson4c

INCLUDE=-I$(CURL)\include -I$(ZLOG)\head -I$(UJSON4C)\include
LDFLAGS=-L$(CURL)\lib\.libs -L$(ZLIB) -L$(BROTLI)\.libs -L$(ZLOG)\lib -L$(UJSON4C)\lib -L.
LDLIBS=-lcurl -lujdecode -lliblogger -lzlog

REFS=liblogger.dll main.o action_wrappers.o test_controller.o test_plan.o
MACROS=-DCURL_STATICLIB
EXENAME=loadtestingtool

all: build

build: $(REFS)	
	$(CC) $^ $(CFLAGS) -o $(EXENAME).exe $(INCLUDE) $(LDFLAGS) $(LDLIBS) 

liblogger.dll: logger.o
	$(CC) $< -Wall -shared -Wl,-soname,liblogger.so -o $@  -L$(ZLOG)\lib -lzlog

main.o: main.c main.h
	$(CC) -c $< $(CFLAGS) -I$(ZLOG)\head

test_plan.o: test_plan.c test_plan.h
	$(CC) -c $< $(CFLAGS) -I$(ZLOG)\head -I$(UJSON4C)\include

test_controller.o: test_controller.c test_controller.h
	$(CC) -c $< $(CFLAGS) -I$(ZLOG)\head

action_wrappers.o: action_wrappers.c action_wrappers.h
	$(CC) -c $< $(CFLAGS) -I$(ZLOG)\head

logger.o: logger.c logger.h
	$(CC) -c -fPIC $< $(CFLAGS) -I$(ZLOG)\head

clean:
	del /S *.o
	del /S liblogger.*
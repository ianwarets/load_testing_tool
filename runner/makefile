# web request
# MAKEFILE

CC=gcc
CFLAGS=-Wall

CURL=C:\Programming\C\libraries\curl-7.61.0
ZLIB=C:\Programming\C\libraries\zlib-1.2.11
OPEN_SSL=C:\Programming\C\libraries\openssl-1.0.2p
BROTLI=C:\Programming\C\libraries\libbrotli-master
ZLOG=C:\Programming\C\libraries\WinZlog\Release
UJSON4C=C:\Programming\C\libraries\ujson4c

INCLUDE=-I$(CURL)\include -I$(ZLOG)\head -I$(UJSON4C)\include
LDFLAGS=-L$(CURL)\lib -L$(ZLIB) -L$(OPEN_SSL)\lib -L$(BROTLI)\.libs -L$(ZLOG)\lib -L$(UJSON4C)\lib
LDLIBS=-lcurl -lgdi32 -lwldap32 -lws2_32 -lz -lssl -lcrypto -lbrotlidec -lzlog -lujdecode

REFS=main.o logger.o action_wrappers.o test_controller.o test_plan.o
MACROS=-DCURL_STATICLIB
EXENAME=loadlestingtool

all: build

build: $(REFS)
	$(CC) $(MACROS) $^ $(CFLAGS) $(INCLUDE) $(LDFLAGS) $(LDLIBS) -o $(EXENAME).exe

debug: $(REFS)
	$(CC) $(MACROS) -DDEBUG $^ $(CFLAGS) -g $(INCLUDE) $(LDFLAGS) $(LDLIBS) -o $(EXENAME).exe

main.o: main.c main.h
	$(CC) -c $< $(CFLAGS) -I$(ZLOG)\head

test_plan.o: test_plan.c test_plan.h
	$(CC) -c $< $(CFLAGS) -I$(ZLOG)\head -I$(UJSON4C)\include

test_controller.o: test_controller.c test_controller.h
	$(CC) -c $< $(CFLAGS) -I$(ZLOG)\head

action_wrappers.o: action_wrappers.c action_wrappers.h
	$(CC) -c $< $(CFLAGS) -I$(ZLOG)\head

logger.o: logger.c logger.h
	$(CC) -c $< $(CFLAGS) -I$(ZLOG)\head
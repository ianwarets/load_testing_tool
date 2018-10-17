# web request
# MAKEFILE

CC=gcc

CC=gcc

CFLAGS=-Wall -fPIC -c

ifeq ($(mode), debug)
	CFLAGS+=-Wall -fPIC -c -g3 -O0 -DDEBUG
endif


CDIR=C:\Programming\C
CURL=$(CDIR)\libraries\curl-7.61.1
ZLIB=$(CDIR)\libraries\zlib-1.2.11
OPEN_SSL=$(CDIR)\libraries\openssl-1.0.2p
BROTLI=$(CDIR)\libraries\libbrotli-master
ZLOG=$(CDIR)\libraries\WinZlog\Release
LTT=..\runner

INCLUDE=-I$(CURL)\include -I$(ZLOG)\head -I$(LTT) -Iinclude
LDPATH=-L$(CURL)\lib\.libs -L$(ZLIB) -L$(OPEN_SSL)\lib -L$(BROTLI)\.libs -L$(ZLOG)\lib -L$(LTT)
LDLIBS=-lcurl -lwldap32 -lz -lssl -lcrypto -lgdi32 -lbrotlidec -lws2_32 -lzlog -lliblogger

REFS=bin/http_requests.o bin/transactions.o
MACROS=-DCURL_STATICLIB

all: $(REFS)
	$(CC) -Wall -shared -fPIC -Wl,-soname,libtrans.so -o bin\libtrans.dll bin/transactions.o -I$(LTT) -L$(LTT) -L$(ZLOG)\lib -lzlog -lliblogger
	$(CC) -Wall -shared -fPIC -Wl,-soname,libhreq.so -o bin\libhreq.dll bin/http_requests.o $(INCLUDE) $(LDPATH) $(LDLIBS) 

bin/http_requests.o: http_requests.c
	$(CC) $(CFLAGS)  $(INCLUDE) $< -o $@

bin/transactions.o: transactions.c
	$(CC) $(CFLAGS) $(INCLUDE) $< -o $@

clean:
	del /S *.o
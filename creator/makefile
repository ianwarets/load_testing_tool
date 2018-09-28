# web request
# MAKEFILE

CC=gcc
CFLAGS=-Wall -c

CDIR=C:\Programming\C
CURL=$(CDIR)\libraries\curl-7.61.0
ZLIB=$(CDIR)\libraries\zlib-1.2.11
OPEN_SSL=$(CDIR)\libraries\openssl-1.0.2p
BROTLI=$(CDIR)\libraries\libbrotli-master
ZLOG=$(CDIR)\libraries\WinZlog\Release
LTT=$(CDIR)\load_testing_tool\runner

INCLUDE=-I$(CURL)\include -I$(ZLOG)\head -I$(LTT)
LDFLAGS=-L$(CURL)\lib -L$(ZLIB) -L$(OPEN_SSL)\lib -L$(BROTLI)\.libs
LDLIBS=-lcurl -lgdi32 -lwldap32 -lws2_32 -lz -lssl -lcrypto -lbrotlidec

REFS=bin/static/http_requests.o bin/static/transactions.o
MACROS=-DCURL_STATICLIB

all: build

static: $(REFS)
	ar rcs  bin/static/libhreq.a bin/static/http_requests.o
	ar rcs bin/static/libtrans.a bin/static/transactions.o

bin/static/http_requests.o: http_requests.c http_requests.h
	$(CC) $(MACROS) $(CFLAGS) $< $(INCLUDE) $(LDFLAGS) $(LDLIBS) -o $@

bin/static/transactions.o: transactions.c transactions.h
	$(CC) $(CFLAGS) $< $(INCLUDE) -o $@
# web request
# MAKEFILE

CC=gcc
CFLAGS=-Wall

CURL=C:\Programming\C\libraries\curl-7.61.0
ZLIB=C:\Programming\C\libraries\zlib-1.2.11
OPEN_SSL=C:\Programming\C\libraries\openssl-1.0.2p
BROTLI=C:\Programming\C\libraries\libbrotli-master
ZLOG=C:\Programming\C\libraries\WinZlog\Release

INCLUDE=-I$(CURL)\include -I$(ZLOG)\head
LDFLAGS=-L$(CURL)\lib -L$(ZLIB) -L$(OPEN_SSL)\lib -L$(BROTLI)\.libs -L$(ZLOG)\lib
LDLIBS=-lcurl -lgdi32 -lwldap32 -lws2_32 -lz -lssl -lcrypto -lbrotlidec -lzlog

SOURCES=main.c http_requests.c

all: build

build: 
	$(CC) -DCURL_STATICLIB $(SOURCES) $(CFLAGS) $(INCLUDE) $(LDFLAGS) $(LDLIBS) -o main.exe

debug:
	$(CC) -DCURL_STATICLIB -DDEBUG $(SOURCES) $(CFLAGS) -g $(INCLUDE) $(LDFLAGS) $(LDLIBS) -o main.exe
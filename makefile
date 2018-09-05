# web request
# MAKEFILE

CC=gcc
CFLAGS=-Wall

CURL=C:\Programming\C\curl-7.61.0
ZLIB=C:\Programming\C\zlib-1.2.11
OPEN_SSL=C:\Programming\C\openssl-1.0.2p
BROTLI=C:\Programming\C\libbrotli-master\brotli\c

INCLUDE=-I$(CURL)\include
LDFLAGS=-L$(CURL)\lib -L$(ZLIB) -L$(OPEN_SSL)\lib -L$(BROTLI)\lib
LDLIBS=-lcurl -lgdi32 -lwldap32 -lws2_32 -lz -lssl -lcrypto -lbrotlidec

SOURCES=main.c http_requests.c

all: build

build: 
	$(CC) -DCURL_STATICLIB $(SOURCES) $(CFLAGS) $(INCLUDE) $(LDFLAGS) $(LDLIBS) -o main.exe

debug:
	$(CC) -DCURL_STATICLIB -DDEBUG $(SOURCES) $(CFLAGS) -g $(INCLUDE) $(LDFLAGS) $(LDLIBS) -o main.exe
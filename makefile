# web request
# MAKEFILE

# libcurl must be downloaded from https://curl.haxx.se/download.html and installed on machine

CC=gcc
CFLAGS=-Wall
CFLAGSDLL=-Wall -fPIC -c
ifeq ($(mode), debug)
	CFLAGS+=-g3 -O0 -DDEBUG
	CFLAGSDLL+=-g3 -O0
else
	CFLAGS+=-O3
	CFLAGSDLL+=-O3
endif

INCLUDE= -Iinclude
LDPATH= -L\lib 
LDLIBSTRANS=-llogger
LDLIBS=-lcurl -lujdecode $(LDLIBSTRANS) -lpthread -ldl -lm -lcdk -lp7-shared
LDLIBSREQUESTS=-lcurl $(LDLIBSTRANS)
# -lwldap32 -lz -lssl -lcrypto -lgdi32 -lbrotlidec -lws2_32
REFS=lib/liblogger.so lib/libtrans.so lib/libhreq.so main.o ltt_common.o action_wrappers.o test_controller.o test_plan.o
MACROS=-DCURL_STATICLIB
# This is for enabling gnu functions such as pthread_tryjoin_n[]
USEGNU=-D_GNU_SOURCE
EXENAME=loadtestingtool
# Pass shared library path to the linker to search at programm start.
RPATH=-Wl,-rpath,'$$ORIGIN/../lib'

all: build

build: $(REFS)	
	$(CC) $^ $(CFLAGS) -o bin/$(EXENAME).exe $(INCLUDE) $(LDPATH) $(LDLIBS) $(RPATH)

lib/liblogger.so: logger.o
	$(CC) $< -Wall -shared -Wl,-soname,liblogger.so -o $@  -Iinclude -Llib

lib/libtrans.so: transactions.o ltt_common.o
	$(CC) $^   -Wall -shared -fPIC -Wl,-soname,libtrans.so -o $@ -Iinclude -Llib $(LDLIBSTRANS)

lib/libhreq.so: http_requests.o
	$(CC) $< -Wall -shared -fPIC -Wl,-soname,libhreq.so -o $@ -Iinclude -Llib $(INCLUDE) $(LDPATH) $(LDLIBSREQUESTS)

main.o: main.c
	$(CC) -c $< $(CFLAGS) -Iinclude $(USEGNU)

test_plan.o: test_plan.c
	$(CC) -c $< $(CFLAGS) -Iinclude

test_controller.o: test_controller.c
	$(CC) -c $< $(CFLAGS) -Iinclude $(USEGNU)

action_wrappers.o: action_wrappers.c
	$(CC) -c $< $(CFLAGS) -Iinclude

ltt_common.o: ltt_common.c
	$(CC) -c $< $(CFLAGS) -Iinclude

logger.o: logger.c
	$(CC) -c -fPIC $< $(CFLAGS) -Iinclude -Iinclude/p7

http_requests.o: http_requests.c
	$(CC) $(CFLAGSDLL) $(INCLUDE) $< -o $@

transactions.o: transactions.c
	$(CC) $(CFLAGSDLL) $(INCLUDE) $< -o $@

clean:
	rm -r *.o
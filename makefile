# web request
# MAKEFILE

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

CURL=C:\Programming\C\libraries\curl-7.61.1
ZLIB=C:\Programming\C\libraries\zlib-1.2.11
BROTLI=C:\Programming\C\libraries\libbrotli-master
ZLOG=C:\Programming\C\libraries\WinZlog\Release
UJSON4C=C:\Programming\C\libraries\ujson4c

INCLUDE=-I$(CURL)\include -I$(ZLOG)\head -I$(UJSON4C)\include -Iinclude
LDPATH=-L$(CURL)\lib\.libs -L$(ZLIB) -L$(OPEN_SSL)\lib -L$(BROTLI)\.libs -L$(ZLOG)\lib -L$(UJSON4C)\lib -Llib 
LDLIBSTRANS=-lzlog -lliblogger
LDLIBS=-lcurl -lujdecode $(LDLIBSTRANS)
LDLIBSREQUESTS=-lcurl -lwldap32 -lz -lssl -lcrypto -lgdi32 -lbrotlidec -lws2_32 $(LDLIBSTRANS)

REFS=lib/liblogger.dll lib/libtrans.dll lib/libhreq.dll main.o action_wrappers.o test_controller.o test_plan.o
MACROS=-DCURL_STATICLIB
EXENAME=loadtestingtool

all: build

build: $(REFS)	
	$(CC) $^ $(CFLAGS) -o bin/$(EXENAME).exe $(INCLUDE) $(LDPATH) $(LDLIBS) 

lib/liblogger.dll: logger.o
	$(CC) $< -Wall -shared -Wl,-soname,liblogger.so -o $@  -Iinclude -L$(ZLOG)\lib -lzlog

lib/libtrans.dll: transactions.o
	$(CC) $< -Wall -shared -fPIC -Wl,-soname,libtrans.so -o $@ -Iinclude -Llib -L$(ZLOG)\lib $(LDLIBSTRANS)

lib/libhreq.dll: http_requests.o
	$(CC) $< -Wall -shared -fPIC -Wl,-soname,libhreq.so -o $@ -Iinclude -Llib $(INCLUDE) $(LDPATH) $(LDLIBSREQUESTS)

main.o: main.c include/main.h
	$(CC) -c $< $(CFLAGS) -Iinclude -I$(ZLOG)\head

test_plan.o: test_plan.c include/test_plan.h
	$(CC) -c $< $(CFLAGS) -Iinclude -I$(ZLOG)\head -I$(UJSON4C)\include

test_controller.o: test_controller.c include/test_controller.h
	$(CC) -c $< $(CFLAGS) -Iinclude -I$(ZLOG)\head

action_wrappers.o: action_wrappers.c include/action_wrappers.h
	$(CC) -c $< $(CFLAGS) -Iinclude -I$(ZLOG)\head

logger.o: logger.c include/logger.h
	$(CC) -c -fPIC $< $(CFLAGS) -Iinclude -I$(ZLOG)\head

http_requests.o: http_requests.c
	$(CC) $(CFLAGSDLL) $(INCLUDE) $< -o $@

transactions.o: transactions.c
	$(CC) $(CFLAGSDLL) $(INCLUDE) $< -o $@

clean:
	del /S *.o
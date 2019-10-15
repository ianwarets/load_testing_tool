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

LIBRARIES=C:\Programming\C\libraries
# CURL=$(LIBRARIES)\curl-7.61.1
# ZLIB=$(LIBRARIES)\zlib-1.2.11
# BROTLI=$(LIBRARIES)\libbrotli-master
UJSON4C=$(LIBRARIES)\ujson4c
P7=C:\Programming\C\libs\libP7Client_v5.1\Headers

INCLUDE=-I$(CURL)\include -I$(UJSON4C)\include -Iinclude
LDPATH= -L\lib 
LDLIBSTRANS=-llogger
LDLIBS=-lcurl -lujdecode $(LDLIBSTRANS) -lpthread -ldl -lm
LDLIBSREQUESTS=-lcurl $(LDLIBSTRANS)
# -lwldap32 -lz -lssl -lcrypto -lgdi32 -lbrotlidec -lws2_32
REFS=lib/liblogger.so lib/libtrans.so lib/libhreq.so main.o action_wrappers.o test_controller.o test_plan.o ltt_common.o
MACROS=-DCURL_STATICLIB
# This is for enabling gnu functions such as pthread_tryjoin_n[]
USEGNU=-D_GNU_SOURCE
EXENAME=loadtestingtool

all: build

build: $(REFS)	
	$(CC) $^ $(CFLAGS) -o bin/$(EXENAME).exe $(INCLUDE) $(LDPATH) $(LDLIBS) 

lib/liblogger.so: logger.o
	$(CC) $< -Wall -shared -Wl,-soname,liblogger.so -o $@  -Iinclude -Llib -lP7

lib/libtrans.so: transactions.o
	$(CC) $< -Wall -shared -fPIC -Wl,-soname,libtrans.so -o $@ -Iinclude -Llib $(LDLIBSTRANS)

lib/libhreq.so: http_requests.o
	$(CC) $< -Wall -shared -fPIC -Wl,-soname,libhreq.so -o $@ -Iinclude -Llib $(INCLUDE) $(LDPATH) $(LDLIBSREQUESTS)

main.o: main.c include/main.h
	$(CC) -c $< $(CFLAGS) -Iinclude $(USEGNU)

test_plan.o: test_plan.c include/test_plan.h
	$(CC) -c $< $(CFLAGS) -Iinclude -I$(UJSON4C)\include

test_controller.o: test_controller.c include/test_controller.h
	$(CC) -c $< $(CFLAGS) -Iinclude $(USEGNU)

action_wrappers.o: action_wrappers.c include/action_wrappers.h
	$(CC) -c $< $(CFLAGS) -Iinclude

ltt_common.o: ltt_common.c include/ltt_common.h
	$(CC) -c $< $(CFLAGS) -Iinclude

logger.o: logger.c include/logger.h
	$(CC) -c -fPIC $< $(CFLAGS) -Iinclude -I$(P7)

http_requests.o: http_requests.c
	$(CC) $(CFLAGSDLL) $(INCLUDE) $< -o $@

transactions.o: transactions.c
	$(CC) $(CFLAGSDLL) $(INCLUDE) $< -o $@

clean:
	rm -r *.o
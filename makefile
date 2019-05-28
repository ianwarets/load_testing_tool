# web request
# MAKEFILE

CC=cl
CFLAGS=--TC -Wall
CFLAGSDLL=-Wall -fPIC -c

LINKER=link
LFLAGS=
LFLAGSDLL=

ifeq ($(mode), debug)
	CFLAGS+=-Z7 -Od -DDEBUG
	CFLAGSDLL+=-Z7 -Od
else
	CFLAGS+=-O2
	CFLAGSDLL+=-O2
endif

LIBRARIES=C:\Programming\C\libraries
CURL=$(LIBRARIES)\curl-7.61.1
ZLIB=$(LIBRARIES)\zlib-1.2.11
BROTLI=$(LIBRARIES)\libbrotli-master
ZLOG=$(LIBRARIES)\WinZlog\Release
UJSON4C=$(LIBRARIES)\ujson4c
LOG4C=$(LIBRARIES)\log4c
P7=C:\Programming\C\libs\libP7Client_v5.1\Headers

INCLUDE=-I$(CURL)\include -I$(UJSON4C)\include -Iinclude
LDPATH=-L$(CURL)\lib\.libs -L$(ZLIB) -L$(OPEN_SSL)\lib -L$(BROTLI)\.libs -L$(UJSON4C)\lib -Llib 
LDLIBSTRANS=-lliblogger
LDLIBS=-lcurl -lujdecode $(LDLIBSTRANS)
LDLIBSREQUESTS=-lcurl -lwldap32 -lz -lssl -lcrypto -lgdi32 -lbrotlidec -lws2_32 $(LDLIBSTRANS)

REFS=lib/liblogger.dll lib/libtrans.dll lib/libhreq.dll main.o action_wrappers.o test_controller.o test_plan.o
EXENAME=loadtestingtool

all: build

build: $(REFS)	
	$(CC) $^ $(CFLAGS) -o bin/$(EXENAME).exe $(INCLUDE) $(LDPATH) $(LDLIBS) 

lib/liblogger.dll: logger.o
	$(CC) $< -Wall -shared -Wl,-soname,liblogger.so -o $@  -Iinclude -Llib -lP7x64

lib/libtrans.dll: transactions.o
	$(CC) $< -Wall -shared -fPIC -Wl,-soname,libtrans.so -o $@ -Iinclude -Llib $(LDLIBSTRANS)

lib/libhreq.dll: http_requests.o
	$(CC) $< -Wall -shared -fPIC -Wl,-soname,libhreq.so -o $@ -Iinclude -Llib $(INCLUDE) $(LDPATH) $(LDLIBSREQUESTS)

main.o: main.c include/main.h
	$(CC) -c $< $(CFLAGS) -Iinclude

test_plan.o: test_plan.c include/test_plan.h
	$(CC) -c $< $(CFLAGS) -Iinclude -I$(UJSON4C)\include

test_controller.o: test_controller.c include/test_controller.h
	$(CC) -c $< $(CFLAGS) -Iinclude

action_wrappers.o: action_wrappers.c include/action_wrappers.h
	$(CC) -c $< $(CFLAGS) -Iinclude

logger.o: logger.c include/logger.h
	$(CC) -c -fPIC $< $(CFLAGS) -Iinclude -I$(P7)

http_requests.o: http_requests.c
	$(CC) $(CFLAGSDLL) $(INCLUDE) $< -o $@

transactions.o: transactions.c
	$(CC) $(CFLAGSDLL) $(INCLUDE) $< -o $@

clean:
	del /S *.o
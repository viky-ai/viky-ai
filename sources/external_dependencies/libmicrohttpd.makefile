#
# Extrenal lib libmicrohttpd
#
# depends makeinfo: sudo apt-get install texinfo

CURRENT_MAKEFILE := $(word $(words $(MAKEFILE_LIST)),$(MAKEFILE_LIST))

include $(OG_REPO_PATH)/sources/makefile.defs.linux

.PHONY: all redebug debug build make clean fullclean

redebug: debug

debug: build

rebuild: build

build: $(DBINPATH)/libmicrohttpd.so $(SRCPATH)/include/microhttpd.h

libmicrohttpd: all

all:
	$(MAKE) -f $(CURRENT_MAKEFILE) fullclean
	$(MAKE) -f $(CURRENT_MAKEFILE) build

fullclean: clean
	cd libmicrohttpd && $(MAKE) clean

clean:
	rm -f $(SRCPATH)/include/microhttpd.h
	rm -f $(DBINPATH)/libmicrohttpd.so*

make: ./Makefile
	cd libmicrohttpd && $(MAKE)

./configure:
	cd libmicrohttpd && autoreconf -fi

./Makefile: ./configure
	cd libmicrohttpd && ./configure --prefix="$(DBUILDPATH)" \
																	--bindir="$(DBINPATH)" \
																	--libdir="$(DLIBPATH)" \
																	--includedir="$(SRCPATH)/include" \
																	--disable-https

$(DBINPATH)/libmicrohttpd.so: make
	cd libmicrohttpd && cp -af src/microhttpd/.libs/libmicrohttpd.so* $(DBINPATH)


$(SRCPATH)/include/microhttpd.h: make
	cd libmicrohttpd && cp -af src/include/microhttpd.h $(SRCPATH)/include/




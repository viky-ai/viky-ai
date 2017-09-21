#
# Extrenal lib libmicrohttpd
#
# depends makeinfo: sudo apt-get install texinfo

CURRENT_MAKEFILE := $(word $(words $(MAKEFILE_LIST)),$(MAKEFILE_LIST))

include $(OG_REPO_PATH)/sources/makefile.defs.linux

.PHONY: all redebug rebuild debug build make clean fullclean

redebug: rebuild

debug: build

rebuild:
	$(MAKE) -f $(CURRENT_MAKEFILE) clean
	$(MAKE) -f $(CURRENT_MAKEFILE) build

build: $(DBINPATH)/libmicrohttpd.so $(SRCPATH)/include/microhttpd.h

libmicrohttpd: all

all:
	$(MAKE) -f $(CURRENT_MAKEFILE) fullclean
	$(MAKE) -f $(CURRENT_MAKEFILE) build

fullclean: clean
	-cd libmicrohttpd && $(MAKE) clean
	rm -f libmicrohttpd/Makefile
	rm -f libmicrohttpd/configure

clean:
	rm -f $(SRCPATH)/include/microhttpd.h
	rm -f $(DBINPATH)/libmicrohttpd.so*

make: libmicrohttpd/Makefile
	cd libmicrohttpd && $(MAKE)

libmicrohttpd/configure:
	cd libmicrohttpd && autoreconf -fi

libmicrohttpd/Makefile: libmicrohttpd/configure
	cd libmicrohttpd && ./configure --prefix="$(DBUILDPATH)" \
																	--bindir="$(DBINPATH)" \
																	--libdir="$(DLIBPATH)" \
																	--includedir="$(SRCPATH)/include" \
																	--disable-https

$(DBINPATH)/libmicrohttpd.so: make
	cp -af libmicrohttpd/src/microhttpd/.libs/libmicrohttpd.so* $(DBINPATH)


$(SRCPATH)/include/microhttpd.h: make
	cp -af libmicrohttpd/src/include/microhttpd.h $(SRCPATH)/include/




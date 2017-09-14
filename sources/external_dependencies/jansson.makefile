#
# Extrenal lib Jansson
#

CURRENT_MAKEFILE := $(word $(words $(MAKEFILE_LIST)),$(MAKEFILE_LIST))

include $(OG_REPO_PATH)/sources/makefile.defs.linux

.PHONY: all redebug debug build make clean fullclean

redebug: rebuild

debug: build

rebuild:
	$(MAKE) -f $(CURRENT_MAKEFILE) clean
	$(MAKE) -f $(CURRENT_MAKEFILE) build

build: $(DBINPATH)/libjansson.so $(SRCPATH)/include/jansson.h $(SRCPATH)/include/jansson_config.h

jansson: all

all:
	$(MAKE) -f $(CURRENT_MAKEFILE) fullclean
	$(MAKE) -f $(CURRENT_MAKEFILE) build

fullclean: clean
	cd jansson && $(MAKE) clean
	rm -rf jansson/Makefile
	rm -rf jansson/configure

clean:
	rm -f $(SRCPATH)/include/jansson_config.h
	rm -f $(SRCPATH)/include/jansson.h
	rm -f $(DBINPATH)/libjansson.so*

make: jansson/Makefile
	cd jansson && $(MAKE)

jansson/configure:
	cd jansson && autoreconf -fi

jansson/Makefile: jansson/configure
	cd jansson && ./configure --prefix="$(DBUILDPATH)" \
																	--bindir="$(DBINPATH)" \
																	--libdir="$(DLIBPATH)" \
																	--includedir="$(SRCPATH)/include"

$(DBINPATH)/libjansson.so: make
	cp -af jansson/src/.libs/libjansson.so* $(DBINPATH)

$(SRCPATH)/include/jansson.h: make
	cp -af jansson/src/jansson.h $(SRCPATH)/include/

$(SRCPATH)/include/jansson_config.h: make
	cp -af jansson/src/jansson_config.h $(SRCPATH)/include/



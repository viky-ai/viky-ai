#
# Extrenal lib Jansson
#

CURRENT_MAKEFILE := $(word $(words $(MAKEFILE_LIST)),$(MAKEFILE_LIST))

include $(OG_REPO_PATH)/sources/makefile.defs.linux

.PHONY: all redebug debug build make clean fullclean


debug: $(DBINPATH)/libjansson.so $(SRCPATH)/include/jansson.h $(SRCPATH)/include/jansson_config.h

build: $(RBINPATH)/libjansson.so $(SRCPATH)/include/jansson.h $(SRCPATH)/include/jansson_config.h

rebuild:
	$(MAKE) -f $(CURRENT_MAKEFILE) clean
	$(MAKE) -f $(CURRENT_MAKEFILE) build

redebug:
	$(MAKE) -f $(CURRENT_MAKEFILE) clean
	$(MAKE) -f $(CURRENT_MAKEFILE) debug

glib: all

all:
	$(MAKE) -f $(CURRENT_MAKEFILE) fullclean
	$(MAKE) -f $(CURRENT_MAKEFILE) build debug

rebuild:
	$(MAKE) -f $(CURRENT_MAKEFILE) clean
	$(MAKE) -f $(CURRENT_MAKEFILE) build

build: $(DBINPATH)/libjansson.so $(SRCPATH)/include/jansson.h $(SRCPATH)/include/jansson_config.h

jansson: all

all:
	$(MAKE) -f $(CURRENT_MAKEFILE) fullclean
	$(MAKE) -f $(CURRENT_MAKEFILE) debug build

fullclean: clean
	-cd jansson && $(MAKE) clean
	-cd jansson && git clean -dfx
	rm -rf jansson/Makefile
	rm -rf jansson/configure

clean:
	rm -f $(SRCPATH)/include/jansson_config.h
	rm -f $(SRCPATH)/include/jansson.h
	rm -f $(DBINPATH)/libjansson.so*
	rm -f $(RBINPATH)/libjansson.so*

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
	mkdir -p $(DBINPATH)
	cp -af jansson/src/.libs/libjansson.so* $(DBINPATH)/

$(RBINPATH)/libjansson.so: $(DBINPATH)/libjansson.so
	mkdir -p $(RBINPATH)
	cp -af jansson/src/.libs/libjansson.so* $(RBINPATH)/

$(SRCPATH)/include/jansson.h: make
	mkdir -p $(SRCPATH)/include/
	cp -af jansson/src/jansson.h $(SRCPATH)/include/

$(SRCPATH)/include/jansson_config.h: make
	mkdir -p $(SRCPATH)/include/
	cp -af jansson/src/jansson_config.h $(SRCPATH)/include/



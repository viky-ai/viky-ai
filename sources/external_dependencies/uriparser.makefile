#
# Extrenal lib uriparser
#

CURRENT_MAKEFILE := $(word $(words $(MAKEFILE_LIST)),$(MAKEFILE_LIST))

include $(OG_REPO_PATH)/sources/makefile.defs.linux

.PHONY: all redebug debug build make clean fullclean uriparser

redebug: rebuild

debug: build

rebuild:
	$(MAKE) -f $(CURRENT_MAKEFILE) clean
	$(MAKE) -f $(CURRENT_MAKEFILE) build

build: $(DBINPATH)/liburiparser.so $(SRCPATH)/include/uriparser/Uri.h

uriparser: all

all:
	$(MAKE) -f $(CURRENT_MAKEFILE) fullclean
	$(MAKE) -f $(CURRENT_MAKEFILE) build

fullclean: clean
	-cd uriparser && $(MAKE) clean
	rm -rf uriparser/Makefile
	rm -rf uriparser/configure

clean:
	rm -f $(SRCPATH)/include/uriparser/*.h
	rm -f $(DBINPATH)/liburiparser.so*

make: uriparser/Makefile
	cd uriparser && $(MAKE)

uriparser/configure:
	cd uriparser && ./autogen.sh

uriparser/Makefile: uriparser/configure
	cd uriparser && ./configure --prefix="$(DBUILDPATH)" \
															--bindir="$(DBINPATH)" \
															--libdir="$(DLIBPATH)" \
															--includedir="$(SRCPATH)/include" \
															--disable-test \
															--disable-doc 

$(DBINPATH)/liburiparser.so: make
	cp -af uriparser/.libs/liburiparser.so* $(DBINPATH)

$(SRCPATH)/include/uriparser/Uri.h: make
	mkdir -p $(SRCPATH)/include/uriparser
	cp -af uriparser/include/uriparser/*.h $(SRCPATH)/include/uriparser



#
# Extrenal lib duktape
#

CURRENT_MAKEFILE := $(word $(words $(MAKEFILE_LIST)),$(MAKEFILE_LIST))

include $(OG_REPO_PATH)/sources/makefile.defs.linux

.PHONY: all redebug debug build make clean fullclean

redebug: rebuild

debug: build

rebuild:
	$(MAKE) -f $(CURRENT_MAKEFILE) clean
	$(MAKE) -f $(CURRENT_MAKEFILE) build

build: $(DBINPATH)/libduktape.so $(SRCPATH)/include/duktape.h

duktape: all

all:
	$(MAKE) -f $(CURRENT_MAKEFILE) fullclean
	$(MAKE) -f $(CURRENT_MAKEFILE) build

fullclean: clean
	-cd duktape && $(MAKE) cleanall

clean:
	rm -f  $(SRCPATH)/include/duktape.h
	rm -f  $(DBINPATH)/libduktape.so*

make: duktape/dist/Makefile.sharedlibrary
	cd duktape/dist/ && $(MAKE) -f Makefile.sharedlibrary

duktape/dist/Makefile.sharedlibrary:
	cd duktape && $(MAKE) dist

$(DBINPATH)/libduktape.so: make
	cd duktape/dist/ && ln -nfs libduktape.so.* libduktape.so
	cp -af duktape/dist/libduktape.so* $(DBINPATH)/

$(SRCPATH)/include/duktape.h: duktape/dist/Makefile.sharedlibrary
	mkdir -p $(SRCPATH)/include/
	cp -af duktape/dist/src/duk_config.h $(SRCPATH)/include/
	cp -af duktape/dist/src/duktape.h    $(SRCPATH)/include/

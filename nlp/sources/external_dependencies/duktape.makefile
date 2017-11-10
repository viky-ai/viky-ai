#
# Extrenal lib duktape
# sudo apt-get install python-yaml
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

duktape/dist/libduktape.so: duktape/dist/duktape.c duktape/dist/duktape.h duktape/dist/duk_config.h
	cd duktape/dist/ && gcc -shared -fPIC -Wall -Wextra -O2 -Wl,-soname,libduktape.so \
		                      -o libduktape.so duktape.c

duktape/dist/duktape.c duktape/dist/duktape.h duktape/dist/duk_config.h:
	cd duktape && python tools/configure.py \
		--output-directory=dist \
		--platform linux \
		--compiler gcc \
		--architecture x64 \
		--option-yaml 'DUK_USE_FASTINT: true'

$(DBINPATH)/libduktape.so: duktape/dist/libduktape.so
	cp -af duktape/dist/libduktape.so $(DBINPATH)/

$(SRCPATH)/include/duktape.h: duktape/dist/duktape.h duktape/dist/duk_config.h
	mkdir -p $(SRCPATH)/include/
	cp -af duktape/dist/duk_config.h $(SRCPATH)/include/
	cp -af duktape/dist/duktape.h    $(SRCPATH)/include/

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

SRCS_C=duktape/dist/duktape.c duktape/dist/duk_module_node.c duktape/dist/duk_console.c

duktape/dist/libduktape.so: $(SRCS_C)
	cd duktape/dist/ && gcc -shared -g -Og -fPIC -Wall -Wextra -Wno-maybe-uninitialized -Wl,-soname,libduktape.so \
		                      -o libduktape.so duktape.c duk_module_node.c duk_console.c

duktape/dist/duktape.c duktape/dist/duktape.h duktape/dist/duk_config.h:
	cd duktape && python tools/configure.py \
		--output-directory=dist \
		--platform linux \
		--compiler gcc \
		--architecture x64 \
		--option-yaml 'DUK_USE_FASTINT: true'

duktape/dist/duk_module_node.c duktape/dist/duk_module_node.h: duktape/extras/module-node/duk_module_node.c duktape/extras/module-node/duk_module_node.h
	cp -af duktape/extras/module-node/duk_module_node.c duktape/dist/
	cp -af duktape/extras/module-node/duk_module_node.h duktape/dist/

duktape/dist/duk_console.c duktape/dist/duk_console.h: duktape/extras/console/duk_console.c duktape/extras/console/duk_console.h
	cp -af duktape/extras/console/duk_console.c duktape/dist/
	cp -af duktape/extras/console/duk_console.h duktape/dist/

$(DBINPATH)/libduktape.so: duktape/dist/libduktape.so
	cp -af duktape/dist/libduktape.so $(DBINPATH)/

$(SRCPATH)/include/duktape.h: duktape/dist/duktape.h duktape/dist/duk_config.h duktape/dist/duk_console.h duktape/dist/duk_module_node.h
	mkdir -p $(SRCPATH)/include/
	cp -af duktape/dist/duk_config.h $(SRCPATH)/include/
	cp -af duktape/dist/duk_module_node.h $(SRCPATH)/include/
	cp -af duktape/dist/duk_console.h     $(SRCPATH)/include/
	cp -af duktape/dist/duktape.h         $(SRCPATH)/include/

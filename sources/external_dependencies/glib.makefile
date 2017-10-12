#
# Extrenal lib glib
# dependencies : gtk-doc-tools
#

CURRENT_MAKEFILE := $(word $(words $(MAKEFILE_LIST)),$(MAKEFILE_LIST))

include $(OG_REPO_PATH)/sources/makefile.defs.linux

.PHONY: all redebug debug build make clean fullclean

redebug: rebuild

debug: build

rebuild:
	$(MAKE) -f $(CURRENT_MAKEFILE) clean
	$(MAKE) -f $(CURRENT_MAKEFILE) build

build: $(DBINPATH)/libglib-2.0.so $(SRCPATH)/include/glib-2.0/glib.h $(SRCPATH)/include/glib-2.0/glibconfig.h

glib: all

all:
	$(MAKE) -f $(CURRENT_MAKEFILE) fullclean
	$(MAKE) -f $(CURRENT_MAKEFILE) build

fullclean: clean
	-cd glib && $(MAKE) clean
	-cd glib && git clean -dfx
	rm -rf glib/Makefile
	rm -rf glib/configure

clean:
	rm -rf $(SRCPATH)/include/gio-unix-2.0/
	rm -rf $(SRCPATH)/include/glib-2.0/
	rm -f  $(DBINPATH)/libglib.so*
	rm -f  $(DBINPATH)/libgio.so*
	rm -f  $(DBINPATH)/libgmodule.so*
	rm -f  $(DBINPATH)/libgobject.so*
	rm -f  $(DBINPATH)/libgthread.so*

make: glib/Makefile
	cd glib && $(MAKE)
	mkdir -p $(DBINPATH)/gio
	mkdir -p $(SRCPATH)/include/
	cd glib && $(MAKE) install
	rm -f glib/gio/gdbus-2.0/codegen/config.py

glib/configure:
	cd glib && NOCONFIGURE=yes ./autogen.sh

glib/Makefile: glib/configure
	cd glib && ./configure  --prefix="$(DBUILDPATH)" \
													--bindir="$(DBINPATH)" \
													--libdir="$(DLIBPATH)" \
													--includedir="$(SRCPATH)/include" \
													--enable-debug \
													--with-pcre=internal \
													--disable-fam	 \
													--disable-selinux \
													--disable-libmount \
													--disable-gtk-doc \
													--disable-man

$(DBINPATH)/libglib-2.0.so: make

$(SRCPATH)/include/glib-2.0/glib.h: make

$(SRCPATH)/include/glib-2.0/glibconfig.h: make
	mkdir -p $(SRCPATH)/include/glib-2.0
	cp -af $(DBINPATH)/glib-2.0/include/glibconfig.h $(SRCPATH)/include/glib-2.0/

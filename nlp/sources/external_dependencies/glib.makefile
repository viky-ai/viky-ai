#
# Extrenal lib glib
# dependencies : gtk-doc-tools
#

CURRENT_MAKEFILE := $(word $(words $(MAKEFILE_LIST)),$(MAKEFILE_LIST))

include $(OG_REPO_PATH)/sources/makefile.defs.linux

.PHONY: all redebug debug build make clean fullclean

debug: $(DBINPATH)/libglib-2.0.so $(SRCPATH)/include/glib-2.0/glib.h $(SRCPATH)/include/glib-2.0/glibconfig.h

build: $(RBINPATH)/libglib-2.0.so $(SRCPATH)/include/glib-2.0/glib.h $(SRCPATH)/include/glib-2.0/glibconfig.h

rebuild:
	$(MAKE) -f $(CURRENT_MAKEFILE) clean
	$(MAKE) -f $(CURRENT_MAKEFILE) build

redebug:
	$(MAKE) -f $(CURRENT_MAKEFILE) clean
	$(MAKE) -f $(CURRENT_MAKEFILE) debug

glib: all

all:
	$(MAKE) -f $(CURRENT_MAKEFILE) fullclean
	$(MAKE) -f $(CURRENT_MAKEFILE) debug build

fullclean: clean
	-cd glib && $(MAKE) clean
	-cd glib && git clean -dfx
	rm -rf glib/Makefile
	rm -rf glib/configure

clean:
	rm -rf $(SRCPATH)/include/gio-unix-2.0/
	rm -rf $(SRCPATH)/include/glib-2.0/
	rm -f  $(DBINPATH)/libglib-2.0.so*
	rm -f  $(DBINPATH)/libgio-2.0.so*
	rm -f  $(DBINPATH)/libgmodule-2.0.so*
	rm -f  $(DBINPATH)/libgobject-2.0.so*
	rm -f  $(DBINPATH)/libgthread-2.0.so*
	rm -f  $(RBINPATH)/libglib-2.0.so*
	rm -f  $(RBINPATH)/libgio-2.0.so*
	rm -f  $(RBINPATH)/libgmodule-2.0.so*
	rm -f  $(RBINPATH)/libgobject-2.0.so*
	rm -f  $(RBINPATH)/libgthread-2.0.so*
	rm -f  glib/gio/gvdb/.dirstamp
	rm -f  glib/glib/deprecated/.dirstamp

make: glib/Makefile
	cd glib && $(MAKE)
	mkdir -p $(DBINPATH)/gio
	mkdir -p $(SRCPATH)/include/
	cd glib && $(MAKE) install
	rm -f  glib/gio/gvdb/.dirstamp
	rm -f  glib/glib/deprecated/.dirstamp

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

$(RBINPATH)/libglib-2.0.so: $(DBINPATH)/libglib-2.0.so
	cp -af $(DBINPATH)/libglib-2.0.so*     $(RBINPATH)/
	cp -af $(DBINPATH)/libgio-2.0.so*      $(RBINPATH)/
	cp -af $(DBINPATH)/libgmodule-2.0.so*  $(RBINPATH)/
	cp -af $(DBINPATH)/libgobject-2.0.so*  $(RBINPATH)/
	cp -af $(DBINPATH)/libgthread-2.0.so*  $(RBINPATH)/

$(SRCPATH)/include/glib-2.0/glib.h: make

$(SRCPATH)/include/glib-2.0/glibconfig.h: make
	mkdir -p $(SRCPATH)/include/glib-2.0
	cp -af $(DBINPATH)/glib-2.0/include/glibconfig.h $(SRCPATH)/include/glib-2.0/

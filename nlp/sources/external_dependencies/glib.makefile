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
	rm -rf glib/meson-build

clean:
	rm -rf $(SRCPATH)/include/gio-unix-2.0/
	rm -rf $(SRCPATH)/include/glib-2.0/
	rm -f  $(DBINPATH)/libglib-2.0.so*
	rm -f  $(DBINPATH)/libgio-2.0.so*
	rm -f  $(DBINPATH)/libgmodule-2.0.so*
	rm -f  $(DBINPATH)/libgobject-2.0.so*
	rm -f  $(DBINPATH)/libgthread-2.0.so*
	rm -rf $(DBINPATH)/gio
	rm -f  $(RBINPATH)/libglib-2.0.so*
	rm -f  $(RBINPATH)/libgio-2.0.so*
	rm -f  $(RBINPATH)/libgmodule-2.0.so*
	rm -f  $(RBINPATH)/libgobject-2.0.so*
	rm -f  $(RBINPATH)/libgthread-2.0.so*
	rm -rf $(RBINPATH)/gio

make: glib/meson-build/build.ninja
	cd glib && ninja -C meson-build

install: make
	mkdir -p $(SRCPATH)/include/
	cd glib && ninja -C meson-build install
	mkdir -p $(SRCPATH)/include/glib-2.0
	mkdir -p $(SRCPATH)/include/gio-unix-2.0/
	cp -arf $(DBUILDPATH)/include/glib-2.0/*     $(SRCPATH)/include/glib-2.0/
	cp -arf $(DBUILDPATH)/include/gio-unix-2.0/* $(SRCPATH)/include/gio-unix-2.0/

glib/meson-build/build.ninja: glib/meson.build
	cd glib && meson meson-build --prefix="$(DBUILDPATH)" \
													     --bindir="$(DBINPATH)" \
													     --libdir="$(DLIBPATH)" \
													     --buildtype debug \
													     --optimization g\
													     -Dfam=false \
													     -Dselinux=disabled \
													     -Dlibmount=false \
													     -Dgtk_doc=false \
													     -Dman=false 

$(DBINPATH)/libglib-2.0.so: install

$(RBINPATH)/libglib-2.0.so: $(DBINPATH)/libglib-2.0.so
	cp -af $(DBINPATH)/libglib-2.0.so*     $(RBINPATH)/
	cp -af $(DBINPATH)/libgio-2.0.so*      $(RBINPATH)/
	cp -af $(DBINPATH)/libgmodule-2.0.so*  $(RBINPATH)/
	cp -af $(DBINPATH)/libgobject-2.0.so*  $(RBINPATH)/
	cp -af $(DBINPATH)/libgthread-2.0.so*  $(RBINPATH)/

$(SRCPATH)/include/glib-2.0/glib.h: install

$(SRCPATH)/include/glib-2.0/glibconfig.h: install
	mkdir -p $(SRCPATH)/include/glib-2.0
	cp -af $(DBINPATH)/glib-2.0/include/glibconfig.h $(SRCPATH)/include/glib-2.0/

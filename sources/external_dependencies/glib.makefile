#
# Glib Makefile
#

CURRENT_MAKEFILE := $(word $(words $(MAKEFILE_LIST)),$(MAKEFILE_LIST))

include $(OG_REPO_PATH)/sources/makefile.defs.linux

LIBVERSION=2.48.2
LIBDIR=glib-$(LIBVERSION)

build:
	@if [ ! -e $(RBINPATH)/libglib-2.0.so ] || [ ! -d glib-$(LIBVERSION) ] ; then $(MAKE) -f $(CURRENT_MAKEFILE) __build; fi

debug:
	@if [ ! -e $(DBINPATH)/libglib-2.0.so ] || [ ! -d glib-$(LIBVERSION) ]; then $(MAKE) -f $(CURRENT_MAKEFILE) __debug; fi

profile:
	@if [ ! -e $(PBINPATH)/libglib-2.0.so ] || [ ! -d glib-$(LIBVERSION) ]; then $(MAKE) -f $(CURRENT_MAKEFILE) __profile; fi

__build:
	$(MAKE) -f $(CURRENT_MAKEFILE) fullclean
	$(MAKE) -f $(CURRENT_MAKEFILE) unpack
	$(MAKE) -f $(CURRENT_MAKEFILE) _build

__debug:
	$(MAKE) -f $(CURRENT_MAKEFILE) fullclean
	$(MAKE) -f $(CURRENT_MAKEFILE) unpack
	$(MAKE) -f $(CURRENT_MAKEFILE) _debug

__profile:
	$(MAKE) -f $(CURRENT_MAKEFILE) fullclean
	$(MAKE) -f $(CURRENT_MAKEFILE) unpack
	$(MAKE) -f $(CURRENT_MAKEFILE) _profile

_build: $(RBINPATH) $(RLINKPATH)
	@echo "run configure script"
	@cd $(LIBDIR) && ./autogen.sh --prefix="$(RBUILDPATH)" --bindir="$(RBINPATH)" --libdir="$(RLIBPATH)" --includedir="$(SRCPATH)/include" --datarootdir=$(SRCPATH)/glib/share --enable-static=yes --disable-fam --with-pcre=internal --disable-libmount
	@cd $(LIBDIR) && $(MAKE)
	@cd $(LIBDIR) && $(MAKE) install
	rm -rf $(RBINPATH)/*.la $(RBINPATH)/glib2-0
	mv $(RBINPATH)/libgio-2.0.a     $(RLINKPATH)
	mv $(RBINPATH)/libglib-2.0.a    $(RLINKPATH)
	mv $(RBINPATH)/libgmodule-2.0.a $(RLINKPATH)
	mv $(RBINPATH)/libgobject-2.0.a $(RLINKPATH)
	mv $(RBINPATH)/libgthread-2.0.a $(RLINKPATH)
	cp $(LIBDIR)/glib/glibconfig.h $(SRCPATH)/include/glib-2.0

_debug: $(DBINPATH) $(DLINKPATH)
	@echo "run configure script"
	@cd $(LIBDIR) && ./autogen.sh --prefix="$(DBUILDPATH)" --bindir="$(DBINPATH)" --libdir="$(DLIBPATH)" --includedir="$(SRCPATH)/include" --datarootdir=$(SRCPATH)/glib/share --enable-static=yes --disable-fam --with-pcre=internal --disable-libmount # --enable-gtk-doc
	@cd $(LIBDIR) && $(MAKE)
	@cd $(LIBDIR) && $(MAKE) install
	rm -rf $(DBINPATH)/*.la $(DBINPATH)/glib2-0
	mv $(DBINPATH)/libgio-2.0.a     $(DLINKPATH)
	mv $(DBINPATH)/libglib-2.0.a    $(DLINKPATH)
	mv $(DBINPATH)/libgmodule-2.0.a $(DLINKPATH)
	mv $(DBINPATH)/libgobject-2.0.a $(DLINKPATH)
	mv $(DBINPATH)/libgthread-2.0.a $(DLINKPATH)
	cp $(LIBDIR)/glib/glibconfig.h $(SRCPATH)/include/glib-2.0

_profile: $(PBINPATH) $(PLINKPATH)
	@echo "run configure script"
	@cd $(LIBDIR) && ./autogen.sh --prefix="$(PBUILDPATH)" --bindir="$(PBINPATH)" --libdir="$(PLIBPATH)" --includedir="$(SRCPATH)/include" --datarootdir=$(SRCPATH)/glib/share --enable-static=yes --disable-fam --with-pcre=internal --disable-libmount
	@cd $(LIBDIR) && $(MAKE)
	@cd $(LIBDIR) && $(MAKE) install
	rm -rf $(PBINPATH)/*.la $(PBINPATH)/glib2-0
	mv $(PBINPATH)/libgio-2.0.a     $(PLINKPATH)
	mv $(PBINPATH)/libglib-2.0.a    $(PLINKPATH)
	mv $(PBINPATH)/libgmodule-2.0.a $(PLINKPATH)
	mv $(PBINPATH)/libgobject-2.0.a $(PLINKPATH)
	mv $(PBINPATH)/libgthread-2.0.a $(PLINKPATH)
	cp $(LIBDIR)/glib/glibconfig.h $(SRCPATH)/include/glib-2.0

all: build debug profile

redebug: debug

rebuild: build

runtest_build:

runtest_debug:

runtest_profile:

GENTEST:

test:

clean:

# Trouble on ubuntu 16.04 : https://github.com/Alexpux/MINGW-packages/issues/1351
unpack:
	tar xvf $(LIBDIR).tar.xz --use-compress-program=xz

release: build

fullclean:
	\rm -rf $(LIBDIR) html share
	\rm -rf $(SRCPATH)/include/glib-2.0 $(SRCPATH)/include/doc $(sRCPATH)/include/html
	\rm -rf $(RBINPATH)/libgio-2.0.* $(RBINPATH)/libglib-2.0.* $(RBINPATH)/libgmodule-2.0.* $(RBINPATH)/libgthread-2.0.* $(RBINPATH)/libgobject-2.0.*
	\rm -rf $(DBINPATH)/libgio-2.0.* $(DBINPATH)/libglib-2.0.* $(DBINPATH)/libgmodule-2.0.* $(DBINPATH)/libgthread-2.0.* $(DBINPATH)/libgobject-2.0.*
	\rm -rf $(PBINPATH)/libgio-2.0.* $(PBINPATH)/libglib-2.0.* $(PBINPATH)/libgmodule-2.0.* $(PBINPATH)/libgthread-2.0.* $(PBINPATH)/libgobject-2.0.*
	\rm -rf $(RBINPATH)/gtester $(RBINPATH)/gobject-query gio $(RBINPATH)/glib-2.0
	\rm -rf $(DBINPATH)/gtester $(DBINPATH)/gobject-query gio $(DBINPATH)/glib-2.0
	\rm -rf $(PBINPATH)/gtester $(PBINPATH)/gobject-query gio $(PBINPATH)/glib-2.0
	\rm -f  $(DLINKPATH)/libgio-2.0.a $(DLINKPATH)/libglib-2.0.a $(DLINKPATH)/libgmodule-2.0.a $(DLINKPATH)/libgobject-2.0.a $(DLINKPATH)/libgthread-2.0.a
	\rm -f  $(RLINKPATH)/libgio-2.0.a $(RLINKPATH)/libglib-2.0.a $(RLINKPATH)/libgmodule-2.0.a $(RLINKPATH)/libgobject-2.0.a $(RLINKPATH)/libgthread-2.0.a
	\rm -f  $(PLINKPATH)/libgio-2.0.a $(PLINKPATH)/libglib-2.0.a $(PLINKPATH)/libgmodule-2.0.a $(PLINKPATH)/libgobject-2.0.a $(PLINKPATH)/libgthread-2.0.a

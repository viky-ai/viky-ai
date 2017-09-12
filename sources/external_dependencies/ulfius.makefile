#
# Extrenal lib ulfius
#

CURRENT_MAKEFILE := $(word $(words $(MAKEFILE_LIST)),$(MAKEFILE_LIST))

include $(OG_REPO_PATH)/sources/makefile.defs.linux

ADDITIONALFLAGS:="-I$(SRCPATH)/include/"
LDFLAGS:="-L$(DBINPATH) -L$(DLINKPATH)"

.PHONY: all redebug debug build make clean fullclean make_yder make_orcania ulfius

redebug: debug

debug: build

rebuild: build

build: $(DBINPATH)/libulfius.so $(SRCPATH)/include/ulfius.h

ulfius: all

all:
	$(MAKE) -f $(CURRENT_MAKEFILE) fullclean
	$(MAKE) -f $(CURRENT_MAKEFILE) build

fullclean: clean
	cd ulfius && $(MAKE) clean

clean:
	rm -f $(SRCPATH)/include/yder.h
	rm -f $(SRCPATH)/include/orcania.h
	rm -f $(SRCPATH)/include/ulfius.h
	rm -f $(DBINPATH)/libulfius.so*
	rm -f $(DBINPATH)/libyder.so*
	rm -f $(DBINPATH)/liborcania.so*

make: $(DBINPATH)/libyder.so $(SRCPATH)/include/yder.h
	cd ulfius && $(MAKE) PREFIX=$(DBUILDPATH) ADDITIONALFLAGS=$(ADDITIONALFLAGS) LDFLAGS=$(LDFLAGS) CURLFLAG=-DU_DISABLE_CURL WEBSOCKETFLAG=-DU_DISABLE_WEBSOCKET

$(DBINPATH)/libulfius.so: make
	cp -af ulfius/src/libulfius.so* $(DBINPATH)

$(SRCPATH)/include/ulfius.h: make
	cp -af ulfius/src/ulfius.h $(SRCPATH)/include/

# Lib liborcania
make_orcania:
	cd ulfius/lib/orcania && $(MAKE) PREFIX=$(DBUILDPATH) ADDITIONALFLAGS=$(ADDITIONALFLAGS) LDFLAGS=$(LDFLAGS) CURLFLAG=-DU_DISABLE_CURL WEBSOCKETFLAG=-DU_DISABLE_WEBSOCKET

$(DBINPATH)/liborcania.so: make_orcania
	cp -af ulfius/lib/orcania/src/liborcania.so* $(DBINPATH)

$(SRCPATH)/include/orcania.h: make_orcania
	cp -af ulfius/lib/orcania/src/orcania.h $(SRCPATH)/include/

# Lib yder
make_yder: $(DBINPATH)/liborcania.so $(SRCPATH)/include/orcania.h
	cd ulfius/lib/yder && $(MAKE) PREFIX=$(DBUILDPATH) ADDITIONALFLAGS=$(ADDITIONALFLAGS) LDFLAGS=$(LDFLAGS) CURLFLAG=-DU_DISABLE_CURL WEBSOCKETFLAG=-DU_DISABLE_WEBSOCKET

$(DBINPATH)/libyder.so: make_yder
	cp -af ulfius/lib/yder/src/libyder.so* $(DBINPATH)

$(SRCPATH)/include/yder.h: make_yder
	cp -af ulfius/lib/yder/src/yder.h $(SRCPATH)/include/

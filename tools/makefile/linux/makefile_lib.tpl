#
# Lib-specific makefile
#

CURRENT_MAKEFILE := $(word $(words $(MAKEFILE_LIST)),$(MAKEFILE_LIST))
SHELL := /bin/bash
OG_REPO_PATH=$$$$OG_REPO_PATH$$$$

include $(OG_REPO_PATH)/sources/makefile.defs.linux

$$$$OPTIONS$$$$

COMPONENT_NAME=$$$$COMPONENT_NAME$$$$

#---------------------------------------------------------------#
#  Source listing for the program                               #
#---------------------------------------------------------------#
SRC = $$$$SOURCES$$$$

#---------------------------------------------------------------#
#  Source Test Plan                                             #
#---------------------------------------------------------------#
SRC_TEST= $$$$SRCTESTS$$$$

#---------------------------------------------------------------#
#  Definitions and targets                                      #
#---------------------------------------------------------------#
EOBJO = $(SRC:%.c=$(ROBJ)/%.o)  $(SRC:%.cpp=$(ROBJ)/%.o)
EOBJD = $(SRC:%.c=$(DOBJ)/%.o)  $(SRC:%.cpp=$(DOBJ)/%.o)
EOBJP = $(SRC:%.c=$(POBJ)/%.o)  $(SRC:%.cpp=$(POBJ)/%.o)

debug_copy: debug
	cp -af $(DLIBPATH)/lib$(NAME).so $(OG_REPO_PATH)/ship/debug/lib/linux/


build: $(RLINKPATH)/lib$(NAME).a $(RLIBPATH)/lib$(NAME).so $(RLIBPATH)/is_debug_enable.conf

debug: $(DLINKPATH)/lib$(NAME).a $(DLIBPATH)/lib$(NAME).so $(DLIBPATH)/is_debug_enable.conf

profile: $(PLINKPATH)/lib$(NAME).a $(PLIBPATH)/lib$(NAME).so $(PLIBPATH)/is_debug_enable.conf

all: build debug

rebuild:
	$(MAKE) -f $(CURRENT_MAKEFILE) cleanrelease
	$(MAKE) -f $(CURRENT_MAKEFILE) build

redebug:
	$(MAKE) -f $(CURRENT_MAKEFILE) cleandebug
	$(MAKE) -f $(CURRENT_MAKEFILE) debug

release:
	$(MAKE) -f $(CURRENT_MAKEFILE) cleanrelease
	$(MAKE) -f $(CURRENT_MAKEFILE) build
	$(MAKE) -f $(CURRENT_MAKEFILE) test

runtest_build:
	@for i in $(SRC_TEST) ; do \
	  echo -e "run test $$i" ; \
	  cd $(RBINPATH) ; \
	  ./$$i.pl ; \
	done

runtest_debug:
	@for i in $(SRC_TEST) ; do \
	  echo -e "run test $$i" ; \
	  cd $(DBINPATH) ; \
	  ./$$i.pl ; \
	done

runtest_profile:
	@for i in $(SRC_TEST) ; do \
	  echo -e "run test $$i" ; \
	  cd $(PBINPATH) ; \
	  ./$$i.pl ; \
	done

fullclean: clean

clean : cleanrelease cleandebug cleanprofile
	@for i in $(SRC_TEST) ; do \
	  rm -f $$i.pl ; \
	done
	rm -rf $(LOBJ)

cleanrelease :
	rm -rf $(ROBJ)
	@for i in $(SRC_TEST) ; do \
	  rm -f $(RBINPATH)/$$i.pl ; \
	done
	rm -f $(RLIBPATH)/lib$(NAME).so*
	rm -f $(RLINKPATH)/lib$(NAME).a

cleandebug :
	rm -rf $(DOBJ)
	@for i in $(SRC_TEST) ; do \
	  rm -f $(DBINPATH)/$$i.pl ;\
	done
	rm -f $(DLIBPATH)/lib$(NAME).so*
	rm -f $(DLINKPATH)/lib$(NAME).a

cleanprofile :
	rm -rf $(POBJ)
	@for i in $(SRC_TEST) ; do \
	  rm -f $(PBINPATH)/$$i.pl ; \
	done
	rm -f $(PLIBPATH)/lib$(NAME).so*
	rm -f $(PLINKPATH)/lib$(NAME).a

#---------------------------------------------------------------#
#  Shared libraries, for the moment, we keep separated targets  #
#  because building a shared library may not be unified         #
#---------------------------------------------------------------#

$(RLINKPATH)/lib$(NAME).a: $(RLINKPATH) $(ROBJ) $(EOBJO)
	cd $(ROBJ) && $(AR) $@ *.o
	$(RANLIB) $@
	@echo "Done: $@"

$(RLIBPATH)/lib$(NAME).so : $(RLIBPATH) $(RLINKPATH)/lib$(NAME).a
	cd $(ROBJ) && $(CC) -rdynamic -shared -Wl,--no-undefined,-soname,lib$(NAME).so $(LFLAGSR_INTERNAL) $(RPATH_LIB) -o $@ *.o -L$(RLIBPATH) $(ADD_LIBR) $(LIBS)
	@echo "Done: $@"

$(RLIBPATH)/is_debug_enable.conf:
	@echo -e "# Flag to indicate if debug is enable or not\n0" | tee $@ > /dev/null

$(DLINKPATH)/lib$(NAME).a: $(DLINKBPATH) $(DOBJ) $(EOBJD)
	cd $(DOBJ) && $(AR) $@ *.o
	$(RANLIB) $@
	@echo "Done: $@"

$(DLIBPATH)/lib$(NAME).so: $(DLIBPATH) $(DLINKPATH)/lib$(NAME).a
	cd $(DOBJ) && $(CC) -rdynamic -shared -Wl,--no-undefined,-soname,lib$(NAME).so $(LFLAGSD_INTERNAL) $(RPATH_LIB) -o $@ *.o -L$(DLIBPATH) $(ADD_LIBD) $(LIBS)
	@echo "Done: $@"

$(DLIBPATH)/is_debug_enable.conf:
	@echo -e "# Flag to indicate if debug is enable or not\n1" | tee $@ > /dev/null

$(PLINKPATH)/lib$(NAME).a: $(PLINKPATH) $(POBJ) $(EOBJP)
	cd $(POBJ) && $(AR) $@ *.o
	$(RANLIB) $@
	@echo "Done: $@"

$(PLIBPATH)/lib$(NAME).so: $(PLIBPATH) $(PLINKPATH)/lib$(NAME).a
	cd $(POBJ) && $(CC) -rdynamic -shared -Wl,--no-undefined,-soname,lib$(NAME).so $(LFLAGSP_INTERNAL) $(RPATH_LIB) -o $@ *.o -L$(PLIBPATH) $(ADD_LIBP) $(LIBS)
	@echo "Done: $@"

$(PLIBPATH)/is_debug_enable.conf:
	@echo -e "# Flag to indicate if debug is enable or not\n0" | tee $@ > /dev/null

#---------------------------------------------------------------#
#  Handling directories creation when necessary                 #
#  Handling symbolic links on shared libraries when necessary   #
#---------------------------------------------------------------#
$(ROBJ):
	@mkdir -p $(ROBJ)

$(DOBJ):
	@mkdir -p $(DOBJ)

$(POBJ):
	@mkdir -p $(POBJ)

#---------------------------------------------------------------#
#  Compilation Rules                                            #
#---------------------------------------------------------------#
$(ROBJ)/%.o : %.c ; @echo $< ; $(CC) -rdynamic -o $@ -c $(CFLAGSR) $(CFLAGSR_INTERNAL) $(ADD_INC) -DDPcInDll $(ADD_CFLAGS) $<
$(DOBJ)/%.o : %.c ; @echo $< ; $(CC) -rdynamic -o $@ -c $(CFLAGSD) $(CFLAGSD_INTERNAL) $(ADD_INC) -DDPcInDll $(ADD_CFLAGS) $<
$(POBJ)/%.o : %.c ; @echo $< ; $(CC) -rdynamic -o $@ -c $(CFLAGSP) $(CFLAGSP_INTERNAL) $(ADD_INC) -DDPcInDll $(ADD_CFLAGS) $<

$(ROBJ)/%.o : %.cpp ; @echo $< ; $(CPP) -rdynamic -o $@ -c $(CFLAGSR) $(CFLAGSR_INTERNAL) $(ADD_INC) -DDPcInDll $(ADD_CFLAGS) $<
$(DOBJ)/%.o : %.cpp ; @echo $< ; $(CPP) -rdynamic -o $@ -c $(CFLAGSD) $(CFLAGSD_INTERNAL) $(ADD_INC) -DDPcInDll $(ADD_CFLAGS) $<
$(POBJ)/%.o : %.cpp ; @echo $< ; $(CPP) -rdynamic -o $@ -c $(CFLAGSP) $(CFLAGSP_INTERNAL) $(ADD_INC) -DDPcInDll $(ADD_CFLAGS) $<

# We do not want .c files to be regenerated by yacc, bison, flex or lex
# but the following commented commands will generally do the trick if necessary
#%.c : %.y
#	header=`basename $@ .c`; bison -o $@ --defines=$$header.h $<
#%.c : %.l
#	flex -o $@ $<
%.c : %.y
%.c : %.l

GENTEST: $(RBINPATH) $(DBINPATH) $(PBINPATH)
	@for i in $(SRC_TEST) ; do \
	  echo -e "generate test $$i" ; \
	  gentest.pl --output $$i.pl $$i ; \
	  chmod a+rx $$i.pl ; \
	  cp $$i.pl $(RBINPATH) ; \
	  cp $$i.pl $(DBINPATH) ; \
	  cp $$i.pl $(PBINPATH) ; \
	done

#---------------------------------------------------------------#
# K&R to ANSI C conversion
#
# Requires gcc 4.4 with protoize support.
#
# Some manual intervention may be required before/after running
# the protoize target.
#
# Here are the restrictions identified so far:
#
# - Combinations of pr_/pr macros and function pointers must
#   be manually cleaned up before running protoize:
#     pr_((int *func)(pr_(int) pr(int)))
#   should at least be manually converted to:
#     (int *func)(pr_(int) pr(int))
#
# - Some DEFPUBLIC declarations are not correctly restored after
#   running protoize, so the DEFPUBLIC macro must be manually
#   restored.
#
# - This is Linux only, so Windows-only headers stay untouched.
#---------------------------------------------------------------#

PROTOFILES = $(wildcard $(SRC) ogm_*.h $(SRCPATH)/include/l$(NAME).h)

protoize: protoize-clean
# store PUBLIC, DEFPUBLIC definitions
	@grep -o "\(DEF\)*PUBLIC([^)]*)\s*\w*(" $(PROTOFILES) > public.protoize
# remove / convert header macros (pr_, pr, STATICF, PUBLIC, DEFPUBLIC)
# WARNING: nested pr macros are not correctly handled and require manual fixing
	@for c in $(PROTOFILES); do echo "sed: $$c"; sed -i \
	    -e 's/\([ (]\)pr_(\([^)]*\))/\1\2,/g' \
	    -e 's/\([ (]\)pr(\([^)]*\))/\1\2/g' \
	    -e 's/STATICF(\([^)]*\))/static \1/g' \
	    -e 's/\(DEF\)*PUBLIC(\([^)]*\))\(\s*\w*\)/\2\3/' \
	    $$c; \
	  done
# protoize (always return true since it usually crash upon cleanup)
	protoize -p gcc-4.4 -c "$(CFLAGSR) $(ADD_INC) $(ADD_CFLAGS)" -g -N $(SRC); true
# restore PUBLIC, DEFPUBLIC macros
	@while read line; do \
	    file=`echo $$line | cut -d ':' -f 1`; \
	    header=`echo $$line | cut -d ':' -f 2`; \
	    sig=`echo $$header | sed -e 's/\(DEF\)*PUBLIC(\([^)]*\))\(\s*\w*(\)/\2\3/'`; \
	    sig=$${sig//\*/\\*}; \
	    sed -i -e "s/$$sig/$$header/g" $$file; \
	  done < "public.protoize"
	@rm -f public.protoize

protoize-clean:
	@rm -f *.o
	@rm -f *.X
	@rm -f public.protoize

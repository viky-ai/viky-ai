#---------------------------------------------------------------#
#  Client module makefile                                       #
#                                                               #
#  makefile must be in [instance_path]/sources/[module_name]    #
#---------------------------------------------------------------#

NAME = [%MODULE%]


SRC = [%SOURCES_LIST%]


ADD_CFLAGS = [%OPT_ADD_CFLAGS%]

ADD_INC = [%OPT_ADD_INC%]

ADD_LIBR = [%OPT_ADD_LIBR%]

ADD_LIBD = [%OPT_ADD_LIBD%]

LINKER_NO_UNDEF = [%OPT_LINKER_NO_UNDEF%]


#---------------------------------------------------------------#
#  Internals                                                    #
#---------------------------------------------------------------#

# Default compilation language
LANG=C.UTF-8
LANGUAGE=en

BUILDPATH = $(PWD)

LIBPATH = $(BUILDPATH)/lib
RLIBPATH = $(LIBPATH)/release
DLIBPATH = $(LIBPATH)/debug

OBJPATH = $(BUILDPATH)/obj
ROBJPATH = $(OBJPATH)/release
DOBJPATH = $(OBJPATH)/debug

BUILD_TYPE=unknown




# v2/v3 compatibility
ifeq ($(wildcard ../../../../private/include/logssil.h),)
	INCLUDES = -I../../include/
	ENGINE_LIBPATH =  -L$(shell pwd)/../../lib/linux/
else
	INCLUDES = -I../../../../private/include/
	ENGINE_LIBPATH =  -L$(shell pwd)/../../../../private/bin/
endif

ifeq "$(LINKER_NO_UNDEF)" "yes"
	LINKER_NO_UNDEF_OPT=,--no-undefined
else
	LINKER_NO_UNDEF_OPT=
endif



# Check gcc version to keep compat with old release
GCC_VERSION_GE_v40 := $(shell expr 4.0 \<= `gcc -dumpversion | cut -f1,2 -d.`)
GCC_VERSION_GE_v44 := $(shell expr 4.4 \<= `gcc -dumpversion | cut -f1,2 -d.`)
GCC_VERSION_GE_v46 := $(shell expr 4.6 \<= `gcc -dumpversion | cut -f1,2 -d.`)
GCC_VERSION_GE_v48 := $(shell expr 4.8 \<= `gcc -dumpversion | cut -f1,2 -d.`)


# select best GCC avaiable
ifeq "$(GCC_VERSION_GE_v40)" "1"
	ifeq "$(GCC_VERSION_GE_v44)" "1"
		CC = gcc
		CPP = gcc
	else
		# RH5 workaround
		GCC44FOUND := $(shell which gcc44 2>/dev/null)
		ifneq ($(strip $(GCC44FOUND)),)
			CC = gcc44
			CPP = gcc44
			GCC_VERSION_GE_v44 = 1
		else
       $(error Could not find GCC >= 4.4)
		endif
	endif
else
	# RH4 workaround
	GCC4FOUND := $(shell which gcc4 2>/dev/null)
	ifneq ($(strip $(GCC4FOUND)),)
		CC = gcc4
		CPP = gcc4
		GCC_VERSION_GE_v40 = 1
	else
    $(error Could not find GCC >= 4)
	endif
endif

# The linker and its options.
LD = ld
LIBS = -lpthread -lm -lz -ldl -lrt

# The archiver and its options.
AR = ar ru
RANLIB = ranlib

# General compile flags
DEBUG_GCC_FLAGS += -g
ADDITIONAL_WARNING_ERROR_FLAG += -Wall -Wshadow -Wpointer-arith

ifeq "$(GCC_VERSION_GE_v44)" "1"

  # These warning are not very critical but there is a hudge of them, therefore they hide real problem
  # See doc to check them : http://gcc.gnu.org/onlinedocs/gcc/Warning-Options.html
  DISABLED_WARNING += -Wno-pointer-sign -Wno-unused-parameter -Wno-ignored-qualifiers

  # Addtional GCC 4.4 warning
  ADDITIONAL_WARNING_ERROR_FLAG += -Wunused -Wextra -Wparentheses -Wmissing-declarations -Wmissing-field-initializers -Wuninitialized -Wimplicit-function-declaration -Wformat-security

  # Addtional GCC 4.4 debug flag
  DEBUG_GCC_FLAGS += -ggdb -fdebug-prefix-map=$(BUILDPATH)=.

endif

ifeq "$(GCC_VERSION_GE_v48)" "1"

  # Add debug and optimisze compatible flag
  #
  # new general optimization level, -Og, has been introduced. It addresses the need for fast compilation and a superior debugging experience
  # while providing a reasonable level of runtime performance.
  # Overall experience for development should be better than the default optimization level -O0.
  # DEBUG_GCC_FLAGS += -Og

endif


CFLAGS  = -std=gnu99 -fPIC $(ADDITIONAL_WARNING_ERROR_FLAG) $(DISABLED_WARNING) -D_REENTRANT -DDPcCompiler=DPcCompilerGcc -DDPcSystem=DPcSystemUnix  -DDPcArch=DPcArchlinux -D_LARGEFILE_SOURCE `getconf LFS_CFLAGS` $(INCLUDES)
CFLAGSR = -O2 -L$(RLIBPATH) $(CFLAGS) -DNDEBUG
CFLAGSD =     -L$(DLIBPATH) $(CFLAGS) $(DEBUG_GCC_FLAGS)

OBJR = $(SRC:%.c=$(ROBJPATH)/%.o) $(SRC:%.cpp=$(ROBJPATH)/%.o)
OBJD = $(SRC:%.c=$(DOBJPATH)/%.o) $(SRC:%.cpp=$(DOBJPATH)/%.o)


all:
	$(MAKE) clean
	$(MAKE) release

release: BUILD_TYPE=release
release: $(RLIBPATH)/lib$(NAME).a $(RLIBPATH)/lib$(NAME).so
	$(gen-version)
	@cp -f $(RLIBPATH)/lib$(NAME).so $(BUILDPATH)

redebug:
	$(MAKE) clean
	$(MAKE) debug

debug: BUILD_TYPE=debug
debug: $(DLIBPATH)/lib$(NAME).a $(DLIBPATH)/lib$(NAME).so
	$(gen-version)
	@cp -f $(DLIBPATH)/lib$(NAME).so $(BUILDPATH)

$(RLIBPATH)/lib$(NAME).a: $(RLIBPATH) $(ROBJPATH) $(OBJR)
	cd $(ROBJPATH) && $(AR) $@ *.o
	$(RANLIB) $@

$(RLIBPATH)/lib$(NAME).so: $(RLIBPATH) $(RLIBPATH)/lib$(NAME).a
	cd $(ROBJPATH) && $(CC) -rdynamic -shared -Wl$(LINKER_NO_UNDEF_OPT),-soname,lib$(NAME).so -o $@ *.o $(ENGINE_LIBPATH) $(ADD_LIBR)

$(DLIBPATH)/lib$(NAME).a: $(DLIBPATH) $(DOBJPATH) $(OBJD)
	cd $(DOBJPATH) && $(AR) $@ *.o
	$(RANLIB) $@

$(DLIBPATH)/lib$(NAME).so: $(DLIBPATH) $(DLIBPATH)/lib$(NAME).a
	cd $(DOBJPATH) && $(CC) -rdynamic -shared -Wl$(LINKER_NO_UNDEF_OPT),-soname,lib$(NAME).so -o $@ *.o $(ENGINE_LIBPATH) $(ADD_LIBD)

$(ROBJPATH)/%.o : %.c ; @echo $< ; $(CC) -rdynamic -o $@ -c $(CFLAGSR) $(ADD_INC) -DDPcInDll $(ADD_CFLAGS) $<
$(DOBJPATH)/%.o : %.c ; @echo $< ; $(CC) -rdynamic -o $@ -c $(CFLAGSD) $(ADD_INC) -DDPcInDll $(ADD_CFLAGS) $<

$(ROBJPATH)/%.o : %.cpp ; @echo $< ; $(CPP) -rdynamic -o $@ -c $(CFLAGSR) $(ADD_INC) -DDPcInDll $(ADD_CFLAGS) $<
$(DOBJPATH)/%.o : %.cpp ; @echo $< ; $(CPP) -rdynamic -o $@ -c $(CFLAGSD) $(ADD_INC) -DDPcInDll $(ADD_CFLAGS) $<

$(RLIBPATH):
	@mkdir -p $(RLIBPATH)

$(ROBJPATH):
	@ mkdir -p $(ROBJPATH)

$(DLIBPATH):
	@mkdir -p $(DLIBPATH)

$(DOBJPATH):
	@mkdir -p $(DOBJPATH)


clean : cleanrelease cleandebug
	rm -rf $(OBJPATH)
	rm -rf $(LIBPATH)
	rm -f $(BUILDPATH)/lib$(NAME).so*
	rm -f $(BUILDPATH)/lib$(NAME).version

cleanrelease :
	rm -rf $(ROBJPATH)
	rm -f $(RLIBPATH)/lib$(NAME).so*
	rm -f $(RLIBPATH)/lib$(NAME).a

cleandebug :
	rm -rf $(DOBJPATH)
	rm -f $(DLIBPATH)/lib$(NAME).so*
	rm -f $(DLIBPATH)/lib$(NAME).a

# product version.txt is in a different place in v2 or v3
define gen-version
	@if [ -f ../../../../version.txt ]; then \
	  echo "product:  `grep 'Version' ../../../../version.txt 2>/dev/null | cut -d ',' -f 1`" > $(BUILDPATH)/lib$(NAME).version; \
	elif [ -f ../../../version.txt ]; then \
	  echo "product:  `grep 'Version' ../../../version.txt 2>/dev/null | cut -d ',' -f 1`" > $(BUILDPATH)/lib$(NAME).version; \
	else \
	  echo "product:  unknown" > $(BUILDPATH)/lib$(NAME).version; \
	fi
	@if [ -f ../../version.txt ]; then \
	  echo "instance: `grep 'Révision' ../../version.txt 2>/dev/null | cut -d ',' -f 1`" >> $(BUILDPATH)/lib$(NAME).version; \
	else \
	  echo "instance: unknown" >> $(BUILDPATH)/lib$(NAME).version; \
	fi
	@echo "platform: `uname -m`" >> $(BUILDPATH)/lib$(NAME).version
	@echo "date:     `date +'%F %T'`" >> $(BUILDPATH)/lib$(NAME).version
	@echo "build:    $(BUILD_TYPE)" >> $(BUILDPATH)/lib$(NAME).version
endef

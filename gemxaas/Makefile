#
# Global Makefile for GEM
#

SUBPACKAGES := \
	gem904 \
	gem904int \
	gem904qc8 \
	gemp5 \

SUBPACKAGES.DEBUG    := $(patsubst %,%.debug,    ${SUBPACKAGES})
SUBPACKAGES.INSTALL  := $(patsubst %,%.install,  ${SUBPACKAGES})
SUBPACKAGES.RPM      := $(patsubst %,%.rpm,      ${SUBPACKAGES})
SUBPACKAGES.CLEANRPM := $(patsubst %,%.cleanrpm, ${SUBPACKAGES})
SUBPACKAGES.CLEAN    := $(patsubst %,%.clean,    ${SUBPACKAGES})

#OS:=linux
#ARCH:=x86_64
#LIBDIR:=lib/$(OS)/$(ARCH)

default: all

all: $(SUBPACKAGES)

release: all doc

doc:  $(SUBPACKAGES)
	@echo "Generating doxygen"
	@rm -fr ./doc/html 2> /dev/null
	@doxygen -s ./doc/cmsgemos.cfg  > /dev/null 2>&1
	#@git checkout gh-pages  > /dev/null 2>&1
	#@git add -f ./doc/html  > /dev/null 2>&1
	#@git commit -m "generating doxygen" ./doc/html  > /dev/null 2>&1
	#@git tag ./doc/html  > /dev/null 2>&1

install: $(SUBPACKAGES) $(SUBPACKAGES.INSTALL)

rpm: $(SUBPACKAGES) $(SUBPACKAGES.RPM)

cleanrpm: $(SUBPACKAGES.CLEANRPM)

clean: $(SUBPACKAGES.CLEAN)

$(SUBPACKAGES):
	$(MAKE) -C $@

$(SUBPACKAGES.RPM):
	$(MAKE) -C $(patsubst %.rpm,%, $@) rpm

$(SUBPACKAGES.CLEANRPM):
	$(MAKE) -C $(patsubst %.cleanrpm,%, $@) cleanrpm

$(SUBPACKAGES.INSTALL):
	-find  $(patsubst %.install,%, $@)/lib -name *.so -print -exec cp {} ${LIBDIR} \;

$(SUBPACKAGES.CLEAN):
	$(MAKE) -C $(patsubst %.clean,%, $@) clean

.PHONY: $(SUBPACKAGES) $(SUBPACKAGES.INSTALL) $(SUBPACKAGES.CLEAN)


gem904int: 

gem904qc8: 

gemp5: 

print-env:
	@echo BUILD_HOME    $(BUILD_HOME)
	@echo XDAQ_ROOT     $(XDAQ_ROOT)
	@echo XDAQ_OS       $(XDAQ_OS)
	@echo XDAQ_PLATFORM $(XDAQ_PLATFORM)
	@echo GIT_VERSION   $(GIT_VERSION)
	@echo GEMDEVLOPER   $(GEMDEVLOPER)
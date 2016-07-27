##
## file Makefile
##   Copyright (C)  2015 - 2016 Basile Starynkevitch (and FSF later)
##  MONIMELT is a monitor for MELT - see http://gcc-melt.org/
##  This file is part of GCC.
##
##  GCC is free software; you can redistribute it and/or modify
##  it under the terms of the GNU General Public License as published by
##  the Free Software Foundation; either version 3, or (at your option)
##  any later version.
##
##  GCC is distributed in the hope that it will be useful,
##  but WITHOUT ANY WARRANTY; without even the implied warranty of
##  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
##  GNU General Public License for more details.
##  You should have received a copy of the GNU General Public License
##  along with GCC; see the file COPYING3.   If not see
##  <http://www.gnu.org/licenses/>.
################################################################
## onion is not packaged, see https://github.com/davidmoreno/onion
## Boehm GC is from http://www.hboehm.info/gc/
CC=gcc
CXX=g++
## see http://stackoverflow.com/a/14868153/841108
DISABLE_ASLR= setarch $(shell uname -m) -R
WARNFLAGS= -Wall -Wextra -fdiagnostics-color=auto
CFLAGS= -std=gnu11 $(WARNFLAGS) $(PREPROFLAGS) $(OPTIMFLAGS)
CXXFLAGS= -std=gnu++11 $(WARNFLAGS) $(PREPROFLAGS) $(OPTIMFLAGS)
INDENT= indent
ASTYLE= astyle
MD5SUM= md5sum
INDENTFLAGS= --gnu-style --no-tabs --honour-newlines
ASTYLEFLAGS= --style=gnu -s2
PACKAGES= glib-2.0 sqlite3 jansson
PKGCONFIG= pkg-config
PREPROFLAGS= -I. -I/usr/local/include $(shell $(PKGCONFIG) --cflags $(PACKAGES))
OPTIMFLAGS= -Og -g3

LIBES= -L/usr/local/lib -lgc $(shell $(PKGCONFIG) --libs $(PACKAGES)) \
        -lhiredis -lgccjit -lonion -lpthread -lcrypt -lm -ldl

PLUGIN_SOURCES= $(sort $(wildcard momplug_*.c momplug_*.cc))
PLUGINS=  $(patsubst %.c,%.so,$(PLUGIN_SOURCES))
# modules are generated inside modules/
MODULE_SOURCES= $(sort $(wildcard modules/momg_*.c))
# generated headers
GENERATED_HEADERS= $(sort $(wildcard _mom*.h))
MODULES=  $(patsubst %.c,%.so,$(MODULE_SOURCES))
CSOURCES= $(sort $(filter-out $(PLUGIN_SOURCES), $(wildcard [a-z]*.c)))
CXXSOURCES= $(sort $(filter-out $(PLUGIN_SOURCES) predefgc.cc, $(wildcard [a-z]*.cc)))
OBJECTS= $(patsubst %.c,%.o,$(CSOURCES))  $(patsubst %.cc,%.o,$(CXXSOURCES)) 
RM= rm -fv
.PHONY: all tags modules plugins clean  predefgc tests test0 test1
all: monimelt


clean:
	$(RM) *~ *% *.o *.so */*.so *.log */*~ */*.orig *.i *.ii *.orig README.html *#
	$(RM) modules/*.so modules/*~ modules/*%
	$(RM) _listpredef*
	$(RM) *.bin
	$(RM) _timestamp*
	$(RM) core*
	$(RM) *memo*



_timestamp.c: global.mom Makefile
	@date +'const char monimelt_timestamp[]="%c";' > _timestamp.tmp
	@(echo -n 'const char monimelt_lastgitcommit[]="' ; \
	   git log --format=oneline --abbrev=12 --abbrev-commit -q  \
	     | head -1 | tr -d '\n\r\f\"' ; \
	   echo '";') >> _timestamp.tmp
	@(echo -n 'const char monimelt_lastgittag[]="'; (git describe --abbrev=0 --all || echo '*notag*') | tr -d '\n\r\f\"'; echo '";') >> _timestamp.tmp
	@(echo 'const char monimelt_compilercommand[]="$(strip $(CC))";') >> _timestamp.tmp
	@(echo -n 'const char monimelt_compilerversion[]="' ; $(CC) -v < /dev/null 2>&1 | grep -i version | tr -d  '\n\r\f\"\\' ; echo '";') >> _timestamp.tmp
	@(echo -n 'const char monimelt_compilerflags[]="' ; echo -n "$(strip $(CFLAGS))" | sed 's:":\\":g' ; echo '";') >> _timestamp.tmp
	@(echo -n 'const char monimelt_optimflags[]="' ; echo -n "$(strip $(OPTIMFLAGS))" | sed 's:":\\":g' ; echo '";') >> _timestamp.tmp
	@(echo -n 'const char monimelt_checksum[]="'; cat meltmoni.h $(GENERATED_HEADERS) $(SOURCES) | $(MD5SUM) | cut -d' ' -f1 | tr -d '\n\r\f\"\\' ; echo '";') >> _timestamp.tmp
	@(echo -n 'const char monimelt_directory[]="'; /bin/pwd | tr -d '\n\\"' ; echo '";') >> _timestamp.tmp
	@(echo -n 'const char monimelt_makefile[]="'; echo -n  $(realpath $(lastword $(MAKEFILE_LIST))); echo '";') >> _timestamp.tmp
	@mv _timestamp.tmp _timestamp.c

$(OBJECTS): meltmoni.h $(GENERATED_HEADERS)
monimelt: $(OBJECTS) global.mom
	@if [ -f $@ ]; then echo -n backup old executable: ' ' ; mv -v $@ $@~ ; fi
	$(MAKE) _timestamp.c _timestamp.o
	$(LINK.cc)  $(LINKFLAGS) $(OPTIMFLAGS) -rdynamic $(OBJECTS) $(LIBES) -o $@  _timestamp.o
	rm _timestamp.*

%.i: %.c meltmoni.h $(GENERATED_HEADERS)
	$(COMPILE.c) -C -E $< -o $@
%.ii: %.cc meltmoni.h $(GENERATED_HEADERS)
	$(COMPILE.cc) -C -E $< -o $@
indent: .indent.pro
	cp -v meltmoni.h meltmoni.h%
	$(ASTYLE) $(ASTYLEFLAGS) meltmoni.h
	for f in $(wildcard [a-z]*.c) ; do \
	  echo indenting $$f ; cp $$f $$f%; \
          $(INDENT) $(INDENTFLAGS) $$f ; $(INDENT)  $(INDENTFLAGS) $$f; \
        done
	for g in $(wildcard [a-z]*.cc) ; do \
	  echo astyling $$g ; cp $$g $$g% ; \
	  $(ASTYLE)  $(ASTYLEFLAGS) $$g ; \
	done

predefgc: $(OBJECTS)  predefgc.cc
	rm -f _listpredefs*; touch _listpredefs
	for f in $(filter-out predefitems.o, $(OBJECTS)) ; do  \
	   nm -u $$f | awk '/ mompredef_/{print $$2;}' | sed -n -e 's/^mompredef_//p' >> _listpredefs ; \
	done
	sort -u _listpredefs > _listpredefs.sorted
	mv _listpredefs.sorted _listpredefs
	$(LINK.cc) predefgc.cc -o predefgc.bin
	./predefgc.bin _listpredefs | tee _listpredefs.useless

modules/momg_%.so: modules/momg_%.c $(OBJECTS)
	$(LINK.c) -fPIC -shared $< -o $@

tests: monimelt global.mom test0 test1 testbty

test0: monimelt global.mom  tests/cmod0c.mb 
	$(DISABLE_ASLR) ./monimelt -Dgencod -B tests/cmod0c.mb --test-arg tiny_module --test-run emitc

test1: monimelt global.mom  tests/cmod1j.mb
	$(DISABLE_ASLR) ./monimelt -Dgencod -B tests/cmod1j.mb --test-arg tiny_jmodule --test-run emitjs


testbty: monimelt global.mom  tests/cmodbty.mb
	$(DISABLE_ASLR) ./monimelt -Dgencod -B tests/cmodbty.mb --test-arg bty_module --test-run emitc


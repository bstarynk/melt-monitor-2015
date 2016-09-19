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
SQLITE= sqlite3
INDENTFLAGS= --gnu-style --no-tabs --honour-newlines
ASTYLEFLAGS= --style=gnu -s2  --convert-tabs
PACKAGES= gtk+-x11-3.0 gtk+-3.0 glib-2.0 sqlite3 jansson
PKGCONFIG= pkg-config
PREPROFLAGS= -I. -I/usr/local/include $(shell $(PKGCONFIG) --cflags $(PACKAGES))
OPTIMFLAGS= -Og -g3

LIBES= -L/usr/local/lib -lgc $(shell $(PKGCONFIG) --libs $(PACKAGES)) \
        -lhiredis -lgccjit -lbacktrace -lonion -lpthread -lcrypt -lm -ldl -latomic

.SUFFIXES= .so
# the persistent state base, probably _momstate
MOM_PERSTATE_BASE=_momstate
MOM_PERSTATE_SQLITE=$(MOM_PERSTATE_BASE).sqlite

PLUGIN_SOURCES= $(sort $(wildcard momplug_*.c momplug_*.cc))
PLUGINS=  $(patsubst %.c,%.so,$(PLUGIN_SOURCES))
# modules are generated inside modules.dir/ ; see MOM_MODULES_DIR constant
MODULE_NAMES:=$(shell $(SQLITE) $(MOM_PERSTATE_SQLITE) 'SELECT mod_oid FROM t_modules')
# keep in sync with MOM_MODULES_DIR &  MOM_MODULE_INFIX in meltmoni.h
MODULE_SOURCES= $(sort $(patsubst %,modules.dir/modu%.c,$(MODULE_NAMES)))
# generated headers
GENERATED_HEADERS= $(sort $(wildcard _mom*.h))
MODULES=  $(patsubst %.c,%.so,$(MODULE_SOURCES))
CSOURCES= $(sort $(filter-out $(PLUGIN_SOURCES), $(wildcard [a-z]*.c)))
SHELLSOURCES= $(sort $(wildcard [a-z]*.sh))
OBJECTS= $(patsubst %.c,%.o,$(CSOURCES))
RM= rm -fv
.PHONY: all checkgithooks installgithooks tags modules plugins clean dumpstate restorestate

all: checkgithooks monimelt


clean:
	$(RM) *~ *% *.o *.so */*.so *.log */*~ */*.orig *.i *.ii *.orig README.html *#
	$(RM) modules.dir/*.so modules.dir/*~ modules.dir/*%
	$(RM) _listpredef*
	$(RM) *.bin
	$(RM) ./monimelt
	$(RM) _timestamp*
	$(RM) core*
	$(RM) *memo*



_timestamp.c: Makefile | $(OBJECTS)
	@echo "/* generated file _timestamp.c - DONT EDIT */" > _timestamp.tmp
	@date +'const char monimelt_timestamp[]="%c";' >> _timestamp.tmp
	@(echo -n 'const char monimelt_lastgitcommit[]="' ; \
	   git log --format=oneline --abbrev=12 --abbrev-commit -q  \
	     | head -1 | tr -d '\n\r\f\"' ; \
	   echo '";') >> _timestamp.tmp
	@(echo -n 'const char monimelt_lastgittag[]="'; (git describe --abbrev=0 --all || echo '*notag*') | tr -d '\n\r\f\"'; echo '";') >> _timestamp.tmp
	@echo >> _timestamp.tmp
	@(echo 'const char monimelt_compilercommand[]="$(strip $(CC))";') >> _timestamp.tmp
	@echo >> _timestamp.tmp
	@(echo -n 'const char monimelt_compilerversion[]="' ; $(CC) -v < /dev/null 2>&1 | grep -i version | tr -d  '\n\r\f\"\\' ; echo '";') >> _timestamp.tmp
	@echo >> _timestamp.tmp
	@(echo -n 'const char monimelt_compilerflags[]="' ; echo -n "$(strip $(CFLAGS))" | sed 's:":\\":g' ; echo '";') >> _timestamp.tmp
	@echo >> _timestamp.tmp
	@(echo -n 'const char monimelt_optimflags[]="' ; echo -n "$(strip $(OPTIMFLAGS))" | sed 's:":\\":g' ; echo '";') >> _timestamp.tmp
	@(echo -n 'const char monimelt_checksum[]="'; cat meltmoni.h $(GENERATED_HEADERS) $(SOURCES) | $(MD5SUM) | cut -d' ' -f1 | tr -d '\n\r\f\"\\' ; echo '";') >> _timestamp.tmp
	@(echo -n 'const char monimelt_directory[]="'; /bin/pwd | tr -d '\n\\"' ; echo '";') >> _timestamp.tmp
	@(echo -n 'const char monimelt_makefile[]="'; echo -n  $(realpath $(lastword $(MAKEFILE_LIST))); echo '";') >> _timestamp.tmp
	@(echo -n 'const char monimelt_sqlite[]="'; echo -n $(SQLITE); echo '";') >> _timestamp.tmp
	@(echo -n 'const char monimelt_perstatebase[]="'; echo -n $(MOM_PERSTATE_BASE); echo '";') >> _timestamp.tmp
	@echo >> _timestamp.tmp
	(echo -n 'const char*const monimelt_csources[]={'; for sf in $(CSOURCES) ; do \
	  echo -n "\"$$sf\", " ; done ; \
	echo '(const char*)0};') >> _timestamp.tmp
	@echo >> _timestamp.tmp
	(echo -n 'const char*const monimelt_shellsources[]={'; for sf in $(SHELLSOURCES) ; do \
	  echo -n "\"$$sf\", " ; done ; \
	echo '(const char*)0};') >> _timestamp.tmp
	@mv _timestamp.tmp _timestamp.c

$(OBJECTS): meltmoni.h $(GENERATED_HEADERS)
monimelt: $(OBJECTS) _timestamp.o
	@if [ -f $@ ]; then echo -n makebackup old executable: ' ' ; mv -v $@ $@~ ; fi
	$(LINK.c)  $(LINKFLAGS) $(OPTIMFLAGS) -rdynamic $(OBJECTS)  _timestamp.o $(LIBES) -o $@ 


%.i: %.c meltmoni.h $(GENERATED_HEADERS)
	$(COMPILE.c) -C -E $< -o - | sed s:^#://#:g > $@
%.ii: %.cc meltmoni.h $(GENERATED_HEADERS)
	$(COMPILE.cc) -C -E $< -o $@
indent: .indent.pro
	cp -v meltmoni.h meltmoni.h%
	$(INDENT) $(INDENTFLAGS) meltmoni.h
	$(ASTYLE) $(ASTYLEFLAGS) meltmoni.h
	for f in $(wildcard [a-z]*.c) ; do \
	  echo indenting $$f ; cp $$f $$f%; \
          $(INDENT) $(INDENTFLAGS) $$f ; $(INDENT)  $(INDENTFLAGS) $$f; \
        done
	for g in $(wildcard [a-z]*.cc) ; do \
	  echo astyling $$g ; cp $$g $$g% ; \
	  $(ASTYLE)  $(ASTYLEFLAGS) $$g ; \
	done

modules.dir/%.so: modules.dir/%.c $(OBJECTS)
	$(LINK.c) -fPIC -shared \
	  $(shell $(SQLITE) $(MOM_PERSTATE_SQLITE) \
	          'SELECT mod_cflags FROM t_modules WHERE mod_oid=$(notdir $(basename $@))') \
	  $< \
	  $(shell $(SQLITE) $(MOM_PERSTATE_SQLITE) \
	          'SELECT mod_ldflags FROM t_modules WHERE mod_oid=$(notdir $(basename $@))') \
	  -o $@

momplug_%.so: momplug_%.c meltmoni.h $(GENERATED_HEADERS)
	$(LINK.c) -fPIC -shared $(LINKFLAGS) $(OPTIMFLAGS) $< -o $@

checkgithooks:
	@for hf in *-githook.sh ; do \
	  [ ! -d .git -o -L .git/hooks/$$(basename $$hf "-githook.sh") ] \
	    || (echo uninstalled git hook $$hf "(run: make installgithooks)" >&2 ; exit 1) ; \
	done
installgithooks:
	for hf in *-githook.sh ; do \
	  ln -sv  "../../$$hf" .git/hooks/$$(basename $$hf "-githook.sh") ; \
	done

### dont change without care the name of the monimelt-dump-state.sh
### script since it appears elsewhere, notably in meltmoni.h
dumpstate: $(MOM_PERSTATE_BASE).sqlite | monimelt-dump-state.sh
	./monimelt-dump-state.sh  $(MOM_PERSTATE_BASE).sqlite  $(MOM_PERSTATE_BASE).sql


restorestate: | $(MOM_PERSTATE_BASE).sql
	@if [ -f $(MOM_PERSTATE_BASE).sqlite ]; then \
	  echo makebackup old: ' ' ; mv -v  $(MOM_PERSTATE_BASE).sqlite  $(MOM_PERSTATE_BASE).sqlite~ ; fi
	$(SQLITE)  $(MOM_PERSTATE_BASE).sqlite < $(MOM_PERSTATE_BASE).sql
	touch -r $(MOM_PERSTATE_BASE).sql -c $(MOM_PERSTATE_BASE).sqlite

##
## file Makefile
##   Copyright (C)  2015 Basile Starynkevitch (and FSF later)
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
CCFLAGS=  -std=gnu11 -Wall -Wextra -fdiagnostics-color=auto
CFLAGS= $(CCFLAGS) $(PREPROFLAGS) $(OPTIMFLAGS)
INDENT= indent
MD5SUM= md5sum
INDENTFLAGS= --gnu-style --no-tabs --honour-newlines
PACKAGES= glib-2.0 sqlite3
PKGCONFIG= pkg-config
PREPROFLAGS= -I. -I/usr/local/include $(shell $(PKGCONFIG) --cflags $(PACKAGES))
OPTIMFLAGS= -Og -g3

LIBES= -L/usr/local/lib -lgc $(shell $(PKGCONFIG) --libs $(PACKAGES)) \
        -lgccjit -lonion -lpthread -lcrypt -lm -ldl

PLUGIN_SOURCES= $(sort $(wildcard momplug_*.c))
PLUGINS=  $(patsubst %.c,%.so,$(PLUGIN_SOURCES))
# modules are generated inside modules/
MODULE_SOURCES= $(sort $(wildcard modules/momg_*.c))
# generated headers
GENERATED_HEADERS= $(sort $(wildcard _mom*.h))
MODULES=  $(patsubst %.c,%.so,$(MODULE_SOURCES))
SOURCES= $(sort $(filter-out $(PLUGIN_SOURCES), $(wildcard [a-z]*.c)))
OBJECTS= $(patsubst %.c,%.o,$(SOURCES))
RM= rm -fv
.PHONY: all tags modules plugins clean tests 
all: monimelt


clean:
	$(RM) *~ *% *.o *.so */*.so *.log */*~ */*.orig *.i *.orig
	$(RM) core*



_timestamp.c:
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
	@(echo -n 'const char monimelt_checksum[]="'; cat monimelt.h $(GENERATED_HEADERS) $(SOURCES) | $(MD5SUM) | cut -d' ' -f1 | tr -d '\n\r\f\"\\' ; echo '";') >> _timestamp.tmp
	@mv _timestamp.tmp _timestamp.c

$(OBJECTS): monimelt.h $(GENERATED_HEADERS)
monimelt: $(OBJECTS) _timestamp.o
	@if [ -f $@ ]; then echo -n backup old executable: ' ' ; mv -v $@ $@~ ; fi
	$(LINK.c)  $(LINKFLAGS) -rdynamic $^ $(LIBES) -o $@
	rm _timestamp.*

indent: .indent.pro
	for f in $(wildcard [a-z]*.[ch]) ; do \
	  echo indenting $$f ; cp $$f $$f%; \
          $(INDENT) $(INDENTFLAGS) $$f ; $(INDENT)  $(INDENTFLAGS) $$f; \
        done

#! /bin/sh
##   Copyright (C) 2016  Basile Starynkevitch and later the FSF
##   MONIMELT is a monitor for MELT - see http://gcc-melt.org/
##   This file is part of GCC.
## 
##   GCC is free software; you can redistribute it and/or modify
##   it under the terms of the GNU General Public License as published by
##   the Free Software Foundation; either version 3, or (at your option)
##   any later version.
## 
##   GCC is distributed in the hope that it will be useful,
##   but WITHOUT ANY WARRANTY; without even the implied warranty of
##   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
##   GNU General Public License for more details.
##   You should have received a copy of the GNU General Public License
##   along with GCC; see the file COPYING3.   If not see
##   <http://www.gnu.org/licenses/>.
echo start $0 "$@"
dbfile=$1
sqlfile=$2

if [ ! -f "$dbfile" ]; then
    echo "$0": missing database file "$dbfile" >& 2
    exit 1
fi

if file "$dbfile" | grep -qi SQLite ; then
    echo "$0:" dumping Monimelt Sqlite database $dbfile
else
    echo "$0:" bad database file "$dbfile" >& 2
    exit 1
fi

tempdump=$(basename $(tempfile -d . -p _tmp_ -s .sql))
trap 'rm -vf $tempdump' EXIT INT QUIT TERM
export LANG=C LC_ALL=C

date -r "$dbfile" +"-- $sqlfile dump %Y %b %d from $dbfile dumped by $0" > $tempdump
echo >> $tempdump
date +' --   Copyright (C) %Y Free Software Foundation, Inc.' >> $tempdump
echo ' --  MONIMELT is a monitor for MELT - see http://gcc-melt.org/' >> $tempdump
echo " --  This sqlite3 dump file $sqlfile is part of GCC." >> $tempdump
echo ' --' >> $tempdump
echo ' --  GCC is free software; you can redistribute it and/or modify' >> $tempdump
echo ' --  it under the terms of the GNU General Public License as published by' >> $tempdump
echo ' --  the Free Software Foundation; either version 3, or (at your option)' >> $tempdump
echo ' --  any later version.' >> $tempdump
echo ' --' >> $tempdump
echo ' --  GCC is distributed in the hope that it will be useful,' >> $tempdump
echo ' --  but WITHOUT ANY WARRANTY; without even the implied warranty of' >> $tempdump
echo ' --  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the' >> $tempdump
echo ' --  GNU General Public License for more details.' >> $tempdump
echo ' --  You should have received a copy of the GNU General Public License' >> $tempdump
echo ' --  along with GCC; see the file COPYING3.   If not see' >> $tempdump
echo ' --  <http://www.gnu.org/licenses/>.' >> $tempdump
echo >> $tempdump


## we probably dont want to put the gitcommit in the dump
#(echo -n ' --- monimelt lastgitcommit ' ;  git log --format=oneline --abbrev=12 --abbrev-commit -q  \
#     | head -1 | tr -d '\n\r\f\"' ; echo ' ---' ) >> $tempdump

echo 'BEGIN TRANSACTION;' >> $tempdump
sqlite3 $dbfile  .schema >> $tempdump || exit 1

echo '-- state-monimelt tables contents' >> $tempdump
### IMPORTANT NOTICE: the list of tables should be kept in sync with
### the mo_dump_initialize_sqlite_database function of file jstate.c
sqlite3 $dbfile >> $tempdump <<EOF
.print ---- TABLE t_params @@@@@@
.mode insert t_params
  SELECT * FROM t_params ORDER BY par_name;
.print ---- TABLE t_names @@@@@@@
.mode insert t_names
  SELECT * FROM t_names ORDER BY nam_str;
.print ---- TABLE t_objects @@@@@@@
.mode insert t_objects
  SELECT * FROM t_objects ORDER BY ob_id;
.print ---- TABLE t_modules @@@@@@@
.mode insert t_modules
  SELECT * FROM t_modules ORDER BY mod_oid;
EOF
echo 'COMMIT;' >> $tempdump
echo "-- monimelt-dump-state end dump $dbfile" >> $tempdump

if [ -e "$sqlfile" ]; then
    echo -n "backup Monimelt Sqlite3 dump:" 
    mv -v "$sqlfile" "$sqlfile~"
fi
## we need that the .sql file has the same date as the .sqlite file
touch -f "$dbfile" "$sqlfile"
mv $tempdump "$sqlfile"
ls -l "$sqlfile"

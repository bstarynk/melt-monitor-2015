-- _momstate.sql dump 2016 Aug 31 from _momstate.sqlite dumped by ./monimelt-dump-state.sh

 --   Copyright (C) 2016 Free Software Foundation, Inc.
 --  MONIMELT is a monitor for MELT - see http://gcc-melt.org/
 --  This sqlite3 dump file _momstate.sql is part of GCC.
 --
 --  GCC is free software; you can redistribute it and/or modify
 --  it under the terms of the GNU General Public License as published by
 --  the Free Software Foundation; either version 3, or (at your option)
 --  any later version.
 --
 --  GCC is distributed in the hope that it will be useful,
 --  but WITHOUT ANY WARRANTY; without even the implied warranty of
 --  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 --  GNU General Public License for more details.
 --  You should have received a copy of the GNU General Public License
 --  along with GCC; see the file COPYING3.   If not see
 --  <http://www.gnu.org/licenses/>.

BEGIN TRANSACTION;
CREATE TABLE t_params (par_name VARCHAR(35) PRIMARY KEY ASC NOT NULL UNIQUE,  par_value TEXT NOT NULL);
CREATE TABLE t_objects (ob_id VARCHAR(20) PRIMARY KEY ASC NOT NULL UNIQUE,  ob_mtime DATETIME,  ob_classid VARCHAR(20) NOT NULL,  ob_paylkid VARCHAR(20) NOT NULL,  ob_paylcont TEXT NOT NULL,  ob_jsoncont TEXT NOT NULL);
CREATE TABLE t_names (nam_str PRIMARY KEY ASC NOT NULL UNIQUE,  nam_oid VARCHAR(20) NOT NULL UNIQUE);
CREATE UNIQUE INDEX x_namedid ON t_names (nam_oid);
-- state-monimelt tables contents
INSERT INTO t_params VALUES('monimelt_format_version','MoniMelt2016B');
INSERT INTO t_objects VALUES('_4xS1CSbRUFBW6PJiJ',0,'','','','
{
 "attrs": null,
 "comps": null
}
');
INSERT INTO t_names VALUES('comment','_4xS1CSbRUFBW6PJiJ');
COMMIT;
-- monimelt-dump-state end dump _momstate.sqlite

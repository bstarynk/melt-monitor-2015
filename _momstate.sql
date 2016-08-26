-- _momstate.sql dump 2016 Aug 26 from _momstate.sqlite dumped by ./monimelt-dump-state.sh

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

 --- monimelt lastgitcommit a0613e2ce3b5 adding pre-commit-githook.sh & an empty _momstate.sql ---
BEGIN TRANSACTION;
CREATE TABLE t_params (par_name VARCHAR(35) PRIMARY KEY ASC NOT NULL UNIQUE, par_value TEXT NOT NULL);
CREATE TABLE t_objects (ob_id VARCHAR(20) PRIMARY KEY ASC NOT NULL UNIQUE,
       	     	        ob_mtime DATETIME,
			ob_jsoncont TEXT NOT NULL);
-- state-monimelt tables contents
INSERT INTO t_params VALUES('monimelt_format_version','MoniMelt2016A');
COMMIT;
-- monimelt-dump-state end dump _momstate.sqlite

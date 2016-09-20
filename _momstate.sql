-- _momstate.sql dump 2016 Sep 20 from _momstate.sqlite dumped by ./monimelt-dump-state.sh .....

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
CREATE TABLE t_objects (ob_id VARCHAR(20) PRIMARY KEY ASC NOT NULL UNIQUE,  ob_mtime DATETIME,  ob_jsoncont TEXT NOT NULL,  ob_classid VARCHAR(20) NOT NULL,  ob_paylkid VARCHAR(20) NOT NULL,  ob_paylcont TEXT NOT NULL,  ob_paylmod VARCHAR(20) NOT NULL);
CREATE TABLE t_names (nam_str PRIMARY KEY ASC NOT NULL UNIQUE,  nam_oid VARCHAR(20) NOT NULL UNIQUE);
CREATE TABLE t_modules (mod_oid VARCHAR(20) PRIMARY KEY ASC NOT NULL UNIQUE,  mod_cflags TEXT NOT NULL, mod_ldflags TEXT NOT NULL);
CREATE UNIQUE INDEX x_namedid ON t_names (nam_oid);
-- state-monimelt tables contents
---- TABLE t_params @@@@@@
INSERT INTO t_params VALUES('monimelt_format_version','MoniMelt2016B');
---- TABLE t_names @@@@@@@
INSERT INTO t_names VALUES('GUI_operation','_4x85ZXWJ9HDCurqHP');
INSERT INTO t_names VALUES('basic_ctype_class','_7zd6NdDtMbPp0s5HT');
INSERT INTO t_names VALUES('bool','_3K62tYRL1azRF0cP2');
INSERT INTO t_names VALUES('c_code','_64d2GXNLEBKFBzxqP');
INSERT INTO t_names VALUES('c_field_class','_6Lu6DXoGivCCHMwzV');
INSERT INTO t_names VALUES('c_include','_0hA5FCeEWGc6gpfnU');
INSERT INTO t_names VALUES('c_type','_8TY6UsXJ3Djt71By3');
INSERT INTO t_names VALUES('char','_2Sj1dLJSbvyN3kMgI');
INSERT INTO t_names VALUES('class_class','_84n6z6xA40eduUUjh');
INSERT INTO t_names VALUES('comment','_4xS1CSbRUFBW6PJiJ');
INSERT INTO t_names VALUES('double','_8BM91hTnefUPcYXzf');
INSERT INTO t_names VALUES('enum_ctype_class','_1xh0H6A2nNw5upcPh');
INSERT INTO t_names VALUES('extend','_15c0LcIrdf2LE5JP8');
INSERT INTO t_names VALUES('fields','_7Pl7foCRjKoT6Gkdr');
INSERT INTO t_names VALUES('file_path','_6F98Bo2ChKhNVEZgt');
INSERT INTO t_names VALUES('formals_ctypes','_8Bq9eNWZ0eUS6BGq8');
INSERT INTO t_names VALUES('int','_0Sp1Lg7ctajS7oX5i');
INSERT INTO t_names VALUES('long','_20B54zdX0j8vYUdsf');
INSERT INTO t_names VALUES('module_class','_22z6Be6oa3IH524Jy');
INSERT INTO t_names VALUES('payload_assoval','_5JG8lVw6jwlUT7PLK');
INSERT INTO t_names VALUES('payload_buffer','_1HW4pIotlYRImRGnL');
INSERT INTO t_names VALUES('payload_c_emit','_8hg5YXTgfHBnV4W8q');
INSERT INTO t_names VALUES('payload_file','_46r6DoRftqviBq5NB');
INSERT INTO t_names VALUES('payload_gobject','_4wr5hqASKB1b0Dd5D');
INSERT INTO t_names VALUES('payload_hashset','_8261sbF1f9ohzu2Iu');
INSERT INTO t_names VALUES('payload_json','_8D28gj8akGrJyyzei');
INSERT INTO t_names VALUES('payload_list','_76f7e2VcL8IJC1hq6');
INSERT INTO t_names VALUES('payload_value','_47n6FfKTuPHyjab71');
INSERT INTO t_names VALUES('payload_vectval','_5Hf0fFKvRVa71ZPM0');
INSERT INTO t_names VALUES('result_ctype','_7vy5VxtVon8EhIF5D');
INSERT INTO t_names VALUES('signature_class','_6S30JEAmywph5MZqd');
INSERT INTO t_names VALUES('signature_object_to_value','_8DT8VbN0FaiS0K6uf');
INSERT INTO t_names VALUES('struct_ctype_class','_3CM77K3hIhblwvxg3');
INSERT INTO t_names VALUES('struct_pointer_ctype_class','_53F6MjNKRJ8dutl09');
INSERT INTO t_names VALUES('test_anonobj','_3J97bCYloYmmXuCR1');
INSERT INTO t_names VALUES('the_GUI','_6Pc0DV2SCkMSiNlcf');
INSERT INTO t_names VALUES('the_system','_0BV96V94PJIn9si1K');
INSERT INTO t_names VALUES('union_ctype_class','_3P48jR3hSzSCgv48c');
INSERT INTO t_names VALUES('void','_1WX2mf4xwC5TU5ml7');
---- TABLE t_objects @@@@@@@
INSERT INTO t_objects VALUES('_0BV96V94PJIn9si1K',1472823516,'
{
 "@name": "the_system",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "describes the entire system"
   }
  ]
 },
 "comps": {
  "vectval": [
   {
    "oid": "_3J97bCYloYmmXuCR1"
   }
  ]
 }
}
','','','','');
INSERT INTO t_objects VALUES('_0Sp1Lg7ctajS7oX5i',1474366358,'
{
 "@name": "int",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "the int ctype"
   }
  ]
 },
 "comps": null
}
','','','','');
INSERT INTO t_objects VALUES('_0hA5FCeEWGc6gpfnU',1474292777,'
{
 "@name": "c_include",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "gives the includes in a module"
   }
  ]
 },
 "comps": null
}
','','','','');
INSERT INTO t_objects VALUES('_15c0LcIrdf2LE5JP8',1474361179,'
{
 "@name": "extend",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "gives the object extending something (e.g. in struct ctype...)"
   }
  ]
 },
 "comps": null
}
','','','','');
INSERT INTO t_objects VALUES('_1HW4pIotlYRImRGnL',1473087576,'
{
 "@name": "payload_buffer",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "for buffer payload"
   }
  ]
 },
 "comps": null
}
','','','','');
INSERT INTO t_objects VALUES('_1Ks2XkEjfBfcFh5WJ',1473701710,'
{
 "attrs": {
  "assoval": [
   {
    "at": "_3J97bCYloYmmXuCR1",
    "va": {
     "oid": "_5Xq6uG5cYt456E98W"
    }
   },
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "our anon2obj for GUI testing"
   }
  ]
 },
 "comps": {
  "vectval": [
   {
    "oid": "_5Xq6uG5cYt456E98W"
   }
  ]
 }
}
','','','','');
INSERT INTO t_objects VALUES('_1WX2mf4xwC5TU5ml7',1474366358,'
{
 "@name": "void",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "the void ctype"
   }
  ]
 },
 "comps": null
}
','','','','');
INSERT INTO t_objects VALUES('_1xh0H6A2nNw5upcPh',1474312783,'
{
 "@name": "enum_ctype_class",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "class for enum ctypes"
   }
  ]
 },
 "comps": null
}
','','','','');
INSERT INTO t_objects VALUES('_20B54zdX0j8vYUdsf',1474366358,'
{
 "@name": "long",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "the long ctype"
   }
  ]
 },
 "comps": null
}
','','','','');
INSERT INTO t_objects VALUES('_22z6Be6oa3IH524Jy',1472999195,'
{
 "@name": "module_class",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "class for modules"
   }
  ]
 },
 "comps": null
}
','','','','');
INSERT INTO t_objects VALUES('_2Sj1dLJSbvyN3kMgI',1474366358,'
{
 "@name": "char",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "the char ctype"
   }
  ]
 },
 "comps": null
}
','','','','');
INSERT INTO t_objects VALUES('_3CM77K3hIhblwvxg3',1474312729,'
{
 "@name": "struct_ctype_class",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "class for struct ctypes"
   }
  ]
 },
 "comps": null
}
','','','','');
INSERT INTO t_objects VALUES('_3J97bCYloYmmXuCR1',1473701710,'
{
 "@name": "test_anonobj",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "to test anonymous GUI display"
   }
  ]
 },
 "comps": {
  "vectval": [
   {
    "oid": "_8vY2GjybG9SSjrgKU"
   }
  ]
 }
}
','','','','');
INSERT INTO t_objects VALUES('_3K62tYRL1azRF0cP2',1474366358,'
{
 "@name": "bool",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "the boolean ctype"
   }
  ]
 },
 "comps": null
}
','','','','');
INSERT INTO t_objects VALUES('_3P48jR3hSzSCgv48c',1474312760,'
{
 "@name": "union_ctype_class",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "class for union ctypes"
   }
  ]
 },
 "comps": null
}
','','','','');
INSERT INTO t_objects VALUES('_46r6DoRftqviBq5NB',1473074275,'
{
 "@name": "payload_file",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "for FILE* payload"
   }
  ]
 },
 "comps": null
}
','','','','');
INSERT INTO t_objects VALUES('_47n6FfKTuPHyjab71',1473065027,'
{
 "@name": "payload_value",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "for value payloads"
   }
  ]
 },
 "comps": null
}
','','','','');
INSERT INTO t_objects VALUES('_4wr5hqASKB1b0Dd5D',1473102723,'
{
 "@name": "payload_gobject",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "for GTK & Gobject payloads"
   }
  ]
 },
 "comps": null
}
','','','','');
INSERT INTO t_objects VALUES('_4x85ZXWJ9HDCurqHP',1474269607,'
{
 "@name": "GUI_operation",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "attribute giving the GUI operation for $oper"
   }
  ]
 },
 "comps": null
}
','','','','');
INSERT INTO t_objects VALUES('_4xS1CSbRUFBW6PJiJ',1472212346,'
{
 "@name": "comment",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "for comments, often a string"
   }
  ]
 },
 "comps": null
}
','','','','');
INSERT INTO t_objects VALUES('_53F6MjNKRJ8dutl09',1474308434,'
{
 "@name": "struct_pointer_ctype_class",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "class of pointers to struct ctypes"
   }
  ]
 },
 "comps": null
}
','','','','');
INSERT INTO t_objects VALUES('_5Hf0fFKvRVa71ZPM0',1473049889,'
{
 "@name": "payload_vectval",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "for vectvaldata payload"
   }
  ]
 },
 "comps": null
}
','','','','');
INSERT INTO t_objects VALUES('_5JG8lVw6jwlUT7PLK',1473050875,'
{
 "@name": "payload_assoval",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "for assovaldata payload"
   }
  ]
 },
 "comps": null
}
','','','','');
INSERT INTO t_objects VALUES('_5Xq6uG5cYt456E98W',1473701710,'
{
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "our anon3obj for GUI testing"
   }
  ]
 },
 "comps": null
}
','','','','');
INSERT INTO t_objects VALUES('_64d2GXNLEBKFBzxqP',1474311988,'
{
 "@name": "c_code",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "give the c code expansion or string"
   }
  ]
 },
 "comps": null
}
','','','','');
INSERT INTO t_objects VALUES('_6F98Bo2ChKhNVEZgt',1474292777,'
{
 "@name": "file_path",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "give some file path, e.g. for c-includes"
   }
  ]
 },
 "comps": null
}
','','','','');
INSERT INTO t_objects VALUES('_6Lu6DXoGivCCHMwzV',1474362254,'
{
 "@name": "c_field_class",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "class for C fields"
   }
  ]
 },
 "comps": null
}
','','','','');
INSERT INTO t_objects VALUES('_6Pc0DV2SCkMSiNlcf',1473159222,'
{
 "@name": "the_GUI",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "object describing the initial GUI"
   }
  ]
 },
 "comps": null
}
','','','','');
INSERT INTO t_objects VALUES('_6S30JEAmywph5MZqd',1472749172,'
{
 "@name": "signature_class",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "class of signatures"
   }
  ]
 },
 "comps": null
}
','','','','');
INSERT INTO t_objects VALUES('_76f7e2VcL8IJC1hq6',1473049630,'
{
 "@name": "payload_list",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "for list payload"
   }
  ]
 },
 "comps": null
}
','','','','');
INSERT INTO t_objects VALUES('_7Pl7foCRjKoT6Gkdr',1474361179,'
{
 "@name": "fields",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "gives the fields tuple (e.g. in struct or union ctype...)"
   }
  ]
 },
 "comps": null
}
','','','','');
INSERT INTO t_objects VALUES('_7vy5VxtVon8EhIF5D',1474354808,'
{
 "@name": "result_ctype",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "gives the result ctype in a signature"
   }
  ]
 },
 "comps": null
}
','','','','');
INSERT INTO t_objects VALUES('_7zd6NdDtMbPp0s5HT',1474308434,'
{
 "@name": "basic_ctype_class",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "class of basic C types like int"
   }
  ]
 },
 "comps": null
}
','','','','');
INSERT INTO t_objects VALUES('_8261sbF1f9ohzu2Iu',1473049630,'
{
 "@name": "payload_hashset",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "for hashset payload"
   }
  ]
 },
 "comps": null
}
','','','','');
INSERT INTO t_objects VALUES('_84n6z6xA40eduUUjh',1474367138,'
{
 "@name": "class_class",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "the class of classes"
   }
  ]
 },
 "comps": null
}
','','','','');
INSERT INTO t_objects VALUES('_8BM91hTnefUPcYXzf',1474366358,'
{
 "@name": "double",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "the double ctype"
   }
  ]
 },
 "comps": null
}
','','','','');
INSERT INTO t_objects VALUES('_8Bq9eNWZ0eUS6BGq8',1474354808,'
{
 "@name": "formals_ctypes",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "gives the formals ctypes in a signature"
   }
  ]
 },
 "comps": null
}
','','','','');
INSERT INTO t_objects VALUES('_8D28gj8akGrJyyzei',1473102098,'
{
 "@name": "payload_json",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "for JSON payload"
   }
  ]
 },
 "comps": null
}
','','','','');
INSERT INTO t_objects VALUES('_8DT8VbN0FaiS0K6uf',1474215262,'
{
 "@name": "signature_object_to_value",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "signature: object -> value"
   }
  ]
 },
 "comps": null
}
','_6S30JEAmywph5MZqd','','','');
INSERT INTO t_objects VALUES('_8TY6UsXJ3Djt71By3',1474294464,'
{
 "@name": "c_type",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "gives the C types in a module"
   }
  ]
 },
 "comps": null
}
','','','','');
INSERT INTO t_objects VALUES('_8hg5YXTgfHBnV4W8q',1474270315,'
{
 "@name": "payload_c_emit",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "transient payload for emission of C code"
   }
  ]
 },
 "comps": null
}
','','','','');
INSERT INTO t_objects VALUES('_8vY2GjybG9SSjrgKU',1473701710,'
{
 "attrs": {
  "assoval": [
   {
    "at": "_3J97bCYloYmmXuCR1",
    "va": {
     "oid": "_1Ks2XkEjfBfcFh5WJ"
    }
   },
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "our anon1obj for GUI testing"
   }
  ]
 },
 "comps": null
}
','','','','');
---- TABLE t_modules @@@@@@@
COMMIT;
-- monimelt-dump-state end dump _momstate.sqlite

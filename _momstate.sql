-- _momstate.sql dump 2016 Sep 26 from _momstate.sqlite dumped by ./monimelt-dump-state.sh .....

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
INSERT INTO t_names VALUES('add_user_action','_3cH0f65UzqnPaGsLY');
INSERT INTO t_names VALUES('add_user_action_useract','_0xA91ltlCCIeN6K9H');
INSERT INTO t_names VALUES('array_ctype_class','_4iH7xI2JtAomiBySj');
INSERT INTO t_names VALUES('basic_ctype_class','_7zd6NdDtMbPp0s5HT');
INSERT INTO t_names VALUES('bool','_3K62tYRL1azRF0cP2');
INSERT INTO t_names VALUES('c_code','_64d2GXNLEBKFBzxqP');
INSERT INTO t_names VALUES('c_field_class','_6Lu6DXoGivCCHMwzV');
INSERT INTO t_names VALUES('c_include','_0hA5FCeEWGc6gpfnU');
INSERT INTO t_names VALUES('c_type','_8TY6UsXJ3Djt71By3');
INSERT INTO t_names VALUES('cemit_module','_6bK8gGYmKZXwwNXj4');
INSERT INTO t_names VALUES('cemit_module_useract','_21c6aoW2jM5uS1jrN');
INSERT INTO t_names VALUES('char','_2Sj1dLJSbvyN3kMgI');
INSERT INTO t_names VALUES('class','_7al47yMPGToSh0ryC');
INSERT INTO t_names VALUES('class_class','_84n6z6xA40eduUUjh');
INSERT INTO t_names VALUES('class_useract','_7831xB7d1ulmsaCYS');
INSERT INTO t_names VALUES('comment','_4xS1CSbRUFBW6PJiJ');
INSERT INTO t_names VALUES('double','_8BM91hTnefUPcYXzf');
INSERT INTO t_names VALUES('enum_ctype_class','_1xh0H6A2nNw5upcPh');
INSERT INTO t_names VALUES('enumaa_test1','_6Xj6iYu8f0Kd5G4fo');
INSERT INTO t_names VALUES('enumab_test1','_8N58oAS4pxRlnRThx');
INSERT INTO t_names VALUES('enumbu_test1','_4Ph3UMWqrIloqawrN');
INSERT INTO t_names VALUES('enumerator_class','_0dK6lxuDBgrY7Zdrb');
INSERT INTO t_names VALUES('enumerators','_4Cz6GxTDLCsTy7bco');
INSERT INTO t_names VALUES('extend','_15c0LcIrdf2LE5JP8');
INSERT INTO t_names VALUES('fieldaa_test1','_1Ma1hIkjW2pwNHiKZ');
INSERT INTO t_names VALUES('fieldab_test1','_4qM1PCt6z5dVadxJG');
INSERT INTO t_names VALUES('fieldbx_test1','_6rV7sjJup5PnMH7Rt');
INSERT INTO t_names VALUES('fields','_7Pl7foCRjKoT6Gkdr');
INSERT INTO t_names VALUES('fielduay_test1','_2zq5GZyUCWamFY8A3');
INSERT INTO t_names VALUES('fielduaz_test1','_7sh3o3kwn9efIpWoS');
INSERT INTO t_names VALUES('fieldubw_test1','_5tY0jFlg4ZFMGRXjL');
INSERT INTO t_names VALUES('file_path','_6F98Bo2ChKhNVEZgt');
INSERT INTO t_names VALUES('fnmatch','_5Ta34TyJJebRyD9sN');
INSERT INTO t_names VALUES('fnmatch_useract','_59g7wsJyXlj962jlx');
INSERT INTO t_names VALUES('formals_ctypes','_8Bq9eNWZ0eUS6BGq8');
INSERT INTO t_names VALUES('get','_1Xe4mcjLDBpvaBiDU');
INSERT INTO t_names VALUES('get_useract','_6VK4TFfclh68txISx');
INSERT INTO t_names VALUES('include_monimelt','_0m37ffFuKe0qxNfTK');
INSERT INTO t_names VALUES('int','_0Sp1Lg7ctajS7oX5i');
INSERT INTO t_names VALUES('int16_t','_1Hx4BMvqv69AA0750');
INSERT INTO t_names VALUES('int32_t','_7EH4RTtyF1BioGEDK');
INSERT INTO t_names VALUES('int64_t','_21x8NL3oau6heNEnt');
INSERT INTO t_names VALUES('int8_t','_62H0GDHPCtIRvAdDd');
INSERT INTO t_names VALUES('intptr_t','_8t42m75faVNE2kWxu');
INSERT INTO t_names VALUES('long','_20B54zdX0j8vYUdsf');
INSERT INTO t_names VALUES('module_class','_22z6Be6oa3IH524Jy');
INSERT INTO t_names VALUES('notice','_4H01ICaNuWncMePhe');
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
INSERT INTO t_names VALUES('pointer_ctype_class','_7JP45UsR282DkTrJS');
INSERT INTO t_names VALUES('remove','_4Ph1Z99dptyJhpZfj');
INSERT INTO t_names VALUES('remove_useract','_8j61MrY76IlS6t10r');
INSERT INTO t_names VALUES('result_ctype','_7vy5VxtVon8EhIF5D');
INSERT INTO t_names VALUES('set','_7h20HFhtjZljRLtbU');
INSERT INTO t_names VALUES('set_useract','_6Z90lCcFC3RhKoHGv');
INSERT INTO t_names VALUES('signature_class','_6S30JEAmywph5MZqd');
INSERT INTO t_names VALUES('signature_int_to_double','_5fb7a3nqXwUKGKblr');
INSERT INTO t_names VALUES('signature_object_to_value','_8DT8VbN0FaiS0K6uf');
INSERT INTO t_names VALUES('string','_2k61pgEBsCRJlTpHG');
INSERT INTO t_names VALUES('struct_ctype_class','_3CM77K3hIhblwvxg3');
INSERT INTO t_names VALUES('struct_pointer_ctype_class','_53F6MjNKRJ8dutl09');
INSERT INTO t_names VALUES('test1_module','_4NE8paRtDiMVlhHXX');
INSERT INTO t_names VALUES('test_anonobj','_3J97bCYloYmmXuCR1');
INSERT INTO t_names VALUES('the_GUI','_6Pc0DV2SCkMSiNlcf');
INSERT INTO t_names VALUES('the_system','_0BV96V94PJIn9si1K');
INSERT INTO t_names VALUES('tuple','_0Ki51w13UIhXkBrtx');
INSERT INTO t_names VALUES('tuple_useract','_80D4pPU1WGn7IZwh6');
INSERT INTO t_names VALUES('type_enuma_test1','_6zI7Ksmo3XlnSBba9');
INSERT INTO t_names VALUES('type_enumb_test1','_2fW8RGq4VsALItRsE');
INSERT INTO t_names VALUES('type_ptrint','_0m46GxlBP9bdo05q4');
INSERT INTO t_names VALUES('type_structa_test1','_7M91oqNZoxo8xMq7c');
INSERT INTO t_names VALUES('type_structb_test1','_6cu0xvaoNV0W4X4aW');
INSERT INTO t_names VALUES('type_uniona_test1','_6Z810NfglNjogmNcG');
INSERT INTO t_names VALUES('type_unionb_test1','_7BI45k1e9Ri0Wv9rm');
INSERT INTO t_names VALUES('uint16_t','_8T17wDyPfJSmGz5Ve');
INSERT INTO t_names VALUES('uint32_t','_6y88mWwevVhETpkig');
INSERT INTO t_names VALUES('uint64_t','_6r06aqWaPdfZgAKBP');
INSERT INTO t_names VALUES('uint8_t','_3WR20H7JRlNWkDV4y');
INSERT INTO t_names VALUES('uintptr_t','_21J3zZDnsKTbAVBFo');
INSERT INTO t_names VALUES('union_ctype_class','_3P48jR3hSzSCgv48c');
INSERT INTO t_names VALUES('user_actions','_5Kq0Pz5eFAULvdMEL');
INSERT INTO t_names VALUES('value','_4Cm8ln5mSkcZHI6WB');
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
    "set": [
     "_0xA91ltlCCIeN6K9H",
     "_0Ki51w13UIhXkBrtx",
     "_0Sp1Lg7ctajS7oX5i",
     "_1Hx4BMvqv69AA0750",
     "_1WX2mf4xwC5TU5ml7",
     "_1Xe4mcjLDBpvaBiDU",
     "_20B54zdX0j8vYUdsf",
     "_21c6aoW2jM5uS1jrN",
     "_21x8NL3oau6heNEnt",
     "_21J3zZDnsKTbAVBFo",
     "_2k61pgEBsCRJlTpHG",
     "_2Sj1dLJSbvyN3kMgI",
     "_3cH0f65UzqnPaGsLY",
     "_3K62tYRL1azRF0cP2",
     "_3WR20H7JRlNWkDV4y",
     "_4iH7xI2JtAomiBySj",
     "_4Ph1Z99dptyJhpZfj",
     "_59g7wsJyXlj962jlx",
     "_5Ta34TyJJebRyD9sN",
     "_62H0GDHPCtIRvAdDd",
     "_6bK8gGYmKZXwwNXj4",
     "_6r06aqWaPdfZgAKBP",
     "_6y88mWwevVhETpkig",
     "_6VK4TFfclh68txISx",
     "_6Z90lCcFC3RhKoHGv",
     "_7831xB7d1ulmsaCYS",
     "_7h20HFhtjZljRLtbU",
     "_7EH4RTtyF1BioGEDK",
     "_7JP45UsR282DkTrJS",
     "_80D4pPU1WGn7IZwh6",
     "_8j61MrY76IlS6t10r",
     "_8t42m75faVNE2kWxu",
     "_8BM91hTnefUPcYXzf",
     "_8T17wDyPfJSmGz5Ve"
    ]
   },
   {
    "oid": "_3J97bCYloYmmXuCR1"
   },
   {
    "oid": "_4NE8paRtDiMVlhHXX"
   },
   {
    "oid": "_6bK8gGYmKZXwwNXj4"
   }
  ]
 }
}
','','','','');
INSERT INTO t_objects VALUES('_0Ki51w13UIhXkBrtx',1474464502,'
{
 "@name": "tuple",
 "attrs": {
  "assoval": [
   {
    "at": "_4x85ZXWJ9HDCurqHP",
    "va": {
     "oid": "_80D4pPU1WGn7IZwh6"
    }
   },
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "for tuple related stuff"
   }
  ]
 },
 "comps": null
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
','_7zd6NdDtMbPp0s5HT','','','');
INSERT INTO t_objects VALUES('_0dK6lxuDBgrY7Zdrb',1474391155,'
{
 "@name": "enumerator_class",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "class of C enumerators"
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
INSERT INTO t_objects VALUES('_0m37ffFuKe0qxNfTK',1474696522,'
{
 "@name": "include_monimelt",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "to include the \"monimelt.h\" header"
   },
   {
    "at": "_6F98Bo2ChKhNVEZgt",
    "va": "monimelt.h"
   }
  ]
 },
 "comps": null
}
','','','','');
INSERT INTO t_objects VALUES('_0m46GxlBP9bdo05q4',1474868749,'
{
 "@name": "type_ptrint",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "ctype pointer to int"
   }
  ]
 },
 "comps": {
  "vectval": [
   {
    "oid": "_0Sp1Lg7ctajS7oX5i"
   }
  ]
 }
}
','_7JP45UsR282DkTrJS','','','');
INSERT INTO t_objects VALUES('_0xA91ltlCCIeN6K9H',1474534835,'
{
 "@name": "add_user_action_useract",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "routine for add_user_action"
   }
  ]
 },
 "comps": null
}
','','_8DT8VbN0FaiS0K6uf','!','.');
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
INSERT INTO t_objects VALUES('_1Hx4BMvqv69AA0750',1474473152,'
{
 "@name": "int16_t",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "the ctype for 16 bits integers"
   }
  ]
 },
 "comps": null
}
','_7zd6NdDtMbPp0s5HT','','','');
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
INSERT INTO t_objects VALUES('_1Ma1hIkjW2pwNHiKZ',1474879001,'
{
 "@name": "fieldaa_test1",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "first field of structa"
   },
   {
    "at": "_8TY6UsXJ3Djt71By3",
    "va": {
     "oid": "_0Sp1Lg7ctajS7oX5i"
    }
   }
  ]
 },
 "comps": null
}
','_6Lu6DXoGivCCHMwzV','','','');
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
','_7zd6NdDtMbPp0s5HT','','','');
INSERT INTO t_objects VALUES('_1Xe4mcjLDBpvaBiDU',1474873708,'
{
 "@name": "get",
 "attrs": {
  "assoval": [
   {
    "at": "_4x85ZXWJ9HDCurqHP",
    "va": {
     "oid": "_6VK4TFfclh68txISx"
    }
   },
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "to get some attribute or component, etc. also $get user action."
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
','_7zd6NdDtMbPp0s5HT','','','');
INSERT INTO t_objects VALUES('_21J3zZDnsKTbAVBFo',1474473958,'
{
 "@name": "uintptr_t",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "ctype of unsigned int of same size as pointers"
   }
  ]
 },
 "comps": null
}
','_7zd6NdDtMbPp0s5HT','','','');
INSERT INTO t_objects VALUES('_21c6aoW2jM5uS1jrN',1474699053,'
{
 "@name": "cemit_module_useract",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "user action for cemit_module"
   }
  ]
 },
 "comps": null
}
','','_8DT8VbN0FaiS0K6uf','!','.');
INSERT INTO t_objects VALUES('_21x8NL3oau6heNEnt',1474473182,'
{
 "@name": "int64_t",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "the ctype for 64 bits integers"
   }
  ]
 },
 "comps": null
}
','_7zd6NdDtMbPp0s5HT','','','');
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
INSERT INTO t_objects VALUES('_2fW8RGq4VsALItRsE',1474884393,'
{
 "@name": "type_enumb_test1",
 "attrs": {
  "assoval": [
   {
    "at": "_15c0LcIrdf2LE5JP8",
    "va": {
     "oid": "_6zI7Ksmo3XlnSBba9"
    }
   },
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "ctype for enumb extending enuma"
   },
   {
    "at": "_4Cz6GxTDLCsTy7bco",
    "va": {
     "tup": [
      "_4Ph3UMWqrIloqawrN"
     ]
    }
   }
  ]
 },
 "comps": null
}
','_1xh0H6A2nNw5upcPh','','','');
INSERT INTO t_objects VALUES('_2k61pgEBsCRJlTpHG',1474462452,'
{
 "@name": "string",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "for string related stuff"
   }
  ]
 },
 "comps": null
}
','','','','');
INSERT INTO t_objects VALUES('_2zq5GZyUCWamFY8A3',1474887310,'
{
 "@name": "fielduay_test1",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "fielduay in uniona for test1"
   },
   {
    "at": "_8TY6UsXJ3Djt71By3",
    "va": {
     "oid": "_8BM91hTnefUPcYXzf"
    }
   }
  ]
 },
 "comps": null
}
','_6Lu6DXoGivCCHMwzV','','','');
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
','_7zd6NdDtMbPp0s5HT','','','');
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
INSERT INTO t_objects VALUES('_3WR20H7JRlNWkDV4y',1474473996,'
{
 "@name": "uint8_t",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "ctype of unsigned 8 bits integers"
   }
  ]
 },
 "comps": null
}
','_7zd6NdDtMbPp0s5HT','','','');
INSERT INTO t_objects VALUES('_3cH0f65UzqnPaGsLY',1474534835,'
{
 "@name": "add_user_action",
 "attrs": {
  "assoval": [
   {
    "at": "_4x85ZXWJ9HDCurqHP",
    "va": {
     "oid": "_0xA91ltlCCIeN6K9H"
    }
   },
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": {
     "ssize": 110,
     "string": [
      "$add_user_action (Action Action_useract",
      ") would add the user action $Action with ",
      "the routine Action_useract...."
     ]
    }
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
INSERT INTO t_objects VALUES('_4Cm8ln5mSkcZHI6WB',1474392475,'
{
 "@name": "value",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "attribute to give some value"
   }
  ]
 },
 "comps": null
}
','','','','');
INSERT INTO t_objects VALUES('_4Cz6GxTDLCsTy7bco',1474391079,'
{
 "@name": "enumerators",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "for enumerators in enum ctypes"
   }
  ]
 },
 "comps": null
}
','','','','');
INSERT INTO t_objects VALUES('_4H01ICaNuWncMePhe',1474896198,'
{
 "@name": "notice",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "attribute to notice something"
   }
  ]
 },
 "comps": null
}
','','','','');
INSERT INTO t_objects VALUES('_4NE8paRtDiMVlhHXX',1474896198,'
{
 "@name": "test1_module",
 "attrs": {
  "assoval": [
   {
    "at": "_0hA5FCeEWGc6gpfnU",
    "va": {
     "tup": [
      "_0m37ffFuKe0qxNfTK"
     ]
    }
   },
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "some module to test c-emission"
   },
   {
    "at": "_4H01ICaNuWncMePhe",
    "va": {
     "oid": "_5fb7a3nqXwUKGKblr"
    }
   },
   {
    "at": "_8TY6UsXJ3Djt71By3",
    "va": {
     "tup": [
      "_0m46GxlBP9bdo05q4",
      "_7M91oqNZoxo8xMq7c",
      "_6cu0xvaoNV0W4X4aW",
      "_6Z810NfglNjogmNcG",
      "_7BI45k1e9Ri0Wv9rm",
      "_6zI7Ksmo3XlnSBba9",
      "_2fW8RGq4VsALItRsE"
     ]
    }
   }
  ]
 },
 "comps": null
}
','_22z6Be6oa3IH524Jy','','','');
INSERT INTO t_objects VALUES('_4Ph1Z99dptyJhpZfj',1474873729,'
{
 "@name": "remove",
 "attrs": {
  "assoval": [
   {
    "at": "_4x85ZXWJ9HDCurqHP",
    "va": {
     "oid": "_8j61MrY76IlS6t10r"
    }
   },
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "to remove something, e.g. $remove(&object &attr) user action"
   }
  ]
 },
 "comps": null
}
','','','','');
INSERT INTO t_objects VALUES('_4Ph3UMWqrIloqawrN',1474891656,'
{
 "@name": "enumbu_test1",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "enumerator bu for type_enumb in test1"
   },
   {
    "at": "_4Cm8ln5mSkcZHI6WB",
    "va": 0
   }
  ]
 },
 "comps": null
}
','_0dK6lxuDBgrY7Zdrb','','','');
INSERT INTO t_objects VALUES('_4iH7xI2JtAomiBySj',1474477481,'
{
 "@name": "array_ctype_class",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": {
     "ssize": 116,
     "string": [
      "class for array ctypes, the first component",
      "#0 is the element ctype; the second component",
      "#1 is the dimension, if any."
     ]
    }
   }
  ]
 },
 "comps": null
}
','_84n6z6xA40eduUUjh','','','');
INSERT INTO t_objects VALUES('_4qM1PCt6z5dVadxJG',1474879036,'
{
 "@name": "fieldab_test1",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "fieldab in structa"
   },
   {
    "at": "_8TY6UsXJ3Djt71By3",
    "va": {
     "oid": "_0m46GxlBP9bdo05q4"
    }
   }
  ]
 },
 "comps": null
}
','_6Lu6DXoGivCCHMwzV','','','');
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
INSERT INTO t_objects VALUES('_59g7wsJyXlj962jlx',1474789841,'
{
 "@name": "fnmatch_useract",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "for implementing $fnmatch command"
   }
  ]
 },
 "comps": null
}
','','_8DT8VbN0FaiS0K6uf','!','.');
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
INSERT INTO t_objects VALUES('_5Kq0Pz5eFAULvdMEL',1474372791,'
{
 "@name": "user_actions",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "attribute to keep the set of user actions in the_GUI"
   }
  ]
 },
 "comps": null
}
','','','','');
INSERT INTO t_objects VALUES('_5Ta34TyJJebRyD9sN',1474795461,'
{
 "@name": "fnmatch",
 "attrs": {
  "assoval": [
   {
    "at": "_4x85ZXWJ9HDCurqHP",
    "va": {
     "oid": "_59g7wsJyXlj962jlx"
    }
   },
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "for $fnmatch(\"*ab*\") to use fnmatch with FNM_EXTMATCH & FNM_CASEFOLD on names"
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
INSERT INTO t_objects VALUES('_5fb7a3nqXwUKGKblr',1474896066,'
{
 "@name": "signature_int_to_double",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "signature of int->double functions"
   }
  ]
 },
 "comps": null
}
','_6S30JEAmywph5MZqd','','','');
INSERT INTO t_objects VALUES('_5tY0jFlg4ZFMGRXjL',1474887990,'
{
 "@name": "fieldubw_test1",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "field ubw in unionb for test1"
   },
   {
    "at": "_8TY6UsXJ3Djt71By3",
    "va": {
     "oid": "_8t42m75faVNE2kWxu"
    }
   }
  ]
 },
 "comps": null
}
','_6Lu6DXoGivCCHMwzV','','','');
INSERT INTO t_objects VALUES('_62H0GDHPCtIRvAdDd',1474473139,'
{
 "@name": "int8_t",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "the ctype for 8 bits signed integers"
   }
  ]
 },
 "comps": null
}
','_7zd6NdDtMbPp0s5HT','','','');
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
INSERT INTO t_objects VALUES('_6Pc0DV2SCkMSiNlcf',1474462452,'
{
 "@name": "the_GUI",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "object describing the initial GUI"
   },
   {
    "at": "_5Kq0Pz5eFAULvdMEL",
    "va": {
     "tup": [
      "_7al47yMPGToSh0ryC",
      "_7h20HFhtjZljRLtbU",
      "_0Ki51w13UIhXkBrtx",
      "_2k61pgEBsCRJlTpHG"
     ]
    }
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
INSERT INTO t_objects VALUES('_6VK4TFfclh68txISx',1474869958,'
{
 "@name": "get_useract",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "user action for $get"
   }
  ]
 },
 "comps": null
}
','','_8DT8VbN0FaiS0K6uf','!','.');
INSERT INTO t_objects VALUES('_6Xj6iYu8f0Kd5G4fo',1474891268,'
{
 "@name": "enumaa_test1",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "first enumerator in type_enuma_test1"
   },
   {
    "at": "_4Cm8ln5mSkcZHI6WB",
    "va": 0
   }
  ]
 },
 "comps": null
}
','_0dK6lxuDBgrY7Zdrb','','','');
INSERT INTO t_objects VALUES('_6Z810NfglNjogmNcG',1474879548,'
{
 "@name": "type_uniona_test1",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "ctype for uniona"
   },
   {
    "at": "_7Pl7foCRjKoT6Gkdr",
    "va": {
     "tup": [
      "_2zq5GZyUCWamFY8A3",
      "_7sh3o3kwn9efIpWoS"
     ]
    }
   }
  ]
 },
 "comps": null
}
','_3P48jR3hSzSCgv48c','','','');
INSERT INTO t_objects VALUES('_6Z90lCcFC3RhKoHGv',1474464458,'
{
 "@name": "set_useract",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "variadic user action to make a set from ingredients (objects or sequences)"
   }
  ]
 },
 "comps": null
}
','','_8DT8VbN0FaiS0K6uf','!','.');
INSERT INTO t_objects VALUES('_6bK8gGYmKZXwwNXj4',1474699623,'
{
 "@name": "cemit_module",
 "attrs": {
  "assoval": [
   {
    "at": "_4x85ZXWJ9HDCurqHP",
    "va": {
     "oid": "_21c6aoW2jM5uS1jrN"
    }
   },
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "operation to emit c code for a module: $cemit_module(TheModule)"
   }
  ]
 },
 "comps": null
}
','','','','');
INSERT INTO t_objects VALUES('_6cu0xvaoNV0W4X4aW',1474879292,'
{
 "@name": "type_structb_test1",
 "attrs": {
  "assoval": [
   {
    "at": "_15c0LcIrdf2LE5JP8",
    "va": {
     "oid": "_7M91oqNZoxo8xMq7c"
    }
   },
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "ctype for structb extending structa"
   },
   {
    "at": "_7Pl7foCRjKoT6Gkdr",
    "va": {
     "tup": [
      "_6rV7sjJup5PnMH7Rt"
     ]
    }
   }
  ]
 },
 "comps": null
}
','_3CM77K3hIhblwvxg3','','','');
INSERT INTO t_objects VALUES('_6r06aqWaPdfZgAKBP',1474474102,'
{
 "@name": "uint64_t",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "ctype for unsigned 64 bits integers"
   }
  ]
 },
 "comps": null
}
','_7zd6NdDtMbPp0s5HT','','','');
INSERT INTO t_objects VALUES('_6rV7sjJup5PnMH7Rt',1474879343,'
{
 "@name": "fieldbx_test1",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "field bx in type_structb_test1 "
   },
   {
    "at": "_8TY6UsXJ3Djt71By3",
    "va": {
     "oid": "_2Sj1dLJSbvyN3kMgI"
    }
   }
  ]
 },
 "comps": null
}
','_6Lu6DXoGivCCHMwzV','','','');
INSERT INTO t_objects VALUES('_6y88mWwevVhETpkig',1474474068,'
{
 "@name": "uint32_t",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "ctype for unsigned 32 bits integers"
   }
  ]
 },
 "comps": null
}
','_7zd6NdDtMbPp0s5HT','','','');
INSERT INTO t_objects VALUES('_6zI7Ksmo3XlnSBba9',1474881756,'
{
 "@name": "type_enuma_test1",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "ctype for enuma"
   },
   {
    "at": "_4Cz6GxTDLCsTy7bco",
    "va": {
     "tup": [
      "_6Xj6iYu8f0Kd5G4fo",
      "_8N58oAS4pxRlnRThx"
     ]
    }
   }
  ]
 },
 "comps": null
}
','_1xh0H6A2nNw5upcPh','','','');
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
INSERT INTO t_objects VALUES('_7831xB7d1ulmsaCYS',1474374401,'
{
 "@name": "class_useract",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "user action for $class (object class)"
   }
  ]
 },
 "comps": null
}
','','_8DT8VbN0FaiS0K6uf','!','.');
INSERT INTO t_objects VALUES('_7BI45k1e9Ri0Wv9rm',1474880778,'
{
 "@name": "type_unionb_test1",
 "attrs": {
  "assoval": [
   {
    "at": "_15c0LcIrdf2LE5JP8",
    "va": {
     "oid": "_6Z810NfglNjogmNcG"
    }
   },
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "ctype for unionb extending uniona"
   },
   {
    "at": "_7Pl7foCRjKoT6Gkdr",
    "va": {
     "tup": [
      "_5tY0jFlg4ZFMGRXjL"
     ]
    }
   }
  ]
 },
 "comps": null
}
','_3P48jR3hSzSCgv48c','','','');
INSERT INTO t_objects VALUES('_7EH4RTtyF1BioGEDK',1474473165,'
{
 "@name": "int32_t",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "the ctype for 32 bits integers"
   }
  ]
 },
 "comps": null
}
','_7zd6NdDtMbPp0s5HT','','','');
INSERT INTO t_objects VALUES('_7JP45UsR282DkTrJS',1474477388,'
{
 "@name": "pointer_ctype_class",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "class of pointer ctypes; the first component #0 is the pointed ctype."
   }
  ]
 },
 "comps": null
}
','_84n6z6xA40eduUUjh','','','');
INSERT INTO t_objects VALUES('_7M91oqNZoxo8xMq7c',1474878786,'
{
 "@name": "type_structa_test1",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "ctype for structa"
   },
   {
    "at": "_7Pl7foCRjKoT6Gkdr",
    "va": {
     "tup": [
      "_1Ma1hIkjW2pwNHiKZ",
      "_4qM1PCt6z5dVadxJG"
     ]
    }
   }
  ]
 },
 "comps": {
  "vectval": []
 }
}
','_3CM77K3hIhblwvxg3','','','');
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
INSERT INTO t_objects VALUES('_7al47yMPGToSh0ryC',1474374401,'
{
 "@name": "class",
 "attrs": {
  "assoval": [
   {
    "at": "_4x85ZXWJ9HDCurqHP",
    "va": {
     "oid": "_7831xB7d1ulmsaCYS"
    }
   },
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "anything related to class, also a user action $class (object class)"
   }
  ]
 },
 "comps": null
}
','','','','');
INSERT INTO t_objects VALUES('_7h20HFhtjZljRLtbU',1474464458,'
{
 "@name": "set",
 "attrs": {
  "assoval": [
   {
    "at": "_4x85ZXWJ9HDCurqHP",
    "va": {
     "oid": "_6Z90lCcFC3RhKoHGv"
    }
   },
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "for set related stuff"
   }
  ]
 },
 "comps": null
}
','','','','');
INSERT INTO t_objects VALUES('_7sh3o3kwn9efIpWoS',1474887609,'
{
 "@name": "fielduaz_test1",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "fielduaz in uniona for test1"
   },
   {
    "at": "_8TY6UsXJ3Djt71By3",
    "va": {
     "oid": "_0m46GxlBP9bdo05q4"
    }
   }
  ]
 },
 "comps": null
}
','_6Lu6DXoGivCCHMwzV','','','');
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
INSERT INTO t_objects VALUES('_80D4pPU1WGn7IZwh6',1474464502,'
{
 "@name": "tuple_useract",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "user action to make a tuple out of ingredients"
   }
  ]
 },
 "comps": null
}
','','_8DT8VbN0FaiS0K6uf','!','.');
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
','_7zd6NdDtMbPp0s5HT','','','');
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
INSERT INTO t_objects VALUES('_8N58oAS4pxRlnRThx',1474891268,'
{
 "@name": "enumab_test1",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "second enumerator in type_enuma_test1"
   },
   {
    "at": "_4Cm8ln5mSkcZHI6WB",
    "va": 0
   }
  ]
 },
 "comps": null
}
','_0dK6lxuDBgrY7Zdrb','','','');
INSERT INTO t_objects VALUES('_8T17wDyPfJSmGz5Ve',1474474042,'
{
 "@name": "uint16_t",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "ctype for unsigned 16 bits integers"
   }
  ]
 },
 "comps": null
}
','_7zd6NdDtMbPp0s5HT','','','');
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
INSERT INTO t_objects VALUES('_8j61MrY76IlS6t10r',1474870024,'
{
 "@name": "remove_useract",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "user action for $remove(&object &attr) or $remove(&object &rank)"
   }
  ]
 },
 "comps": null
}
','','_8DT8VbN0FaiS0K6uf','!','.');
INSERT INTO t_objects VALUES('_8t42m75faVNE2kWxu',1474473117,'
{
 "@name": "intptr_t",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "the ctype of signed integers \"equivalent\" (castable) to pointers"
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

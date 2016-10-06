-- _momstate.sql dump 2016 Oct 06 from _momstate.sqlite dumped by ./monimelt-dump-state.sh .....

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
INSERT INTO t_names VALUES('assignment_instruction_class','_4Yd3gqjKLi1hAKztj');
INSERT INTO t_names VALUES('basic_ctype_class','_7zd6NdDtMbPp0s5HT');
INSERT INTO t_names VALUES('block','_8fs1NMLlhPjN6eHN5');
INSERT INTO t_names VALUES('body','_49p2zcDH6JgIxjue1');
INSERT INTO t_names VALUES('bool','_3K62tYRL1azRF0cP2');
INSERT INTO t_names VALUES('c_aggregate_initialization','_40K767SbwcfEJVyXr');
INSERT INTO t_names VALUES('c_block_class','_1lU1wXlGlR9TdmE89');
INSERT INTO t_names VALUES('c_code','_64d2GXNLEBKFBzxqP');
INSERT INTO t_names VALUES('c_field_class','_6Lu6DXoGivCCHMwzV');
INSERT INTO t_names VALUES('c_include','_0hA5FCeEWGc6gpfnU');
INSERT INTO t_names VALUES('c_inlined_class','_0XX587h4W56J6nk3U');
INSERT INTO t_names VALUES('c_role_class','_7bJ4E6Mui5Ls7Tkx5');
INSERT INTO t_names VALUES('c_routine_class','_3pK47DLRw3N5Ghv0R');
INSERT INTO t_names VALUES('c_type','_8TY6UsXJ3Djt71By3');
INSERT INTO t_names VALUES('c_variable_class','_3hq90zS3jpqaVDbDy');
INSERT INTO t_names VALUES('call_instruction_class','_3nV79jeVZkZ8kdSWp');
INSERT INTO t_names VALUES('case_instruction_class','_72P3hApuaup7FsP08');
INSERT INTO t_names VALUES('cast_expression_class','_6lF04Imjl60Y6iYYc');
INSERT INTO t_names VALUES('cemit_module','_6bK8gGYmKZXwwNXj4');
INSERT INTO t_names VALUES('cemit_module_useract','_21c6aoW2jM5uS1jrN');
INSERT INTO t_names VALUES('char','_2Sj1dLJSbvyN3kMgI');
INSERT INTO t_names VALUES('chunk_expression_class','_5WG7GBNdjmzhANitX');
INSERT INTO t_names VALUES('chunk_instruction_class','_4Fo3f0PrwI6hwjSzM');
INSERT INTO t_names VALUES('class','_7al47yMPGToSh0ryC');
INSERT INTO t_names VALUES('class_class','_84n6z6xA40eduUUjh');
INSERT INTO t_names VALUES('class_useract','_7831xB7d1ulmsaCYS');
INSERT INTO t_names VALUES('code','_1DA4KKiwji2gafLj0');
INSERT INTO t_names VALUES('comment','_4xS1CSbRUFBW6PJiJ');
INSERT INTO t_names VALUES('conditional_instruction_class','_5ln7PDl9eFYwMPwx0');
INSERT INTO t_names VALUES('data','_6Dq8qo0MSqgbL0ESc');
INSERT INTO t_names VALUES('double','_8BM91hTnefUPcYXzf');
INSERT INTO t_names VALUES('enum_ctype_class','_1xh0H6A2nNw5upcPh');
INSERT INTO t_names VALUES('enumerator_class','_0dK6lxuDBgrY7Zdrb');
INSERT INTO t_names VALUES('enumerators','_4Cz6GxTDLCsTy7bco');
INSERT INTO t_names VALUES('expression','_64h4VCbZPNyxt22DN');
INSERT INTO t_names VALUES('extend','_15c0LcIrdf2LE5JP8');
INSERT INTO t_names VALUES('extern','_5dn8uwCs0vUJ9AG7y');
INSERT INTO t_names VALUES('fields','_7Pl7foCRjKoT6Gkdr');
INSERT INTO t_names VALUES('file_path','_6F98Bo2ChKhNVEZgt');
INSERT INTO t_names VALUES('fnmatch','_5Ta34TyJJebRyD9sN');
INSERT INTO t_names VALUES('fnmatch_useract','_59g7wsJyXlj962jlx');
INSERT INTO t_names VALUES('formals','_8Cl7x1LpaH2GzkdbW');
INSERT INTO t_names VALUES('formals_ctypes','_8Bq9eNWZ0eUS6BGq8');
INSERT INTO t_names VALUES('get','_1Xe4mcjLDBpvaBiDU');
INSERT INTO t_names VALUES('get_useract','_6VK4TFfclh68txISx');
INSERT INTO t_names VALUES('global_c_data_class','_5fr2Xe93UTwNlLIIo');
INSERT INTO t_names VALUES('in','_3St1x2rkjnH28CyRy');
INSERT INTO t_names VALUES('include_meltmoni','_0mn1b5GSeM3pB0Bfw');
INSERT INTO t_names VALUES('instruction','_0lI8AP6Gb2vPAtY1d');
INSERT INTO t_names VALUES('int','_0Sp1Lg7ctajS7oX5i');
INSERT INTO t_names VALUES('int16_t','_1Hx4BMvqv69AA0750');
INSERT INTO t_names VALUES('int32_t','_7EH4RTtyF1BioGEDK');
INSERT INTO t_names VALUES('int64_t','_21x8NL3oau6heNEnt');
INSERT INTO t_names VALUES('int8_t','_62H0GDHPCtIRvAdDd');
INSERT INTO t_names VALUES('intptr_t','_8t42m75faVNE2kWxu');
INSERT INTO t_names VALUES('jump_instruction_class','_6dw6NAxxyyrUS1716');
INSERT INTO t_names VALUES('locals','_5t61z2vkWfR1NbtiF');
INSERT INTO t_names VALUES('long','_20B54zdX0j8vYUdsf');
INSERT INTO t_names VALUES('macro','_5pl24omBRy4z1HrtN');
INSERT INTO t_names VALUES('macro_block_class','_6VV8wX6A6EdY0C70Z');
INSERT INTO t_names VALUES('macro_expression_class','_7C68VV2V6VKuvWqUW');
INSERT INTO t_names VALUES('member_access_class','_5WP5sMWxwATJgf7UY');
INSERT INTO t_names VALUES('module_class','_22z6Be6oa3IH524Jy');
INSERT INTO t_names VALUES('notice','_4H01ICaNuWncMePhe');
INSERT INTO t_names VALUES('null_ctype','_77D6zusmyq0jAM0Du');
INSERT INTO t_names VALUES('object','_7R18oBKd7iMYZd7gN');
INSERT INTO t_names VALUES('object_ctype','_3S16HFayBKLRo6Ivb');
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
INSERT INTO t_names VALUES('put_attr_cemitact','_34X8LzaKgjC0yxZXC');
INSERT INTO t_names VALUES('r_test1','_6gj4ZHinqqqvdWKyg');
INSERT INTO t_names VALUES('reference','_52u5dMMjzLjdlgIMF');
INSERT INTO t_names VALUES('remove','_4Ph1Z99dptyJhpZfj');
INSERT INTO t_names VALUES('remove_useract','_8j61MrY76IlS6t10r');
INSERT INTO t_names VALUES('result','_7he4MBcSuccJxtj7q');
INSERT INTO t_names VALUES('result_ctype','_7vy5VxtVon8EhIF5D');
INSERT INTO t_names VALUES('set','_7h20HFhtjZljRLtbU');
INSERT INTO t_names VALUES('set_useract','_6Z90lCcFC3RhKoHGv');
INSERT INTO t_names VALUES('signature','_1ZY6i2rKse5FsKrsv');
INSERT INTO t_names VALUES('signature_class','_6S30JEAmywph5MZqd');
INSERT INTO t_names VALUES('signature_int_to_double','_5fb7a3nqXwUKGKblr');
INSERT INTO t_names VALUES('signature_int_to_int','_0Ze1dDI740vldabtI');
INSERT INTO t_names VALUES('signature_object_to_value','_8DT8VbN0FaiS0K6uf');
INSERT INTO t_names VALUES('signature_two_objects_to_object','_4pb5Vror0ehNFKXfT');
INSERT INTO t_names VALUES('signature_two_objects_to_value','_1ky69RT7bZKd1tySD');
INSERT INTO t_names VALUES('signature_two_objects_to_void','_0dV2PYaSBUDE7lsYq');
INSERT INTO t_names VALUES('string','_2k61pgEBsCRJlTpHG');
INSERT INTO t_names VALUES('struct_ctype_class','_3CM77K3hIhblwvxg3');
INSERT INTO t_names VALUES('struct_pointer_ctype_class','_53F6MjNKRJ8dutl09');
INSERT INTO t_names VALUES('test1_idint','_32t79xvXNDPCYP09j');
INSERT INTO t_names VALUES('test1_module','_4NE8paRtDiMVlhHXX');
INSERT INTO t_names VALUES('test_anonobj','_3J97bCYloYmmXuCR1');
INSERT INTO t_names VALUES('the_GUI','_6Pc0DV2SCkMSiNlcf');
INSERT INTO t_names VALUES('the_system','_0BV96V94PJIn9si1K');
INSERT INTO t_names VALUES('threadlocal_c_data_class','_0Zs6lGxUXXurCpaYD');
INSERT INTO t_names VALUES('tuple','_0Ki51w13UIhXkBrtx');
INSERT INTO t_names VALUES('tuple_useract','_80D4pPU1WGn7IZwh6');
INSERT INTO t_names VALUES('uint16_t','_8T17wDyPfJSmGz5Ve');
INSERT INTO t_names VALUES('uint32_t','_6y88mWwevVhETpkig');
INSERT INTO t_names VALUES('uint64_t','_6r06aqWaPdfZgAKBP');
INSERT INTO t_names VALUES('uint8_t','_3WR20H7JRlNWkDV4y');
INSERT INTO t_names VALUES('uintptr_t','_21J3zZDnsKTbAVBFo');
INSERT INTO t_names VALUES('union_ctype_class','_3P48jR3hSzSCgv48c');
INSERT INTO t_names VALUES('user_actions','_5Kq0Pz5eFAULvdMEL');
INSERT INTO t_names VALUES('value','_4Cm8ln5mSkcZHI6WB');
INSERT INTO t_names VALUES('value_ctype','_7mW2hPaN6NfV95VUY');
INSERT INTO t_names VALUES('verbatim','_1rV3tPbwX88LqztK0');
INSERT INTO t_names VALUES('verbatim_expression_class','_16y1NXTBqDPWHLfoI');
INSERT INTO t_names VALUES('void','_1WX2mf4xwC5TU5ml7');
INSERT INTO t_names VALUES('x_test1','_4an3PsIjc3EDLchlg');
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
     "_34X8LzaKgjC0yxZXC",
     "_3cH0f65UzqnPaGsLY",
     "_3K62tYRL1azRF0cP2",
     "_3S16HFayBKLRo6Ivb",
     "_3WR20H7JRlNWkDV4y",
     "_40K767SbwcfEJVyXr",
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
     "_7mW2hPaN6NfV95VUY",
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
INSERT INTO t_objects VALUES('_0XX587h4W56J6nk3U',1474973691,'
{
 "@name": "c_inlined_class",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "class for inlined C routines"
   }
  ]
 },
 "comps": null
}
','','','','');
INSERT INTO t_objects VALUES('_0Ze1dDI740vldabtI',1475728002,'
{
 "@name": "signature_int_to_int",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "signature: int -> int"
   },
   {
    "at": "_7vy5VxtVon8EhIF5D",
    "va": {
     "oid": "_0Sp1Lg7ctajS7oX5i"
    }
   },
   {
    "at": "_8Bq9eNWZ0eUS6BGq8",
    "va": {
     "tup": [
      "_0Sp1Lg7ctajS7oX5i"
     ]
    }
   }
  ]
 },
 "comps": null
}
','_6S30JEAmywph5MZqd','','','');
INSERT INTO t_objects VALUES('_0Zs6lGxUXXurCpaYD',1474973691,'
{
 "@name": "threadlocal_c_data_class",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "class for threadlocal C data"
   }
  ]
 },
 "comps": null
}
','','','','');
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
INSERT INTO t_objects VALUES('_0dV2PYaSBUDE7lsYq',1475036015,'
{
 "@name": "signature_two_objects_to_void",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "signature: object -> object -> void"
   }
  ]
 },
 "comps": null
}
','_6S30JEAmywph5MZqd','','','');
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
INSERT INTO t_objects VALUES('_0lI8AP6Gb2vPAtY1d',1475588023,'
{
 "@name": "instruction",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "give the set of instructions in chunks"
   }
  ]
 },
 "comps": null
}
','','','','');
INSERT INTO t_objects VALUES('_0mn1b5GSeM3pB0Bfw',1475065467,'
{
 "@name": "include_meltmoni",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "include \"meltmoni.h\""
   },
   {
    "at": "_6F98Bo2ChKhNVEZgt",
    "va": "meltmoni.h"
   }
  ]
 },
 "comps": null
}
','','','','');
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
INSERT INTO t_objects VALUES('_16y1NXTBqDPWHLfoI',1475567881,'
{
 "@name": "verbatim_expression_class",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "class of verbatim expressions"
   }
  ]
 },
 "comps": null
}
','','','','');
INSERT INTO t_objects VALUES('_1DA4KKiwji2gafLj0',1474962483,'
{
 "@name": "code",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "give code, e.g. in module"
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
INSERT INTO t_objects VALUES('_1ZY6i2rKse5FsKrsv',1474975183,'
{
 "@name": "signature",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "give signature of e.g. functions"
   }
  ]
 },
 "comps": null
}
','','','','');
INSERT INTO t_objects VALUES('_1ky69RT7bZKd1tySD',1475589506,'
{
 "@name": "signature_two_objects_to_value",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "signature: object object -> value"
   }
  ]
 },
 "comps": null
}
','','','','');
INSERT INTO t_objects VALUES('_1lU1wXlGlR9TdmE89',1475154030,'
{
 "@name": "c_block_class",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "the class of C blocks"
   }
  ]
 },
 "comps": null
}
','','','','');
INSERT INTO t_objects VALUES('_1rV3tPbwX88LqztK0',1475567881,'
{
 "@name": "verbatim",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "attribute giving the verbatim object inside instances of verbatim_expression_class"
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
INSERT INTO t_objects VALUES('_32t79xvXNDPCYP09j',1475728983,'
{
 "@name": "test1_idint",
 "attrs": {
  "assoval": [
   {
    "at": "_1DA4KKiwji2gafLj0",
    "va": {
     "oid": "_5yC9bVC2dye6hgmVq"
    }
   },
   {
    "at": "_1ZY6i2rKse5FsKrsv",
    "va": {
     "oid": "_0Ze1dDI740vldabtI"
    }
   },
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "in test1 module identity function on ints"
   },
   {
    "at": "_7he4MBcSuccJxtj7q",
    "va": {
     "oid": "_6gj4ZHinqqqvdWKyg"
    }
   },
   {
    "at": "_8Cl7x1LpaH2GzkdbW",
    "va": {
     "tup": [
      "_4an3PsIjc3EDLchlg"
     ]
    }
   }
  ]
 },
 "comps": null
}
','_0XX587h4W56J6nk3U','','','');
INSERT INTO t_objects VALUES('_34X8LzaKgjC0yxZXC',1475046592,'
{
 "@name": "put_attr_cemitact",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": {
     "ssize": 110,
     "string": [
      "action for C code emitter: (&put_attr",
      "_cemitact &Obj &Attr &Val) to delay put ",
      "an attribute at end of c-emission"
     ]
    }
   }
  ]
 },
 "comps": null
}
','','_0dV2PYaSBUDE7lsYq','!','.');
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
INSERT INTO t_objects VALUES('_3S16HFayBKLRo6Ivb',1475649018,'
{
 "@name": "object_ctype",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "the ctype of objects"
   }
  ]
 },
 "comps": null
}
','','','','');
INSERT INTO t_objects VALUES('_3St1x2rkjnH28CyRy',1475079734,'
{
 "@name": "in",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "to give some container, etc...."
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
INSERT INTO t_objects VALUES('_3hq90zS3jpqaVDbDy',1475135979,'
{
 "@name": "c_variable_class",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "class of c variables"
   }
  ]
 },
 "comps": null
}
','','','','');
INSERT INTO t_objects VALUES('_3nV79jeVZkZ8kdSWp',1475302768,'
{
 "@name": "call_instruction_class",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "class for call instructions"
   }
  ]
 },
 "comps": null
}
','','','','');
INSERT INTO t_objects VALUES('_3pK47DLRw3N5Ghv0R',1474973691,'
{
 "@name": "c_routine_class",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "class for global C routines"
   }
  ]
 },
 "comps": null
}
','','','','');
INSERT INTO t_objects VALUES('_40K767SbwcfEJVyXr',1474983635,'
{
 "@name": "c_aggregate_initialization",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "give the aggregate initialization in a non-scalar c-type (e.g. pthread_mutex_t)"
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
INSERT INTO t_objects VALUES('_49p2zcDH6JgIxjue1',1475154030,'
{
 "@name": "body",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "gives the body block inside a function"
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
INSERT INTO t_objects VALUES('_4Fo3f0PrwI6hwjSzM',1475274437,'
{
 "@name": "chunk_instruction_class",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "class for C code chunk instructions"
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
INSERT INTO t_objects VALUES('_4NE8paRtDiMVlhHXX',1475730877,'
{
 "@name": "test1_module",
 "attrs": {
  "assoval": [
   {
    "at": "_0hA5FCeEWGc6gpfnU",
    "va": {
     "tup": [
      "_0mn1b5GSeM3pB0Bfw"
     ]
    }
   },
   {
    "at": "_1DA4KKiwji2gafLj0",
    "va": {
     "tup": [
      "_32t79xvXNDPCYP09j"
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
    "at": "_7h20HFhtjZljRLtbU",
    "va": {
     "set": [
      "_0BV96V94PJIn9si1K",
      "_0Ki51w13UIhXkBrtx",
      "_0Sp1Lg7ctajS7oX5i",
      "_4H01ICaNuWncMePhe",
      "_4NE8paRtDiMVlhHXX"
     ]
    }
   },
   {
    "at": "_8TY6UsXJ3Djt71By3",
    "va": {
     "tup": [
      "_0Ze1dDI740vldabtI"
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
INSERT INTO t_objects VALUES('_4Yd3gqjKLi1hAKztj',1475302768,'
{
 "@name": "assignment_instruction_class",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "class for assignment instruction"
   }
  ]
 },
 "comps": null
}
','','','','');
INSERT INTO t_objects VALUES('_4an3PsIjc3EDLchlg',1475728399,'
{
 "@name": "x_test1",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "variable x in test1"
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
','_3hq90zS3jpqaVDbDy','','','');
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
INSERT INTO t_objects VALUES('_4pb5Vror0ehNFKXfT',1475335399,'
{
 "@name": "signature_two_objects_to_object",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "signature: object object -> object"
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
INSERT INTO t_objects VALUES('_52u5dMMjzLjdlgIMF',1475582058,'
{
 "@name": "reference",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "give the reference ctype in chunks"
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
INSERT INTO t_objects VALUES('_5WG7GBNdjmzhANitX',1475581021,'
{
 "@name": "chunk_expression_class",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "class of chunk expressions or reference"
   }
  ]
 },
 "comps": null
}
','','','','');
INSERT INTO t_objects VALUES('_5WP5sMWxwATJgf7UY',1475573036,'
{
 "@name": "member_access_class",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "class of member access"
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
INSERT INTO t_objects VALUES('_5dn8uwCs0vUJ9AG7y',1474962483,'
{
 "@name": "extern",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "give external resources, e.g. in modules"
   }
  ]
 },
 "comps": null
}
','','','','');
INSERT INTO t_objects VALUES('_5fb7a3nqXwUKGKblr',1474909861,'
{
 "@name": "signature_int_to_double",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "signature of int->double functions"
   },
   {
    "at": "_7vy5VxtVon8EhIF5D",
    "va": {
     "oid": "_8BM91hTnefUPcYXzf"
    }
   },
   {
    "at": "_8Bq9eNWZ0eUS6BGq8",
    "va": {
     "tup": [
      "_0Sp1Lg7ctajS7oX5i"
     ]
    }
   }
  ]
 },
 "comps": null
}
','_6S30JEAmywph5MZqd','','','');
INSERT INTO t_objects VALUES('_5fr2Xe93UTwNlLIIo',1474973691,'
{
 "@name": "global_c_data_class",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "class for global C data"
   }
  ]
 },
 "comps": null
}
','','','','');
INSERT INTO t_objects VALUES('_5ln7PDl9eFYwMPwx0',1475274437,'
{
 "@name": "conditional_instruction_class",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "class for conditional instructions"
   }
  ]
 },
 "comps": null
}
','','','','');
INSERT INTO t_objects VALUES('_5pl24omBRy4z1HrtN',1475336840,'
{
 "@name": "macro",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "give the macro, e.g. in a macro_block_class"
   }
  ]
 },
 "comps": null
}
','','','','');
INSERT INTO t_objects VALUES('_5qn90tdCdn7nHFNJe',1475729238,'
{
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "assign r_test1 := x_test1"
   }
  ]
 },
 "comps": null
}
','_4Yd3gqjKLi1hAKztj','','','');
INSERT INTO t_objects VALUES('_5t61z2vkWfR1NbtiF',1475176212,'
{
 "@name": "locals",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "local variables in block"
   }
  ]
 },
 "comps": null
}
','','','','');
INSERT INTO t_objects VALUES('_5yC9bVC2dye6hgmVq',1475728983,'
{
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "body of test1_idint\n"
   }
  ]
 },
 "comps": {
  "vectval": [
   {
    "oid": "_5qn90tdCdn7nHFNJe"
   }
  ]
 }
}
','_1lU1wXlGlR9TdmE89','','','');
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
INSERT INTO t_objects VALUES('_64h4VCbZPNyxt22DN',1475588023,'
{
 "@name": "expression",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "give the set of expressions in chunks"
   }
  ]
 },
 "comps": null
}
','','','','');
INSERT INTO t_objects VALUES('_6Dq8qo0MSqgbL0ESc',1474962483,'
{
 "@name": "data",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "give data, e.g. in module"
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
INSERT INTO t_objects VALUES('_6VV8wX6A6EdY0C70Z',1475274437,'
{
 "@name": "macro_block_class",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "class for macro blocks"
   }
  ]
 },
 "comps": null
}
','','','','');
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
INSERT INTO t_objects VALUES('_6dw6NAxxyyrUS1716',1475302768,'
{
 "@name": "jump_instruction_class",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "class for jump (to block) instructions"
   }
  ]
 },
 "comps": null
}
','','','','');
INSERT INTO t_objects VALUES('_6gj4ZHinqqqvdWKyg',1475728433,'
{
 "@name": "r_test1",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "variable r in test1"
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
','_3hq90zS3jpqaVDbDy','','','');
INSERT INTO t_objects VALUES('_6lF04Imjl60Y6iYYc',1475655272,'
{
 "@name": "cast_expression_class",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "class for cast expressions, with a c_type attribute"
   }
  ]
 },
 "comps": null
}
','','','','');
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
INSERT INTO t_objects VALUES('_72P3hApuaup7FsP08',1475302768,'
{
 "@name": "case_instruction_class",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "class for case instructions"
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
INSERT INTO t_objects VALUES('_77D6zusmyq0jAM0Du',1475408154,'
{
 "@name": "null_ctype",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "ctype of nil"
   }
  ]
 },
 "comps": null
}
','_7zd6NdDtMbPp0s5HT','','','');
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
INSERT INTO t_objects VALUES('_7C68VV2V6VKuvWqUW',1475573036,'
{
 "@name": "macro_expression_class",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "class of macro expr"
   }
  ]
 },
 "comps": null
}
','','','','');
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
INSERT INTO t_objects VALUES('_7R18oBKd7iMYZd7gN',1475478556,'
{
 "@name": "object",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "related to objects, e.g. for their c-type"
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
INSERT INTO t_objects VALUES('_7bJ4E6Mui5Ls7Tkx5',1475136142,'
{
 "@name": "c_role_class",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "class of c roles"
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
INSERT INTO t_objects VALUES('_7he4MBcSuccJxtj7q',1475133644,'
{
 "@name": "result",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "the result variable inside a C function"
   }
  ]
 },
 "comps": null
}
','','','','');
INSERT INTO t_objects VALUES('_7mW2hPaN6NfV95VUY',1475649221,'
{
 "@name": "value_ctype",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "the ctype of values, can be a tagged pointer..."
   }
  ]
 },
 "comps": null
}
','_7zd6NdDtMbPp0s5HT','','','');
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
INSERT INTO t_objects VALUES('_8Cl7x1LpaH2GzkdbW',1475133644,'
{
 "@name": "formals",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "the tuple of formals inside a C function"
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
INSERT INTO t_objects VALUES('_8fs1NMLlhPjN6eHN5',1475588023,'
{
 "@name": "block",
 "attrs": {
  "assoval": [
   {
    "at": "_4xS1CSbRUFBW6PJiJ",
    "va": "give the set of blocks in chunks"
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

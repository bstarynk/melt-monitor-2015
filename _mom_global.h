/// *** generated file _mom_global.h - DO NOT EDIT 
/// Copyright (C) 2015 - 2016 Free Software Foundation, Inc. 
/// MONIMELT is a monitor for MELT - see http://gcc-melt.org/ 
/// This generated file _mom_global.h is part of MONIMELT, part of GCC 
///
/// GCC is free software; you can redistribute it and/or modify 
/// it under the terms of the GNU General Public License as published by 
/// the Free Software Foundation; either version 3, or (at your option) 
/// any later version. 
///
///  GCC is distributed in the hope that it will be useful, 
///  but WITHOUT ANY WARRANTY; without even the implied warranty of 
///  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
///  GNU General Public License for more details. 
///  You should have received a copy of the GNU General Public License 
///  along with GCC; see the file COPYING3.   If not see 
///  <http://www.gnu.org/licenses/>. 
///

#ifndef MOM_HAS_GLOBAL
#error missing MOM_HAS_GLOBAL
#endif


#undef MOM_NB_GLOBAL
#define MOM_NB_GLOBAL 38


//// MOM_HAS_GLOBAL(Name,Idstr,Hid,Loid,Hash)

//+ $add_user_action (Action Action_useract) would add the user action $Action with 
MOM_HAS_GLOBAL(add_user_action,_3cH0f65UzqnPaGsLY,757794550,888791185045795966,2268284358)

//+ routine for add_user_action
MOM_HAS_GLOBAL(add_user_action_useract,_0xA91ltlCCIeN6K9H,132184458,3965990523290568871,2505338322)

//+ class for array ctypes, the first component#0 is the element ctype; the second c
MOM_HAS_GLOBAL(array_ctype_class,_4iH7xI2JtAomiBySj,1017368870,4801852483639973667,2717929419)

//+ the boolean ctype
MOM_HAS_GLOBAL(bool,_3K62tYRL1azRF0cP2,889079074,5625971388704506234,1367612090)

//+ give the aggregate initialization in a non-scalar c-type (e.g. pthread_mutex_t)
MOM_HAS_GLOBAL(c_aggregate_initialization,_40K767SbwcfEJVyXr,946783341,266003986256195287,887013504)

//+ operation to emit c code for a module: $cemit_module(TheModule)
MOM_HAS_GLOBAL(cemit_module,_6bK8gGYmKZXwwNXj4,1461904630,5629140260882290120,1714563547)

//+ user action for cemit_module
MOM_HAS_GLOBAL(cemit_module_useract,_21c6aoW2jM5uS1jrN,476621502,6878332404461070709,1509457310)

//+ the char ctype
MOM_HAS_GLOBAL(char,_2Sj1dLJSbvyN3kMgI,677585412,3745052711434301676,2350112261)

//+ user action for $class (object class)
MOM_HAS_GLOBAL(class_useract,_7831xB7d1ulmsaCYS,1683172135,6288468238231678380,3817197504)

//+ the double ctype
MOM_HAS_GLOBAL(double,_8BM91hTnefUPcYXzf,2036136323,2502319301662783315,3623603139)

//+ for $fnmatch("*ab*") to use fnmatch with FNM_EXTMATCH & FNM_CASEFOLD on names
MOM_HAS_GLOBAL(fnmatch,_5Ta34TyJJebRyD9sN,1388729658,731614781365540241,2389107487)

//+ for implementing $fnmatch command
MOM_HAS_GLOBAL(fnmatch_useract,_59g7wsJyXlj962jlx,1216139410,3556611507501676989,1836628331)

//+ to get some attribute or component, etc. also $get user action.
MOM_HAS_GLOBAL(get,_1Xe4mcjLDBpvaBiDU,461011165,3588994974126708538,3054508454)

//+ user action for $get
MOM_HAS_GLOBAL(get_useract,_6VK4TFfclh68txISx,1634895712,3524192943039125457,235853902)

//+ the int ctype
MOM_HAS_GLOBAL(int,_0Sp1Lg7ctajS7oX5i,206123378,1657221063490954190,2737754805)

//+ the ctype for 16 bits integers
MOM_HAS_GLOBAL(int16_t,_1Hx4BMvqv69AA0750,407207950,7980475163603109788,1383035942)

//+ the ctype for 32 bits integers
MOM_HAS_GLOBAL(int32_t,_7EH4RTtyF1BioGEDK,1811646092,4716021617753196882,2754588426)

//+ the ctype for 64 bits integers
MOM_HAS_GLOBAL(int64_t,_21x8NL3oau6heNEnt,478016564,8407009047818543553,249091822)

//+ the ctype for 8 bits signed integers
MOM_HAS_GLOBAL(int8_t,_62H0GDHPCtIRvAdDd,1426265002,5367769641288350209,2306438205)

//+ the ctype of signed integers "equivalent" (castable) to pointers
MOM_HAS_GLOBAL(intptr_t,_8t42m75faVNE2kWxu,2001748354,8327845228897843290,1787024167)

//+ the long ctype
MOM_HAS_GLOBAL(long,_20B54zdX0j8vYUdsf,474319974,7179427869252810223,314494134)

//+ the ctype of objects
MOM_HAS_GLOBAL(object_ctype,_3S16HFayBKLRo6Ivb,912374292,8044247218361316495,1864724234)

//+ class of pointer ctypes; the first component #0 is the pointed ctype.
MOM_HAS_GLOBAL(pointer_ctype_class,_7JP45UsR282DkTrJS,1831760218,601741714220354864,2152013395)

//+ action for C code emitter: (&put_attr_cemitact &Obj &Attr &Val) to delay put an 
MOM_HAS_GLOBAL(put_attr_cemitact,_34X8LzaKgjC0yxZXC,727315249,1647905027261026546,1798594494)

//+ to remove something, e.g. $remove(&object &attr) user action
MOM_HAS_GLOBAL(remove,_4Ph1Z99dptyJhpZfj,1141454572,7970905774666212359,747100231)

//+ user action for $remove(&object &attr) or $remove(&object &rank)
MOM_HAS_GLOBAL(remove_useract,_8j61MrY76IlS6t10r,1962553855,7308997705184094443,3623953716)

//+ for set related stuff
MOM_HAS_GLOBAL(set,_7h20HFhtjZljRLtbU,1718490147,6717538159123036634,3433581947)

//+ variadic user action to make a set from ingredients (objects or sequences)
MOM_HAS_GLOBAL(set_useract,_6Z90lCcFC3RhKoHGv,1648167417,4195768607671425703,4059762204)

//+ for string related stuff
MOM_HAS_GLOBAL(string,_2k61pgEBsCRJlTpHG,550905679,1952392727993788814,226985679)

//+ for tuple related stuff
MOM_HAS_GLOBAL(tuple,_0Ki51w13UIhXkBrtx,182094590,1030173250716890461,1953491597)

//+ user action to make a tuple out of ingredients
MOM_HAS_GLOBAL(tuple_useract,_80D4pPU1WGn7IZwh6,1890024075,1167320895490554690,4093394012)

//+ ctype for unsigned 16 bits integers
MOM_HAS_GLOBAL(uint16_t,_8T17wDyPfJSmGz5Ve,2095960231,8754725683271532674,3685407533)

//+ ctype for unsigned 32 bits integers
MOM_HAS_GLOBAL(uint32_t,_6y88mWwevVhETpkig,1549854677,77780675169495112,928133734)

//+ ctype for unsigned 64 bits integers
MOM_HAS_GLOBAL(uint64_t,_6r06aqWaPdfZgAKBP,1521789634,5736498198274362046,3596712632)

//+ ctype of unsigned 8 bits integers
MOM_HAS_GLOBAL(uint8_t,_3WR20H7JRlNWkDV4y,931346341,4733086605554652354,26685428)

//+ ctype of unsigned int of same size as pointers
MOM_HAS_GLOBAL(uintptr_t,_21J3zZDnsKTbAVBFo,478765968,1910349516108587076,2786648670)

//+ the ctype of values, can be a tagged pointer...
MOM_HAS_GLOBAL(value_ctype,_7mW2hPaN6NfV95VUY,1741701001,3414523499108887058,2787277674)

//+ the void ctype
MOM_HAS_GLOBAL(void,_1WX2mf4xwC5TU5ml7,459882898,2997967141540374291,478990535)



#undef moid_add_user_action
#define moid_add_user_action _3cH0f65UzqnPaGsLY
#undef monam_3cH0f65UzqnPaGsLY
#define monam_3cH0f65UzqnPaGsLY add_user_action

#undef moid_add_user_action_useract
#define moid_add_user_action_useract _0xA91ltlCCIeN6K9H
#undef monam_0xA91ltlCCIeN6K9H
#define monam_0xA91ltlCCIeN6K9H add_user_action_useract

#undef moid_array_ctype_class
#define moid_array_ctype_class _4iH7xI2JtAomiBySj
#undef monam_4iH7xI2JtAomiBySj
#define monam_4iH7xI2JtAomiBySj array_ctype_class

#undef moid_bool
#define moid_bool _3K62tYRL1azRF0cP2
#undef monam_3K62tYRL1azRF0cP2
#define monam_3K62tYRL1azRF0cP2 bool

#undef moid_c_aggregate_initialization
#define moid_c_aggregate_initialization _40K767SbwcfEJVyXr
#undef monam_40K767SbwcfEJVyXr
#define monam_40K767SbwcfEJVyXr c_aggregate_initialization

#undef moid_cemit_module
#define moid_cemit_module _6bK8gGYmKZXwwNXj4
#undef monam_6bK8gGYmKZXwwNXj4
#define monam_6bK8gGYmKZXwwNXj4 cemit_module

#undef moid_cemit_module_useract
#define moid_cemit_module_useract _21c6aoW2jM5uS1jrN
#undef monam_21c6aoW2jM5uS1jrN
#define monam_21c6aoW2jM5uS1jrN cemit_module_useract

#undef moid_char
#define moid_char _2Sj1dLJSbvyN3kMgI
#undef monam_2Sj1dLJSbvyN3kMgI
#define monam_2Sj1dLJSbvyN3kMgI char

#undef moid_class_useract
#define moid_class_useract _7831xB7d1ulmsaCYS
#undef monam_7831xB7d1ulmsaCYS
#define monam_7831xB7d1ulmsaCYS class_useract

#undef moid_double
#define moid_double _8BM91hTnefUPcYXzf
#undef monam_8BM91hTnefUPcYXzf
#define monam_8BM91hTnefUPcYXzf double

#undef moid_fnmatch
#define moid_fnmatch _5Ta34TyJJebRyD9sN
#undef monam_5Ta34TyJJebRyD9sN
#define monam_5Ta34TyJJebRyD9sN fnmatch

#undef moid_fnmatch_useract
#define moid_fnmatch_useract _59g7wsJyXlj962jlx
#undef monam_59g7wsJyXlj962jlx
#define monam_59g7wsJyXlj962jlx fnmatch_useract

#undef moid_get
#define moid_get _1Xe4mcjLDBpvaBiDU
#undef monam_1Xe4mcjLDBpvaBiDU
#define monam_1Xe4mcjLDBpvaBiDU get

#undef moid_get_useract
#define moid_get_useract _6VK4TFfclh68txISx
#undef monam_6VK4TFfclh68txISx
#define monam_6VK4TFfclh68txISx get_useract

#undef moid_int
#define moid_int _0Sp1Lg7ctajS7oX5i
#undef monam_0Sp1Lg7ctajS7oX5i
#define monam_0Sp1Lg7ctajS7oX5i int

#undef moid_int16_t
#define moid_int16_t _1Hx4BMvqv69AA0750
#undef monam_1Hx4BMvqv69AA0750
#define monam_1Hx4BMvqv69AA0750 int16_t

#undef moid_int32_t
#define moid_int32_t _7EH4RTtyF1BioGEDK
#undef monam_7EH4RTtyF1BioGEDK
#define monam_7EH4RTtyF1BioGEDK int32_t

#undef moid_int64_t
#define moid_int64_t _21x8NL3oau6heNEnt
#undef monam_21x8NL3oau6heNEnt
#define monam_21x8NL3oau6heNEnt int64_t

#undef moid_int8_t
#define moid_int8_t _62H0GDHPCtIRvAdDd
#undef monam_62H0GDHPCtIRvAdDd
#define monam_62H0GDHPCtIRvAdDd int8_t

#undef moid_intptr_t
#define moid_intptr_t _8t42m75faVNE2kWxu
#undef monam_8t42m75faVNE2kWxu
#define monam_8t42m75faVNE2kWxu intptr_t

#undef moid_long
#define moid_long _20B54zdX0j8vYUdsf
#undef monam_20B54zdX0j8vYUdsf
#define monam_20B54zdX0j8vYUdsf long

#undef moid_object_ctype
#define moid_object_ctype _3S16HFayBKLRo6Ivb
#undef monam_3S16HFayBKLRo6Ivb
#define monam_3S16HFayBKLRo6Ivb object_ctype

#undef moid_pointer_ctype_class
#define moid_pointer_ctype_class _7JP45UsR282DkTrJS
#undef monam_7JP45UsR282DkTrJS
#define monam_7JP45UsR282DkTrJS pointer_ctype_class

#undef moid_put_attr_cemitact
#define moid_put_attr_cemitact _34X8LzaKgjC0yxZXC
#undef monam_34X8LzaKgjC0yxZXC
#define monam_34X8LzaKgjC0yxZXC put_attr_cemitact

#undef moid_remove
#define moid_remove _4Ph1Z99dptyJhpZfj
#undef monam_4Ph1Z99dptyJhpZfj
#define monam_4Ph1Z99dptyJhpZfj remove

#undef moid_remove_useract
#define moid_remove_useract _8j61MrY76IlS6t10r
#undef monam_8j61MrY76IlS6t10r
#define monam_8j61MrY76IlS6t10r remove_useract

#undef moid_set
#define moid_set _7h20HFhtjZljRLtbU
#undef monam_7h20HFhtjZljRLtbU
#define monam_7h20HFhtjZljRLtbU set

#undef moid_set_useract
#define moid_set_useract _6Z90lCcFC3RhKoHGv
#undef monam_6Z90lCcFC3RhKoHGv
#define monam_6Z90lCcFC3RhKoHGv set_useract

#undef moid_string
#define moid_string _2k61pgEBsCRJlTpHG
#undef monam_2k61pgEBsCRJlTpHG
#define monam_2k61pgEBsCRJlTpHG string

#undef moid_tuple
#define moid_tuple _0Ki51w13UIhXkBrtx
#undef monam_0Ki51w13UIhXkBrtx
#define monam_0Ki51w13UIhXkBrtx tuple

#undef moid_tuple_useract
#define moid_tuple_useract _80D4pPU1WGn7IZwh6
#undef monam_80D4pPU1WGn7IZwh6
#define monam_80D4pPU1WGn7IZwh6 tuple_useract

#undef moid_uint16_t
#define moid_uint16_t _8T17wDyPfJSmGz5Ve
#undef monam_8T17wDyPfJSmGz5Ve
#define monam_8T17wDyPfJSmGz5Ve uint16_t

#undef moid_uint32_t
#define moid_uint32_t _6y88mWwevVhETpkig
#undef monam_6y88mWwevVhETpkig
#define monam_6y88mWwevVhETpkig uint32_t

#undef moid_uint64_t
#define moid_uint64_t _6r06aqWaPdfZgAKBP
#undef monam_6r06aqWaPdfZgAKBP
#define monam_6r06aqWaPdfZgAKBP uint64_t

#undef moid_uint8_t
#define moid_uint8_t _3WR20H7JRlNWkDV4y
#undef monam_3WR20H7JRlNWkDV4y
#define monam_3WR20H7JRlNWkDV4y uint8_t

#undef moid_uintptr_t
#define moid_uintptr_t _21J3zZDnsKTbAVBFo
#undef monam_21J3zZDnsKTbAVBFo
#define monam_21J3zZDnsKTbAVBFo uintptr_t

#undef moid_value_ctype
#define moid_value_ctype _7mW2hPaN6NfV95VUY
#undef monam_7mW2hPaN6NfV95VUY
#define monam_7mW2hPaN6NfV95VUY value_ctype

#undef moid_void
#define moid_void _1WX2mf4xwC5TU5ml7
#undef monam_1WX2mf4xwC5TU5ml7
#define monam_1WX2mf4xwC5TU5ml7 void


#ifndef MOM_GLOBAL_HASHES
#define MOM_GLOBAL_HASHES 1
enum mom_global_hashes_en {
  momghash_add_user_action=2268284358,
  momghash_add_user_action_useract=2505338322,
  momghash_array_ctype_class=2717929419,
  momghash_bool=1367612090,
  momghash_c_aggregate_initialization=887013504,
  momghash_cemit_module=1714563547,
  momghash_cemit_module_useract=1509457310,
  momghash_char=2350112261,
  momghash_class_useract=3817197504,
  momghash_double=3623603139,
  momghash_fnmatch=2389107487,
  momghash_fnmatch_useract=1836628331,
  momghash_get=3054508454,
  momghash_get_useract=235853902,
  momghash_int=2737754805,
  momghash_int16_t=1383035942,
  momghash_int32_t=2754588426,
  momghash_int64_t=249091822,
  momghash_int8_t=2306438205,
  momghash_intptr_t=1787024167,
  momghash_long=314494134,
  momghash_object_ctype=1864724234,
  momghash_pointer_ctype_class=2152013395,
  momghash_put_attr_cemitact=1798594494,
  momghash_remove=747100231,
  momghash_remove_useract=3623953716,
  momghash_set=3433581947,
  momghash_set_useract=4059762204,
  momghash_string=226985679,
  momghash_tuple=1953491597,
  momghash_tuple_useract=4093394012,
  momghash_uint16_t=3685407533,
  momghash_uint32_t=928133734,
  momghash_uint64_t=3596712632,
  momghash_uint8_t=26685428,
  momghash_uintptr_t=2786648670,
  momghash_value_ctype=2787277674,
  momghash_void=478990535,
}; // end mom_global_hashes_en
#endif /*MOM_GLOBAL_HASHES */



#undef MOM_NB_ANONYMOUS_GLOBAL
#define MOM_NB_ANONYMOUS_GLOBAL 0

#undef MOM_NB_NAMED_GLOBAL
#define MOM_NB_NAMED_GLOBAL 38


#undef MOM_HAS_GLOBAL
// end of generated global file _mom_global.h

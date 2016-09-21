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
#define MOM_NB_GLOBAL 12


//// MOM_HAS_GLOBAL(Name,Idstr,Hid,Loid,Hash)

//+ the boolean ctype
MOM_HAS_GLOBAL(bool,_3K62tYRL1azRF0cP2,889079074,5625971388704506234,1367612090)

//+ the char ctype
MOM_HAS_GLOBAL(char,_2Sj1dLJSbvyN3kMgI,677585412,3745052711434301676,2350112261)

//+ user action for $class (object class)
MOM_HAS_GLOBAL(class_useract,_7831xB7d1ulmsaCYS,1683172135,6288468238231678380,3817197504)

//+ the double ctype
MOM_HAS_GLOBAL(double,_8BM91hTnefUPcYXzf,2036136323,2502319301662783315,3623603139)

//+ the int ctype
MOM_HAS_GLOBAL(int,_0Sp1Lg7ctajS7oX5i,206123378,1657221063490954190,2737754805)

//+ the long ctype
MOM_HAS_GLOBAL(long,_20B54zdX0j8vYUdsf,474319974,7179427869252810223,314494134)

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

//+ the void ctype
MOM_HAS_GLOBAL(void,_1WX2mf4xwC5TU5ml7,459882898,2997967141540374291,478990535)



#undef moid_bool
#define moid_bool _3K62tYRL1azRF0cP2
#undef monam_3K62tYRL1azRF0cP2
#define monam_3K62tYRL1azRF0cP2 bool

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

#undef moid_int
#define moid_int _0Sp1Lg7ctajS7oX5i
#undef monam_0Sp1Lg7ctajS7oX5i
#define monam_0Sp1Lg7ctajS7oX5i int

#undef moid_long
#define moid_long _20B54zdX0j8vYUdsf
#undef monam_20B54zdX0j8vYUdsf
#define monam_20B54zdX0j8vYUdsf long

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

#undef moid_void
#define moid_void _1WX2mf4xwC5TU5ml7
#undef monam_1WX2mf4xwC5TU5ml7
#define monam_1WX2mf4xwC5TU5ml7 void


#ifndef MOM_GLOBAL_HASHES
#define MOM_GLOBAL_HASHES 1
enum mom_global_hashes_en {
  momghash_bool=1367612090,
  momghash_char=2350112261,
  momghash_class_useract=3817197504,
  momghash_double=3623603139,
  momghash_int=2737754805,
  momghash_long=314494134,
  momghash_set=3433581947,
  momghash_set_useract=4059762204,
  momghash_string=226985679,
  momghash_tuple=1953491597,
  momghash_tuple_useract=4093394012,
  momghash_void=478990535,
}; // end mom_global_hashes_en
#endif /*MOM_GLOBAL_HASHES */



#undef MOM_NB_ANONYMOUS_GLOBAL
#define MOM_NB_ANONYMOUS_GLOBAL 0

#undef MOM_NB_NAMED_GLOBAL
#define MOM_NB_NAMED_GLOBAL 12


#undef MOM_HAS_GLOBAL
// end of generated global file _mom_global.h

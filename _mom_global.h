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
#define MOM_NB_GLOBAL 2


//// MOM_HAS_GLOBAL(Name,Idstr,Hid,Loid,Hash)

//+ user action for $class (object class)
MOM_HAS_GLOBAL(class_useract,_7831xB7d1ulmsaCYS,1683172135,6288468238231678380,3817197504)

//+ the int ctype
MOM_HAS_GLOBAL(int,_0Sp1Lg7ctajS7oX5i,206123378,1657221063490954190,2737754805)



#undef moid_class_useract
#define moid_class_useract _7831xB7d1ulmsaCYS
#undef monam_7831xB7d1ulmsaCYS
#define monam_7831xB7d1ulmsaCYS class_useract

#undef moid_int
#define moid_int _0Sp1Lg7ctajS7oX5i
#undef monam_0Sp1Lg7ctajS7oX5i
#define monam_0Sp1Lg7ctajS7oX5i int


#ifndef MOM_GLOBAL_HASHES
#define MOM_GLOBAL_HASHES 1
enum mom_global_hashes_en {
  momghash_class_useract=3817197504,
  momghash_int=2737754805,
}; // end mom_global_hashes_en
#endif /*MOM_GLOBAL_HASHES */



#undef MOM_NB_ANONYMOUS_GLOBAL
#define MOM_NB_ANONYMOUS_GLOBAL 0

#undef MOM_NB_NAMED_GLOBAL
#define MOM_NB_NAMED_GLOBAL 2


#undef MOM_HAS_GLOBAL
// end of generated global file _mom_global.h

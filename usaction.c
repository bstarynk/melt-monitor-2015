// file usaction.c - user actions

/**   Copyright (C)  2016  Basile Starynkevitch and later the FSF
      MONIMELT is a monitor for MELT - see http://gcc-melt.org/
      This file is part of GCC.
  
      GCC is free software; you can redistribute it and/or modify
      it under the terms of the GNU General Public License as published by
      the Free Software Foundation; either version 3, or (at your option)
      any later version.

      GCC is distributed in the hope that it will be useful,
      but WITHOUT ANY WARRANTY; without even the implied warranty of
      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
      GNU General Public License for more details.
      You should have received a copy of the GNU General Public License
      along with GCC; see the file COPYING3.   If not see
      <http://www.gnu.org/licenses/>.
**/

#include "meltmoni.h"

/// the $class (object class) GUI user action is setting the
/// mo_ob_class of object to class
#define moid_class_useract _7831xB7d1ulmsaCYS
const char
MOM_PREFIXID (mosig_, class_useract)[] = "signature_object_to_value";

     extern mo_signature_object_to_value_sigt
       MOM_PREFIXID (mofun_, class_useract) __attribute__ ((optimize ("O2")));


     extern mo_signature_object_to_value_sigt mofun_class_useract;

mo_value_t
MOM_PREFIXID (mofun_, class_useract) (mo_objref_t obuact)
{
  return mofun_class_useract (obuact);
}

mo_value_t
mofun_class_useract (mo_objref_t obuact)
{
  MOM_ASSERTPRINTF (mo_dyncast_object (obuact), "class_useract: bad obuact");
  // the obuact starts with the user action, e.g. class
  enum
  {
    MOMIX_OPER,
    MOMIX_OBJECT,
    MOMIX_CLASS,
    MOMIX__LAST
  };
  mo_objref_t operobr =
    mo_dyncast_objref (mo_objref_get_comp (obuact, MOMIX_OPER));
  mo_objref_t obr =
    mo_dyncast_objref (mo_objref_get_comp (obuact, MOMIX_OBJECT));
  mo_objref_t classobr =
    mo_dyncast_objref (mo_objref_get_comp (obuact, MOMIX_CLASS));
  if (mo_objref_comp_count (obuact) != MOMIX__LAST)
    mom_gui_fail_user_action
      ("class_useract for $%s: wants two user arguments, got %d in %s",
       mo_objref_pnamestr(operobr),
       mo_objref_comp_count (obuact) - 1, mo_objref_pnamestr (obuact));
  // temporary, for debugging
  MOM_BACKTRACEPRINTF
    ("class_useract: obuact=%s operobr=%s obr=%s classobr=%s",
     mo_objref_pnamestr (obuact), mo_objref_pnamestr (operobr),
     mo_objref_pnamestr (obr), mo_objref_pnamestr (classobr));
  if (!obr)
    mom_gui_fail_user_action ("class_useract: no object given");
  if (!classobr)
    mom_gui_fail_user_action ("class_useract: no class given for object %s",
                              mo_objref_pnamestr (obr));
  if (obr->mo_ob_class != NULL)
    MOM_WARNPRINTF ("class_useract: object %s had old class %s, wanting %s",
                    mo_objref_pnamestr (obr),
                    mo_objref_pnamestr (obr->mo_ob_class),
                    mo_objref_pnamestr (classobr));
  if (obr->mo_ob_class == MOM_PREDEF (class_class)
      && classobr != MOM_PREDEF (class_class))
    mom_gui_fail_user_action
      ("class_useract: object %s is of class_class, cannot reclass as %s",
       mo_objref_pnamestr (obr), mo_objref_pnamestr (classobr));
  if (classobr->mo_ob_class != NULL
      && classobr->mo_ob_class != MOM_PREDEF (class_class))
    mom_gui_fail_user_action
      ("class_useract: object %s cannot be reclassed to %s which is not a class_class but a %s",
       mo_objref_pnamestr (obr), mo_objref_pnamestr (classobr),
       mo_objref_pnamestr (classobr->mo_ob_class));
  obr->mo_ob_class = classobr;
  mom_gui_cmdstatus_printf ("class_useract: class of %s becomes %s",
                            mo_objref_pnamestr (obr),
                            mo_objref_pnamestr (classobr));
  return obr;
}                               /* end of mofun_class_useract */

// end of file usaction.c

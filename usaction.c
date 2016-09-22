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


/// momglob_class_useract is needed
/// the $class (object class) GUI user action is setting the
/// mo_ob_class of object to class
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
       mo_objref_pnamestr (operobr),
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

// momglob_set  &  momglob_set_useract
const char
MOM_PREFIXID (mosig_, set_useract)[] = "signature_object_to_value";

     extern mo_signature_object_to_value_sigt
       MOM_PREFIXID (mofun_, set_useract) __attribute__ ((optimize ("O2")));


     extern mo_signature_object_to_value_sigt mofun_set_useract;

mo_value_t
MOM_PREFIXID (mofun_, set_useract) (mo_objref_t obuact)
{
  return mofun_set_useract (obuact);
}

mo_value_t
mofun_set_useract (mo_objref_t obuact)
{
  MOM_ASSERTPRINTF (mo_dyncast_object (obuact), "set_useract: bad obuact");
  unsigned nbargs = mo_objref_comp_count (obuact);
  mo_hashsetpayl_ty *hset = mo_hashset_reserve (NULL, 3 * nbargs / 2 + 20);
  for (unsigned ix = 1; ix < nbargs; ix++)
    {
      mo_value_t curargv = mo_objref_get_comp (obuact, ix);
      enum mo_valkind_en k = mo_kind_of_value (curargv);
      switch (k)
        {
        case mo_KNONE:
        case mo_KINT:
        case mo_KSTRING:
          break;
        case mo_KTUPLE:
        case mo_KSET:
          {
            const mo_sequencevalue_ty *seq =
              (const mo_sequencevalue_ty *) curargv;
            unsigned siz = mo_sequence_size (seq);
            hset = mo_hashset_reserve (hset, 3 * siz / 2 + 5);
            for (unsigned ix = 0; ix < siz; ix++)
              hset = mo_hashset_put (hset, seq->mo_seqobj[ix]);
          }
          break;
        case mo_KOBJECT:
          hset = mo_hashset_put (hset, (mo_objref_t) (curargv));
          break;
        }
    }
  return mo_hashset_elements_set (hset);
}                               /* end mofun_set_useract */


////////////////
// momglob_tuple  &  momglob_tuple_useract


const char
MOM_PREFIXID (mosig_, tuple_useract)[] = "signature_object_to_value";

     extern mo_signature_object_to_value_sigt
       MOM_PREFIXID (mofun_, tuple_useract) __attribute__ ((optimize ("O2")));


     extern mo_signature_object_to_value_sigt mofun_tuple_useract;

mo_value_t
MOM_PREFIXID (mofun_, tuple_useract) (mo_objref_t obuact)
{
  return mofun_tuple_useract (obuact);
}

mo_value_t
mofun_tuple_useract (mo_objref_t obuact)
{
  MOM_ASSERTPRINTF (mo_dyncast_object (obuact), "tuple_useract: bad obuact");
  unsigned nbargs = mo_objref_comp_count (obuact);
  mo_vectvaldatapayl_ty *vect =
    mo_vectval_reserve (NULL, 3 * nbargs / 2 + 20);
  for (unsigned ix = 1; ix < nbargs; ix++)
    {
      mo_value_t curargv = mo_objref_get_comp (obuact, ix);
      enum mo_valkind_en k = mo_kind_of_value (curargv);
      switch (k)
        {
        case mo_KNONE:
        case mo_KINT:
        case mo_KSTRING:
          break;
        case mo_KTUPLE:
        case mo_KSET:
          {
            const mo_sequencevalue_ty *seq =
              (const mo_sequencevalue_ty *) curargv;
            unsigned siz = mo_sequence_size (seq);
            vect = mo_vectval_reserve (vect, 3 * siz / 2 + 5);
            for (unsigned ix = 0; ix < siz; ix++)
              if (mo_dyncast_object (seq->mo_seqobj[ix]))
                vect = mo_vectval_append (vect, seq->mo_seqobj[ix]);
          }
          break;
        case mo_KOBJECT:
          vect = mo_vectval_append (vect, (mo_objref_t) (curargv));
          break;
        }
    }
  return mo_vectval_objects_tuple (vect);
}                               /* end mofun_tuple_useract */


// momglob_string
const char
MOM_PREFIXID (mosig_, string_useract)[] = "signature_object_to_value";

     extern mo_signature_object_to_value_sigt
       MOM_PREFIXID (mofun_, string_useract)
  __attribute__ ((optimize ("O2")));


     extern mo_signature_object_to_value_sigt mofun_string_useract;

mo_value_t
MOM_PREFIXID (mofun_, string_useract) (mo_objref_t obuact)
{
  return mofun_string_useract (obuact);
}

mo_value_t
mofun_string_useract (mo_objref_t obuact)
{
  MOM_ASSERTPRINTF (mo_dyncast_object (obuact), "string_useract: bad obuact");
  unsigned nbargs = mo_objref_comp_count (obuact);
  char *buf = NULL;
  size_t siz = 0;
  FILE *fmem = open_memstream (&buf, &siz);
  if (!fmem)
    MOM_FATAPRINTF ("string_useract: open_memstream fail");
  for (unsigned ix = 1; ix < nbargs; ix++)
    {
      mo_value_t curargv = mo_objref_get_comp (obuact, ix);
      enum mo_valkind_en k = mo_kind_of_value (curargv);
      if (k == mo_KNONE)
        continue;
      else if (k == mo_KSTRING)
        fputs (mo_string_cstr (curargv), fmem);
      else if (k == mo_KINT)
        fprintf (fmem, "%ld", (long) mo_value_to_int (curargv, -1));
      else
        fputs (mo_value_pnamestr (curargv), fmem);
    }
  long ln = ftell (fmem);
  fflush (fmem);
  mo_value_t vstr = mo_make_string_len (buf, ln);
  fclose (fmem);
  free (buf), buf = NULL;
  return vstr;
}                               /* end of mofun_string_useract */

// momglob_add_user_action momglob_add_user_action_useract
const char
MOM_PREFIXID (mosig_, add_user_action_useract)[] =
  "signature_object_to_value";

     extern mo_signature_object_to_value_sigt
       MOM_PREFIXID (mofun_, add_user_action_useract)
  __attribute__ ((optimize ("O2")));


     extern mo_signature_object_to_value_sigt mofun_add_user_action_useract;

mo_value_t
MOM_PREFIXID (mofun_, add_user_action_useract) (mo_objref_t obuact)
{
  return mofun_add_user_action_useract (obuact);
}

/// for $add_user_action(Action Action_useract)
mo_value_t
mofun_add_user_action_useract (mo_objref_t obuact)
{
  MOM_ASSERTPRINTF (mo_dyncast_object (obuact),
                    "add_user_action_useract: bad obuact");
  unsigned nbargs = mo_objref_comp_count (obuact);
  enum
  {
    MOMIX_OPER,
    MOMIX_ACTION,
    MOMIX_USERACT,
    MOMIX__LAST
  };
  mo_objref_t operobr =
    mo_dyncast_objref (mo_objref_get_comp (obuact, MOMIX_OPER));
  mo_objref_t actionobr =
    mo_dyncast_objref (mo_objref_get_comp (obuact, MOMIX_ACTION));
  mo_objref_t useractobr =
    mo_dyncast_objref (mo_objref_get_comp (obuact, MOMIX_USERACT));
  if (nbargs != MOMIX__LAST)
    mom_gui_fail_user_action
      ("add_user_action_useract for $add_user_action wants two user arguments, got %d in %s",
       mo_objref_comp_count (obuact) - 1, mo_objref_pnamestr (obuact));
  if (!actionobr)
    mom_gui_fail_user_action
      ("add_user_action_useract: missing action (first argument) in %s",
       mo_objref_pnamestr (obuact));
  if (!useractobr)
    mom_gui_fail_user_action
      ("add_user_action_useract: missing useract (second argument) in %s",
       mo_objref_pnamestr (obuact));
  char useractid[MOM_CSTRIDSIZ];
  memset (useractid, 0, sizeof (useractid));
  mo_objref_idstr (useractid, useractobr);
  char funsymbuf[MOM_CSTRIDSIZ + 24];
  memset (funsymbuf, 0, sizeof (funsymbuf));
  snprintf (funsymbuf, sizeof (funsymbuf), MOM_FUNC_PREFIX "%s", useractid);
  void *funad = dlsym (mom_prog_dlhandle, funsymbuf);
  if (!funad)
    mom_gui_fail_user_action
      ("add_user_action_useract: function %s not found (%s) for $%s",
       funsymbuf, dlerror (), mo_objref_pnamestr (actionobr));
  char sigsymbuf[MOM_CSTRIDSIZ + 24];
  memset (sigsymbuf, 0, sizeof (sigsymbuf));
  snprintf (sigsymbuf, sizeof (sigsymbuf),
            MOM_SIGNATURE_PREFIX "%s", useractid);
  const char *sigad = dlsym (mom_prog_dlhandle, sigsymbuf);
  if (!sigad)
    mom_gui_fail_user_action
      ("add_user_action_useract: signature %s not found (%s) for $%s",
       sigsymbuf, dlerror (), mo_objref_pnamestr (actionobr));
  if (strcmp (sigad, "signature_object_to_value")
      && strcmp (sigad, MOM_IDENTOFNAME (signature_object_to_value)))
    mom_gui_fail_user_action
      ("add_user_action_useract: signature %s unexpected '%s' for $%s",
       sigsymbuf, sigad, mo_objref_pnamestr (actionobr));

  MOM_FATAPRINTF ("add_user_action_useract unimplemented obuact=%s",
                  mo_objref_pnamestr (obuact));
#warning mofun_add_user_action_useract unimplemented
}                               /* end of mofun_add_user_action_useract */

// end of file usaction.c

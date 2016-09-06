// file momplug_unpredef.c

/* a temporary plugin to unpredefine a given element */
#include "meltmoni.h"

void
momplugin_startup (const char *arg)
{
  if (!arg)
    MOM_FATAPRINTF ("missing argument to unpredef plugin");
  mo_objref_t obr = find_named_cstr (arg);
  if (!obr)
    MOM_FATAPRINTF ("unpredef plugin don't find object named %s", arg);
  mo_objref_put_space (obr, mo_SPACE_GLOBAL);
  char bufoid[MOM_CSTRIDSIZ];
  memset (bufoid, 0, sizeof (bufoid));
  mo_cstring_from_hi_lo_ids (bufoid, obr->mo_ob_hid, obr->mo_ob_loid);
  MOM_INFORMPRINTF ("unpredef plugin put %s = %s in global space",
                    mo_object_pnamestr (obr), bufoid);
}

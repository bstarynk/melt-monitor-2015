// file momplug_testcemit.c

/* a temporary plugin to test c-code emission */
#include "meltmoni.h"


void
momplugin_startup (const char *arg)
{
  if (!arg)
    MOM_FATAPRINTF ("testcemit plugin requires an argument");
  MOM_INFORMPRINTF ("testcemit plugin start arg='%s'", arg);
  mo_objref_t modulobr = mo_find_named_cstr (arg);
  if (!modulobr)
    MOM_FATAPRINTF ("testcemit plugin don't find module:%s", arg);
  mo_objref_t cemitobr = mo_make_object ();
  mo_objref_put_cemit_payload (cemitobr, modulobr);
  MOM_INFORMPRINTF
    ("testcemit before generation of module %s thru cemitobr %s",
     mo_objref_pnamestr (modulobr), mo_objref_pnamestr (cemitobr));
  mo_value_t val = mo_objref_cemit_generate (cemitobr);
  if (val)
    MOM_WARNPRINTF ("testcemit for module %s thru cemitobr %s failed with %s",
                    mo_objref_pnamestr (modulobr),
                    mo_objref_pnamestr (cemitobr), mo_value_pnamestr (val));
  else
    {
      MOM_INFORMPRINTF ("testcemit for module %s thru cemitobr %s successful",
                        mo_objref_pnamestr (modulobr),
                        mo_objref_pnamestr (cemitobr));
      char *cmdbuf = NULL;
      asprintf (&cmdbuf, "make modules.dir/modu_%s.so",
                mo_objref_shortnamestr (modulobr));
      if (!cmdbuf)
        MOM_FATAPRINTF ("failed to build command");
      MOM_INFORMPRINTF ("testcemit will %s", cmdbuf);
      fflush (NULL);
      int rc = system (cmdbuf);
      if (rc)
        MOM_WARNPRINTF ("testcemit command %s failed %d", cmdbuf, rc);
      else
        MOM_INFORMPRINTF ("testcemit succeeded : %s", cmdbuf);
      fflush (NULL);
    }
}                               /* end momplugin_startup */


// eof momplug_testcemit.c

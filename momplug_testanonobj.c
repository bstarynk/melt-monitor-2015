// file momplug_testanonobj.c

/* a temporary plugin to create a few anonymous objects inside the
   named test_anonobj which is added inside the_system */
#include "meltmoni.h"

void
momplugin_startup (const char *arg)
{
  mo_objref_t test_anonobj = NULL;
  mo_objref_t anon1obj = NULL;
  mo_objref_t anon2obj = NULL;
  mo_objref_t anon3obj = NULL;
  if (!(test_anonobj = mo_find_named_cstr ("test_anonobj")))
    {
      test_anonobj = mo_make_global_object ();
      mo_objref_put_attr (test_anonobj, MOM_PREDEF (comment),
                          mo_make_string_cstr
                          ("to test anonymous GUI display"));
      mo_register_named (test_anonobj, "test_anonobj");
      mo_objref_comp_append (MOM_PREDEF (the_system), test_anonobj);
    };
  if (mo_objref_comp_count (test_anonobj) == 0)
    {
      anon1obj = mo_make_global_object ();
      mo_objref_put_attr (anon1obj, MOM_PREDEF (comment),
                          mo_make_string_cstr
                          ("our anon1obj for GUI testing"));
      anon2obj = mo_make_global_object ();
      mo_objref_put_attr (anon2obj, MOM_PREDEF (comment),
                          mo_make_string_cstr
                          ("our anon2obj for GUI testing"));
      mo_objref_put_attr (anon1obj, test_anonobj, anon2obj);
      mo_objref_comp_append (test_anonobj, anon1obj);
      anon3obj = mo_make_global_object ();
      mo_objref_put_attr (anon3obj, MOM_PREDEF (comment),
                          mo_make_string_cstr
                          ("our anon3obj for GUI testing"));
      mo_objref_put_attr (anon2obj, test_anonobj, anon3obj);
      mo_objref_comp_append (anon2obj, anon3obj);
    }
  MOM_INFORMPRINTF
    ("testanon plugin done anon1obj=%s anon2obj=%s anon3obj=%s",
     mo_objref_pnamestr (anon1obj), mo_objref_pnamestr (anon2obj),
     mo_objref_pnamestr (anon3obj));
}                               /* end momplugin_startup in momplug_testanonobj.c */

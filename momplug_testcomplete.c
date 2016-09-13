// file momplug_testcomplete.c

/* a temporary plugin to test completion */
#include "meltmoni.h"

void
momplugin_startup (const char *arg)
{
  unsigned card = 0;
  if (!arg)
    MOM_FATAPRINTF ("testcomplete plugin requires an argument");
  MOM_INFORMPRINTF ("testcomplete plugin start arg='%s'", arg);
  mo_value_t setv = NULL;
  if (arg[0] == '_')
    setv = mom_set_complete_objectid (arg);
  else if (isalpha (arg[0]))
    setv = mo_named_set_of_prefix (arg);
  if (!setv)
    MOM_WARNPRINTF ("testcomplete plugin failing (no result) for arg='%s'",
		    arg);
  else
    {
      MOM_ASSERTPRINTF (mo_dyncast_set (setv), "bad setv in testcomplete");
      card = mo_set_size (setv);
      for (unsigned ix = 0; ix < card; ix++)
	{
	  if (ix % 4 == 0 && ix > 0)
	    putchar ('\n');
	  printf (" %s", mo_objref_pnamestr (mo_set_nth (setv, ix)));
	}
      puts (".\n");
    }
  MOM_INFORMPRINTF ("testcomplete arg='%s' gave %u completions\n", arg, card);
}				/* end momplugin_startup in momplug_testanonobj.c */

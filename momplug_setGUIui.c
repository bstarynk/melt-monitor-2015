/* file momplug_setGUIui.c */
/* a temporary plugin to set the_GUI's ui_xml from file misc_ui.xml */
#include "meltmoni.h"

void
momplugin_startup (const char *arg)
{
  if (!arg)
    arg = "misc_ui.xml";
  MOM_INFORMPRINTF ("setGUIui plugin start arg=%s", arg);
  FILE *fil = fopen (arg, "r");
  if (!fil)
    MOM_FATAPRINTF ("setGUIui plugin fopen failed arg=%s", arg);
  mo_value_t vstr = mo_make_string_from_skipped_textual_file (fil, 1);
  MOM_ASSERTPRINTF (mo_dyncast_string (vstr), "non-string vstr");
  mo_objref_put_attr (MOM_PREDEF (the_GUI), MOM_PREDEF (xml_gtkbuild), vstr);
  MOM_INFORMPRINTF ("setGUIui plugin done vstr.size=%d",
                    mo_string_size (vstr));
}                               /* end of momplugin_startup */

// file momplug_testmany.c

/* a temporary plugin to test many objects */
#include "meltmoni.h"

void
momplugin_startup (const char *arg)
{
  /// test operations
  enum
  {
    tsop_PUTATTROBJ,
    tsop_PUTATTROBJBIS,         /// same thing, but more frequent
    tsop_PUTATTRINT,
    tsop_PUTATTRINTBIS,
    tsop_PUTATTRSTR,
    tsop_PUTATTRSET,
    tsop_PUTATTRTUP,
    tsop_PUTATTRTUPBIS,
    tsop_PUTATTRCOMP,
    tsop_PUTATTRCOMPBIS,
    tsop_APPENDOBJ,
    tsop_APPENDSET,
    tsop_APPENDTUP,
    tsop_APPENDINT,
    tsop_APPENDSTR,
    tsop__LAST
  };
  int nbtest = 3000;
  int nbob = 0;
  if (arg && isdigit (arg[0]))
    nbtest = atoi (arg);
  if (nbtest <= 20)
    MOM_FATAPRINTF
      ("testmany with too small nbtest=%d (should be above twenty)", nbtest);
  double startreal = mom_elapsed_real_time ();
  double startcpu = mom_process_cpu_time ();
  int widtest = ((int) sqrt (nbtest * 2.5)) + 5;
  MOM_INFORMPRINTF ("testmany start with nbtest=%d widtest=%d\n", nbtest,
                    widtest);
  mo_objref_t *tabobj = mom_gc_alloc (sizeof (mo_objref_t) * (widtest + 1));
  // initial loop to fill the tabobj
  for (int ix = 0; ix < widtest; ix++)
    {
      nbob++;
      mo_objref_t curobj = mo_make_object ();
      mo_objref_put_space (curobj, mo_SPACE_USER);
      tabobj[ix] = curobj;
    }
  // make the set of initial tabobj and put it into the_system attribute value
  {
    mo_sequencevalue_ty *sq = mo_sequence_allocate (widtest);
    for (int ix = 0; ix < widtest; ix++)
      mo_sequence_put (sq, ix, tabobj[ix]);
    mo_value_t setv = mo_make_set_closeq (sq);
    sq = NULL;
    mo_objref_put_attr (MOM_PREDEF (the_system), MOM_PREDEF (value), setv);
  }
  // testing loop
  for (int tix = 1; tix <= nbtest; tix++)
    {
      if (tix % 8192 == 0)
        MOM_INFORMPRINTF ("testmany tix#%d %.1f%%", tix,
                          100.0 * (double) tix / nbtest);
      if (mom_random_uint32 () % (4 + widtest / 64) == 0)
        {
          int rix = (mom_random_uint32 () & 0xfffffff) % widtest;
          mo_objref_t newobj = mo_make_object ();
          mo_objref_put_space (newobj, mo_SPACE_USER);
          tabobj[rix] = newobj;
          nbob++;
        }
      unsigned op = mom_random_uint32 () % tsop__LAST;
      switch (op)
        {
        case tsop_PUTATTROBJ:
        case tsop_PUTATTROBJBIS:
          {
            mo_objref_t ob1 = tabobj[mom_random_uint32 () % widtest];
            mo_objref_t ob2 = tabobj[mom_random_uint32 () % widtest];
            mo_objref_t ob3 = tabobj[mom_random_uint32 () % widtest];
            mo_objref_put_attr (ob1, ob2, (mo_value_t) ob3);
          }
          break;
        case tsop_PUTATTRINT:
        case tsop_PUTATTRINTBIS:
          {
            mo_objref_t ob1 = tabobj[mom_random_uint32 () % widtest];
            mo_objref_t ob2 = tabobj[mom_random_uint32 () % widtest];
            int i = (int) (mom_random_uint32 () & 0xffff) - 512;
            mo_objref_put_attr (ob1, ob2, mo_int_to_value (i));
          }
          break;
        case tsop_PUTATTRCOMP:
        case tsop_PUTATTRCOMPBIS:
          {
            mo_objref_t ob1 = tabobj[mom_random_uint32 () % widtest];
            mo_objref_t ob2 = tabobj[mom_random_uint32 () % widtest];
            mo_objref_t ob3 = tabobj[mom_random_uint32 () % widtest];
            unsigned ln3 = mo_objref_comp_count (ob3);
            if (ln3 > 4)
              {
                mo_value_t val =
                  mo_objref_get_comp (ob3, mom_random_uint32 () % ln3);
                mo_objref_put_attr (ob1, ob2, val);
              }
          }
          break;
        case tsop_PUTATTRSTR:
          {
            const char *cs =
              "abc.d efgh! ijk lmn opq,rstu vwx yz_ABCD EFG HIJKL MNOPQRS% TU VW XYZ";
            int cslen = strlen (cs);
            mo_objref_t ob1 = tabobj[mom_random_uint32 () % widtest];
            mo_objref_t ob2 = tabobj[mom_random_uint32 () % widtest];
            char buf[80];
            memset (buf, 0, sizeof (buf));
            int ln = 10 + mom_random_uint32 () % (sizeof (buf) - 12);
            for (int j = 0; j < ln; j++)
              buf[j] = cs[mom_random_uint32 () % cslen];
            mo_value_t strv = mo_make_string_sprintf ("%d+%s&%d",
                                                      (int) (mom_random_uint32
                                                             () & 0xffffff),
                                                      buf,
                                                      (int) (mom_random_uint32
                                                             () & 0xffffff));
            mo_objref_put_attr (ob1, ob2, strv);
          }
          break;
        case tsop_PUTATTRSET:
          {
            mo_objref_t ob1 = tabobj[mom_random_uint32 () % widtest];
            mo_objref_t ob2 = tabobj[mom_random_uint32 () % widtest];
            int rix = mom_random_uint32 () % (widtest / 3 + widtest / 16);
            int ln = mom_random_uint32 () % (2 * widtest / 3);
            if (rix + ln + 2 >= widtest)
              ln = widtest - rix - 1;
            mo_sequencevalue_ty *sq = mo_sequence_allocate (ln);
            for (int ixs = 0; ixs < ln; ixs++)
              mo_sequence_put (sq, ixs, tabobj[rix + ixs]);
            mo_value_t setv = mo_make_set_closeq (sq);
            sq = NULL;
            mo_objref_put_attr (ob1, ob2, setv);
          }
          break;
        case tsop_PUTATTRTUP:
        case tsop_PUTATTRTUPBIS:
          {
            mo_objref_t ob1 = tabobj[mom_random_uint32 () % widtest];
            mo_objref_t ob2 = tabobj[mom_random_uint32 () % widtest];
            int ln = mom_random_uint32 () % (5 * widtest / 4 + 3);
            mo_sequencevalue_ty *sq = mo_sequence_allocate (ln);
            for (int ixs = 0; ixs < ln; ixs++)
              mo_sequence_put (sq, ixs,
                               tabobj[mom_random_uint32 () % widtest]);
            mo_value_t tupv = mo_make_tuple_closeq (sq);
            sq = NULL;
            mo_objref_put_attr (ob1, ob2, tupv);
          }
          break;
        case tsop_APPENDOBJ:
          {
            mo_objref_t ob1 = tabobj[mom_random_uint32 () % widtest];
            mo_objref_t ob2 = tabobj[mom_random_uint32 () % widtest];
            mo_objref_comp_append (ob1, ob2);
          }
          break;
        case tsop_APPENDSET:
          {
            int ln = mom_random_uint32 () % (widtest / 4) + (widtest / 8) + 2;
            mo_objref_t ob1 = tabobj[mom_random_uint32 () % widtest];
            mo_sequencevalue_ty *sq = mo_sequence_allocate (ln);
            for (int ixs = 0; ixs < ln; ixs++)
              mo_sequence_put (sq, ixs,
                               tabobj[mom_random_uint32 () % widtest]);
            mo_value_t setv = mo_make_set_closeq (sq);
            sq = NULL;
            mo_objref_comp_append (ob1, setv);
          }
          break;
        case tsop_APPENDTUP:
          {
            int ln = mom_random_uint32 () % (widtest / 4) + (widtest / 8) + 3;
            mo_objref_t ob1 = tabobj[mom_random_uint32 () % widtest];
            mo_sequencevalue_ty *sq = mo_sequence_allocate (ln);
            for (int ixs = 0; ixs < ln; ixs++)
              mo_sequence_put (sq, ixs,
                               tabobj[mom_random_uint32 () % widtest]);
            mo_value_t tupv = mo_make_tuple_closeq (sq);
            sq = NULL;
            mo_objref_comp_append (ob1, tupv);
          }
          break;
        case tsop_APPENDINT:
          {
            mo_objref_t ob1 = tabobj[mom_random_uint32 () % widtest];
            int i = (int) (mom_random_uint32 () & 0x7ffff) - 128;
            mo_objref_comp_append (ob1, mo_int_to_value (i));
          }
          break;
        case tsop_APPENDSTR:
          {
            mo_objref_t ob1 = tabobj[mom_random_uint32 () % widtest];
            mo_value_t strv = mo_make_string_sprintf ("%c%c%c_%x",
                                                      '^' +
                                                      mom_random_uint32 () %
                                                      32,
                                                      '@' +
                                                      mom_random_uint32 () %
                                                      32,
                                                      ' ' +
                                                      mom_random_uint32 () %
                                                      64,
                                                      mom_random_uint32 () &
                                                      0xffffff);
            mo_objref_comp_append (ob1, strv);
          }
          break;
        }
    }                           /* end for tix */
  double endreal = mom_elapsed_real_time ();
  double endcpu = mom_process_cpu_time ();
  double tireal = endreal - startreal;
  double ticpu = endcpu - startcpu;
  MOM_INFORMPRINTF
    ("testmany ending nbtest=%d nbob=%d in %.3f real %.3f cpu seconds,\n"
     "\t .... %.2f real %.2f cpu µs/test\n", nbtest, nbob, tireal, ticpu,
     1.0e6 * (tireal / nbtest), 1.0e6 * (ticpu / nbtest));
}                               /* end momplugin_startup */

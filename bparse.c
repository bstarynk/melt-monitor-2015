// file bparse.c - boot parser

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

enum momlexkind_en
{
  MOLEX__NONE,
  MOLEX_DELIM,
  MOLEX_INT,
  MOLEX_STRING,
  MOLEX_ITEM,
  MOLEX_OPERITEM,
};

enum momdelim_en
{
  MODLM__NONE,
  MODLM_LPAR,                   /// (
  MODLM_RPAR,                   /// )
  MODLM_LBRACE,                 /// {
  MODLM_RBRACE,                 /// }
  MODLM_LBRACKET,               /// [
  MODLM_RBRACKET,               /// ]
  MODLM_PERCENT,                /// %
  MODLM_COMMA,                  /// ,
  MODLM_COLON,                  /// :
  MODLM_SLASH,                  /// /
  MODLM_TILDE,                  /// ~
};

struct momtoken_st
{
  enum momlexkind_en mtok_kind;
  int mtok_lin;
  union
  {
    int64_t mtok_int;
    enum momdelim_en mtok_delim;
    struct mom_item_st *mtok_itm;
    const struct mom_boxstring_st *mtok_str;
  };
};

struct momtokvect_st
{
  char *mtv_filename;
  unsigned mtv_size;
  unsigned mtv_len;
  struct momtoken_st mtv_arr[];
};

struct momtokvect_st *
momtok_append (struct momtokvect_st *tvec, struct momtoken_st tok)
{
  if (MOM_UNLIKELY (tok.mtok_kind == MOLEX__NONE))
    MOM_FATAPRINTF ("cannot append empty token");
  if (MOM_UNLIKELY (tvec == NULL))
    MOM_FATAPRINTF ("null tokvect");
  unsigned len = tvec->mtv_len;
  if (MOM_UNLIKELY (len + 2 >= tvec->mtv_size))
    {
      unsigned newsiz = mom_prime_above (4 * len / 3 + 10);
      assert (newsiz > tvec->mtv_size);
      struct momtokvect_st *newvec =    //
        mom_gc_alloc (sizeof (struct momtokvect_st) +
                      newsiz * sizeof (struct momtoken_st));
      newvec->mtv_size = newsiz;
      newvec->mtv_filename = tvec->mtv_filename;
      if (len > 0)
        memcpy (newvec->mtv_arr, tvec->mtv_arr,
                len * sizeof (struct momtoken_st));
      tvec = newvec;
    };
  tvec->mtv_arr[len] = tok;
  tvec->mtv_len = len + 1;
  return tvec;
}


struct momtokvect_st *
momtok_tokenize (const char *filnam)
{
  if (MOM_UNLIKELY (!filnam || !filnam[0]))
    MOM_FATAPRINTF ("bad filename to momtok_parse");
  FILE *fil = fopen (filnam, "r");
  if (!fil)
    MOM_FATAPRINTF ("failed to open %s (%m)", filnam);
  struct stat fst = { };
  memset (&fst, 0, sizeof (fst));
  if (fstat (fileno (fil), &fst))
    MOM_FATAPRINTF ("failed to fstat %s fd#%d (%m)", filnam, fileno (fil));
  size_t fsz = fst.st_size;
  struct momtokvect_st *tovec = NULL;
  char *linbuf = NULL;
  size_t linsiz = 0;
  ssize_t linlen = 0;
  int lineno = 0;
  long linoff = 0;
  char *ptok = NULL;
  {
    unsigned tosiz = mom_prime_above (10 + fsz / 32);
    tovec =
      mom_gc_alloc (sizeof (struct momtokvect_st) +
                    tosiz * sizeof (struct momtoken_st));
    tovec->mtv_size = tosiz;
    tovec->mtv_filename = GC_STRDUP (filnam);
  }
  do
    {
      if (!linbuf || *ptok == (char) 0)
        {
          linoff = ftell (fil);
          linlen = getline (&linbuf, &linsiz, fil);
          if (linlen >= 0)
            lineno++;
          ptok = linbuf;
        };
      while (isspace (*ptok))
        ptok++;
      if (*ptok == '#')
        {
          ptok = linbuf + linlen - 1;
          continue;
        };
      if (isdigit (*ptok) || *ptok == '+' || *ptok == '-')
        {
          char *endnum = NULL;
          long long num = strtoll (ptok, &endnum, 0);
          if (endnum > ptok)
            {
              tovec = momtok_append (tovec, (struct momtoken_st)
                                     {
                                     .mtok_kind = MOLEX_INT,.mtok_lin =
                                     lineno,.mtok_int = num});
              ptok = endnum;
              continue;
            }
        };
      if (isalpha (*ptok))
        {
          const char *endnam = NULL;
          struct mom_item_st *itm = mom_find_item_from_string (ptok, &endnam);
          if (!itm)
            MOM_FATAPRINTF ("invalid name near %s file %s line %d", ptok,
                            filnam, lineno);
          ptok = (char *) endnam;
          tovec = momtok_append (tovec, (struct momtoken_st)
                                 {
                                 .mtok_kind = MOLEX_ITEM,.mtok_lin =
                                 lineno,.mtok_itm = itm});
          continue;
        }
      if (*ptok == '!' && isalpha (ptok[1]))
        {
          const char *endnam = NULL;
          struct mom_item_st *itm =
            mom_make_item_from_string (ptok + 1, &endnam);
          if (!itm)
            MOM_FATAPRINTF ("invalid new name near %s file %s line %d", ptok,
                            filnam, lineno);
          ptok = (char *) endnam;
          tovec = momtok_append (tovec, (struct momtoken_st)
                                 {
                                 .mtok_kind = MOLEX_ITEM,.mtok_lin =
                                 lineno,.mtok_itm = itm});
          continue;
        }
      if (*ptok == '^' && isalpha (ptok[1]))
        {
          const char *endnam = NULL;
          struct mom_item_st *itm =
            mom_find_item_from_string (ptok + 1, &endnam);
          if (!itm)
            MOM_FATAPRINTF ("invalid oper name near %s file %s line %d", ptok,
                            filnam, lineno);
          ptok = (char *) endnam;
          tovec = momtok_append (tovec, (struct momtoken_st)
                                 {
                                 .mtok_kind = MOLEX_OPERITEM,.mtok_lin =
                                 lineno,.mtok_itm = itm});
          continue;
        }
      if (*ptok == '"')
        {
          ptok++;
          int quotoff = ptok - linbuf;
          long oldoff = ftell (fil);
          fseek (fil, linoff + quotoff, SEEK_SET);
          struct mom_string_and_size_st ss = mom_input_quoted_utf8 (fil);
          ptok = linbuf + (ftell (fil) - linoff);
          fseek (fil, oldoff, SEEK_SET);
          const struct mom_boxstring_st *bstr =
            mom_boxstring_make_len (ss.ss_str, ss.ss_len);
          tovec = momtok_append (tovec, (struct momtoken_st)
                                 {
                                 .mtok_kind = MOLEX_STRING,.mtok_lin =
                                 lineno,.mtok_str = bstr});
          continue;
        }
      if (*ptok == '(')
        {
          tovec = momtok_append (tovec, (struct momtoken_st)
                                 {
                                 .mtok_kind = MOLEX_DELIM,.mtok_lin =
                                 lineno,.mtok_delim = MODLM_LPAR});
          ptok++;
          continue;
        }
      if (*ptok == ')')
        {
          tovec =               //
            momtok_append (tovec, (struct momtoken_st)
                           {
                           .mtok_kind = MOLEX_DELIM,.mtok_lin =
                           lineno,.mtok_delim = MODLM_RPAR});
          ptok++;
          continue;
        }
      if (*ptok == '[')
        {
          tovec = momtok_append (tovec, (struct momtoken_st)
                                 {
                                 .mtok_kind = MOLEX_DELIM,.mtok_lin =
                                 lineno,.mtok_delim = MODLM_LBRACKET});
          ptok++;
          continue;
        }
      if (*ptok == ']')
        {
          tovec =               //
            momtok_append (tovec, (struct momtoken_st)
                           {
                           .mtok_kind = MOLEX_DELIM,.mtok_lin =
                           lineno,.mtok_delim = MODLM_RBRACKET});
          ptok++;
          continue;
        }
      if (*ptok == '{')
        {
          tovec =               //
            momtok_append (tovec, (struct momtoken_st)
                           {
                           .mtok_kind = MOLEX_DELIM,.mtok_lin =
                           lineno,.mtok_delim = MODLM_LBRACE});
          ptok++;
          continue;
        }
      if (*ptok == '}')
        {
          tovec =               //
            momtok_append (tovec, (struct momtoken_st)
                           {
                           .mtok_kind = MOLEX_DELIM,.mtok_lin =
                           lineno,.mtok_delim = MODLM_RBRACE});
          ptok++;
          continue;
        }
      if (*ptok == '%')
        {
          tovec =               //
            momtok_append (tovec, (struct momtoken_st)
                           {
                           .mtok_kind = MOLEX_DELIM,.mtok_lin =
                           lineno,.mtok_delim = MODLM_PERCENT});
          ptok++;
          continue;
        }
      if (*ptok == '/')
        {
          tovec =               //
            momtok_append (tovec, (struct momtoken_st)
                           {
                           .mtok_kind = MOLEX_DELIM,.mtok_lin =
                           lineno,.mtok_delim = MODLM_SLASH});
          ptok++;
          continue;
        }
      if (*ptok == ':')
        {
          tovec =               //
            momtok_append (tovec, (struct momtoken_st)
                           {
                           .mtok_kind = MOLEX_DELIM,.mtok_lin =
                           lineno,.mtok_delim = MODLM_COLON});
          ptok++;
          continue;
        }
      if (*ptok == '~')
        {
          tovec =               //
            momtok_append (tovec, (struct momtoken_st)
                           {
                           .mtok_kind = MOLEX_DELIM,.mtok_lin =
                           lineno,.mtok_delim = MODLM_TILDE});
          ptok++;
          continue;
        }
      MOM_FATAPRINTF ("invalid token in file %s line %d near %s",
                      filnam, lineno, ptok);
    }
  while (!feof (fil) && *ptok);
  fclose (fil);
  return tovec;
}                               /* end momtok_tokenize */

const void *
momtok_parse (struct momtokvect_st *tovec, int topos, int *endposptr)
{
  const void *res = NULL;
  assert (tovec != NULL);
  assert (endposptr != NULL);
  int tolen = (int) tovec->mtv_len;
  if (topos < 0 || topos >= tolen)
    {
      *endposptr = topos;
      return NULL;
    }
  struct momtoken_st *curtok = tovec->mtv_arr + topos;
  switch (curtok->mtok_kind)
    {
    case MOLEX_INT:
      res = mom_boxint_make (curtok->mtok_int);
      *endposptr = topos + 1;
      return res;
    case MOLEX_ITEM:
      res = curtok->mtok_itm;
      *endposptr = topos + 1;
      return res;
    case MOLEX_STRING:
      res = curtok->mtok_str;
      *endposptr = topos + 1;
      return res;
    default:
      break;
    }
  if (curtok->mtok_kind == MOLEX_DELIM && curtok->mtok_delim == MODLM_LBRACE)
    {
      int ln = 0;
      for (int ix = topos + 1;
           ix < tolen && tovec->mtv_arr[ix].mtok_kind == MOLEX_ITEM; ix++)
        ln++;
      if (topos + ln + 2 >= tolen
          || tovec->mtv_arr[topos + ln + 1].mtok_kind != MOLEX_DELIM
          || tovec->mtv_arr[topos + ln + 1].mtok_delim != MODLM_RBRACE)
        MOM_FATAPRINTF ("invalid set starting line %d of file %s",
                        curtok->mtok_lin, tovec->mtv_filename);
      const struct mom_item_st **itemarr =
        mom_gc_alloc ((ln + 1) * sizeof (struct mom_item_st *));
      for (int ix = 0; ix < ln; ix++)
        itemarr[ix] = tovec->mtv_arr[topos + ix + 1].mtok_itm;
      res = mom_boxset_make_arr (ln, itemarr);
      *endposptr = topos + ln + 2;
      return res;
    }
  if (curtok->mtok_kind == MOLEX_DELIM
      && curtok->mtok_delim == MODLM_LBRACKET)
    {
      int ln = 0;
      for (int ix = topos + 1;
           ix < tolen
           && (tovec->mtv_arr[ix].mtok_kind == MOLEX_ITEM
               || (tovec->mtv_arr[ix].mtok_kind == MOLEX_DELIM
                   && tovec->mtv_arr[ix].mtok_delim == MODLM_TILDE)); ix++)
        ln++;
      if (topos + ln + 2 >= tolen
          || tovec->mtv_arr[topos + ln + 1].mtok_kind != MOLEX_DELIM
          || tovec->mtv_arr[topos + ln + 1].mtok_delim != MODLM_RBRACKET)
        MOM_FATAPRINTF ("invalid tuple starting line %d of file %s",
                        curtok->mtok_lin, tovec->mtv_filename);
      const struct mom_item_st **itemarr =
        mom_gc_alloc ((ln + 1) * sizeof (struct mom_item_st *));
      for (int ix = 0; ix < ln; ix++)
        if (tovec->mtv_arr[topos + ix + 1].mtok_kind == MOLEX_ITEM)
          itemarr[ix] = tovec->mtv_arr[topos + ix + 1].mtok_itm;
        else
          itemarr[ix] = NULL;
      res = mom_boxtuple_make_arr (ln, itemarr);
      *endposptr = topos + ln + 2;
      return res;
    }
  if (curtok->mtok_kind == MOLEX_DELIM && curtok->mtok_delim == MODLM_PERCENT)
    {
      bool withmeta = false;
      if (topos + 4 >= tolen
          || tovec->mtv_arr[topos + 1].mtok_kind != MOLEX_ITEM)
        MOM_FATAPRINTF ("invalid node starting line %d of file %s",
                        curtok->mtok_lin, tovec->mtv_filename);
      struct mom_item_st *connitm = tovec->mtv_arr[topos + 1].mtok_itm;
      struct mom_item_st *metaitm = NULL;
      long metarank = 0;
      topos += 2;
      if (tovec->mtv_arr[topos].mtok_kind == MOLEX_DELIM
          && tovec->mtv_arr[topos].mtok_delim == MODLM_SLASH)
        {
          if (topos + 5 > tolen
              || tovec->mtv_arr[topos + 1].mtok_kind != MOLEX_ITEM
              || tovec->mtv_arr[topos + 2].mtok_kind != MOLEX_DELIM
              || tovec->mtv_arr[topos + 2].mtok_delim != MODLM_COLON
              || tovec->mtv_arr[topos + 3].mtok_kind != MOLEX_INT)
            MOM_FATAPRINTF
              ("invalid meta for node %s starting line %d of file %s",
               mom_item_cstring (connitm), curtok->mtok_lin,
               tovec->mtv_filename);
          withmeta = true;
          metaitm = tovec->mtv_arr[topos + 1].mtok_itm;
          metarank = tovec->mtv_arr[topos + 3].mtok_int;
          topos += 4;
        };
      if (topos + 2 >= tolen || tovec->mtv_arr[topos].mtok_kind != MOLEX_DELIM
          || tovec->mtv_arr[topos].mtok_delim != MODLM_LPAR)
        MOM_FATAPRINTF
          ("missing left parenthesis for node %s starting line %d of file %s",
           mom_item_cstring (connitm), curtok->mtok_lin, tovec->mtv_filename);
      topos++;
      const void *smallarr[5] = { NULL };
      const void **arr = smallarr;
      unsigned siz = sizeof (smallarr) / sizeof (void *);
      int nbsons = 0;
      while (topos < tolen
             && (tovec->mtv_arr[topos].mtok_kind != MOLEX_DELIM
                 || tovec->mtv_arr[topos].mtok_delim != MODLM_RPAR))
        {
          const void *sonval = momtok_parse (tovec, topos, &topos);
          if (nbsons >= (int) siz)
            {
              unsigned newsiz = mom_prime_above (4 * siz / 3 + 4);
              const void **newarr = mom_gc_alloc (newsiz * sizeof (void *));
              memcpy (newarr, arr, nbsons * sizeof (void *));
              if (arr != smallarr)
                GC_FREE (arr);
              arr = newarr;
              siz = newsiz;
            };
          arr[nbsons++] = sonval;
        }
      if (topos < tolen && tovec->mtv_arr[topos].mtok_kind == MOLEX_DELIM
          && tovec->mtv_arr[topos].mtok_delim == MODLM_RPAR)
        topos++;
      if (withmeta)
        res =
          mom_boxnode_make_meta (connitm, nbsons,
                                 (const struct mom_hashedvalue_st **) arr,
                                 metaitm, metarank);
      else
        res =
          mom_boxnode_make (connitm, nbsons,
                            (const struct mom_hashedvalue_st **) arr);
      if (arr != smallarr)
        GC_FREE (arr), arr = NULL;
      *endposptr = topos;
      return res;
    }
  if (curtok->mtok_kind == MOLEX_DELIM && curtok->mtok_delim == MODLM_TILDE)
    {
      res = NULL;
      *endposptr = topos + 1;
      return NULL;
    }
  if (curtok->mtok_kind == MOLEX_OPERITEM
      && curtok->mtok_itm == MOM_PREDEFITM (get))
    {
      topos++;
      struct mom_item_st *fromitm =
        mom_dyncast_item (momtok_parse (tovec, topos, &topos));
      if (!fromitm)
        MOM_FATAPRINTF ("bad from for ^get at line %d of file %s",
                        curtok->mtok_lin, tovec->mtv_filename);
      if (topos < tolen && tovec->mtv_arr[topos].mtok_kind == MOLEX_INT)
        {
          int rk = tovec->mtv_arr[topos].mtok_int;
          mom_item_lock (fromitm);
          res = mom_vectvaldata_nth (fromitm->itm_pcomp, rk);
          mom_item_unlock (fromitm);
          *endposptr = topos;
          return res;
        }
      const void *atval = momtok_parse (tovec, topos, &topos);
      unsigned atyp = mom_itype (atval);
      if (atyp == MOMITY_BOXINT)
        {
          int rk = mom_boxint_val_def (atval, 0);
          mom_item_lock (fromitm);
          res = mom_vectvaldata_nth (fromitm->itm_pcomp, rk);
          mom_item_unlock (fromitm);
          *endposptr = topos;
          return res;
        }
      else if (atyp == MOMITY_ITEM)
        {
          struct mom_item_st *atitm = mom_dyncast_item (atval);
          mom_item_lock (fromitm);
          res = mom_unsync_item_get_phys_attr (fromitm, atitm);
          mom_item_unlock (fromitm);
          *endposptr = topos;
          return res;
        }
      else if (atyp != MOMITY_NONE)
        MOM_FATAPRINTF ("bad ^get at line %d of file %s from %s at %s",
                        curtok->mtok_lin, tovec->mtv_filename,
                        mom_item_cstring (fromitm),
                        mom_value_cstring (atval));
      *endposptr = topos;
      return NULL;
    }
  if (curtok->mtok_kind == MOLEX_OPERITEM
      && curtok->mtok_itm == MOM_PREDEFITM (put))
    {
      topos++;
      struct mom_item_st *fromitm =
        mom_dyncast_item (momtok_parse (tovec, topos, &topos));
      if (!fromitm)
        MOM_FATAPRINTF ("bad from for ^put at line %d of file %s",
                        curtok->mtok_lin, tovec->mtv_filename);
      bool atindex = false;
      int atrk = 0;
      const void *atval = NULL;
      const void *newval = NULL;
      if (topos < tolen && tovec->mtv_arr[topos].mtok_kind == MOLEX_INT)
        {
          atrk = tovec->mtv_arr[topos].mtok_int;
          atindex = true;
        }
      else
        atval = momtok_parse (tovec, topos, &topos);
      newval = momtok_parse (tovec, topos, &topos);
      if (atindex)
        {
          mom_item_lock (fromitm);
          mom_vectvaldata_put_nth (fromitm->itm_pcomp, atrk, newval);
          fromitm->itm_mtime = time (NULL);
          mom_item_unlock (fromitm);
          *endposptr = topos;
          return fromitm;
        }
      unsigned atyp = mom_itype (atval);
      if (atyp == MOMITY_BOXINT)
        {
          int rk = mom_boxint_val_def (atval, 0);
          mom_item_lock (fromitm);
          mom_vectvaldata_put_nth (fromitm->itm_pcomp, rk, newval);
          fromitm->itm_mtime = time (NULL);
          mom_item_unlock (fromitm);
          *endposptr = topos;
          return fromitm;
        }
      else if (atyp == MOMITY_ITEM)
        {
          struct mom_item_st *atitm = mom_dyncast_item (atval);
          mom_item_lock (fromitm);
          mom_unsync_item_put_phys_attr (fromitm, atitm, newval);
          fromitm->itm_mtime = time (NULL);
          mom_item_unlock (fromitm);
          *endposptr = topos;
          return fromitm;
        }
      else if (atyp != MOMITY_NONE)
        MOM_FATAPRINTF ("bad ^put at line %d of file %s from %s at %s",
                        curtok->mtok_lin, tovec->mtv_filename,
                        mom_item_cstring (fromitm),
                        mom_value_cstring (atval));
      *endposptr = topos;
      return fromitm;
    }
  if (curtok->mtok_kind == MOLEX_OPERITEM
      && curtok->mtok_itm == MOM_PREDEFITM (display))
    {
      topos++;
      const struct mom_boxstring_st *msgv = NULL;
      if (topos < tolen && tovec->mtv_arr[topos].mtok_kind == MOLEX_STRING)
        msgv = tovec->mtv_arr[topos].mtok_str, topos++;
      else
        MOM_FATAPRINTF
          ("^display at line %d of file %s not followed by message string",
           curtok->mtok_lin, tovec->mtv_filename);
      const void *dispval = momtok_parse (tovec, topos, &topos);
      if (mom_itype (dispval) == MOMITY_ITEM)
        {
          struct mom_item_st *dispitm = mom_dyncast_item (dispval);
          printf ("\n## display item %s from %s line %d\n",
                  mom_item_cstring (dispitm), tovec->mtv_filename,
                  curtok->mtok_lin);
          fputs (mom_boxstring_cstr (msgv), stdout);
          putchar ('\n');
          long lastnl = ftell (stdout);
          mom_item_lock (dispitm);
          mom_output_item_content (stdout, &lastnl, dispitm);
          mom_item_unlock (dispitm);
          putchar ('\n');
          *endposptr = topos;
          return dispitm;
        }
      else
        {
          printf ("\n## display value from %s line %d\n",
                  tovec->mtv_filename, curtok->mtok_lin);
          long lastnl = ftell (stdout);
          fputs (mom_boxstring_cstr (msgv), stdout);
          putchar ('\n');
          mom_output_value (stdout, &lastnl, 0,
                            (const struct mom_hashedvalue_st *) dispval);
          putchar ('\n');
          *endposptr = topos;
          return dispval;
        }
      *endposptr = topos;
      return dispval;
    }
  if (curtok->mtok_kind == MOLEX_OPERITEM
      && curtok->mtok_itm == MOM_PREDEFITM (resize))
    {
      topos++;
      struct mom_item_st *gritm =
        mom_dyncast_item (momtok_parse (tovec, topos, &topos));
      if (!gritm)
        MOM_FATAPRINTF ("bad item for ^resize at line %d of file %s",
                        curtok->mtok_lin, tovec->mtv_filename);
      int atsz = 0;
      const void *atval = NULL;
      if (topos < tolen && tovec->mtv_arr[topos].mtok_kind == MOLEX_INT)
        {
          atsz = tovec->mtv_arr[topos].mtok_int;
          mom_item_lock (gritm);
          gritm->itm_pcomp = mom_vectvaldata_resize (gritm->itm_pcomp, atsz);
          gritm->itm_mtime = time (NULL);
          mom_item_unlock (gritm);
          *endposptr = topos;
          return gritm;
        }
      else
        atval = momtok_parse (tovec, topos, &topos);
      unsigned atyp = mom_itype (atval);
      if (atyp == MOMITY_BOXINT)
        {
          int rk = mom_boxint_val_def (atval, 0);
          mom_item_lock (gritm);
          gritm->itm_pcomp = mom_vectvaldata_resize (gritm->itm_pcomp, rk);
          gritm->itm_mtime = time (NULL);
          mom_item_unlock (gritm);
          *endposptr = topos;
          return gritm;
        }
      else if (atyp != MOMITY_NONE)
        MOM_FATAPRINTF
          ("^resize of %s with bad argument %s at line %d of file %s",
           mom_item_cstring (gritm), mom_value_cstring (atval),
           curtok->mtok_lin, tovec->mtv_filename);
      *endposptr = topos;
      return gritm;

    }
  MOM_FATAPRINTF ("syntax error line %d file %s", curtok->mtok_lin,
                  tovec->mtv_filename);
}                               // end momtok_parse


void
mom_boot_file (const char *path)
{
  struct momtokvect_st *tokvec = momtok_tokenize (path);
  assert (tokvec != NULL);
  int tolen = tokvec->mtv_len;
  int topos = 0;
  int nbval = 0;
  while (topos < tolen)
    {
      (void) momtok_parse (tokvec, topos, &topos);
      nbval++;
    };
  MOM_INFORMPRINTF ("booted file %s with %d tokens & %d values",
                    path, tolen, nbval);
}                               /* end mom_boot_file */

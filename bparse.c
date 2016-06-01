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
  if (topos < 0 || topos >= (int)tovec->mtv_len)
    {
      *endposptr = -1;
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
      int tolen = tovec->mtv_len;
      int ln = 0;
      for (int ix = topos + 1;
           ix < tolen && tovec->mtv_arr[ix].mtok_kind ==  MOLEX_ITEM; ix++)
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
}                               // end momtok_parse

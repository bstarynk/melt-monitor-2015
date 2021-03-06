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
    MOLEX_NAMESTR,
    MOLEX_ITEM,
    MOLEX_OPERITEM,
    MOLEX_CODECHUNK,
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
    const struct mom_boxnode_st *mtok_chunk;
  };
};

struct momtokvect_st
{
  char *mtv_filename;
  struct mom_hashassoc_st *mtv_dict;
  unsigned mtv_size;
  unsigned mtv_len;
  struct momtoken_st mtv_arr[];
};

const struct momtoken_st *
momtok_nth_token (struct momtokvect_st *tvec, int pos)
{
  if (!tvec)
    return NULL;
  unsigned ln = tvec->mtv_len;
  assert (ln < tvec->mtv_size && ln <= MOM_SIZE_MAX);
  if (pos < 0)
    pos += ln;
  if (pos >= 0 && pos < (int) ln)
    return tvec->mtv_arr + pos;
  return NULL;
}                               /* end momtok_nth_token */

const char *
momtok_gcstr (const struct momtoken_st *ptok)
{
  if (!ptok)
    return "~";
  if (ptok == MOM_EMPTY_SLOT)
    return "??";
  char *buf = NULL;
  size_t bufsiz = 0;
  FILE *f = open_memstream (&buf, &bufsiz);
  if (MOM_UNLIKELY (!f))
    MOM_FATAPRINTF ("open_memstream failed for momtok_cstr");
  switch (ptok->mtok_kind)
    {
    case MOLEX__NONE:
      MOM_FATAPRINTF ("bad ptok@%p", (void *) ptok);
    case MOLEX_DELIM:
      {
        fprintf (f, "delim @%d ", ptok->mtok_lin);
        switch (ptok->mtok_delim)
          {
          case MODLM__NONE:
            MOM_FATAPRINTF ("no delim @%p", (void *) ptok);
          case MODLM_LPAR:
            fputs ("LPAR      (", f);
            break;
          case MODLM_RPAR:
            fputs ("RPAR      )", f);
            break;
          case MODLM_LBRACE:
            fputs ("LBRACE    {", f);
            break;
          case MODLM_RBRACE:
            fputs ("RBRACE    }", f);
            break;
          case MODLM_LBRACKET:
            fputs ("LBRACKET  [", f);
            break;
          case MODLM_RBRACKET:
            fputs ("RBRACKET  ]", f);
            break;
          case MODLM_PERCENT:
            fputs ("PERCENT   %", f);
            break;
          case MODLM_COMMA:
            fputs ("COMMA     ,", f);
            break;
          case MODLM_COLON:
            fputs ("COLON     :", f);
            break;
          case MODLM_SLASH:
            fputs ("SLASH     /", f);
            break;
          case MODLM_TILDE:
            fputs ("TILDE     ~", f);
            break;
          }
      }
      break;
    case MOLEX_INT:
      fprintf (f, "int @%d: %lld", ptok->mtok_lin,
               (long long) ptok->mtok_int);
      break;
    case MOLEX_STRING:
      {
        fprintf (f, "string @%d: ", ptok->mtok_lin);
        long lastnl = ftell (f);
        mom_output_value (f, &lastnl, 0,
                          (const struct mom_hashedvalue_st *) ptok->mtok_str);
      }
      break;
    case MOLEX_NAMESTR:
      {
        fprintf (f, "namestr @%d: ", ptok->mtok_lin);
        long lastnl = ftell (f);
        mom_output_value (f, &lastnl, 0,
                          (const struct mom_hashedvalue_st *) ptok->mtok_str);
      }
      break;
    case MOLEX_ITEM:
      fprintf (f, "item @%d: %s", ptok->mtok_lin,
               mom_item_cstring (ptok->mtok_itm));
      break;
    case MOLEX_OPERITEM:
      fprintf (f, "operitem @%d: ^%s", ptok->mtok_lin,
               mom_item_cstring (ptok->mtok_itm));
      break;
    case MOLEX_CODECHUNK:
      {
        fprintf (f, "codechunk @%d:\n", ptok->mtok_lin);
        long nl = ftell (f);
        mom_output_value (f, &nl, 0, (const void *) ptok->mtok_chunk);
        fprintf (f, "\n#end chunk @%d\n", ptok->mtok_lin);
      }
      break;
    }
  fflush (f);
  char *res = mom_gc_strdup (buf);
  free (buf), buf = 0;
  return res;
}                               /* end of momtok_gcstr */

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
  FILE *fil = NULL;
  size_t fsz = 0;
  char *inbuf = NULL;
  if (!strcmp (filnam, "-"))
    {
      MOM_INFORMPRINTF ("tokenizing from stdin");
      unsigned siz = 4001;
      unsigned len = 0;
      inbuf = calloc (1, siz);
      if (!inbuf)
        MOM_FATAPRINTF ("failed to allocate buffer of %d for stdin", siz);
      int c = EOF;
      do
        {
          c = getchar ();
          if (c != EOF)
            {
              if (MOM_UNLIKELY (len + 2 >= siz))
                {
                  unsigned newsiz = mom_prime_above (4 * siz / 3 + 10);
                  if (newsiz > MOM_SIZE_MAX)
                    MOM_FATAPRINTF
                      ("too big size %u for growing stding buffer", newsiz);
                  char *newbuf = calloc (1, newsiz);
                  if (!newbuf)
                    MOM_FATAPRINTF ("failed to grow buffer of %d for stdin",
                                    newsiz);
                  memcpy (newbuf, inbuf, len);
                  free (inbuf);
                  inbuf = newbuf;
                  siz = newsiz;
                };
              inbuf[len++] = c;
            }
        }
      while (c != EOF);
      fil = fmemopen (inbuf, len, "r");
      if (!fil)
        MOM_FATAPRINTF ("failed to fmemopen buffer of %d for stdin", len);
      fsz = len;
    }
  else
    {
      fil = fopen (filnam, "r");
      if (!fil)
        MOM_FATAPRINTF ("failed to open %s (%m)", filnam);
      struct stat fst = { };
      memset (&fst, 0, sizeof (fst));
      if (fstat (fileno (fil), &fst))
        MOM_FATAPRINTF ("failed to fstat %s fd#%d (%m)", filnam,
                        fileno (fil));
      fsz = fst.st_size;
    }
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
    tovec->mtv_filename = mom_gc_strdup (filnam);
    MOM_DEBUGPRINTF (boot, "filnam=%s tosiz=%u", filnam, tosiz);
  }
  do
    {
      if (!linbuf || *ptok == (char) 0)
        {
          linoff = ftell (fil);
          if (linsiz > 0)
            memset (linbuf, 0, linsiz);
          linlen = getline (&linbuf, &linsiz, fil);
          if (linlen >= 0)
            {
              lineno++;
              MOM_DEBUGPRINTF (boot, "filnam=%s line#%d: %s", filnam, lineno,
                               linbuf);
            }
          else
            break;
          ptok = linbuf;
        };
      while (isspace (*ptok))
        ptok++;
      MOM_DEBUGPRINTF (boot, "lineno#%d ptok:%s", lineno, ptok);
      if (*ptok == (char) 0)
        continue;
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
              MOM_DEBUGPRINTF (boot, "lineno#%d num=%lld", lineno, num);
              tovec = momtok_append (tovec, (struct momtoken_st)
                                     {
				       .mtok_kind = MOLEX_INT,.mtok_lin =
					 lineno,.mtok_int = num});
              ptok = endnum;
              continue;
            }
        };
      if (isalpha (*ptok))      /// 'foo1' is an item
        {
          const char *endnam = NULL;
          struct mom_item_st *itm = mom_find_item_from_string (ptok, &endnam);
          MOM_DEBUGPRINTF (boot, "lineno#%d col#%d itm=%s",
                           lineno, (int) (ptok - linbuf),
                           mom_item_cstring (itm));
          if (!itm)
            MOM_FATAPRINTF ("invalid or missing name near %s file %s line %d",
                            ptok, filnam, lineno);
          ptok = (char *) endnam;
          tovec = momtok_append (tovec, (struct momtoken_st)
                                 {
				   .mtok_kind = MOLEX_ITEM,.mtok_lin =
				     lineno,.mtok_itm = itm});
          continue;
        }
      if (*ptok == '!' && isalpha (ptok[1]))    /// '!foo2' is a made item
        {
          const char *endnam = NULL;
          struct mom_item_st *itm =
            mom_make_item_from_string (ptok + 1, &endnam);
          MOM_DEBUGPRINTF (boot, "lineno#%d col#%d made itm=%s",
                           lineno, (int) (ptok - linbuf),
                           mom_item_cstring (itm));
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
      if (*ptok == '$' && isalpha (ptok[1]))    //// '$xy2z' is a name
        {
          const char *endnam = ptok + 1;
          while (isalnum (*endnam))
            endnam++;
          const struct mom_boxstring_st *bs =
            mom_boxstring_make_len (ptok + 1, (int) (endnam - (ptok + 1)));
          MOM_DEBUGPRINTF (boot, "lineno#%d col#%d name %s",
                           lineno, (int) (ptok - linbuf),
                           mom_value_cstring (bs));
          ptok = (char *) endnam;
          tovec = momtok_append (tovec, (struct momtoken_st)
                                 {
				   .mtok_kind = MOLEX_NAMESTR,    //
				     .mtok_lin = lineno,    //
				     .mtok_str = bs});
          continue;
        }
      if (*ptok == '?' && isalpha (ptok[1]))    //// '?abc1' is a cloned item
        {
          const char *endnam = NULL;
          struct mom_item_st *itm =
            mom_find_item_from_string (ptok + 1, &endnam);
          MOM_DEBUGPRINTF (boot, "lineno#%d col#%d clone itm=%s",
                           lineno, (int) (ptok - linbuf),
                           mom_item_cstring (itm));
          if (!itm)
            MOM_FATAPRINTF ("invalid name to clone near %s file %s line %d",
                            ptok, filnam, lineno);
          ptok = (char *) endnam;
          struct mom_item_st *clitm = mom_clone_item (itm);
          MOM_DEBUGPRINTF (boot, "lineno#%d col#%d cloned itm=%s is %s",
                           lineno, (int) (ptok - linbuf),
                           mom_item_cstring (itm), mom_item_cstring (clitm));
          tovec = momtok_append (tovec, (struct momtoken_st)
                                 {
				   .mtok_kind = MOLEX_ITEM,.mtok_lin =
				     lineno,.mtok_itm = clitm});
          continue;
        }
      if (*ptok == '^' && isalpha (ptok[1]))
        {
          const char *endnam = NULL;
          struct mom_item_st *itm =
            mom_find_item_from_string (ptok + 1, &endnam);
          MOM_DEBUGPRINTF (boot, "lineno#%d col#%d oper itm=%s",
                           lineno, (int) (ptok - linbuf),
                           mom_item_cstring (itm));
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
      // ^* is a shorthand for ^at
      else if (ptok[0] == '^' && ptok[1] == '*')
        {
          tovec = momtok_append (tovec, (struct momtoken_st)
                                 {
				   .mtok_kind = MOLEX_OPERITEM,.mtok_lin =
				     lineno,.mtok_itm = MOM_PREDEFITM (at)});
          ptok += 2;
          continue;
        }
      // ^: is a shorthand for ^sequence
      else if (ptok[0] == '^' && ptok[1] == ':')
        {
          tovec = momtok_append (tovec, (struct momtoken_st)
                                 {
				   .mtok_kind = MOLEX_OPERITEM,.mtok_lin =
				     lineno,.mtok_itm =
				     MOM_PREDEFITM (sequence)});
          ptok += 2;
          continue;
        }
      if (*ptok == '"')
        {
          ptok++;
          int quotoff = ptok - linbuf;
          long oldoff = ftell (fil);
          fseek (fil, linoff + quotoff, SEEK_SET);
          struct mom_string_and_size_st ss = mom_input_quoted_utf8 (fil);
          const struct mom_boxstring_st *bstr =
            mom_boxstring_make_len (ss.ss_str, ss.ss_len);
          MOM_DEBUGPRINTF (boot, "lineno#%d col#%d str %s",
                           lineno, (int) (ptok - linbuf),
                           mom_value_cstring ((const void *) bstr));
          ptok = linbuf + (ftell (fil) - linoff) + 1;
          fseek (fil, oldoff, SEEK_SET);
          tovec = momtok_append (tovec, (struct momtoken_st)
                                 {
				   .mtok_kind = MOLEX_STRING,.mtok_lin =
				     lineno,.mtok_str = bstr});
          continue;
        }
      if (*ptok == '(')
        {
          MOM_DEBUGPRINTF (boot, "lineno#%d col#%d LPAR",
                           lineno, (int) (ptok - linbuf));
          tovec = momtok_append (tovec, (struct momtoken_st)
                                 {
				   .mtok_kind = MOLEX_DELIM,.mtok_lin =
				     lineno,.mtok_delim = MODLM_LPAR});
          ptok++;
          continue;
        }
      if (*ptok == ')')
        {
          MOM_DEBUGPRINTF (boot, "lineno#%d col#%d RPAR",
                           lineno, (int) (ptok - linbuf));
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
          MOM_DEBUGPRINTF (boot, "lineno#%d col#%d LBRACKET",
                           lineno, (int) (ptok - linbuf));
          tovec = momtok_append (tovec, (struct momtoken_st)
                                 {
				   .mtok_kind = MOLEX_DELIM,.mtok_lin =
				     lineno,.mtok_delim = MODLM_LBRACKET});
          ptok++;
          continue;
        }
      if (*ptok == ']')
        {
          MOM_DEBUGPRINTF (boot, "lineno#%d col#%d RBRACKET",
                           lineno, (int) (ptok - linbuf));
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
          MOM_DEBUGPRINTF (boot, "lineno#%d col#%d LBRACE",
                           lineno, (int) (ptok - linbuf));
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
          MOM_DEBUGPRINTF (boot, "lineno#%d col#%d RBRACE",
                           lineno, (int) (ptok - linbuf));
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
          MOM_DEBUGPRINTF (boot, "lineno#%d col#%d PERCENT",
                           lineno, (int) (ptok - linbuf));
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
          MOM_DEBUGPRINTF (boot, "lineno#%d col#%d SLASH",
                           lineno, (int) (ptok - linbuf));
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
          MOM_DEBUGPRINTF (boot, "lineno#%d col#%d COLON",
                           lineno, (int) (ptok - linbuf));
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
          MOM_DEBUGPRINTF (boot, "lineno#%d col#%d TILDE",
                           lineno, (int) (ptok - linbuf));
          tovec =               //
            momtok_append (tovec, (struct momtoken_st)
                           {
			     .mtok_kind = MOLEX_DELIM,.mtok_lin =
			       lineno,.mtok_delim = MODLM_TILDE});
          ptok++;
          continue;
        }
      if (*ptok == '$' && ptok[1] == '{')
        {
#warning should parse code chunk in momtok_tokenize
          MOM_FATAPRINTF
            ("unimplemented code chunk parsing in file %s line %d near %s",
             filnam, lineno, ptok);
        }
      MOM_FATAPRINTF ("invalid token in file %s line %d near %s",
                      filnam, lineno, ptok);
    }
  while (!feof (fil) && ptok);
  fclose (fil);
  if (inbuf != NULL)
    free (inbuf);
  return tovec;
}                               /* end momtok_tokenize */


const void *momtok_parse (struct momtokvect_st *tovec, int topos,
                          int *endposptr);

void
momtok_inside_item (struct mom_item_st *itm,
                    struct momtokvect_st *tovec, int topos, int *endposptr)
{
  assert (tovec != NULL);
  assert (endposptr != NULL);
  assert (itm != NULL && itm != MOM_EMPTY_SLOT
          && itm->va_itype == MOMITY_ITEM);
  int tolen = tovec->mtv_len;
  struct momtoken_st *curtok = NULL;
  MOM_DEBUGPRINTF (boot, "momtok_inside_item start itm:%s topos#%d tolen=%d",
                   mom_item_cstring (itm), topos, tolen);
  if (topos < tolen && (curtok = tovec->mtv_arr + topos)
      && curtok->mtok_kind == MOLEX_DELIM
      && tovec->mtv_arr[topos].mtok_delim == MODLM_LPAR)
    topos++;
  else
    MOM_FATAPRINTF
      ("expecting left parenthesis after ^item %s at line %d of file %s",
       mom_item_cstring (itm), curtok->mtok_lin, tovec->mtv_filename);
  MOM_DEBUGPRINTF (boot, "momtok_parse topos#%d ^item %s", topos,
                   mom_item_cstring (itm));
  mom_item_lock (itm);
  itm->itm_pattr = NULL;
  itm->itm_pcomp = NULL;
  mom_unsync_item_clear_payload (itm);
  while (((curtok = NULL), (topos < tolen))
         && (curtok = tovec->mtv_arr + topos)
         && (curtok->mtok_kind != MOLEX_DELIM
             || curtok->mtok_delim != MODLM_RPAR))
    {
      MOM_DEBUGPRINTF (boot, "momtok_inside_item ^item %s topos#%d curtok %s",
                       mom_item_cstring (itm), topos, momtok_gcstr (curtok));
      if (curtok->mtok_kind != MOLEX_OPERITEM)
        MOM_FATAPRINTF
          ("expecting operator but has %s after ^item %s at line %d of file %s",
           momtok_gcstr (curtok),
           mom_item_cstring (itm), curtok->mtok_lin, tovec->mtv_filename);
      if (curtok->mtok_itm == MOM_PREDEFITM (at))
        {
          topos++;
          int atrk = 0;
          bool atindex = false;
          const void *atval = NULL;
          struct mom_item_st *atitm = NULL;
          const void *newval = NULL;
          if (topos < tolen && tovec->mtv_arr[topos].mtok_kind == MOLEX_INT)
            {
              atrk = tovec->mtv_arr[topos].mtok_int;
              topos++;
              atindex = true;
            }
          else
            {
              atval = momtok_parse (tovec, topos, &topos);
              unsigned atyp = mom_itype (atval);
              if (atyp == MOMITY_INT)
                {
                  atrk = mom_int_val_def (atval, 0);
                  atindex = true;
                }
              else if (atyp == MOMITY_ITEM)
                atitm = mom_dyncast_item (atval);
              else if (atyp == MOMITY_NONE)
                {
                }
              else
                MOM_FATAPRINTF
                  ("bad ^at %s after ^item %s at line %d of file %s",
                   mom_value_cstring (atval), mom_item_cstring (itm),
                   curtok->mtok_lin, tovec->mtv_filename);
            };
          newval = momtok_parse (tovec, topos, &topos);
          MOM_DEBUGPRINTF (boot,
                           "momtok_inside_item topos#%d itm %s atindex %s atrk#%d atitm %s newval %s",
                           topos, mom_item_cstring (itm),
                           atindex ? "true" : "false",
                           atrk, mom_item_cstring (atitm),
                           mom_value_cstring (newval));
          if (atindex)
            mom_vectvaldata_put_nth (itm->itm_pcomp, atrk, newval);
          else if (atitm)
            mom_unsync_item_put_phys_attr (itm, atitm, newval);
        }
      else if (curtok->mtok_itm == MOM_PREDEFITM (sequence))
        {
          topos++;
          if (topos < tolen
              && (curtok = tovec->mtv_arr + topos)
              && curtok->mtok_kind == MOLEX_DELIM
              && curtok->mtok_delim == MODLM_LPAR)
            topos++;
          else
            MOM_FATAPRINTF
              ("leftparen expected after ^sequence for ^item %s at line %d of file %s",
               mom_item_cstring (itm), curtok->mtok_lin, tovec->mtv_filename);
          int nbval = 0;
          const void *smallarr[5] = { NULL };
          const void **arr = smallarr;
          unsigned siz = sizeof (smallarr) / sizeof (void *);
          while (topos < tolen
                 && (curtok = tovec->mtv_arr + topos)
                 && (curtok->mtok_kind != MOLEX_DELIM
                     || curtok->mtok_delim != MODLM_RPAR))
            {
              int oldtopos = topos;
              const void *newval = momtok_parse (tovec, topos, &topos);
              MOM_DEBUGPRINTF (boot, "sequence in itm=%s nbval=%d newval=%s",
                               mom_item_cstring (itm), nbval,
                               mom_value_cstring (newval));
              if (!newval && topos == oldtopos)
                MOM_FATAPRINTF
                  ("value expected  after ^sequence for ^item %s at line %d of file %s",
                   mom_item_cstring (itm), curtok->mtok_lin,
                   tovec->mtv_filename);
              if (nbval >= (int) siz)
                {
                  unsigned newsiz = mom_prime_above (4 * siz / 3 + 4);
                  const void **newarr =
                    mom_gc_alloc (newsiz * sizeof (void *));
                  memcpy (newarr, arr, nbval * sizeof (void *));
                  if (arr != smallarr)
                    GC_FREE (arr);
                  arr = newarr;
                  siz = newsiz;
                }
              arr[nbval++] = newval;
            };
          if (topos < tolen
              && tovec->mtv_arr[topos].mtok_kind == MOLEX_DELIM
              && tovec->mtv_arr[topos].mtok_delim == MODLM_RPAR)
            topos++;
          if (nbval > 0)
            {
              itm->itm_pcomp = mom_vectvaldata_resize (NULL, nbval);
              for (int ix = 0; ix < nbval; ix++)
                mom_vectvaldata_put_nth (itm->itm_pcomp, ix, arr[ix]);
            }
          else
            itm->itm_pcomp = NULL;
          if (arr != smallarr)
            GC_FREE (arr);
        }
      else if (curtok->mtok_itm == MOM_PREDEFITM (resize))
        {
          topos++;
          if (topos < tolen
              && (curtok = tovec->mtv_arr + topos)
              && curtok->mtok_kind == MOLEX_INT)
            {
              int newsize = curtok->mtok_int;
              itm->itm_pcomp =
                mom_vectvaldata_resize (itm->itm_pcomp, newsize);
              topos++;
            }
          else
            MOM_FATAPRINTF
              ("int expected after ^resize for ^item %s at line %d of file %s",
               mom_item_cstring (itm), curtok->mtok_lin, tovec->mtv_filename);
        }
      else if (curtok->mtok_itm == MOM_PREDEFITM (global))
        {
          topos++;
          if (mom_item_space (itm) == MOMSPA_PREDEF)
            MOM_WARNPRINTF
              ("item %s already predefined cannot be made global",
               mom_item_cstring (itm));
          else
            {
              mom_item_put_space (itm, MOMSPA_GLOBAL);
              MOM_INFORMPRINTF ("item %s is made global",
                                mom_item_cstring (itm));
            }
        }
      else
        MOM_FATAPRINTF
          ("bad operator ^%s after ^item %s at line %d of file %s",
           mom_item_cstring (curtok->mtok_itm), mom_item_cstring (itm),
           curtok->mtok_lin, tovec->mtv_filename);
    }
  if (topos < tolen && tovec->mtv_arr[topos].mtok_kind == MOLEX_DELIM
      && tovec->mtv_arr[topos].mtok_delim == MODLM_RPAR)
    topos++;
  itm->itm_mtime = time (NULL);
  mom_item_unlock (itm);
  MOM_DEBUGPRINTF (boot, "momtok_inside_item end itm:%s topos#%d",
                   mom_item_cstring (itm), topos);
  *endposptr = topos;
}                               /* end momtok_inside_item */


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
  errno = 0;
  struct momtoken_st *curtok = tovec->mtv_arr + topos;
  MOM_DEBUGPRINTF (boot, "momtok_parse file %s topos#%d curtok: %s",
                   tovec->mtv_filename, topos, momtok_gcstr (curtok));
  switch (curtok->mtok_kind)
    {
    case MOLEX_INT:
      res = mom_int_make (curtok->mtok_int);
      MOM_DEBUGPRINTF (boot, "momtok_parse topos#%d int %s", topos,
                       mom_value_cstring (res));
      *endposptr = topos + 1;
      return res;
    case MOLEX_ITEM:
      res = curtok->mtok_itm;
      MOM_DEBUGPRINTF (boot, "momtok_parse topos#%d item %s", topos,
                       mom_item_cstring (res));
      *endposptr = topos + 1;
      return res;
    case MOLEX_STRING:
      res = curtok->mtok_str;
      MOM_DEBUGPRINTF (boot, "momtok_parse topos#%d string %s", topos,
                       mom_value_cstring (res));
      *endposptr = topos + 1;
      return res;
    case MOLEX_NAMESTR:
      {
        res =
          mom_hashassoc_get (tovec->mtv_dict,
                             (const void *) curtok->mtok_str);
        MOM_DEBUGPRINTF (boot, "momtok_parse topos#%d name %s is %s", topos,
                         mom_value_cstring ((const void *) curtok->mtok_str),
                         mom_value_cstring (res));
        *endposptr = topos + 1;
        return res;
      }
    default:
      break;
    }
  if (curtok->mtok_kind == MOLEX_DELIM && curtok->mtok_delim == MODLM_LBRACE)
    {
      int ln = 0;
      topos++;
      MOM_DEBUGPRINTF (boot, "momtok_parse start set topos#%d", topos);
      const struct mom_item_st *smallitemarr[8] = { };
      const struct mom_item_st **itemarr = smallitemarr;
      unsigned itemsiz = sizeof (smallitemarr) / sizeof (smallitemarr[0]);
      while (topos < tolen
             && ((curtok = tovec->mtv_arr + topos)->mtok_kind != MOLEX_DELIM
                 || curtok->mtok_delim != MODLM_RBRACE))
        {
          MOM_DEBUGPRINTF (boot, "momtok_parse set ln=%d topos#%d curtok %s",
                           ln, topos, momtok_gcstr (curtok));
          if (ln + 1 >= (int) itemsiz)
            {
              unsigned newsiz = mom_prime_above (5 * ln / 4 + 3);
              const struct mom_item_st **newitemarr =
                mom_gc_alloc (newsiz * sizeof (struct mom_item_st *));
              memcpy (newitemarr, itemarr,
                      ln * sizeof (struct mom_item_st *));
              if (itemarr != smallitemarr)
                GC_FREE (itemarr);
              itemarr = newitemarr;
              itemsiz = newsiz;
            };
          const void *itv = momtok_parse (tovec, topos, &topos);
          if (mom_itype (itv) != MOMITY_ITEM)
            MOM_FATAPRINTF
              ("parsing set, expecting item after %s in file %s but got %s",
               momtok_gcstr (curtok), tovec->mtv_filename,
               mom_value_cstring (itv));
          itemarr[ln] = (const struct mom_item_st *) itv;
          ln++;
        };
      res = mom_boxset_make_arr (ln, itemarr);
      if (itemarr != smallitemarr)
        GC_FREE (itemarr);
      *endposptr = topos + 1;
      MOM_DEBUGPRINTF (boot, "momtok_parse topos#%d set %s endpos#%d", topos,
                       mom_value_cstring (res), *endposptr);
      return res;
    }
  if (curtok->mtok_kind == MOLEX_DELIM
      && curtok->mtok_delim == MODLM_LBRACKET)
    {
      int ln = 0;
      topos++;
      MOM_DEBUGPRINTF (boot, "momtok_parse start tuple topos#%d tok:%s",
                       topos, momtok_gcstr (momtok_nth_token (tovec, topos)));
      const struct mom_item_st *smallitemarr[8] = { };
      const struct mom_item_st **itemarr = smallitemarr;
      unsigned itemsiz = sizeof (smallitemarr) / sizeof (smallitemarr[0]);
      while (topos < tolen
             && ((curtok = tovec->mtv_arr + topos)->mtok_kind != MOLEX_DELIM
                 || curtok->mtok_delim != MODLM_RBRACKET))
        {
          MOM_DEBUGPRINTF (boot,
                           "momtok_parse tuple ln=%d topos#%d curtok %s", ln,
                           topos, momtok_gcstr (curtok));
          if (ln + 1 >= (int) itemsiz)
            {
              unsigned newsiz = mom_prime_above (5 * ln / 4 + 3);
              const struct mom_item_st **newitemarr =
                mom_gc_alloc (newsiz * sizeof (struct mom_item_st *));
              memcpy (newitemarr, itemarr,
                      ln * sizeof (struct mom_item_st *));
              if (itemarr != smallitemarr)
                GC_FREE (itemarr);
              itemarr = newitemarr;
              itemsiz = newsiz;
            };
          int oldtopos = topos;
          const void *itv = momtok_parse (tovec, topos, &topos);
          if ((itv && mom_itype (itv) != MOMITY_ITEM) || oldtopos == topos)
            MOM_FATAPRINTF
              ("parsing tuple, expecting item or nil after %s in file %s but got %s",
               momtok_gcstr (curtok), tovec->mtv_filename,
               mom_value_cstring (itv));
          itemarr[ln] = (const struct mom_item_st *) itv;
          ln++;
        };
      res = mom_boxtuple_make_arr (ln, itemarr);
      *endposptr = topos + 1;
      if (itemarr != smallitemarr)
        GC_FREE (itemarr);
      MOM_DEBUGPRINTF (boot,
                       "momtok_parse topos#%d  tok:%s tuple %s endpos %d",
                       topos, momtok_gcstr (momtok_nth_token (tovec, topos)),
                       mom_value_cstring (res), *endposptr);
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
      MOM_DEBUGPRINTF (boot, "momtok_parse node connitm=%s topos#%d tok:%s",
                       mom_item_cstring (connitm), topos,
                       momtok_gcstr (momtok_nth_token (tovec, topos)));
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
              unsigned newsiz = mom_prime_above ((4 * nbsons) / 3 + 4);
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
        res = mom_boxnode_make_meta (connitm, nbsons, arr, metaitm, metarank);
      else
        res = mom_boxnode_make (connitm, nbsons, arr);
      MOM_DEBUGPRINTF (boot, "momtok_parse topos#%d node %s", topos,
                       mom_value_cstring (res));
      if (arr != smallarr)
        GC_FREE (arr), arr = NULL;
      *endposptr = topos;
      return res;
    }
  if (curtok->mtok_kind == MOLEX_DELIM && curtok->mtok_delim == MODLM_TILDE)
    {
      res = NULL;
      *endposptr = topos + 1;
      MOM_DEBUGPRINTF (boot, "momtok_parse topos#%d nil", topos);
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
          MOM_DEBUGPRINTF (boot, "momtok_parse topos#%d get %s", topos,
                           mom_value_cstring (res));
          return res;
        }
      const void *atval = momtok_parse (tovec, topos, &topos);
      unsigned atyp = mom_itype (atval);
      if (atyp == MOMITY_INT)
        {
          int rk = mom_int_val_def (atval, 0);
          mom_item_lock (fromitm);
          res = mom_vectvaldata_nth (fromitm->itm_pcomp, rk);
          mom_item_unlock (fromitm);
          *endposptr = topos;
          MOM_DEBUGPRINTF (boot, "momtok_parse topos#%d get %s", topos,
                           mom_value_cstring (res));
          return res;
        }
      else if (atyp == MOMITY_ITEM)
        {
          struct mom_item_st *atitm = mom_dyncast_item (atval);
          mom_item_lock (fromitm);
          res = mom_unsync_item_get_phys_attr (fromitm, atitm);
          mom_item_unlock (fromitm);
          MOM_DEBUGPRINTF (boot, "momtok_parse topos#%d get %s", topos,
                           mom_value_cstring (res));
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
      && curtok->mtok_itm == MOM_PREDEFITM (name))
    {
      topos++;
      const struct mom_boxstring_st *namstr = NULL;
      if (topos < tolen && tovec->mtv_arr[topos].mtok_kind == MOLEX_NAMESTR)
        namstr = tovec->mtv_arr[topos].mtok_str;
      else
        MOM_FATAPRINTF
          ("bad ^name (expecting $SOMENAME) at line %d of file %s",
           curtok->mtok_lin, tovec->mtv_filename);
      topos++;
      const void *nval = momtok_parse (tovec, topos, &topos);
      if (!nval)
        MOM_FATAPRINTF ("^name %s bound to nil at line %d of file %s",
                        mom_value_cstring ((const void *) namstr),
                        curtok->mtok_lin, tovec->mtv_filename);
      tovec->mtv_dict =
        mom_hashassoc_put (tovec->mtv_dict, (const void *) namstr, nval);
      MOM_DEBUGPRINTF (boot,
                       "momtok_parse bound name %s to value %s topos#%d",
                       mom_value_cstring ((const void *) namstr),
                       mom_value_cstring (nval), topos);
      *endposptr = topos;
      return nval;
    }
  if (curtok->mtok_kind == MOLEX_OPERITEM
      && curtok->mtok_itm == MOM_PREDEFITM (put))
    {
      topos++;
      struct mom_item_st *fromitm =
        mom_dyncast_item (momtok_parse (tovec, topos, &topos));
      MOM_DEBUGPRINTF (boot,
                       "momtok_parse ^put fromitm=%s topos#%d",
                       mom_item_cstring (fromitm), topos);
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
          MOM_DEBUGPRINTF (boot,
                           "momtok_parse ^put fromitm=%s atrk#%d",
                           mom_item_cstring (fromitm), atrk);
        }
      else
        atval = momtok_parse (tovec, topos, &topos);
      MOM_DEBUGPRINTF (boot,
                       "momtok_parse ^put atval=%s atrk#%d atindex %s topos#%d",
                       mom_value_cstring (atval), atrk,
                       atindex ? "true" : "false", topos);
      newval = momtok_parse (tovec, topos, &topos);
      MOM_DEBUGPRINTF (boot,
                       "momtok_parse ^put fromitm=%s newval=%s topos#%d atindex %s ",
                       mom_item_cstring (fromitm),
                       mom_value_cstring (newval), topos,
                       atindex ? "true" : "false");
      if (atindex)
        {
          mom_item_lock (fromitm);
          mom_vectvaldata_put_nth (fromitm->itm_pcomp, atrk, newval);
          fromitm->itm_mtime = time (NULL);
          mom_item_unlock (fromitm);
          *endposptr = topos;
          MOM_DEBUGPRINTF (boot,
                           "momtok_parse ^put updated atindex topos#%d fromitm:=%s",
                           topos, mom_item_content_cstring (fromitm));
          return fromitm;
        }
      unsigned atyp = mom_itype (atval);
      if (atyp == MOMITY_INT)
        {
          int rk = mom_int_val_def (atval, 0);
          mom_item_lock (fromitm);
          mom_vectvaldata_put_nth (fromitm->itm_pcomp, rk, newval);
          fromitm->itm_mtime = time (NULL);
          mom_item_unlock (fromitm);
          *endposptr = topos;
          MOM_DEBUGPRINTF (boot,
                           "momtok_parse ^put updated rk#%d topos#%d fromitm:=%s",
                           rk, topos, mom_item_content_cstring (fromitm));
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
          MOM_DEBUGPRINTF (boot,
                           "momtok_parse ^put updated atitm=%s topos#%d fromitm:=%s",
                           mom_item_cstring (atitm),
                           topos, mom_item_content_cstring (fromitm));
          return fromitm;
        }
      else if (atyp != MOMITY_NONE)
        MOM_FATAPRINTF ("bad ^put at line %d of file %s from %s at %s",
                        curtok->mtok_lin, tovec->mtv_filename,
                        mom_item_cstring (fromitm),
                        mom_value_cstring (atval));
      *endposptr = topos;
      MOM_DEBUGPRINTF (boot, "momtok_parse topos#%d did put fromitm:=%s",
                       topos, mom_item_content_cstring (fromitm));
      return fromitm;
    }
  if (curtok->mtok_kind == MOLEX_OPERITEM
      && curtok->mtok_itm == MOM_PREDEFITM (display))
    {
      MOM_DEBUGPRINTF (boot, "momtok_parse ^display topos=%d tolen=%d", topos,
                       tolen);
      topos++;
      const struct mom_boxstring_st *msgv = NULL;
      if (topos < tolen && (curtok = tovec->mtv_arr + topos)
          && curtok->mtok_kind == MOLEX_STRING)
        msgv = tovec->mtv_arr[topos].mtok_str;
      else
        MOM_FATAPRINTF
          ("^display at line %d of file %s not followed by message string but by %s",
           curtok->mtok_lin, tovec->mtv_filename, momtok_gcstr (curtok));
      MOM_DEBUGPRINTF (boot, "momtok_parse topos#%d msgv=%s", topos,
                       mom_value_cstring ((void *) msgv));
      topos++;
      const void *dispval = momtok_parse (tovec, topos, &topos);
      MOM_DEBUGPRINTF (boot, "momtok_parse topos#%d dispval:%s", topos,
                       mom_value_cstring (dispval));
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
      MOM_DEBUGPRINTF (boot, "momtok_parse topos#%d display %s", topos,
                       mom_value_cstring (dispval));
      return dispval;
    }
  if (curtok->mtok_kind == MOLEX_OPERITEM
      && curtok->mtok_itm == MOM_PREDEFITM (resize))
    {
      topos++;
      struct mom_item_st *gritm =
        mom_dyncast_item (momtok_parse (tovec, topos, &topos));
      MOM_DEBUGPRINTF (boot, "momtok_parse topos#%d gritm:%s", topos,
                       mom_item_cstring (gritm));
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
      if (atyp == MOMITY_INT)
        {
          int rk = mom_int_val_def (atval, 0);
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
      MOM_DEBUGPRINTF (boot, "momtok_parse topos#%d resize %s", topos,
                       mom_item_cstring (gritm));
      return gritm;
    }
  if (curtok->mtok_kind == MOLEX_OPERITEM
      && curtok->mtok_itm == MOM_PREDEFITM (item))
    {
      topos++;
      struct mom_item_st *itm =
        mom_dyncast_item (momtok_parse (tovec, topos, &topos));

      if (!itm)
        MOM_FATAPRINTF ("bad item for ^item at line %d of file %s",
                        curtok->mtok_lin, tovec->mtv_filename);
      momtok_inside_item (itm, tovec, topos, &topos);
      *endposptr = topos;
      MOM_DEBUGPRINTF (boot, "topos#%d done item %s", topos,
                       mom_item_cstring (itm));
      return itm;
    }

  if (curtok->mtok_kind == MOLEX_OPERITEM
      && curtok->mtok_itm == MOM_PREDEFITM (global))
    {
      topos++;
      struct mom_item_st *itm =
        mom_dyncast_item (momtok_parse (tovec, topos, &topos));
      if (!itm)
        MOM_FATAPRINTF ("bad item for ^global at line %d of file %s",
                        curtok->mtok_lin, tovec->mtv_filename);
      if (mom_item_space (itm) == MOMSPA_PREDEF)
        MOM_WARNPRINTF ("item %s already predefined cannot be made global",
                        mom_item_cstring (itm));
      else
        {
          mom_item_put_space (itm, MOMSPA_GLOBAL);
        }
      momtok_inside_item (itm, tovec, topos, &topos);
      *endposptr = topos;
      MOM_DEBUGPRINTF (boot, "topos#%d done global %s", topos,
                       mom_item_cstring (itm));
      return itm;
    }

  MOM_FATAPRINTF ("syntax error line %d file %s", curtok->mtok_lin,
                  tovec->mtv_filename);
}                               // end momtok_parse


void
mom_boot_file (const char *path)
{
  MOM_DEBUGPRINTF (boot, "mom_boot_file start path=%s", path);
  struct momtokvect_st *tokvec = momtok_tokenize (path);
  assert (tokvec != NULL);
  int tolen = tokvec->mtv_len;
  MOM_DEBUGPRINTF (boot, "mom_boot_file file %s tolen=%d",
                   tokvec->mtv_filename, tolen);
  if (MOM_IS_DEBUGGING (boot))
    {
      for (int ix = 0; ix < tolen; ix++)
        MOM_DEBUGPRINTF (boot, "token#%d: %s",
                         ix, momtok_gcstr (tokvec->mtv_arr + ix));
    }
  int topos = 0;
  int nbval = 0;
  while (topos < tolen)
    {
      MOM_DEBUGPRINTF (boot, "mom_boot_file topos#%d before parse nbval#%d",
                       topos, nbval);
      const void *vp = momtok_parse (tokvec, topos, &topos);
      MOM_DEBUGPRINTF (boot,
                       "mom_boot_file topos#%d after parse nbval#%d vp:%s",
                       topos, nbval, mom_value_cstring (vp));
      nbval++;
    };
  MOM_INFORMPRINTF ("booted file %s with %d tokens & %d values",
                    path, tolen, nbval);
  MOM_DEBUGPRINTF (boot, "mom_boot_file end path=%s", path);
}                               /* end mom_boot_file */



// eof bparse.c

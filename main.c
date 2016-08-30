// file main.c - main program and utilities

/**   Copyright (C)  2015 - 2016  Basile Starynkevitch and later the FSF
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

#define BASE_YEAR_MOM 2015


#define MAX_ADDED_PREDEF_MOM 16
struct predefadd_mom_st
{
  const char *predef_name;
  const char *predef_comment;
};
static struct predefadd_mom_st added_predef_mom[MAX_ADDED_PREDEF_MOM];
static unsigned count_added_predef_mom;

void *mom_prog_dlhandle;

const char *
mom_hostname (void)
{
  static char hostname_mom[80];
  if (MOM_UNLIKELY (hostname_mom[0] == (char) 0))
    {
      if (gethostname (hostname_mom, sizeof (hostname_mom) - 1))
        MOM_FATAPRINTF ("gethostname failed");
    }
  return hostname_mom;
};

void
mom_output_gplv3_notice (FILE *out, const char *prefix, const char *suffix,
                         const char *filename)
{
  time_t now = 0;
  time (&now);
  struct tm nowtm;
  memset (&nowtm, 0, sizeof (nowtm));
  localtime_r (&now, &nowtm);
  if (!prefix)
    prefix = "";
  if (!suffix)
    suffix = "";
  fprintf (out, "%s *** generated file %s - DO NOT EDIT %s\n", prefix,
           filename, suffix);
  if (1900 + nowtm.tm_year != BASE_YEAR_MOM)
    fprintf (out,
             "%s Copyright (C) %d - %d Free Software Foundation, Inc. %s\n",
             prefix, BASE_YEAR_MOM, 1900 + nowtm.tm_year, suffix);
  else
    fprintf (out,
             "%s Copyright (C) %d Basile Starynkevitch & later the Free Software Foundation, Inc. %s\n",
             prefix, BASE_YEAR_MOM, suffix);
  fprintf (out,
           "%s MONIMELT is a monitor for MELT - see http://gcc-melt.org/ %s\n",
           prefix, suffix);
  fprintf (out,
           "%s This generated file %s is part of MONIMELT, part of GCC %s\n",
           prefix, filename, suffix);
  fprintf (out, "%s%s\n", prefix, suffix);
  fprintf (out,
           "%s GCC is free software; you can redistribute it and/or modify %s\n",
           prefix, suffix);
  fprintf (out,
           "%s it under the terms of the GNU General Public License as published by %s\n",
           prefix, suffix);
  fprintf (out,
           "%s the Free Software Foundation; either version 3, or (at your option) %s\n",
           prefix, suffix);
  fprintf (out, "%s any later version. %s\n", prefix, suffix);
  fprintf (out, "%s%s\n", prefix, suffix);
  fprintf (out,
           "%s  GCC is distributed in the hope that it will be useful, %s\n",
           prefix, suffix);
  fprintf (out,
           "%s  but WITHOUT ANY WARRANTY; without even the implied warranty of %s\n",
           prefix, suffix);
  fprintf (out,
           "%s  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the %s\n",
           prefix, suffix);
  fprintf (out, "%s  GNU General Public License for more details. %s\n",
           prefix, suffix);
  fprintf (out,
           "%s  You should have received a copy of the GNU General Public License %s\n",
           prefix, suffix);
  fprintf (out,
           "%s  along with GCC; see the file COPYING3.   If not see %s\n",
           prefix, suffix);
  fprintf (out, "%s  <http://www.gnu.org/licenses/>. %s\n", prefix, suffix);
  fprintf (out, "%s%s\n", prefix, suffix);
}                               /* end mom_output_gplv3_notice */



const char *
mom_gc_printf (const char *fmt, ...)
{
  char *buf = NULL;
  const char *res = NULL;
  char smallbuf[96];
  int ln = 0;
  memset (smallbuf, 0, sizeof (smallbuf));
  va_list args;
  va_start (args, fmt);
  ln = vsnprintf (smallbuf, sizeof (smallbuf), fmt, args);
  va_end (args);
  if (ln >= 0 && ln < (int) sizeof (smallbuf) - 1)
    res = mom_gc_strdup (smallbuf);
  else
    {
      res = mom_gc_alloc_scalar (1 + ((ln + 2) | 3));
      va_start (args, fmt);
      if (MOM_UNLIKELY (vsnprintf ((char *) res, ln + 1, fmt, args) != ln))
        MOM_FATAPRINTF ("vsnprintf failure with fmt=%s (%m)", fmt);
      va_end (args);
    }
  if (buf)
    free (buf);
  return res;
}                               /* end of mom_gc_printf */


void
mom_output_utf8_escaped (FILE *f, const char *str, int len,
                         mom_utf8escape_sig_t * rout, void *clientdata)
{
  if (!f)
    return;
  if (!str)
    return;
  assert (rout != NULL);
  if (len < 0)
    len = strlen (str);
  const char *end = str + len;
  gunichar uc = 0;
  const char *s = str;
  assert (s && g_utf8_validate (s, len, NULL));
  assert (s && g_utf8_validate (s, len, NULL));
  for (const char *pc = s; pc < end; pc = g_utf8_next_char (pc), uc = 0)
    {
      uc = g_utf8_get_char (pc);
      switch (uc)
        {
        case 0:
          (*rout) (f, uc, "\\0", clientdata);
          break;
        case '\"':
          (*rout) (f, uc, "\\\"", clientdata);
          break;
        case '\'':
          (*rout) (f, uc, "\\\'", clientdata);
          break;
        case '&':
          (*rout) (f, uc, "&", clientdata);
          break;
        case '<':
          (*rout) (f, uc, "<", clientdata);
          break;
        case '>':
          (*rout) (f, uc, ">", clientdata);
          break;
          break;
        case '\a':
          (*rout) (f, uc, "\\a", clientdata);
          break;
        case '\b':
          (*rout) (f, uc, "\\b", clientdata);
          break;
        case '\f':
          (*rout) (f, uc, "\\f", clientdata);
          break;
        case '\n':
          (*rout) (f, uc, "\\n", clientdata);
          break;
        case '\r':
          (*rout) (f, uc, "\\r", clientdata);
          break;
        case '\t':
          (*rout) (f, uc, "\\t", clientdata);
          break;
        case '\v':
          (*rout) (f, uc, "\\v", clientdata);
          break;
        case '\033' /*ESCAPE*/:
          (*rout) (f, uc, "\\e", clientdata);
          break;
        case '0' ... '9':
        case 'A' ... 'Z':
        case 'a' ... 'z':
        case '+':
        case '-':
        case '*':
        case '/':
        case ',':
        case ';':
        case '.':
        case ':':
        case '^':
        case '(':
        case ')':
        case '[':
        case ']':
        case '{':
        case '}':
        case ' ':
          goto print1char;
        default:
          if (uc < 127 && uc > 0 && isprint ((char) uc))
          print1char:
            fputc ((char) uc, f);
          else
            {
              char buf[16];
              memset (buf, 0, sizeof (buf));
              if (uc <= 0xffff)
                {
                  snprintf (buf, sizeof (buf), "\\u%04x", (int) uc);
                }
              else
                {
                  snprintf (buf, sizeof (buf), "\\U%08x", (int) uc);
                }
              (*rout) (f, uc, buf, clientdata);
            }
          break;
        }
    };
}                               /* end of mom_output_utf8_escaped */




void
mom_output_utf8_encoded (FILE *f, const char *str, int len)
{
  if (!f)
    return;

  if (len < 0)
    len = strlen (str);
  const char *end = str + len;
  gunichar uc = 0;
  const char *s = str;
  assert (s && g_utf8_validate (s, len, NULL));
  for (const char *pc = s; pc < end; pc = g_utf8_next_char (pc), uc = 0)
    {
      /// notice that the single quote should not be escaped, e.g. for JSON
      uc = g_utf8_get_char (pc);
      switch (uc)
        {
        case 0:
          fputs ("\\0", f);
          break;
        case '\"':
          fputs ("\\\"", f);
          break;
        case '\\':
          fputs ("\\\\", f);
          break;
        case '\a':
          fputs ("\\a", f);
          break;
        case '\b':
          fputs ("\\b", f);
          break;
        case '\f':
          fputs ("\\f", f);
          break;
        case '\n':
          fputs ("\\n", f);
          break;
        case '\r':
          fputs ("\\r", f);
          break;
        case '\t':
          fputs ("\\t", f);
          break;
        case '\v':
          fputs ("\\v", f);
          break;
        case '\033' /*ESCAPE*/:
          fputs ("\\e", f);
          break;
        default:
          if (uc < 127 && uc > 0 && isprint ((char) uc))
            fputc ((char) uc, f);
          else
            {
              if (uc <= 0xffff)
                {
                  fprintf (f, "\\u%04x", (int) uc);
                }
              else
                {
                  fprintf (f, "\\U%08x", (int) uc);
                }
            }
          break;
        }
    }
}                               // end mom_output_utf8_encoded


void
mom_output_utf8_html (FILE *f, const char *str, int len, bool nlisbr)
{
  if (!f)
    return;
  if (len < 0)
    len = strlen (str);
  const char *end = str + len;
  gunichar uc = 0;
  const char *s = str;
  assert (s && g_utf8_validate (s, len, NULL));
  for (const char *pc = s; pc < end; pc = g_utf8_next_char (pc), uc = 0)
    {
      uc = g_utf8_get_char (pc);
      switch (uc)
        {
        case '\'':
          fputs ("&apos;", f);
          break;
        case '\"':
          fputs ("&quot;", f);
          break;
        case '<':
          fputs ("&lt;", f);
          break;
        case '>':
          fputs ("&gt;", f);
          break;
        case '&':
          fputs ("&amp;", f);
          break;
        case '\n':
          if (nlisbr)
            fputs ("<br/>", f);
          else
            fputc ('\n', f);
          break;
        case ' ':
        case 'a' ... 'z':
        case 'A' ... 'Z':
        case '0' ... '9':
        case '+':
        case '-':
        case '*':
        case '/':
        case ',':
        case ';':
        case '.':
        case ':':
        case '^':
        case '(':
        case ')':
        case '[':
        case ']':
        case '{':
        case '}':
          fputc ((char) uc, f);
          break;
        default:
          if (uc < 127 && isprint (uc))
            fputc ((char) uc, f);
          else
            fprintf (f, "&#%d;", uc);
          break;
        }
    }
}                               /* end of mom_output_utf8_html */


const char *
mom_hexdump_data (char *buf, unsigned buflen, const unsigned char *data,
                  unsigned datalen)
{
  if (!buf || !data)
    return NULL;
  if (2 * datalen + 3 < buflen)
    {
      for (unsigned ix = 0; ix < datalen; ix++)
        snprintf (buf + 2 * ix, 3, "%02x", (unsigned) data[ix]);
    }
  else
    {
      unsigned maxln = (buflen - 3) / 2;
      if (maxln > datalen)
        maxln = datalen;
      for (unsigned ix = 0; ix < maxln; ix++)
        snprintf (buf + 2 * ix, 3, "%02x", (unsigned) data[ix]);
      if (maxln > datalen)
        strcpy (buf + 2 * maxln, "..");
    }
  return buf;
}                               /* end of mom_hexdump_data */



struct mom_string_and_size_st
mom_input_quoted_utf8 (FILE *f)
{
  struct mom_string_and_size_st ss = { NULL, 0 };
  if (!f)
    return ss;
  char bufarr[64];
  int bufsiz = sizeof (bufarr);
  char *bufzon = bufarr;
  int bufoff = 0;
  long off = ftell (f);
  do
    {
      if (MOM_UNLIKELY (bufoff > INT32_MAX / 3))
        {
          fseek (f, off, SEEK_SET);
          MOM_FATAPRINTF ("too long (%d) input quoted UTF-8 string %.50s...",
                          bufoff, bufzon);
        }
      if (MOM_UNLIKELY (bufoff + 9 >= bufsiz))
        {
          int newsiz = ((6 * bufsiz / 5 + 30) | 0x1f) + 1;
          char *newbuf = mom_gc_alloc_scalar (newsiz);
          memcpy (newbuf, bufzon, bufoff);
          if (bufzon != bufarr)
            GC_FREE (bufzon);
          bufzon = newbuf;
          bufsiz = newsiz;
        };
      int c = fgetc (f);
      if (c == EOF)
        break;
      if (iscntrl (c) || c == '"')
        {
          ungetc (c, f);
          break;
        }
      else if (c == '\\')
        {
          int pos = -1;
          unsigned b = 0;
          int nc = fgetc (f);
          if (nc == EOF || iscntrl (nc))
            {
              if (nc != EOF)
                ungetc (nc, f);
              break;
            }
          switch (nc)
            {
            case '\'':
            case '\"':
            case '\\':
              bufzon[bufoff++] = nc;
              break;
            case 'a':
              bufzon[bufoff++] = '\a';
              break;
            case 'b':
              bufzon[bufoff++] = '\b';
              break;
            case 'f':
              bufzon[bufoff++] = '\f';
              break;
            case 'n':
              bufzon[bufoff++] = '\n';
              break;
            case 'r':
              bufzon[bufoff++] = '\r';
              break;
            case 't':
              bufzon[bufoff++] = '\t';
              break;
            case 'v':
              bufzon[bufoff++] = '\v';
              break;
            case 'e':
              bufzon[bufoff++] = '\033' /* ESCAPE */ ;
              break;
            case 'x':
              if (fscanf (f, "%02x%n", &b, &pos) > 0 && pos > 0)
                bufzon[bufoff++] = b;
              break;
            case 'u':
              if (fscanf (f, "%04x%n", &b, &pos) > 0 && pos > 0)
                {
                  char ebuf[8];
                  memset (ebuf, 0, sizeof (ebuf));
                  g_unichar_to_utf8 ((gunichar) b, ebuf);
                  strcpy (bufzon + bufoff, ebuf);
                  bufoff += strlen (ebuf);
                };
              break;
            case 'U':
              if (fscanf (f, "%08x%n", &b, &pos) > 0 && pos > 0)
                {
                  char ebuf[8];
                  memset (ebuf, 0, sizeof (ebuf));
                  g_unichar_to_utf8 ((gunichar) b, ebuf);
                  strcpy (bufzon + bufoff, ebuf);
                  bufoff += strlen (ebuf);
                };
              break;
            default:
              bufzon[bufoff++] = nc;
              break;
            }
          continue;
        }
      else
        {
          bufzon[bufoff++] = c;
          continue;
        }
    }
  while (!feof (f));
  char *res = mom_gc_alloc_scalar (bufoff + 1);
  memcpy (res, bufzon, bufoff);
  if (bufzon != bufarr)
    GC_FREE (bufzon);
  ss.ss_str = res;
  ss.ss_len = bufoff;
  return ss;
}                               /* end of mom_input_quoted_utf8 */




/************************* inform *************************/

void
mom_informprintf_at (const char *fil, int lin, const char *fmt, ...)
{
  int len = 0;
  char thrname[24];
  char buf[160];
  char timbuf[64];
  char *bigbuf = NULL;
  char *msg = NULL;
  memset (buf, 0, sizeof (buf));
  memset (thrname, 0, sizeof (thrname));
  memset (timbuf, 0, sizeof (timbuf));
  pthread_getname_np (pthread_self (), thrname, sizeof (thrname) - 1);
  fflush (NULL);
  mom_now_strftime_bufcenti (timbuf, "%Y-%b-%d %H:%M:%S.__ %Z");
  va_list alist;
  va_start (alist, fmt);
  len = vsnprintf (buf, sizeof (buf), fmt, alist);
  va_end (alist);
  if (MOM_UNLIKELY (len >= (int) sizeof (buf) - 1))
    {
      bigbuf = malloc (len + 10);
      if (bigbuf)
        {
          memset (bigbuf, 0, len + 10);
          va_start (alist, fmt);
          (void) vsnprintf (bigbuf, len + 1, fmt, alist);
          va_end (alist);
          msg = bigbuf;
        }
    }
  msg = buf;
  {
    fprintf (stderr, "MONIMELT INFORM @%s:%d <%s:%d> %s %s\n",
             fil, lin, thrname, (int) mom_gettid (), timbuf, msg);
    fflush (NULL);
  }
  if (bigbuf)
    free (bigbuf);
}

/************************* warning *************************/

atomic_int mom_nb_warnings;
void
mom_warnprintf_at (const char *fil, int lin, const char *fmt, ...)
{
  int len = 0;
  char thrname[24];
  char buf[160];
  char timbuf[64];
  char *bigbuf = NULL;
  char *msg = NULL;
  int err = errno;
  int nbwarn = 1 + atomic_fetch_add (&mom_nb_warnings, 1);
  memset (buf, 0, sizeof (buf));
  memset (thrname, 0, sizeof (thrname));
  memset (timbuf, 0, sizeof (timbuf));
  pthread_getname_np (pthread_self (), thrname, sizeof (thrname) - 1);
  fflush (NULL);
  mom_now_strftime_bufcenti (timbuf, "%Y-%b-%d %H:%M:%S.__ %Z");
  va_list alist;
  va_start (alist, fmt);
  len = vsnprintf (buf, sizeof (buf), fmt, alist);
  va_end (alist);
  if (MOM_UNLIKELY (len >= (int) sizeof (buf) - 2))
    {
      bigbuf = malloc (len + 10);
      if (bigbuf)
        {
          memset (bigbuf, 0, len + 10);
          va_start (alist, fmt);
          (void) vsnprintf (bigbuf, len + 1, fmt, alist);
          va_end (alist);
          msg = bigbuf;
        }
    }
  else
    msg = buf;
  {
    if (err)
      fprintf (stderr, "MONIMELT WARNING#%d @%s:%d <%s:%d> %s %s (%s)\n",
               nbwarn, fil, lin, thrname, (int) mom_gettid (), timbuf,
               msg, strerror (err));
    else
      fprintf (stderr, "MONIMELT WARNING#%d @%s:%d <%s:%d> %s %s\n",
               nbwarn, fil, lin, thrname, (int) mom_gettid (), timbuf, msg);
    fflush (NULL);
  }
  if (bigbuf)
    free (bigbuf);
}                               /* end of mom_warnprintf_at */


/************************* fatal *************************/

/* A callback function passed to the backtrace_full function.  */

static int
mom_bt_callback (void *data, uintptr_t pc, const char *filename, int lineno,
                 const char *function)
{
  int *pcount = (int *) data;

  /* If we don't have any useful information, don't print
     anything.  */
  if (filename == NULL && function == NULL)
    return 0;

  /* Print up to 40 functions.    */
  if (*pcount >= 40)
    {
      /* Returning a non-zero value stops the backtrace.  */
      return 1;
    }
  ++*pcount;


  fprintf (stderr, "MonimeltB[0x%lx] %s\n\t%s:%d\n",
           (unsigned long) pc,
           function == NULL ? "???" : function,
           filename == NULL ? "???" : filename, lineno);

  return 0;
}

/* An error callback function passed to the backtrace_full function.  This is
   called if backtrace_full has an error.  */

static void
mom_bt_err_callback (void *data MOM_UNUSED, const char *msg, int errnum)
{
  if (errnum < 0)
    {
      /* This means that no debug info was available.  Just quietly
         skip printing backtrace info.  */
      return;
    }
  fprintf (stderr, "%s%s%s\n", msg, errnum == 0 ? "" : ": ",
           errnum == 0 ? "" : strerror (errnum));
}

void
mom_fataprintf_at (const char *fil, int lin, const char *fmt, ...)
{
  int len = 0;
  char thrname[24];
  char buf[256];
  char timbuf[64];
  char *bigbuf = NULL;
  char *msg = NULL;
  int err = errno;
  memset (buf, 0, sizeof (buf));
  memset (thrname, 0, sizeof (thrname));
  memset (timbuf, 0, sizeof (timbuf));
  pthread_getname_np (pthread_self (), thrname, sizeof (thrname) - 1);
  mom_now_strftime_bufcenti (timbuf, "%Y-%b-%d %H:%M:%S.__ %Z");
  fflush (NULL);
  va_list alist;
  va_start (alist, fmt);
  len = vsnprintf (buf, sizeof (buf), fmt, alist);
  va_end (alist);
  if (MOM_UNLIKELY (len >= (int) sizeof (buf) - 1))
    {
      bigbuf = malloc (len + 10);
      if (bigbuf)
        {
          memset (bigbuf, 0, len + 10);
          va_start (alist, fmt);
          (void) vsnprintf (bigbuf, len + 1, fmt, alist);
          va_end (alist);
          msg = bigbuf;
        }
    }
  else
    msg = buf;

  if (err)
    fprintf (stderr, "MONIMELT FATAL @%s:%d <%s:%d> %s %s (%s)\n",
             fil, lin, thrname, (int) mom_gettid (), timbuf,
             msg, strerror (err));
  else
    fprintf (stderr, "MONIMELT FATAL @%s:%d <%s:%d> %s %s\n",
             fil, lin, thrname, (int) mom_gettid (), timbuf, msg);
  struct backtrace_state *btstate =
    backtrace_create_state (NULL, 0, mom_bt_err_callback, NULL);
  if (btstate != NULL)
    {
      int count = 0;
      backtrace_full (btstate, 1, mom_bt_callback, mom_bt_err_callback,
                      (void *) &count);
      if (count > 0)
        fprintf (stderr,
                 "Please include the complete backtrace of %d levels in error reports\n",
                 count);
    }
#if __GLIBC__
#define BACKTRACE_MAX_MOM 100
  if (!btstate)
    {
      void *bbuf[BACKTRACE_MAX_MOM];
      int blev = 0;
      memset (bbuf, 0, sizeof (bbuf));
      blev = backtrace (bbuf, BACKTRACE_MAX_MOM - 1);
      char **bsym = backtrace_symbols (bbuf, blev);
      {
        for (int i = 0; i < blev; i++)
          fprintf (stderr, "MONIMELTB[%d]: %s\n", i, bsym[i]);
        fflush (NULL);
      }
    }
#endif
  if (bigbuf)
    free (bigbuf);
  abort ();
}





char *
mom_strftime_centi (char *buf, size_t len, const char *fmt, double ti)
{
  struct tm tm;
  time_t tim = (time_t) ti;
  memset (&tm, 0, sizeof (tm));
  if (!buf || !fmt || !len)
    return NULL;
  strftime (buf, len, fmt, localtime_r (&tim, &tm));
  char *dotundund = strstr (buf, ".__");
  if (dotundund)
    {
      double ind = 0.0;
      double fra = modf (ti, &ind);
      char minibuf[16];
      memset (minibuf, 0, sizeof (minibuf));
      snprintf (minibuf, sizeof (minibuf), "%.02f", fra);
      strncpy (dotundund, strchr (minibuf, '.'), 3);
    }
  return buf;
}


////////////////
#ifndef mom_gc_calloc
void *
mom_gc_calloc (size_t nmemb, size_t size)
{
  if (nmemb == 0 || size == 0)
    return NULL;
  uint64_t totsz = 0;
  if (nmemb > INT32_MAX / 3 || size > INT32_MAX / 3
      || (totsz = nmemb * size) > INT32_MAX)
    MOM_FATAPRINTF ("too big nmemb=%zd or size=%zd for mom_gc_calloc",
                    nmemb, size);
  return mom_gc_alloc (totsz);
}
#endif /*undefined mom_gc_calloc */


static int randomfd_mom;

void
closerandomfile_mom (void)
{
  if (randomfd_mom > 2)
    {
      close (randomfd_mom), randomfd_mom = 0;
    }
}

void
mom_random_init_genrand (void)
{
  if (!randomfd_mom)
    {
      randomfd_mom = open ("/dev/urandom", O_RDONLY);
      if (!randomfd_mom)
        MOM_FATAPRINTF ("failed to open /dev/random %m");
      atexit (closerandomfile_mom);
    }
  unsigned long initarr[24];
  read (randomfd_mom, initarr, sizeof (initarr));
  momrand_init_by_array (initarr, sizeof (initarr) / sizeof (initarr[0]));
  // warmup the PRNG
  for (int i =
       (((17 * initarr[0]) ^ (211L * getpid ())) + 313 * time (NULL)) % 32 +
       10; i >= 0; i--)
    (void) momrand_genrand_int32 ();
}


static struct timespec start_realtime_ts_mom;

double
mom_elapsed_real_time (void)
{
  struct timespec curts = { 0, 0 };
  clock_gettime (CLOCK_REALTIME, &curts);
  return 1.0 * (curts.tv_sec - start_realtime_ts_mom.tv_sec)
    + 1.0e-9 * (curts.tv_nsec - start_realtime_ts_mom.tv_nsec);
}

double
mom_process_cpu_time (void)
{
  struct timespec curts = { 0, 0 };
  clock_gettime (CLOCK_PROCESS_CPUTIME_ID, &curts);
  return 1.0 * (curts.tv_sec) + 1.0e-9 * (curts.tv_nsec);
}

double
mom_thread_cpu_time (void)
{
  struct timespec curts = { 0, 0 };
  clock_gettime (CLOCK_THREAD_CPUTIME_ID, &curts);
  return 1.0 * (curts.tv_sec) + 1.0e-9 * (curts.tv_nsec);
}




momhash_t
mom_cstring_hash_len (const char *str, int len)
{
  if (!str)
    return 0;
  if (len < 0)
    len = strlen (str);
  if (MOM_UNLIKELY (len > MOM_SIZE_MAX))
    MOM_FATAPRINTF
      ("too big (%d) string to compute hash, starting with %.60s", len, str);
  int l = len;
  momhash_t h1 = 0, h2 = len, h;
  while (l > 4)
    {
      h1 =
        (509 * h2 +
         307 * ((signed char *) str)[0]) ^ (1319 * ((signed char *) str)[1]);
      h2 =
        (17 * l + 5 + 5309 * h2) ^ ((3313 * ((signed char *) str)[2]) +
                                    9337 * ((signed char *) str)[3] + 517);
      l -= 4;
      str += 4;
    }
  if (l > 0)
    {
      h1 = (h1 * 7703) ^ (503 * ((signed char *) str)[0]);
      if (l > 1)
        {
          h2 = (h2 * 7717) ^ (509 * ((signed char *) str)[1]);
          if (l > 2)
            {
              h1 = (h1 * 9323) ^ (11 + 523 * ((signed char *) str)[2]);
              if (l > 3)
                {
                  h2 =
                    (h2 * 7727 + 127) ^ (313 +
                                         547 * ((signed char *) str)[3]);
                }
            }
        }
    }
  h = (h1 * 29311 + 59) ^ (h2 * 7321 + 120501);
  if (!h)
    {
      h = h1;
      if (!h)
        {
          h = h2;
          if (!h)
            h = (len & 0xffffff) + 11;
        }
    }
  return h;
}                               /* end mom_cstring_hash_len */

void
mom_print_info (void)
{
  printf ("MONIMELT info:\n" " timestamp: %s\n", monimelt_timestamp);
  printf (" lastgitcommit: %s\n", monimelt_lastgitcommit);
  printf (" lastgittag: %s\n", monimelt_lastgittag);
  printf (" compilercommand: %s\n", monimelt_compilercommand);
  printf (" compilerversion: %s\n", monimelt_compilerversion);
  printf (" compilerflags: %s\n", monimelt_compilerflags);
  printf (" optimflags: %s\n", monimelt_optimflags);
  printf (" checksum: %s\n", monimelt_checksum);
  printf (" directory: %s\n", monimelt_directory);
  printf (" makefile: %s\n", monimelt_makefile);
  printf (" sqlite: %s\n", monimelt_sqlite);
  printf (" perstatebase: %s\n", monimelt_perstatebase);
  printf (" libsqlite3version: %s\n", sqlite3_libversion ());
  printf (" glibversion: major %d minor %d micro %d\n",
          glib_major_version, glib_minor_version, glib_micro_version);
  printf (" gtkversion: major %d minor %d micro %d\n",
          gtk_major_version, gtk_minor_version, gtk_micro_version);
#if __GLIBC__
  printf (" glibc: %s\n", gnu_get_libc_version ());
#endif
  printf (" MOM_SIZE_MAX: %d=%#x=%dM\n",
          MOM_SIZE_MAX, MOM_SIZE_MAX, MOM_SIZE_MAX >> 20);
#define PRINT_SIZEOF(T)							\
  printf("  sizeof(" #T ") = %d byt = %d wd, alignof(" #T ") = %d\n",	\
	 (int)sizeof(T),						\
	 (int) ((sizeof(T)+sizeof(void*)-1)/sizeof(void*)),		\
	 (int)__alignof__(T))
  PRINT_SIZEOF (int);
  PRINT_SIZEOF (long);
  PRINT_SIZEOF (void *);
  PRINT_SIZEOF (clock_t);
  PRINT_SIZEOF (time_t);
  PRINT_SIZEOF (pthread_mutex_t);
  PRINT_SIZEOF (pthread_rwlock_t);
  PRINT_SIZEOF (pthread_cond_t);
  PRINT_SIZEOF (mo_hashedvalue_ty);
  PRINT_SIZEOF (mo_sizedvalue_ty);
  PRINT_SIZEOF (mo_stringvalue_ty);
  PRINT_SIZEOF (mo_sequencevalue_ty);
  PRINT_SIZEOF (mo_tuplevalue_ty);
  PRINT_SIZEOF (mo_setvalue_ty);
  PRINT_SIZEOF (mo_objectvalue_ty);
  PRINT_SIZEOF (mo_countedpayl_ty);
  PRINT_SIZEOF (mo_assovaldatapayl_ty);
  PRINT_SIZEOF (mo_hashsetpayl_ty);
  PRINT_SIZEOF (mom_int128_t);
  PRINT_SIZEOF (FILE);
  {
    mo_hid_t hid = 0;
    mo_loid_t loid = 0;
    mo_get_some_random_hi_lo_ids (&hid, &loid);
    char rbuf[MOM_CSTRIDLEN];
    memset (rbuf, 0, sizeof (rbuf));
    mo_cstring_from_hi_lo_ids (rbuf, hid, loid);
    momhash_t h = mo_hash_from_hi_lo_ids (hid, loid);
    printf
      (" randomid: hid=%lx loid=%llx %s (of %d chars and hash %9u=%#08x)\n",
       (long) hid, (long long) loid, rbuf, (int) strlen (rbuf), (unsigned) h,
       (unsigned) h);
    mo_hid_t revhid = 0;
    mo_loid_t revloid = 0;
    MOM_ASSERTPRINTF (mo_get_hi_lo_ids_from_cstring (&revhid, &revloid, rbuf)
                      && revhid == hid && revloid == loid,
                      "reverse conversion failed revid=%lx revloid=%llx",
                      (long) revhid, (long long) revloid);
  }
}                               /* end mom_print_info */


/* Option specification for getopt_long.  */
enum extraopt_en
{
  xtraopt__none = 0,
  xtraopt_info = 1024,
  xtraopt_addpredef,
  xtraopt_commentpredef,
};

static const struct option mom_long_options[] = {
  {"help", no_argument, NULL, 'h'},
  {"version", no_argument, NULL, 'V'},
  {"add-predef", required_argument, NULL, xtraopt_addpredef},
  {"comment-predef", required_argument, NULL, xtraopt_commentpredef},
  {"info", no_argument, NULL, xtraopt_info},
  /* Terminating NULL placeholder.  */
  {NULL, no_argument, NULL, 0},
};


static void
usage_mom (const char *argv0)
{
  printf ("Usage: %s\n", argv0);
  printf ("\t -h | --help " " \t# Give this help.\n");
  printf ("\t -V | --version " " \t# Give version information.\n");
  printf ("\t --add-predefined predefname" " \t#Add a predefined\n");
  printf ("\t --comment-predefined comment"
          " \t#Set comment of next predefined\n");
  printf ("\t --info" " \t#Give various information\n");
}



static void
print_version_mom (const char *argv0)
{
  printf ("%s built on %s gitcommit %s\n", argv0,
          monimelt_timestamp, monimelt_lastgitcommit);
}


static void
parse_program_arguments_mom (int *pargc, char ***pargv)
{
  int argc = *pargc;
  char **argv = *pargv;
  int opt = -1;
  char *commentstr = NULL;
  while ((opt = getopt_long (argc, argv, "hV", mom_long_options, NULL)) >= 0)
    {
      switch (opt)
        {
        case 'h':              /* --help */
          usage_mom (argv[0]);
          putchar ('\n');
          fputs ("\nVersion info:::::\n", stdout);
          print_version_mom (argv[0]);
          exit (EXIT_FAILURE);
          return;
        case 'V':              /* --version */
          print_version_mom (argv[0]);
          exit (EXIT_SUCCESS);
          return;
        case xtraopt_commentpredef:
          commentstr = optarg;
          break;
        case xtraopt_addpredef:
          if (!optarg)
            MOM_FATAPRINTF ("missing predefined name for --add-predefined");
          if (!isalpha (optarg[0]) || !mom_valid_name (optarg))
            MOM_FATAPRINTF ("invalid predefined name %s", optarg);
          if (count_added_predef_mom >= MAX_ADDED_PREDEF_MOM)
            MOM_FATAPRINTF ("too many %d added predefined",
                            count_added_predef_mom);
          added_predef_mom[count_added_predef_mom].predef_name = optarg;
          added_predef_mom[count_added_predef_mom].predef_comment =
            commentstr;
          commentstr = NULL;
          count_added_predef_mom++;
          break;
        case xtraopt_info:
          mom_print_info ();
          break;
        default:
          MOM_FATAPRINTF ("bad option (%c) at %d", isalpha (opt) ? opt : '?',
                          optind);
          return;
        }
    }
}                               /* end of parse_program_arguments_mom */




static void
do_add_predefined_mom (void)
{
  for (unsigned ix = 0; ix < count_added_predef_mom; ix++)
    {
      const char *curname = added_predef_mom[ix].predef_name;
      if (!mom_valid_name (curname))
        MOM_FATAPRINTF ("invalid predefined name %s", curname);
      const char *comm = added_predef_mom[ix].predef_comment;
#warning do_add_predefined_mom unimplemented
      MOM_FATAPRINTF
        ("do_add_predefined_mom unimplemented curname=%s comm=%s", curname,
         comm);
      if (comm && comm[0])
        {
          MOM_INFORMPRINTF ("made predefined %s with comment %s",
                            curname, comm);
        }
      else
        {
          MOM_INFORMPRINTF ("made predefined %s without comment", curname);
        }
    }
}                               /* end of do_add_predefined_mom */

int
main (int argc_main, char **argv_main)
{
  clock_gettime (CLOCK_REALTIME, &start_realtime_ts_mom);
  GC_INIT ();
  GC_set_handle_fork (1);
  char **argv = argv_main;
  int argc = argc_main;
  mom_prog_dlhandle = dlopen (NULL, RTLD_NOW);
  if (MOM_UNLIKELY (!mom_prog_dlhandle))
    MOM_FATAPRINTF ("failed to dlopen program (%s)", dlerror ());
  mom_random_init_genrand ();
  json_object_seed (0);
  mom_init_objects ();
  parse_program_arguments_mom (&argc, &argv);
  {
    fflush (NULL);
    int okmaket = system ("make -q monimelt");
    if (!okmaket)
      MOM_INFORMPRINTF ("monimelt is made up to date");
    else
      {
        MOM_FATAPRINTF
          ("monimelt is not up to date, 'make -t monimelt' gave %d", okmaket);
      }
  }
  if (count_added_predef_mom > 0)
    do_add_predefined_mom ();
  int nbwarn = atomic_load (&mom_nb_warnings);
  if (nbwarn > 0)
    MOM_INFORMPRINTF
      ("end %s pid %d (elapsed real %.3f, cpu %.3f seconds, %d warnings)\n",
       argv[0], (int) getpid (), mom_elapsed_real_time (),
       mom_process_cpu_time (), nbwarn);
  else
    MOM_INFORMPRINTF
      ("end %s pid %d (elapsed real %.3f, cpu %.3f seconds)\n", argv[0],
       (int) getpid (), mom_elapsed_real_time (), mom_process_cpu_time ());
  return 0;
}                               /* end of main */

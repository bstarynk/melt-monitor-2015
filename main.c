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

bool mom_without_gui;


void *mom_prog_dlhandle;

char *mom_dump_dir;

char *mom_gtk_style_path = MONIMELT_GTK_STYLE;

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
}                               /* end mom_hostname */

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




/************************* backtrace *************************/

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
}                               /* end mom_bt_callback */

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
}                               /* end mom_bt_err_callback */


void
mom_backtraceprintf_at (const char *fil, int lin, const char *fmt, ...)
{
  int len = 0;
  char thrname[24];
  char buf[256];
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
  if (MOM_UNLIKELY (len >= (int) sizeof (buf) - 10))
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
    fprintf (stderr, "MONIMELT BACKTRACE @%s:%d <%s:%d> %s %s\n",
             fil, lin, thrname, (int) mom_gettid (), timbuf, msg);
    fflush (NULL);
  }
  struct backtrace_state *btstate =
    backtrace_create_state (NULL, 0, mom_bt_err_callback, NULL);
  if (btstate != NULL)
    {
      int count = 0;
      backtrace_full (btstate, 1, mom_bt_callback, mom_bt_err_callback,
                      (void *) &count);
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
  fprintf (stderr, "--- end backtrace from %s:%d\n\n", fil, lin);
  fflush (NULL);
  if (bigbuf)
    free (bigbuf);
}                               /* end mom_backtraceprintf_at */

/************************* inform *************************/

void
mom_informprintf_at (const char *fil, int lin, const char *fmt, ...)
{
  int len = 0;
  char thrname[24];
  char buf[256];
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
  if (MOM_UNLIKELY (len >= (int) sizeof (buf) - 10))
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
  int nbwarn = 1 + atomic_fetch_add ((int *) &mom_nb_warnings, 1);
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



void mom_abort (void) __attribute__ ((noreturn));

void
mom_abort (void)
{
  fflush (NULL);
  abort ();
}                               /* end mom_abort */

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
    fprintf (stderr, "MONIMELT FATAL @%s:%d <%s:%d> %s\n.. %s (%s)\n",
             fil, lin, thrname, (int) mom_gettid (), timbuf,
             msg, strerror (err));
  else
    fprintf (stderr, "MONIMELT FATAL @%s:%d <%s:%d> %s\n.. %s\n",
             fil, lin, thrname, (int) mom_gettid (), timbuf, msg);
  fflush (NULL);
  if (bigbuf)
    free (bigbuf);
  mom_abort ();
}                               /* end mom_fataprintf_at */





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
        MOM_FATAPRINTF ("failed to open /dev/urandom %m");
      atexit (closerandomfile_mom);
    }
  uint64_t initarr[32];
  if ((int) read (randomfd_mom, initarr, sizeof (initarr))
      < (int) sizeof (initarr))
    MOM_FATAPRINTF ("failed to read %zd from randomfile", sizeof (initarr));
  momrand_init_by_array (initarr, sizeof (initarr) / sizeof (initarr[0]));
}                               /* end mom_random_init_genrand */


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
  MOM_INFORMPRINTF ("various information for monimelt of %s",
                    monimelt_timestamp);
  printf ("MONIMELT info:\n" " timestamp: %s\n", monimelt_timestamp);
#ifdef NDEBUG
  printf ("*** ASSERT disabled thru NDEBUG\n");
#else
  printf ("*** ASSERT enabled without NDEBUG\n");
#endif
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
  printf (" csources {");
  for (const char *const *pfilnam = monimelt_csources;
       pfilnam && *pfilnam; pfilnam++)
    printf (" %s", *pfilnam);
  fputs (" }\n", stdout);
  printf (" shellsources {");
  for (const char *const *pfilnam = monimelt_shellsources;
       pfilnam && *pfilnam; pfilnam++)
    printf (" %s", *pfilnam);
  fputs (" }\n", stdout);
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
  PRINT_SIZEOF (mo_bufferpayl_ty);
  PRINT_SIZEOF (mom_int128_t);
  PRINT_SIZEOF (FILE);
  PRINT_SIZEOF (GtkWidget);
  PRINT_SIZEOF (GtkTextIter);
  PRINT_SIZEOF (GtkTextTag);
  PRINT_SIZEOF (GtkTextMark);
  {
    Dl_info dif;
    memset (&dif, 0, sizeof (dif));
    // dladdr on mom_dump_state
    if (dladdr ((const void *) mom_dump_state, &dif)
        && dif.dli_saddr == (const void *) mom_dump_state)
      {
        printf
          (" dladdr(mom_dump_state): dli_saddr@%p dli_fname=%s dli_sname=%s\n",
           dif.dli_saddr, dif.dli_fname, dif.dli_sname);
      }
    else
      MOM_WARNPRINTF ("mom_dump_state@%p not found with dladdr",
                      (const void *) mom_dump_state);
    // dladdr on gtk_application_new
    memset (&dif, 0, sizeof (dif));
    if (dladdr ((const void *) gtk_application_new, &dif)
        && dif.dli_saddr == (const void *) gtk_application_new)
      {
        printf
          (" dladdr(gtk_application_new): dli_saddr@%p dli_fname=%s dli_sname=%s\n",
           dif.dli_saddr, dif.dli_fname, dif.dli_sname);
      }
    else
      MOM_WARNPRINTF ("gtk_application_new@%p not found with dladdr",
                      (const void *) gtk_application_new);
  }
  {
    mo_hid_t hid = 0;
    mo_loid_t loid = 0;
    mo_get_some_random_hi_lo_ids (&hid, &loid);
    char rbuf[MOM_CSTRIDSIZ];
    memset (rbuf, 0, sizeof (rbuf));
    mo_cstring_from_hi_lo_ids (rbuf, hid, loid);
    momhash_t h = mo_hash_from_hi_lo_ids (hid, loid);
    printf
      (" randomid: hid=%#lx=%ld loid=%#llx=%lld:\n"
       "   idstr %s (of %d chars and hash %9u=%#08x)\n",
       (long) hid, (long) hid, (long long) loid, (long long) loid, rbuf,
       (int) strlen (rbuf), (unsigned) h, (unsigned) h);
    mo_hid_t revhid = 0;
    mo_loid_t revloid = 0;
    MOM_ASSERTPRINTF (mo_get_hi_lo_ids_from_cstring (&revhid, &revloid, rbuf)
                      && revhid == hid && revloid == loid,
                      "reverse conversion failed revid=%lx revloid=%llx",
                      (long) revhid, (long long) revloid);
  }
}                               /* end mom_print_info */



static gboolean want_version_mom;
static gboolean want_info_mom;
static gboolean no_gui_mom;
static gboolean no_custom_gliblog_mom;
static gboolean silent_big_alloc_mom;
static char *initrand_mom;
static char **predef_names_mom;
static char **predef_comments_mom;
static char **displayed_objects_mom;
static char **plugins_mom;
static int bench_count_mom;
static const GOptionEntry mom_goptions[] = {
  {"version", 'v', G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE,
   &want_version_mom, "version info", NULL},
  {"info", 0, G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE,
   &want_info_mom, "give various information", NULL},
  {"dump", 'd', G_OPTION_FLAG_NONE, G_OPTION_ARG_STRING,
   &mom_dump_dir, "dump into directory D", "D"},
  {"no-gui", 'N', G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE,
   &no_gui_mom, "without GTK graphical user interface", NULL},
  {"silent-big-alloc", 0, G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE,
   &silent_big_alloc_mom, "don't warn for large allocation", NULL},
  {"no-custom-glib-log", 0, G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE,
   &no_custom_gliblog_mom, "don't install our Glib/Gtk log handler", NULL},
  {"add-predef", 0, G_OPTION_FLAG_NONE, G_OPTION_ARG_STRING_ARRAY,
   &predef_names_mom, "add predefined of name N with comment C", "N"},
  {"comment-predef", 0, G_OPTION_FLAG_NONE, G_OPTION_ARG_STRING_ARRAY,
   &predef_comments_mom, "comment string C for predefined of name N", "C"},
  {"dispobj", 0, G_OPTION_FLAG_NONE, G_OPTION_ARG_STRING_ARRAY,
   &displayed_objects_mom, "display object O thru the graphical interface",
   "O"},
  {"bench", 0, G_OPTION_FLAG_NONE, G_OPTION_ARG_INT,
   &bench_count_mom, "benchmark count B", "B"},
  {"plugin", 'P', G_OPTION_FLAG_NONE, G_OPTION_ARG_STRING_ARRAY,
   &plugins_mom,
   "add plugin specified by P, e.g. foo:bar loading momplug_foo.so with argument bar",
   "P"},
  {"init-random", 0, G_OPTION_FLAG_NONE, G_OPTION_ARG_FILENAME,
   &initrand_mom, "initialize random with file F; should be first arg", "F"},
  {"gtk-style", 0, G_OPTION_FLAG_NONE, G_OPTION_ARG_FILENAME,
   &mom_gtk_style_path,
   "use F as Gtk3 CSS style sheet, default is " MONIMELT_GTK_STYLE, "F"},
  {}
};

void
mom_gc_warn_big_alloc (size_t sz)
{
  if (silent_big_alloc_mom)
    return;
  MOM_BACKTRACEPRINTF ("BIG ALLOCATION of %d megabytes, %zd bytes",
                       (int) (sz >> 20), sz);
}                               /* end mom_gc_warn_big_alloc */



static void
print_version_mom (const char *argv0)
{
  printf ("%s built on %s gitcommit %s\n", argv0,
          monimelt_timestamp, monimelt_lastgitcommit);
}                               /* end print_version_mom */





static void
do_add_predefined_mom (void)
{
  gchar **pname = predef_names_mom;
  gchar **pcomm = predef_comments_mom;
  if (!predef_names_mom)
    return;
  while (*pname)
    {
      mo_objref_t obr = NULL;
      const char *curname = *pname;
      const char *comm = *pcomm;
      char obidbuf[MOM_CSTRIDSIZ];
      memset (obidbuf, 0, sizeof (obidbuf));
      if (!mom_valid_name (curname))
        MOM_FATAPRINTF ("invalid new predefined name %s", curname);
      obr = mo_find_named_cstr (curname);
      if (!obr)
        {
          obr = mo_make_object ();
          mo_register_named (obr, curname);
        };
      mo_cstring_from_hi_lo_ids (obidbuf,
                                 ((mo_objectvalue_ty *) obr)->mo_ob_hid,
                                 ((mo_objectvalue_ty *) obr)->mo_ob_loid);
      mo_objref_put_space (obr, mo_SPACE_PREDEF);
      if (comm && comm[0])
        {
          mo_value_t commv = mo_make_string_cstr (comm);
          mo_objref_put_attr (obr, MOM_PREDEF (comment), commv);
          MOM_INFORMPRINTF ("made predefined %s (%s) with comment %s",
                            curname, obidbuf, comm);
        }
      else
        {
          MOM_INFORMPRINTF ("made predefined %s (%s) without comment",
                            curname, obidbuf);
        }
      pname++;
      if (*pcomm)
        pcomm++;
    };                          /* end while *pname */
}                               /* end of do_add_predefined_mom */

static void
do_load_plugins_mom (void)
{
  int nbplugins = 0;
  int plugincumlen = 0;
  for (char **pc = plugins_mom; pc && *pc; pc++)
    {
      nbplugins++;
      plugincumlen += strlen (*pc);
    }
  int cntplugin = 0;
  size_t plugcmdsiz = 0;
  char *plugcmdbuf = NULL;
  FILE *fcmd = open_memstream (&plugcmdbuf, &plugcmdsiz);
  if (!fcmd)
    MOM_FATAPRINTF ("open_memstream failed for plugin command buffer");
  fputs ("make -j 3 ", fcmd);
  char **pluginamesarr = mom_gc_alloc ((nbplugins + 1) * sizeof (char *));
  char **pluginargsarr = mom_gc_alloc ((nbplugins + 1) * sizeof (char *));
  void **plugindlharr =
    mom_gc_alloc_scalar ((nbplugins + 1) * sizeof (void *));
  for (char **pc = plugins_mom; pc && *pc; pc++)
    {
      char *curplug = *pc;
      char pluginambuf[60];
      memset (pluginambuf, 0, sizeof (pluginambuf));
      int pos = -1;
      if ((sscanf (curplug, "%70[A-Za-z0-9_]:%n", pluginambuf, &pos) >= 1
           && pos > 2)
          || (memset (pluginambuf, 0, sizeof (pluginambuf)),
              (sscanf (curplug, "%70[A-Za-z0-9_]%n", pluginambuf, &pos) >= 1)
              && pos > 1))
        {
          char *curpluginame =
            mom_gc_alloc (1 + ((strlen (pluginambuf) + 1) | 7));
          char *curpluginarg = NULL;
          strcpy (curpluginame, pluginambuf);
          char *pluginarg = curplug + pos;
          if (pos > 1 && curplug[pos] == ':')
            pos--;
          if (pluginarg && pluginarg[0])
            curpluginarg = GC_STRDUP (pluginarg);
          char pluginsource[sizeof (pluginambuf) + 16];
          memset (pluginsource, 0, sizeof (pluginsource));
          snprintf (pluginsource, sizeof (pluginsource),
                    MOM_PLUGIN_PREFIX "%s.c", curpluginame);
          if (access (pluginsource, R_OK))
            MOM_FATAPRINTF ("unreadable plugin#%d source %s",
                            cntplugin + 1, pluginsource);
          fprintf (fcmd, " " MOM_PLUGIN_PREFIX "%s" MOM_PLUGIN_SUFFIX,
                   curpluginame);
          pluginamesarr[cntplugin] = curpluginame;
          pluginargsarr[cntplugin] = curpluginarg;
        }
      else
        MOM_FATAPRINTF ("invalid plugin specification#%d %s\n"
                        " - should be -Pfoo:bar for plugin " MOM_PLUGIN_PREFIX
                        "foo.c with argument bar", cntplugin + 1, curplug);
      cntplugin++;
    }
  MOM_ASSERTPRINTF (cntplugin == nbplugins, "bad cntplugin=%d", cntplugin);
  fflush (fcmd);
  long lencmd = ftell (fcmd);
  MOM_ASSERTPRINTF (lencmd > 0, "bad lencmd");
  plugcmdbuf[lencmd] = (char) 0;
  MOM_INFORMPRINTF ("making shared objects for %d plugins, with\n\t%s",
                    nbplugins, plugcmdbuf);
  fflush (NULL);
  int rc = system (plugcmdbuf);
  if (rc != 0)
    MOM_FATAPRINTF ("failed (%d) to make %d plugins: %s",
                    rc, nbplugins, plugcmdbuf);
  fflush (NULL);
  MOM_INFORMPRINTF ("made shared objects for %d plugins", nbplugins);
  for (int pix = 0; pix < nbplugins; pix++)
    {
      const char *curpluginame = pluginamesarr[pix];
      char curplugpath[96];
      memset (curplugpath, 0, sizeof (curplugpath));
      snprintf (curplugpath, sizeof (curplugpath),
                "./" MOM_PLUGIN_PREFIX "%s" MOM_PLUGIN_SUFFIX, curpluginame);
      MOM_ASSERTPRINTF (strlen (curplugpath) < sizeof (curplugpath) - 2,
                        "too long curplugpath %s", curplugpath);
      void *dlh = dlopen (curplugpath, RTLD_LAZY | RTLD_GLOBAL);
      if (!dlh)
        MOM_FATAPRINTF ("failed to dlopen plugin#%d %s (%s)",
                        pix + 1, curplugpath, dlerror ());
      if (!dlsym (dlh, MOM_PLUGIN_STARTUP))
        MOM_FATAPRINTF ("failed to dlsym %s in plugin#%d %s (%s)",
                        MOM_PLUGIN_STARTUP,
                        pix + 1, curpluginame, dlerror ());
      plugindlharr[pix] = dlh;
    }
  MOM_INFORMPRINTF ("loaded %d plugins", nbplugins);
  for (int pix = 0; pix < nbplugins; pix++)
    {
      const char *curpluginame = pluginamesarr[pix];
      const char *curpluginarg = pluginargsarr[pix];
      void *curplugindlh = plugindlharr[pix];
      momplugin_startup_sigt *curplugstart
        = dlsym (curplugindlh, MOM_PLUGIN_STARTUP);
      if (!curplugstart)
        MOM_FATAPRINTF ("failed to dlsym %s in plugin#%d %s (%s)",
                        MOM_PLUGIN_STARTUP,
                        pix + 1, curpluginame, dlerror ());
      if (curpluginarg)
        MOM_INFORMPRINTF ("starting plugin#%d : %s with %s",
                          pix + 1, curpluginame, curpluginarg);
      else
        MOM_INFORMPRINTF ("starting plugin#%d : %s without argument",
                          pix + 1, curpluginame);
      fflush (NULL);
      (*curplugstart) (curpluginarg);
      fflush (NULL);
    }
  MOM_INFORMPRINTF ("started %d plugins", nbplugins);
}                               /* end do_load_plugins_mom */



#ifndef MOM_BENCHWIDTH
#define MOM_BENCHWIDTH 256
#endif /* MOM_BENCHWIDTH */

#if MOM_BENCHMARK_MANY < 2048
#undef MOM_BENCHMARK_MANY
#define MOM_BENCHMARK_MANY 2048
#endif



void
mom_run_benchmark_many (int benchcount)
{
  enum mombhop_en
  {
    MOMBHOP_NONE,
    MOMBHOP_PUT_OBR1_OBR2_STRING,
    MOMBHOP_PUT_OBR3_OBR4_OBR5,
    MOMBHOP_PUT_OBR1_OBR3_OBR6,
    MOMBHOP_PUT_OBR3_SET_OBR1_OBR2_OBR3,
    MOMBHOP_PUT_OBR5_OBR1_INT,
    MOMBHOP_PUT_OBR6_OBR5_TUPLE,
    MOMBHOP_PUT_OBR6_OBR2_SET,
    MOMBHOP_PUT_OBR6_TUPLE_OBR1_OBR2_OBR3_OBR4_OBR5,
    MOMBHOP_RESIZE_OBR5,
    MOMBHOP_RESIZE_OBR6,
    MOMBHOP_APPEND_OBR4_OBR2,
    MOMBHOP_PUTCOMP_OBR5_OBR1,
    MOMBHOP_PUTCOMP_OBR5_OBR2,
    MOMBHOP_PUTCOMP_OBR6_OBR3,
    MOMBHOP_PUTCOMP_OBR6_TUPLE_OBR1_OBR2,
    MOMBHOP_CLEAR,
    MOMBHOP_CLEAROTHER,
    MOMBHOP__LAST
  } bhop = 0;
  if (benchcount < 2 * MOM_BENCHWIDTH)
    {
      benchcount = 3 * MOM_BENCHWIDTH;
      MOM_WARNPRINTF ("benchcount set to %d", benchcount);
    }
  else if (benchcount > MOM_SIZE_MAX / 4)
    MOM_FATAPRINTF ("too large benchcount %d", benchcount);
  MOM_INFORMPRINTF ("start of benchmark of count %d", benchcount);
  double startelapsedtime = mom_elapsed_real_time ();
  double startcputime = mom_process_cpu_time ();
  mo_objref_t bencharr[MOM_BENCHWIDTH];
  memset (bencharr, 0, sizeof (bencharr));
  for (int lix = 0; lix < benchcount; lix++)
    {
      int oix1 = momrand_genrand_int31 () % MOM_BENCHWIDTH;
      mo_objref_t obr1 = mo_make_object ();
      mo_objref_touch (obr1);
      mo_objref_put_space (obr1, mo_SPACE_GLOBAL);
      bencharr[oix1] = obr1;
      int oix2 = momrand_genrand_int31 () % MOM_BENCHWIDTH;
      mo_objref_t obr2 = mo_make_object ();
      mo_objref_touch (obr2);
      mo_objref_put_space (obr2, mo_SPACE_GLOBAL);
      bencharr[oix2] = obr2;
      bhop = momrand_genrand_int31 () % MOMBHOP__LAST;
      int oix3 = momrand_genrand_int31 () % MOM_BENCHWIDTH;
      mo_objref_t obr3 = bencharr[oix3];
      int oix4 = momrand_genrand_int31 () % MOM_BENCHWIDTH;
      mo_objref_t obr4 = bencharr[oix4];
      int oix5 = momrand_genrand_int31 () % MOM_BENCHWIDTH;
      mo_objref_t obr5 = bencharr[oix5];
      int oix6 = momrand_genrand_int31 () % MOM_BENCHWIDTH;
      mo_objref_t obr6 = bencharr[oix6];
      switch (bhop)
        {
        case MOMBHOP_NONE:
          break;
        case MOMBHOP_PUT_OBR1_OBR2_STRING:
          mo_objref_put_attr (obr1, obr2,
                              mo_make_string_sprintf ("loop#%u", lix));
          break;
        case MOMBHOP_PUT_OBR3_OBR4_OBR5:
          mo_objref_put_attr (obr3, obr4, obr5);
          break;
        case MOMBHOP_PUT_OBR1_OBR3_OBR6:
          mo_objref_put_attr (obr1, obr3, obr6);
          break;
        case MOMBHOP_PUT_OBR3_SET_OBR1_OBR2_OBR3:
          mo_objref_put_attr (obr3, obr5, MOM_MAKE_SENSET (obr1, obr2, obr3));
          break;
        case MOMBHOP_PUT_OBR5_OBR1_INT:
          mo_objref_put_attr (obr5, obr1, mo_int_to_value (lix));
          break;
        case MOMBHOP_PUT_OBR6_OBR5_TUPLE:
          {
            unsigned siz = momrand_genrand_int31 () % (MOM_BENCHWIDTH / 5);
            mo_sequencevalue_ty *seq = mo_sequence_allocate (siz);
            unsigned off = momrand_genrand_int31 () % (MOM_BENCHWIDTH / 4);
            for (int nix = 0; nix < (int) siz; nix++)
              seq->mo_seqobj[nix] = bencharr[off + nix];
            mo_value_t tupv = mo_make_tuple_closeq (seq);
            seq = NULL;
            mo_objref_put_attr (obr6, obr5, tupv);
          }
          break;
        case MOMBHOP_PUT_OBR6_OBR2_SET:
          {
            unsigned siz = momrand_genrand_int31 () % (MOM_BENCHWIDTH / 6);
            mo_sequencevalue_ty *seq = mo_sequence_allocate (siz);
            unsigned off = momrand_genrand_int31 () % (MOM_BENCHWIDTH / 4);
            for (int nix = 0; nix < (int) siz; nix++)
              seq->mo_seqobj[nix] = bencharr[off + nix];
            mo_value_t setv = mo_make_set_closeq (seq);
            seq = NULL;
            mo_objref_put_attr (obr6, obr2, setv);
          }
          break;
        case MOMBHOP_PUT_OBR6_TUPLE_OBR1_OBR2_OBR3_OBR4_OBR5:
          mo_objref_put_attr (obr6, obr5,
                              MOM_MAKE_SENTUPLE (obr1, obr2, obr3, obr4,
                                                 obr5));
          break;
        case MOMBHOP_RESIZE_OBR5:
          mo_objref_comp_resize (obr5,
                                 2 +
                                 momrand_genrand_int31 () % (MOM_BENCHWIDTH /
                                                             10));
          break;
        case MOMBHOP_RESIZE_OBR6:
          mo_objref_comp_resize (obr6,
                                 momrand_genrand_int31 () % (MOM_BENCHWIDTH /
                                                             16));
          break;
        case MOMBHOP_APPEND_OBR4_OBR2:
          mo_objref_comp_append (obr4, obr2);
          break;
        case MOMBHOP_CLEAR:
        case MOMBHOP_CLEAROTHER:
          {
            int oixc = momrand_genrand_int31 () % MOM_BENCHWIDTH;
            bencharr[oixc] = NULL;
          }
          break;
        case MOMBHOP_PUTCOMP_OBR5_OBR1:
          {
            unsigned cnt = mo_objref_comp_count (obr5);
            mo_objref_put_comp (obr5,
                                (momrand_genrand_int31 () % (cnt + 10)) - 20,
                                obr1);
          }
          break;
        case MOMBHOP_PUTCOMP_OBR5_OBR2:
          {
            unsigned cnt = mo_objref_comp_count (obr5);
            mo_objref_put_comp (obr5,
                                (momrand_genrand_int31 () % (cnt + 4)) - 30,
                                obr2);
          }
          break;
        case MOMBHOP_PUTCOMP_OBR6_OBR3:
          {
            unsigned cnt = mo_objref_comp_count (obr6);
            mo_objref_put_comp (obr3,
                                (momrand_genrand_int31 () % (cnt + 2)) -
                                cnt / 3, obr3);
          }
          break;
        case MOMBHOP_PUTCOMP_OBR6_TUPLE_OBR1_OBR2:
          {
            unsigned cnt = mo_objref_comp_count (obr6);
            mo_objref_put_comp (obr6,
                                (momrand_genrand_int31 () % (cnt + 2)) -
                                cnt / 3, MOM_MAKE_SENTUPLE (obr1, obr2));
          }
          break;
        case MOMBHOP__LAST:
          MOM_FATAPRINTF ("cannot happen");
        }
    }
  // we need to put some of the objects in a dumped root, that is the_system
  unsigned tupsiz = 5 + (momrand_genrand_int31 () % (MOM_BENCHWIDTH / 8));
  mo_sequencevalue_ty *seq = mo_sequence_allocate (tupsiz);
  memcpy (seq->mo_seqobj, bencharr, tupsiz * sizeof (mo_objref_t));
  mo_value_t setv = mo_make_set_closeq (seq);
  mo_objref_put_attr (MOM_PREDEF (the_system), MOM_PREDEF (the_system), setv);
  double endelapsedtime = mom_elapsed_real_time ();
  double endcputime = mom_process_cpu_time ();
  MOM_INFORMPRINTF ("end of benchmark counted %d loop, final tuple size %u\n"
                    ".. in %.4f ms (%.3f µs/loop) elapsed, %.4f ms (%.3f µs/loop) cpu\n",
                    benchcount, tupsiz,
                    1.0e3 * (endelapsedtime - startelapsedtime),
                    1.0e6 * (endelapsedtime - startelapsedtime) / benchcount,
                    1.0e3 * (endcputime - startcputime),
                    1.0e6 * (endcputime - startcputime) / benchcount);
}                               /* end of mom_run_benchmark_many */

void
mom_run_small_benchmark (int cnt)
{
  if (cnt < 64)
    cnt = 64;
  if (cnt > MOM_SIZE_MAX / 4)
    MOM_FATAPRINTF ("too large small benchmark cnt %d", cnt);
  MOM_INFORMPRINTF ("start of small benchmark of count %d", cnt);
  double startelapsedtime = mom_elapsed_real_time ();
  double startcputime = mom_process_cpu_time ();
  mo_objref_t *objarr = mom_gc_alloc (cnt * sizeof (mo_objref_t));
  for (int i = 0; i < cnt; i++)
    {
      mo_objref_t obr = mo_make_object ();
      mo_objref_touch (obr);
      mo_objref_put_space (obr, mo_SPACE_GLOBAL);
      mo_objref_put_attr (obr, MOM_PREDEF (comment),
                          mo_make_string_sprintf ("small-obj#%d", i));
      objarr[i] = obr;
    };
  for (long ix = 0; ix < (long) cnt - 6; ix += 2)
    {
      mo_objref_put_attr (objarr[ix], objarr[ix + 1], objarr[ix + 2]);
    };
  for (long ix = 0; ix < (long) cnt / 2; ix += 3)
    {
      if (ix % 2 == 0)
        mo_objref_put_attr (objarr[ix], objarr[ix + 2],
                            MOM_MAKE_SENTUPLE (objarr[ix + 1], objarr[ix + 3],
                                               objarr[ix + 5]));
      else
        mo_objref_put_attr (objarr[ix], objarr[ix + 2],
                            MOM_MAKE_SENSET (objarr[ix + 1], objarr[ix + 3],
                                             objarr[ix + 5], objarr[ix / 3]));
      if (ix * ix < cnt)
        {
          MOM_ASSERTPRINTF (ix < MOM_SIZE_MAX, "bad ix=%ld", ix);
          mo_objref_comp_append (objarr[ix / 5], objarr[ix * ix]);
        }
      mo_objref_comp_append (objarr[ix % 16],
                             MOM_MAKE_SENSET (objarr[ix + 1], objarr[ix + 2],
                                              objarr[ix % (3 + cnt / 4)]));
    }
  unsigned tupsiz = cnt / 5 + 8;
  mo_sequencevalue_ty *seq = mo_sequence_allocate (tupsiz);
  for (int ix = 0; ix < (int) tupsiz && 2 * ix < cnt; ix++)
    seq->mo_seqobj[ix] = objarr[ix + (int) sqrt ((double) (ix + 1))];
  mo_value_t tupv = mo_make_tuple_closeq (seq);
  seq = NULL;
  objarr = NULL;
  mo_objref_put_attr (MOM_PREDEF (the_system), MOM_PREDEF (the_system), tupv);
  double endelapsedtime = mom_elapsed_real_time ();
  double endcputime = mom_process_cpu_time ();
  MOM_INFORMPRINTF
    ("end of small benchmark counted %d loop, final tuple size %u\n"
     ".. in %.5f (%.3f µs/loop) elapsed, %.5f (%.3f µs/loop) cpu milliseconds\n",
     cnt, tupsiz, 1.0e3 * (endelapsedtime - startelapsedtime),
     1.0e6 * (endelapsedtime - startelapsedtime) / cnt,
     1.0e3 * (endcputime - startcputime),
     1.0e6 * (endcputime - startcputime) / cnt);
}                               /* end mom_run_small_benchmark */


extern void mom_g_log_serious (void);
// put a breakpoint in GDB for
void
mom_g_log_serious (void)
{
  fflush (NULL);
}                               /* end mom_g_log_serious */

static void
mom_g_log_handler (const gchar * logdomain,
                   GLogLevelFlags loglevel,
                   const gchar * message, gpointer userdata MOM_UNUSED)
{
  bool serious = false;
  if (loglevel & G_LOG_LEVEL_DEBUG)
    {
      MOM_INFORMPRINTF ("Gtk/Glib DEBUG %s : %s", logdomain, message);
    }
  else if (loglevel & G_LOG_LEVEL_INFO)
    {
      MOM_INFORMPRINTF ("Gtk/Glib INFO %s : %s", logdomain, message);
    }
  else if (loglevel & G_LOG_LEVEL_MESSAGE)
    {
      MOM_INFORMPRINTF ("Gtk/Glib MESSAGE %s : %s", logdomain, message);
    }
  else if (loglevel & G_LOG_LEVEL_WARNING)
    {
      serious = true;
      MOM_BACKTRACEPRINTF ("Gtk/Glib WARNING %s *:* %s", logdomain, message);
    }
  else if (loglevel & G_LOG_LEVEL_CRITICAL)
    {
      serious = true;
      MOM_FATAPRINTF ("Gtk/Glib CRITICAL %s *:* %s", logdomain, message);
    }
  else if (loglevel & G_LOG_LEVEL_ERROR)
    {
      serious = true;
      MOM_FATAPRINTF ("Gtk/Glib ERROR %s *:* %s", logdomain, message);
    }
  if (serious)
    mom_g_log_serious ();
}                               /* end of mom_g_log_handler */

int
main (int argc_main, char **argv_main)
{
  clock_gettime (CLOCK_REALTIME, &start_realtime_ts_mom);
  GC_INIT ();
  GC_set_handle_fork (1);
  GError *opterror = NULL;
  GOptionContext *optcontext = NULL;
  char **argv = argv_main;
  int argc = argc_main;
  mom_prog_dlhandle = dlopen (NULL, RTLD_NOW);
  if (MOM_UNLIKELY (!mom_prog_dlhandle))
    MOM_FATAPRINTF ("failed to dlopen program (%s)", dlerror ());
  if (argc_main > 3 && !strcmp (argv_main[1], "--init-random"))
    {
      randomfd_mom = open (argv_main[2], O_RDONLY);
      if (randomfd_mom < 0)
        MOM_FATAPRINTF ("failed to open random fd for %s : %m", argv_main[2]);
      MOM_INFORMPRINTF ("using %s as the random file", argv_main[2]);
    }
  mom_random_init_genrand ();
  sqlite3_config (SQLITE_CONFIG_LOG, mo_dump_errorlog, NULL);
  json_object_seed (momrand_genrand_int31 ());
  optcontext = g_option_context_new ("- the MELT monitor");
  g_option_context_add_main_entries (optcontext, mom_goptions,
                                     MOM_GETTEXT_PACKAGE);
  g_option_context_add_group (optcontext, gtk_get_option_group (FALSE));
  if (!g_option_context_parse (optcontext, &argc, &argv, &opterror))
    MOM_FATAPRINTF ("option parsing failed: %s\n", opterror->message);
  if (want_version_mom)
    print_version_mom (argv[0]);
  if (want_info_mom)
    mom_print_info ();
  mom_init_objects ();
  {
    char cwdbuf[160];
    memset (cwdbuf, 0, sizeof (cwdbuf));
    if (!getcwd (cwdbuf, sizeof (cwdbuf)))
      strcpy (cwdbuf, "./");
    MOM_INFORMPRINTF ("starting on %s pid %d in %s, randoms %u %u",
                      mom_hostname (), (int) getpid (), cwdbuf,
                      (unsigned) momrand_genrand_int31 (),
                      (unsigned) momrand_genrand_int31 ());
    // check our int <-> value conversion
#ifndef NDEBUG
    unsigned r = (unsigned) (momrand_genrand_int31 () & 0xfffff);
    MOM_ASSERTPRINTF (mo_value_is_int (mo_int_to_value (r)),
                      "bad boxed int for r=%u", r);
    MOM_ASSERTPRINTF (mo_value_to_int (mo_int_to_value (r), -99999) == r,
                      "bad int <-> value conversions for r=%u", r);
    MOM_ASSERTPRINTF (mo_value_is_int (mo_int_to_value (-(int) r)),
                      "bad boxed int for -r=%d", -(int) r);
    MOM_ASSERTPRINTF (mo_value_to_int (mo_int_to_value (-(int) r), 99999) ==
                      -(int) r, "bad int <-> value conversions for -r=%d",
                      -(int) r);
#endif /*NDEBUG*/
  }
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
  if (!no_custom_gliblog_mom)
    {
      g_log_set_default_handler (mom_g_log_handler, NULL);
      g_log_set_handler ("Gtk", G_LOG_LEVEL_CRITICAL | G_LOG_FLAG_FATAL
                         | G_LOG_LEVEL_ERROR | G_LOG_LEVEL_WARNING
                         | G_LOG_FLAG_RECURSION, mom_g_log_handler, NULL);
      MOM_INFORMPRINTF
        ("added mom_g_log_handler as Glib default log handler & Gtk log");
    }
  else
    MOM_INFORMPRINTF
      ("no custom Glib/Gtk default log handler, since --no-custom-glib-log");
  mom_load_state ();
  if (predef_names_mom)
    do_add_predefined_mom ();
  if (plugins_mom)
    do_load_plugins_mom ();
  if (bench_count_mom != 0)
    {
      if (bench_count_mom > 0)
        mom_run_benchmark_many (bench_count_mom);
      else
        mom_run_small_benchmark (-bench_count_mom);
    }
  if (no_gui_mom)
    {
      mom_without_gui = true;
      MOM_INFORMPRINTF
        ("monimelt don't run the GTK graphical interface (-N | --no-gui)");
    }
  else
    mom_run_gtk (&argc, &argv, displayed_objects_mom);
  if (mom_dump_dir && !strcmp (mom_dump_dir, "-"))
    MOM_INFORMPRINTF
      ("monimelt explicitly not dumping because of program argument -d- or --dump -");
  else
    {
      MOM_INFORMPRINTF ("monimelt will dump into %s", mom_dump_dir);
      mom_dump_state (mom_dump_dir);
    }
  int nbwarn = atomic_load ((int *) &mom_nb_warnings);
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

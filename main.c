// file main.c - main program and utilities

/**   Copyright (C)  2015, 2016  Basile Starynkevitch and later the FSF
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

bool mom_skip_dump_hooks;
bool mom_dont_make_after_dump;

atomic_int mom_nb_warnings;

static char hostname_mom[80];

void *mom_prog_dlhandle;
const char *mom_webdir[MOM_MAX_WEBDIR];
volatile atomic_bool mom_should_run;
#define MOM_DEFAULT_NB_JOBS 3
unsigned mom_nbjobs = MOM_DEFAULT_NB_JOBS;
static bool syslogging_mom;
static bool should_dump_mom;
static bool skipmadecheck_mom;
static char *dir_after_load_mom;
static char *load_state_mom;
static char *web_service_mom;

#define MAX_ADDED_PREDEF_MOM 16
struct predefadd_mom_st
{
  const char *predef_name;
  const char *predef_comment;
};
static struct predefadd_mom_st added_predef_mom[MAX_ADDED_PREDEF_MOM];
static unsigned count_added_predef_mom;

#define MAX_UNPREDEF_MOM 24
static const char *unpredefined_mom[MAX_UNPREDEF_MOM];
static int count_unpredef_mom;


#define BASE_YEAR_MOM 2015

#define MAX_TEST_MOM 16
struct test_mom_st
{
  const char *test_name;
  const char *test_arg;
};
static struct test_mom_st testarr_mom[MAX_TEST_MOM];
static unsigned testcount_mom;

#define MAX_OUTPUTCONTENT_MOM 24
static const char *outcont_name_mom[MAX_OUTPUTCONTENT_MOM];
unsigned outcont_count_mom;

const char *
mom_hostname (void)
{
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
}				/* end mom_output_gplv3_notice */



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
}				/* end of mom_output_utf8_escaped */




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
}				// end mom_output_utf8_encoded


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
}				/* end of mom_output_utf8_html */


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
}				/* end of mom_hexdump_data */



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
	  char *newbuf = mom_gc_alloc_atomic (newsiz);
	  memcpy (newbuf, bufzon, bufoff);
	  if (bufzon != bufarr)
	    GC_FREE (bufzon);
	  bufzon = newbuf;
	  bufsiz = newsiz;
	};
      int c = fgetc (f);
      if (c == EOF)
	break;
      if (iscntrl (c) || c == '\'' || c == '"')
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
  char *res = mom_gc_alloc_atomic (bufoff + 1);
  memcpy (res, bufzon, bufoff);
  if (bufzon != bufarr)
    GC_FREE (bufzon);
  ss.ss_str = res;
  ss.ss_len = bufoff;
  return ss;
}				/* end of mom_input_quoted_utf8 */



static pthread_mutex_t dbgmtx_mom = PTHREAD_MUTEX_INITIALIZER;
static const char *dbg_level_mom (enum mom_debug_en dbg);
void
mom_debugprintf_at (const char *fil, int lin, enum mom_debug_en dbg,
		    const char *fmt, ...)
{
  static long countdbg;
  char thrname[24];
  char buf[160];
  char timbuf[64];
  int len = 0;
  char *msg = NULL;
  char *bigbuf = NULL;
  memset (thrname, 0, sizeof (thrname));
  memset (buf, 0, sizeof (buf));
  memset (timbuf, 0, sizeof (timbuf));
  pthread_getname_np (pthread_self (), thrname, sizeof (thrname) - 1);
  fflush (NULL);
  mom_now_strftime_bufcenti (timbuf, "%H:%M:%S.__ ");
  va_list alist;
  va_start (alist, fmt);
  len = vsnprintf (buf, sizeof (buf), fmt, alist);
  va_end (alist);
  if (MOM_UNLIKELY (len >= (int) sizeof (buf) - 1))
    {
      char *bigbuf = malloc (len + 10);
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
    pthread_mutex_lock (&dbgmtx_mom);
    long nbdbg = countdbg++;
#define DEBUG_DATE_PERIOD_MOM 64
    char datebuf[48] = { 0 };
    if (nbdbg % DEBUG_DATE_PERIOD_MOM == 0)
      {
	mom_now_strftime_bufcenti (datebuf, "%Y-%b-%d@%H:%M:%S.__ %Z");
      };
    if (syslogging_mom)
      {
	syslog (LOG_DEBUG, "MONIMELT DEBUG %7s <%s> @%s:%d %s %s",
		dbg_level_mom (dbg), thrname, fil, lin, timbuf, msg);
	if (nbdbg % DEBUG_DATE_PERIOD_MOM == 0)
	  syslog (LOG_DEBUG, "MONIMELT DEBUG#%04ld ~ %s *^*^*", nbdbg,
		  datebuf);
      }
    else
      {
	fprintf (stderr, "MONIMELT DEBUG %7s <%s> @%s:%d %s %s\n",
		 dbg_level_mom (dbg), thrname, fil, lin, timbuf, msg);
	fflush (stderr);
	if (nbdbg % DEBUG_DATE_PERIOD_MOM == 0)
	  fprintf (stderr, "MONIMELT DEBUG#%04ld ~ %s *^*^*\n", nbdbg,
		   datebuf);
	fflush (NULL);
      }
    pthread_mutex_unlock (&dbgmtx_mom);
  }
  if (bigbuf)
    free (bigbuf);
}


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
  else
    msg = buf;
  if (syslogging_mom)
    {
      syslog (LOG_INFO, "MONIMELT INFORM @%s:%d <%s:%d> %s %s",
	      fil, lin, thrname, (int) mom_gettid (), timbuf, msg);
    }
  else
    {
      fprintf (stderr, "MONIMELT INFORM @%s:%d <%s:%d> %s %s\n",
	       fil, lin, thrname, (int) mom_gettid (), timbuf, msg);
      fflush (NULL);
    }
  if (bigbuf)
    free (bigbuf);
}

/************************* warning *************************/

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
  if (syslogging_mom)
    {
      if (err)
	syslog (LOG_WARNING, "MONIMELT WARNING#%d @%s:%d <%s:%d> %s %s (%s)",
		nbwarn, fil, lin, thrname, (int) mom_gettid (), timbuf,
		msg, strerror (err));
      else
	syslog (LOG_WARNING, "MONIMELT WARNING#%d @%s:%d <%s:%d> %s %s",
		nbwarn, fil, lin, thrname, (int) mom_gettid (), timbuf, msg);
    }
  else
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
}				/* end of mom_warnprintf_at */


/************************* fatal *************************/
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
#if __GLIBC__
#define BACKTRACE_MAX_MOM 100
  void *bbuf[BACKTRACE_MAX_MOM];
  int blev = 0;
  memset (bbuf, 0, sizeof (bbuf));
  blev = backtrace (bbuf, BACKTRACE_MAX_MOM - 1);
  char **bsym = backtrace_symbols (bbuf, blev);
  if (syslogging_mom)
    {
      if (err)
	syslog (LOG_ALERT, "MONIMELT FATAL! @%s:%d <%s:%d> %s %s (%s)",
		fil, lin, thrname, (int) mom_gettid (), timbuf,
		msg, strerror (err));
      else
	syslog (LOG_ALERT, "MONIMELT FATAL! @%s:%d <%s:%d> %s %s",
		fil, lin, thrname, (int) mom_gettid (), timbuf, msg);
      for (int i = 0; i < blev; i++)
	syslog (LOG_ALERT, "MONIMELTB![%d]: %s", i, bsym[i]);
    }
  else
    {
      if (err)
	fprintf (stderr, "MONIMELT FATAL @%s:%d <%s:%d> %s %s (%s)\n",
		 fil, lin, thrname, (int) mom_gettid (), timbuf,
		 msg, strerror (err));
      else
	fprintf (stderr, "MONIMELT FATAL @%s:%d <%s:%d> %s %s\n",
		 fil, lin, thrname, (int) mom_gettid (), timbuf, msg);
      for (int i = 0; i < blev; i++)
	fprintf (stderr, "MONIMELTB[%d]: %s\n", i, bsym[i]);
      fflush (NULL);
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


static int randomfd_mom;

MOM_PRIVATE void
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



static const char *
dbg_level_mom (enum mom_debug_en dbg)
{
#define LEVDBG(Dbg) case momdbg_##Dbg: return #Dbg;
  switch (dbg)
    {
      MOM_DEBUG_LIST_OPTIONS (LEVDBG);
    default:
      {
	static char dbglev[16];
	snprintf (dbglev, sizeof (dbglev), "?DBG?%d", (int) dbg);
	return GC_STRDUP (dbglev);
      }
    }
#undef LEVDBG
}

const char *const mom_debug_names[momdbg__last] = {
#define DEFINE_DBG_NAME_MOM(Dbg) [momdbg_##Dbg]= #Dbg,
  MOM_DEBUG_LIST_OPTIONS (DEFINE_DBG_NAME_MOM)
};

#undef DEFINE_DBG_NAME_MOM




momhash_t
mom_cstring_hash_len (const char *str, int len)
{
  if (!str)
    return 0;
  if (len < 0)
    len = strlen (str);
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
}				/* end mom_cstring_hash_len */

void
mom_print_sizes (void)
{
#define PRINT_SIZEOF(T)						\
  printf("sizeof(" #T ") = %d by = %d wd\n",			\
	 (int)sizeof(T),					\
	 (int) ((sizeof(T)+sizeof(void*)-1)/sizeof(void*)))
  PRINT_SIZEOF (int);
  PRINT_SIZEOF (long);
  PRINT_SIZEOF (void *);
  PRINT_SIZEOF (pthread_mutex_t);
  PRINT_SIZEOF (pthread_cond_t);
  PRINT_SIZEOF (struct mom_anyvalue_st);
  PRINT_SIZEOF (struct mom_boxint_st);
  PRINT_SIZEOF (struct mom_boxdouble_st);
  PRINT_SIZEOF (struct mom_boxstring_st);
  PRINT_SIZEOF (struct mom_seqitems_st);
  PRINT_SIZEOF (struct mom_boxnode_st);
  PRINT_SIZEOF (struct mom_assovaldata_st);
  PRINT_SIZEOF (struct mom_vectvaldata_st);
  PRINT_SIZEOF (struct mom_tasklet_st);
  PRINT_SIZEOF (struct mom_taskstepper_st);
  PRINT_SIZEOF (struct mom_item_st);
  PRINT_SIZEOF (struct mom_nanoeval_st);
  PRINT_SIZEOF (struct mom_nanotaskstep_st);
}

void
mom_set_debugging (const char *dbgopt)
{
  char dbuf[256];
  if (!dbgopt)
    return;
  memset (dbuf, 0, sizeof (dbuf));
  if (strlen (dbgopt) >= sizeof (dbuf) - 1)
    MOM_FATAPRINTF ("too long debug option %s", dbgopt);
  strcpy (dbuf, dbgopt);
  char *comma = NULL;
  if (!strcmp (dbuf, ".") || !strcmp (dbuf, "_"))
    {
      mom_debugflags = ~0;
      MOM_INFORMPRINTF ("set all debugging");
    }
  else
    for (char *pc = dbuf; pc != NULL; pc = comma ? comma + 1 : NULL)
      {
	comma = strchr (pc, ',');
	if (comma)
	  *comma = (char) 0;
#define MOM_TEST_DEBUG_OPTION(Nam)			\
	if (!strcmp(pc,#Nam))		{		\
	  mom_debugflags |=  (1<<momdbg_##Nam); } else	\
	  if (!strcmp(pc,"!"#Nam))			\
	    mom_debugflags &=  ~(1<<momdbg_##Nam); else
	if (!pc)
	  break;
	MOM_DEBUG_LIST_OPTIONS (MOM_TEST_DEBUG_OPTION) if (pc && *pc)
	  MOM_WARNPRINTF ("unrecognized debug flag %s", pc);
      }
  char alldebugflags[2 * sizeof (dbuf) + 120];
  memset (alldebugflags, 0, sizeof (alldebugflags));
  int nbdbg = 0;
#define MOM_SHOW_DEBUG_OPTION(Nam) do {		\
    if (mom_debugflags & (1<<momdbg_##Nam)) {	\
     strcat(alldebugflags, " " #Nam);		\
     assert (strlen(alldebugflags)		\
	     <sizeof(alldebugflags)-3);		\
     nbdbg++;					\
    } } while(0);
  MOM_DEBUG_LIST_OPTIONS (MOM_SHOW_DEBUG_OPTION);
  if (nbdbg > 0)
    MOM_INFORMPRINTF ("%d debug flags active:%s.", nbdbg, alldebugflags);
  else
    MOM_INFORMPRINTF ("no debug flags active.");
}


/* Option specification for getopt_long.  */
enum extraopt_en
{
  xtraopt__none = 0,
  xtraopt_chdir_first = 1024,
  xtraopt_chdir_after_load,
  xtraopt_addpredef,
  xtraopt_commentpredef,
  xtraopt_unpredef,
  xtraopt_webdir,
  xtraopt_skipmadecheck,
  xtraopt_skipdumphooks,
  xtraopt_dontmakeafterdump,
  xtraopt_info,
  xtraopt_testarg,
  xtraopt_testrun,
  xtraopt_outputcontent,
};

static const struct option mom_long_options[] = {
  {"help", no_argument, NULL, 'h'},
  {"version", no_argument, NULL, 'V'},
  {"debug", required_argument, NULL, 'D'},
  {"load", required_argument, NULL, 'L'},
  {"web", required_argument, NULL, 'W'},
  {"jobs", required_argument, NULL, 'J'},
  {"dump", no_argument, NULL, 'd'},
  {"syslog", no_argument, NULL, 's'},
  {"skip-made-check", no_argument, NULL, xtraopt_skipmadecheck},
  {"skip-dump-hooks", no_argument, NULL, xtraopt_skipdumphooks},
  {"dont-make-after-dump", no_argument, NULL, xtraopt_dontmakeafterdump},
  {"chdir-first", required_argument, NULL, xtraopt_chdir_first},
  {"chdir-after-load", required_argument, NULL, xtraopt_chdir_after_load},
  {"add-predefined", required_argument, NULL, xtraopt_addpredef},
  {"unpredef", required_argument, NULL, xtraopt_unpredef},
  {"comment-predefined", required_argument, NULL, xtraopt_commentpredef},
  {"output-content", required_argument, NULL, xtraopt_outputcontent},
  {"test-arg", required_argument, NULL, xtraopt_testarg},
  {"test-run", required_argument, NULL, xtraopt_testrun},
  {"webdir", required_argument, NULL, xtraopt_webdir},
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
  printf ("\t -W | --web <webservice>"
	  " \t# start a web service, e.g. localhost.localdomain:8085 or _:8085\n");
  printf ("\t -J | --jobs <nbjobs>"
	  " \t#set number of jobs, default %d, max %d\n", mom_nbjobs,
	  MOM_JOB_MAX);
  printf ("\t -d | --dump " " \t# Dump the state.\n");
  printf ("\t -s | --syslog " " \t# Use system log.\n");
  printf ("\t -D | --debug <debug-features>"
	  " \t# Debugging comma separated features\n\t\t##");
  for (unsigned ix = 1; ix < momdbg__last; ix++)
    printf (" %s", mom_debug_names[ix]);
  putchar ('\n');
  printf ("\t -L | --load statefile" " \t#Load a state \n");
  printf ("\t --chdir-first dirpath" " \t#Change directory at first \n");
  printf ("\t --chdir-after-load dirpath"
	  " \t#Change directory after load\n");
  printf ("\t --add-predefined predefname" " \t#Add a predefined\n");
  printf ("\t --comment-predefined comment"
	  " \t#Set comment of next predefined\n");
  printf ("\t --output-content <item>"
	  " \t#output the content of given <item>\n");
  printf ("\t --skip-made-check"
	  " \t#Skip the check that the binary is made up to date\n");
  printf ("\t --skip-dump-hooks"
	  " \t#Ignore the `dump_before` and `dump_after` hooks\n");
  printf ("\t --dont-make-after-dump"
	  " \t#Don't rebuild with make the binary after a dump\n");
  printf ("\t --test-arg <testarg>" " \t#Argument to next test.\n");
  printf ("\t --unpredef <predefname>" " \t#Unpredefine an item.\n");
  printf ("\t --test-run <testname>" " \t#Name of test to run after load.\n");
  printf ("\t --info" " \t#Give various information\n");
}



static void
print_version_mom (const char *argv0)
{
  printf ("%s built on %s gitcommit %s\n", argv0,
	  monimelt_timestamp, monimelt_lastgitcommit);
}


MOM_PRIVATE void
parse_program_arguments_mom (int *pargc, char ***pargv)
{
  int argc = *pargc;
  char **argv = *pargv;
  int opt = -1;
  char *commentstr = NULL;
  char *testargstr = NULL;
  char *suffix = NULL;
  while ((opt = getopt_long (argc, argv, "hVdsD:L:W:J:",
			     mom_long_options, NULL)) >= 0)
    {
      switch (opt)
	{
	case 'h':		/* --help */
	  usage_mom (argv[0]);
	  putchar ('\n');
	  fputs ("\nVersion info:::::\n", stdout);
	  print_version_mom (argv[0]);
	  exit (EXIT_FAILURE);
	  return;
	case 'V':		/* --version */
	  print_version_mom (argv[0]);
	  exit (EXIT_SUCCESS);
	  return;
	case 'W':		/* --web */
	  if (!optarg)
	    MOM_FATAPRINTF ("missing web service");
	  web_service_mom = optarg;
	  break;
	case 'd':		/* --dump */
	  should_dump_mom = true;
	  break;
	case 'D':		/* --debug */
	  mom_set_debugging (optarg);
	  break;
	case 'L':		/* load */
	  if (!optarg || access (optarg, R_OK))
	    MOM_FATAPRINTF ("bad load state %s : %m", optarg);
	  load_state_mom = optarg;
	  break;
	case 's':		/* --syslog */
	  openlog ("monimelt", LOG_PID | LOG_PERROR, LOG_LOCAL1);
	  syslogging_mom = true;
	  MOM_INFORMPRINTF ("syslogging activated");
	  break;
	case 'J':		/* --jobs <nbjobs> */
	  {
	    if (!optarg)
	      MOM_FATAPRINTF ("--jobs require a <nb-jobs>");
	    int n = atoi (optarg);
	    if (n < 2 || n > MOM_JOB_MAX)
	      MOM_FATAPRINTF
		("bad number %d of jobs, should be at least 2 and less than %d",
		 n, MOM_JOB_MAX);
	    mom_nbjobs = n;
	  }
	  break;
	case xtraopt_chdir_first:	/* --chdir-first <dirpath> */
	  {
	    if (!optarg)
	      MOM_FATAPRINTF ("missing --chdir-first argument");
	    if (chdir (optarg))
	      MOM_FATAPRINTF ("--chdir-first %s failed %m", optarg);
	    char cwdbuf[256];
	    memset (cwdbuf, 0, sizeof (cwdbuf));
	    if (!getcwd (cwdbuf, sizeof (cwdbuf) - 1))
	      strcpy (cwdbuf, ".");
	    MOM_INFORMPRINTF ("changed directory at first to %s", cwdbuf);
	  }
	  break;
	case xtraopt_chdir_after_load:	/* --chdir-after-load <dirpath> */
	  {
	    if (!optarg)
	      MOM_FATAPRINTF ("missing --chdir-after-load argument");
	    if (access (optarg, F_OK))
	      {
		if (mkdir (optarg, 0750))
		  MOM_FATAPRINTF ("failed to mkdir %s (for after load) : %m",
				  optarg);
		else
		  MOM_INFORMPRINTF ("made directory %s (for after load)",
				    optarg);
	      }
	    struct stat stdir = { 0 };
	    if (stat (optarg, &stdir) || (stdir.st_mode & S_IFMT) != S_IFDIR)
	      MOM_WARNPRINTF ("%s is not a directory for --chdir-after-load",
			      optarg);
	    dir_after_load_mom = GC_STRDUP (optarg);
	  }
	  break;
	case xtraopt_skipmadecheck:
	  skipmadecheck_mom = true;
	  break;
	case xtraopt_skipdumphooks:
	  mom_skip_dump_hooks = true;
	  MOM_INFORMPRINTF ("dump hooks will be skipped");
	  break;
	case xtraopt_dontmakeafterdump:
	  mom_dont_make_after_dump = true;
	  MOM_INFORMPRINTF ("won't run make after dump");
	  break;
	case xtraopt_commentpredef:
	  commentstr = optarg;
	  break;
	case xtraopt_addpredef:
	  if (!optarg)
	    MOM_FATAPRINTF ("missing predefined name for --add-predefined");
	  if (!isalpha (optarg[0]) || !mom_valid_name_radix (optarg))
	    MOM_FATAPRINTF ("invalid predefined name %s", optarg);
	  if (count_added_predef_mom >= MAX_ADDED_PREDEF_MOM)
	    MOM_FATAPRINTF ("too many %d added predefined",
			    count_added_predef_mom);
	  added_predef_mom[count_added_predef_mom].predef_name = optarg;
	  added_predef_mom[count_added_predef_mom].predef_comment =
	    commentstr;
	  commentstr = NULL;
	  should_dump_mom = true;
	  count_added_predef_mom++;
	  break;
	case xtraopt_unpredef:
	  if (!optarg)
	    MOM_FATAPRINTF ("missing predefined name for --unpredef");
	  if (!isalpha (optarg[0]) || !mom_valid_name_radix (optarg))
	    MOM_FATAPRINTF ("invalid unpredefined name %s", optarg);
	  if (count_unpredef_mom >= MAX_UNPREDEF_MOM)
	    MOM_FATAPRINTF ("too many %d unpredefined", count_unpredef_mom);
	  unpredefined_mom[count_unpredef_mom] = optarg;
	  count_unpredef_mom++;
	  should_dump_mom = true;
	  break;
	case xtraopt_outputcontent:
	  if (optarg)
	    suffix = strstr (optarg, "__");
	  if (!suffix && optarg)
	    suffix = optarg + strlen (optarg);
	  if (!optarg || !isalpha (optarg[0])
	      || !mom_valid_name_radix_len (optarg, suffix - optarg))
	    MOM_FATAPRINTF ("missing or invalid name %s for --output-content",
			    optarg ? optarg : "??");
	  if (outcont_count_mom >= MAX_OUTPUTCONTENT_MOM)
	    MOM_FATAPRINTF
	      ("too many %d items for --output-content (last is %s)",
	       outcont_count_mom, optarg);
	  outcont_name_mom[outcont_count_mom++] = optarg;
	  break;
	case xtraopt_testarg:
	  testargstr = optarg;
	  break;
	case xtraopt_testrun:
	  {
	    const char *testarg = testargstr ? GC_strdup (testargstr) : NULL;
	    testargstr = NULL;
	    const char *testname = optarg;
	    if (testcount_mom >= MAX_TEST_MOM)
	      MOM_FATAPRINTF ("too many %d tests", testcount_mom);
	    for (const char *tp = testname; *tp; tp++)
	      if (!isalnum (*tp) && *tp != '_')
		MOM_FATAPRINTF ("invalid testname %s", testname);
	    testarr_mom[testcount_mom].test_name = testname;
	    testarr_mom[testcount_mom].test_arg = testarg;
	    testcount_mom++;
	  }
	  break;
	case xtraopt_info:
	  mom_print_sizes ();
	  break;
	case xtraopt_webdir:
	  {
	    if (!optarg)
	      MOM_FATAPRINTF ("missing --webdir");
	    char *rwdirpath = realpath (optarg, NULL);
	    char *rwdirdup = GC_STRDUP (rwdirpath);
	    struct stat rwdirstat = { 0 };
	    free (rwdirpath), rwdirpath = NULL;
	    int olderrno = errno;
	    errno = ENOTDIR;
	    if (stat (rwdirdup, &rwdirstat)
		|| (rwdirstat.st_mode & S_IFMT) != S_IFDIR)
	      MOM_FATAPRINTF ("invalid webdir %s (%m)", rwdirdup);
	    errno = olderrno;
	    int wix = -1;
	    for (int ix = 0; ix < MOM_MAX_WEBDIR; ix++)
	      if (!mom_webdir[ix])
		{
		  wix = ix;
		  break;
		};
	    if (wix < 0)
	      MOM_FATAPRINTF ("too many (%d) webdir for %s", MOM_MAX_WEBDIR,
			      optarg);
	    mom_webdir[wix] = rwdirdup;
	    MOM_DEBUGPRINTF (web, "webdir#%d %s", wix, rwdirdup);
	  }
	default:
	  MOM_FATAPRINTF ("bad option (%c) at %d", isalpha (opt) ? opt : '?',
			  optind);
	  return;
	}
    }
}				/* end of parse_program_arguments_mom */




MOM_PRIVATE void
do_add_predefined_mom (void)
{
  for (unsigned ix = 0; ix < count_added_predef_mom; ix++)
    {
      const char *end = NULL;
      struct mom_item_st *preditm =
	mom_make_item_from_string (added_predef_mom[ix].predef_name, &end);
      if (end && *end)
	MOM_FATAPRINTF ("bad predefined name '%s'",
			added_predef_mom[ix].predef_name);
      assert (preditm && preditm->va_itype == MOMITY_ITEM);
      mom_item_put_space (preditm, MOMSPA_PREDEF);
      const char *comm = added_predef_mom[ix].predef_comment;
      if (comm && comm[0])
	{
	  pthread_mutex_lock (&preditm->itm_mtx);
	  time (&preditm->itm_mtime);
	  preditm->itm_pattr =	//
	    mom_assovaldata_put (preditm->itm_pattr,
				 MOM_PREDEFITM (comment),
				 (struct mom_hashedvalue_st *)
				 mom_boxstring_make (comm));
	  pthread_mutex_unlock (&preditm->itm_mtx);
	  MOM_INFORMPRINTF ("made predefined %s with comment %s",
			    mom_item_cstring (preditm), comm);
	}
      else
	{
	  MOM_INFORMPRINTF ("made predefined %s without comment",
			    mom_item_cstring (preditm));
	}
    }
}				/* end of do_add_predefined_mom */

MOM_PRIVATE void
do_unpredefine_mom (void)
{
  for (int ix = 0; ix < count_unpredef_mom; ix++)
    {
      const char *unpname = unpredefined_mom[ix];
      struct mom_item_st *unpitm = mom_find_item_by_string (unpname);
      if (!unpitm)
	MOM_FATAPRINTF ("unknown name#%d %s to unpredefine", ix + 1, unpname);
      if (unpitm->va_ixv != MOMSPA_PREDEF)
	MOM_WARNPRINTF ("name#%d to unpredefine %s is not predefined",
			ix + 1, mom_item_cstring (unpitm));
      else
	{
	  mom_item_put_space (unpitm, MOMSPA_GLOBAL);
	  MOM_INFORMPRINTF ("unpredefined item %s",
			    mom_item_cstring (unpitm));
	}
    }
}				/* end do_unpredefine_mom */

void
momtest_comparesig (const char *arg)
{
  MOM_INFORMPRINTF ("start momtest_comparesig arg=%s", arg);
  MOM_FATAPRINTF ("missing momtest_comparesig arg=%s", arg);
#warning momtest_comparesig missing
}				/* end of momtest_comparesig */


MOM_PRIVATE void
do_run_tests_mom (void)
{
  MOM_INFORMPRINTF ("should run %d tests.", testcount_mom);
  assert (testcount_mom <= MAX_TEST_MOM);
  for (unsigned tix = 0; tix < testcount_mom; tix++)
    {
      char tnambuf[80];
      memset (tnambuf, 0, sizeof (tnambuf));
      const char *testname = testarr_mom[tix].test_name;
      const char *testarg = testarr_mom[tix].test_arg;
      MOM_INFORMPRINTF ("running test#%d %s with %s", tix, testname, testarg);
      snprintf (tnambuf, sizeof (tnambuf), "momtest_%s", testname);
      void *ad = dlsym (mom_prog_dlhandle, tnambuf);
      if (!ad)
	MOM_FATAPRINTF ("missing test %s - %s", tnambuf, dlerror ());
      mom_test_sig_t *tfun = (mom_test_sig_t *) ad;
      (*tfun) (testarg);
      MOM_INFORMPRINTF ("done test#%d %s with %s\n", tix, testname, testarg);
    }
  MOM_INFORMPRINTF ("*all %d tests run successfully*", testcount_mom);
}				/* end of do_run_tests_mom */

void
mom_stop_and_dump (void)
{
  should_dump_mom = true;
  mom_stop ();
}

int
main (int argc_main, char **argv_main)
{
  clock_gettime (CLOCK_REALTIME, &start_realtime_ts_mom);
  gethostname (hostname_mom, sizeof (hostname_mom) - 1);
  GC_INIT ();
  GC_set_handle_fork (1);
  GC_register_displacement (offsetof (struct mom_itemname_tu, itname_string));
  char **argv = argv_main;
  int argc = argc_main;
  mom_prog_dlhandle = dlopen (NULL, RTLD_NOW);
  if (!mom_prog_dlhandle)
    MOM_FATAPRINTF ("failed to dlopen program (%s)", dlerror ());
  mom_random_init_genrand ();
  if (argc > 1 && argv[1][0] == '-' && argv[1][1] == 'D')
    mom_set_debugging (argv[1] + 2);
  mom_initialize_items ();
  parse_program_arguments_mom (&argc, &argv);
  if (!skipmadecheck_mom)
    {
      MOM_DEBUGPRINTF (run, "before running 'make -q monimelt'");
      fflush (NULL);
      int okmaket = system ("make -q monimelt");
      if (!okmaket)
	MOM_INFORMPRINTF ("monimelt is made up to date");
      else
	{
	  MOM_WARNPRINTF ("pass --skip-made-check to avoid up to date check");
	  MOM_FATAPRINTF
	    ("monimelt is not up to date, 'make -t monimelt' gave %d",
	     okmaket);
	}
    }
  if (!load_state_mom && !access (MOM_GLOBAL_STATE, R_OK))
    {
      load_state_mom = MOM_GLOBAL_STATE;
      MOM_INFORMPRINTF ("will load state from default global state %s",
			MOM_GLOBAL_STATE);
    }
  if (load_state_mom)
    {
      char webuf[256];
      memset (webuf, 0, sizeof (webuf));
      if (strlen (load_state_mom) >= sizeof (webuf) - 8)
	MOM_WARNPRINTF ("too long load state %s", load_state_mom);
      if (web_service_mom)
	{
	  strncpy (webuf, load_state_mom,
		   sizeof (webuf) - sizeof (MOM_LOAD_WEBDIR) - 2);
	  char *lastslash = strrchr (webuf, '/');
	  if (lastslash)
	    *lastslash = 0;
	  else
	    strcpy (webuf, ".");
	  strcat (webuf, "/" MOM_LOAD_WEBDIR);
	  MOM_DEBUGPRINTF (web, "implicit webuf=%s", webuf);
	  struct stat webstat = { 0 };
	  if (!stat (webuf, &webstat)
	      && (webstat.st_mode & S_IFMT) == S_IFDIR)
	    {
	      char *rwdirpath = realpath (webuf, NULL);
	      char *rwdirdup = GC_STRDUP (rwdirpath);
	      free (rwdirpath), rwdirpath = NULL;
	      int wix = -1;
	      for (int ix = 0; ix < MOM_MAX_WEBDIR; ix++)
		if (!mom_webdir[ix])
		  {
		    wix = ix;
		    break;
		  };
	      if (wix < 0)
		MOM_FATAPRINTF ("too many (%d) webdir for %s",
				MOM_MAX_WEBDIR, optarg);
	      mom_webdir[wix] = rwdirdup;
	      MOM_DEBUGPRINTF (web, "implicit webdir#%d %s", wix, rwdirdup);
	    }
	}
      mom_load_state (load_state_mom);
    }
  if (count_added_predef_mom > 0)
    do_add_predefined_mom ();
  if (count_unpredef_mom > 0)
    do_unpredefine_mom ();
  if (outcont_count_mom > 0)
    {
      MOM_INFORMPRINTF ("will output content of %d items", outcont_count_mom);
      for (unsigned ix = 0; ix < outcont_count_mom; ix++)
	{
	  const char *outname = outcont_name_mom[ix];
	  struct mom_item_st *outitm = mom_find_item_by_string (outname);
	  if (!outitm)
	    MOM_FATAPRINTF ("cannot find item '%s' to be output", outname);
	  MOM_INFORMPRINTF ("output #%d content of %s ::", ix, outname);
	  mom_output_item_content (stdout, NULL, outitm);
	  fputc ('\n', stdout);
	  fflush (stdout);
	}
    }
  if (testcount_mom > 0)
    do_run_tests_mom ();
  if (dir_after_load_mom)
    {
      if (chdir (dir_after_load_mom))
	MOM_FATAPRINTF ("failed to chdir to %s after load : %m",
			dir_after_load_mom);
      else
	{
	  char cwdbuf[128];
	  memset (cwdbuf, 0, sizeof (cwdbuf));
	  if (!getcwd (cwdbuf, sizeof (cwdbuf) - 1))
	    strcpy (cwdbuf, "./");
	  MOM_INFORMPRINTF ("changed directory to %s after load, now in %s",
			    dir_after_load_mom, cwdbuf);
	}
    }
  if (web_service_mom)
    {
      mom_start_web (web_service_mom);
      mom_start_agenda ();
    }
  long nbwaitloop = 0;
  while (atomic_load (&mom_should_run))
    {
      usleep ((MOM_IS_DEBUGGING (run) ? 4500 : 200) * 1000);
      nbwaitloop++;
      MOM_DEBUGPRINTF (run, "waiting while should run nbwaitloop=%ld",
		       nbwaitloop);
    }
  usleep (10 * 1000);
  MOM_INFORMPRINTF ("stop running pid %d", (int) getpid ());
  if (web_service_mom)
    {
      usleep (5 * 1000);
      mom_stop_web ();
      usleep (20 * 1000);
    }
  if (should_dump_mom)
    {
      char cwdbuf[128];
      memset (cwdbuf, 0, sizeof (cwdbuf));
      if (!getcwd (cwdbuf, sizeof (cwdbuf) - 1))
	strcpy (cwdbuf, "./");
      MOM_INFORMPRINTF ("dumping state in %s", cwdbuf);
      mom_dump_state ();
    }
  if (count_added_predef_mom > 0 && !dir_after_load_mom)
    {
      MOM_INFORMPRINTF ("making again after adding %d predefined",
			count_added_predef_mom);
      char cmdbuf[128];
      memset (cmdbuf, 0, sizeof (cmdbuf));
      if (snprintf
	  (cmdbuf, sizeof (cmdbuf), "make -j 3 OPTIMFLAGS='%s'",
	   monimelt_optimflags) >= (int) sizeof (cmdbuf) - 1)
	MOM_FATAPRINTF ("too small command buffer %s", cmdbuf);
      int bad = system (cmdbuf);
      if (bad)
	MOM_FATAPRINTF ("%s failed (%d)", cmdbuf, bad);
      MOM_INFORMPRINTF ("made ok after adding %d predefined",
			count_added_predef_mom);
    }
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
}				/* end of main */

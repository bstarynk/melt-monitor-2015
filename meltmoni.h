// file meltmoni.h - common header file to be included everywhere.

/**   Copyright (C)  2016 Basile Starynkevitch, later FSF
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
#ifndef MONIMELT_HEADER
#define MONIMELT_HEADER "meltmoni.h"
#define MONIMELT_GTK_STYLE "gtk-monimelt.css"

#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif /*_GNU_SOURCE*/

#define GC_THREADS 1
#define HAVE_PTHREADS 1
#define MOM_GETTEXT_PACKAGE "monimelt"


#include <features.h>           // GNU things
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdatomic.h>
#include <string.h>
#include <gc/gc.h>
#include <stdbool.h>
#include <stdlib.h>
#include <setjmp.h>
#include <inttypes.h>
#include <string.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <pthread.h>
#include <sched.h>
#include <syslog.h>
#include <math.h>
#include <float.h>
#include <limits.h>
#include <ctype.h>
#include <getopt.h>
#include <errno.h>
#include <fnmatch.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/poll.h>
#include <sys/select.h>
#include <sys/signalfd.h>
#include <sys/timerfd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/wait.h>
#include <sys/file.h>
#include <sys/resource.h>
#include <sys/un.h>
#include <fcntl.h>
#include <dlfcn.h>

//- // libonion from http://www.coralbits.com/libonion/ &
//- // https://github.com/davidmoreno/onion
//- #include <onion/onion.h>
//- #include <onion/version.h>
//- #include <onion/low.h>
//- #include <onion/codecs.h>
//- #include <onion/request.h>
//- #include <onion/response.h>
//- #include <onion/block.h>
//- #include <onion/handler.h>
//- #include <onion/dict.h>
//- #include <onion/log.h>
//- #include <onion/shortcuts.h>
//- #include <onion/exportlocal.h>
//- #include <onion/internal_status.h>
//- #include <onion/websocket.h>

// jansson, a JSON library in C which is Boehm-GC friendly
// see http://www.digip.org/jansson/
#include <jansson.h>
// Glib & GTK3 see http://gtk.org/
#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
// Sqlite3 see http://sqlite.org
#include <sqlite3.h>
#if __GLIBC__
#include <execinfo.h>
#include <gnu/libc-version.h>
#endif
// libbacktrace from GCC 6, i.e. libgcc-6-dev package
#include <backtrace.h>

// mark unlikely conditions to help optimization
#ifdef __GNUC__
#define MOM_UNLIKELY(P) __builtin_expect(!!(P),0)
#define MOM_LIKELY(P) !__builtin_expect(!(P),0)
#define MOM_UNUSED __attribute__((unused))
#define MOM_OPTIMIZEDFUN __attribute__((optimize("O2")))
#else
#define MOM_UNLIKELY(P) (P)
#define MOM_LIKELY(P) (P)
#define MOM_UNUSED
#define MOM_OPTIMIZEDFUN
#endif

/// see https://en.wikipedia.org/wiki/ANSI_escape_code
extern gboolean mom_no_color_stderr;
#define MOM_TERMWARNCOLOR (mom_no_color_stderr?"":"\033[1m""\033[34m" /*bold blue*/)
#define MOM_TERMFATALCOLOR (mom_no_color_stderr?"":"\033[1m""\033[31m" /*bold red*/)
#define MOM_TERMPLAIN (mom_no_color_stderr?"":"\033[0m")

void
mom_warnprintf_at (const char *fil, int lin, const char *fmt, ...)
__attribute__ ((format (printf, 3, 4)));


#define MOM_WARNPRINTF_AT(Fil,Lin,Fmt,...) do { \
    mom_warnprintf_at (Fil,Lin,Fmt,   \
           ##__VA_ARGS__);    \
  } while(0)

#define MOM_WARNPRINTF_AT_BIS(Fil,Lin,Fmt,...)  \
  MOM_WARNPRINTF_AT(Fil,Lin,Fmt,    \
        ##__VA_ARGS__)

#define MOM_WARNPRINTF(Fmt,...)     \
  MOM_WARNPRINTF_AT_BIS(__FILE__,__LINE__,Fmt,  \
      ##__VA_ARGS__)


typedef atomic_bool mom_atomic_bool_t;
typedef atomic_int mom_atomic_int_t;
typedef atomic_int_least16_t mom_atomic_int16_t;
typedef FILE *_Atomic mom_atomic_fileptr_t;
typedef json_t *mo_json_t;
typedef struct mo_dumper_st mo_dumper_ty;
#define thread_local _Thread_local


#define MOM_DUMP_VERSIONID "MoniMelt2016B"

// the generated modules directory; known to the Makefile
#define MOM_MODULES_DIR "modules.dir"
// in which a module of id FooId is in moduFooId.c & moduFooId.so
#define MOM_MODULE_INFIX "modu"
#define MOM_MODULE_SUFFIX ".so"
// the generated header file for predefined objects
#define MOM_PREDEF_HEADER "_mom_predef.h"
// the generated header file for global objects
#define MOM_GLOBAL_HEADER "_mom_global.h"

// the dump shell script
#define MOM_DUMP_SCRIPT "monimelt-dump-state.sh"


// in generated _timestamp.c
extern const char monimelt_timestamp[];
extern const char monimelt_lastgitcommit[];
extern const char monimelt_lastgittag[];
extern const char monimelt_compilercommand[];
extern const char monimelt_compilerflags[];
extern const char monimelt_compilerversion[];
extern const char monimelt_optimflags[];
extern const char monimelt_checksum[];
extern const char monimelt_directory[];
extern const char monimelt_makefile[];
extern const char monimelt_sqlite[];
extern const char monimelt_perstatebase[];
extern const char *const monimelt_csources[];
extern const char *const monimelt_shellsources[];

// increasing array of primes and its size
extern const int64_t mom_primes_tab[];
extern const unsigned mom_primes_num;
/// give a prime number above or below a given n, or else 0
int64_t mom_prime_above (int64_t n);
int64_t mom_prime_below (int64_t n);

static_assert (sizeof (intptr_t) == sizeof (double)
               || 2 * sizeof (intptr_t) == sizeof (double),
               "double-s should be the same size or twice as intptr_t");

const char *mom_hostname (void);


// A non nil address which is *never* dereferencable and can be used
// as an empty placeholder; in practice all Unix & POSIX systems dont
// use that address
#define MOM_EMPTY_SLOT ((void*)(2*sizeof(void*)))
// maximum number of threads
#define MOM_JOB_MAX 16
extern __thread int mom_worker_num;

#define MOM_SIZE_MAX (1<<28)

static inline pid_t
mom_gettid (void)
{
  return syscall (SYS_gettid, 0L);
}

/// generate a GPLv3 notice
void mom_output_gplv3_notice (FILE *out, const char *prefix,
                              const char *suffix, const char *filename);

void
mom_fataprintf_at (const char *fil, int lin, const char *fmt, ...)
__attribute__ ((format (printf, 3, 4), noreturn));

#define MOM_FATAPRINTF_AT(Fil,Lin,Fmt,...) do { \
    mom_fataprintf_at (Fil,Lin,Fmt,   \
           ##__VA_ARGS__);    \
  } while(0)

#define MOM_FATAPRINTF_AT_BIS(Fil,Lin,Fmt,...)  \
  MOM_FATAPRINTF_AT(Fil,Lin,Fmt,    \
        ##__VA_ARGS__)

#define MOM_FATAPRINTF(Fmt,...)     \
  MOM_FATAPRINTF_AT_BIS(__FILE__,__LINE__,Fmt,  \
      ##__VA_ARGS__)



#ifdef NDEBUG
// just call printf under if false, to get it typechecked
#define MOM_ASSERTPRINTF(Cond,Fmt,...) \
  do { if (false && (Cond)) printf(Fmt, ##__VA_ARGS__); } while(0)
#else

#define MOM_ASSERTPRINTF_AT(Fil,Lin,Cond,Fmt,...) do {  \
  if (MOM_UNLIKELY(!(Cond)))        \
    MOM_FATAPRINTF_AT(Fil,Lin,        \
          "MOM_ASSERT FAIL(" #Cond    \
          "): \n *@* "      \
          Fmt "\n",       \
          ##__VA_ARGS__); }while(0)

#define MOM_ASSERTPRINTF_AT_BIS(Fil,Lin,Cond,Fmt,...) \
  MOM_ASSERTPRINTF_AT(Fil,Lin,Cond,Fmt,     \
        ##__VA_ARGS__)

#define MOM_ASSERTPRINTF(Cond,Fmt,...)    \
  MOM_ASSERTPRINTF_AT_BIS(__FILE__,__LINE__,Cond,Fmt, \
      ##__VA_ARGS__)

#endif /*NDEBUG*/
void mom_gc_warn_big_alloc (size_t sz);

#define MOM_BIGALLOC_THRESHOLD (sizeof(void*)<<20)
static inline void *
mom_gc_alloc (size_t sz)
{
  if (MOM_UNLIKELY (sz > MOM_BIGALLOC_THRESHOLD))
    mom_gc_warn_big_alloc (sz);
  void *p = GC_MALLOC (sz);
  if (MOM_UNLIKELY (p == NULL))
    MOM_FATAPRINTF ("failed to allocate %zd bytes", sz);
  memset (p, 0, sz);
  return p;
}

static inline void *
mom_gc_alloc_scalar (size_t sz)
{
  if (MOM_UNLIKELY (sz > MOM_BIGALLOC_THRESHOLD))
    mom_gc_warn_big_alloc (sz);
  void *p = GC_MALLOC_ATOMIC (sz);
  if (MOM_UNLIKELY (p == NULL))
    MOM_FATAPRINTF ("failed to allocate %zd scalar bytes", sz);
  memset (p, 0, sz);
  return p;
}

static inline void *
mom_gc_alloc_uncollectable (size_t sz)
{
  if (MOM_UNLIKELY (sz > MOM_BIGALLOC_THRESHOLD))
    mom_gc_warn_big_alloc (sz);
  void *p = GC_MALLOC_UNCOLLECTABLE (sz);
  if (MOM_UNLIKELY (p == NULL))
    MOM_FATAPRINTF ("failed to allocate %zd uncollectable bytes", sz);
  memset (p, 0, sz);
  return p;
}

static inline char *
mom_gc_strdup (const char *s)
{
  if (!s || s == MOM_EMPTY_SLOT)
    return NULL;
  size_t slen = strlen (s);
  if (MOM_UNLIKELY (slen > MOM_BIGALLOC_THRESHOLD))
    mom_gc_warn_big_alloc (slen);
  unsigned sz = mom_prime_above ((1 + (slen + 1) / 4)) * 4;
  MOM_ASSERTPRINTF ((size_t) sz > slen + 1,
                    "bad sz %u for slen %zd", sz, slen);
  char *p = GC_MALLOC_ATOMIC (sz);
  memcpy (p, s, slen + 1);
  if (MOM_UNLIKELY (p == NULL))
    MOM_FATAPRINTF ("failed to gc strdup %s", s);
  return p;
}                               /* end of mom_gc_strdup */

const char *mom_gc_printf (const char *fmt, ...)
__attribute__ ((format (printf, 1, 2)));

void
mom_informprintf_at (const char *fil, int lin, const char *fmt, ...)
__attribute__ ((format (printf, 3, 4)));

#define MOM_INFORMPRINTF_AT(Fil,Lin,Fmt,...) do { \
    mom_informprintf_at (Fil,Lin,Fmt,     \
       ##__VA_ARGS__);    \
  } while(0)

#define MOM_INFORMPRINTF_AT_BIS(Fil,Lin,Fmt,...)  \
  MOM_INFORMPRINTF_AT(Fil,Lin,Fmt,      \
          ##__VA_ARGS__)

#define MOM_INFORMPRINTF(Fmt,...)     \
  MOM_INFORMPRINTF_AT_BIS(__FILE__,__LINE__,Fmt,  \
        ##__VA_ARGS__)


void
mom_backtraceprintf_at (const char *fil, int lin, const char *fmt, ...)
__attribute__ ((format (printf, 3, 4)));

#define MOM_BACKTRACEPRINTF_AT(Fil,Lin,Fmt,...) do { \
    mom_backtraceprintf_at (Fil,Lin,Fmt,     \
       ##__VA_ARGS__);    \
  } while(0)

#define MOM_BACKTRACEPRINTF_AT_BIS(Fil,Lin,Fmt,...)  \
  MOM_BACKTRACEPRINTF_AT(Fil,Lin,Fmt,      \
          ##__VA_ARGS__)

#define MOM_BACKTRACEPRINTF(Fmt,...)     \
  MOM_BACKTRACEPRINTF_AT_BIS(__FILE__,__LINE__,Fmt,  \
        ##__VA_ARGS__)

/// thread local Mersenne Twister pseudo-random number generation see
/// file mt19937ar.c a tiny adaptation from
/// http://www.math.sci.hiroshima-u.ac.jp/~m-mat/MT/VERSIONS/C-LANG/c-lang.html

void momrand_init_genrand (unsigned long s);
void momrand_init_by_array (unsigned long init_key[], int key_length);
/* generates a random number on [0,0xffffffff]-interval */
unsigned long momrand_genrand_int32 (void);
/* generates a random number on [0,0x7fffffff]-interval */
long momrand_genrand_int31 (void);
/* generates a random number on [0,1]-real-interval */
double momrand_genrand_real1 (void);
/* generates a random number on [0,1)-real-interval */
double momrand_genrand_real2 (void);
/* generates a random number on (0,1)-real-interval */
double momrand_genrand_real3 (void);
/* generates a random number on [0,1) with 53-bit resolution*/
double momrand_genrand_res53 (void);

extern void mom_random_init_genrand (void);
extern void mom_init_objects (void);

static inline uint32_t
mom_random_uint32 (void)
{
  static _Thread_local int count;
  if (MOM_UNLIKELY (count % 4096 == 0))
    mom_random_init_genrand ();
  count++;
  return (uint32_t) momrand_genrand_int32 ();
}

// the program handle from GC_dlopen with NULL
extern void *mom_prog_dlhandle;

// time measurement, in seconds
// query a clock
static inline double
mom_clock_time (clockid_t cid)
{
  struct timespec ts = { 0, 0 };
  if (clock_gettime (cid, &ts))
    return NAN;
  else
    return (double) ts.tv_sec + 1.0e-9 * ts.tv_nsec;
}

static inline struct timespec
mom_timespec (double t)
{
  struct timespec ts = { 0, 0 };
  if (isnan (t) || t < 0.0)
    return ts;
  double fl = floor (t);
  ts.tv_sec = (time_t) fl;
  ts.tv_nsec = (long) ((t - fl) * 1.0e9);
  // this should not happen
  if (MOM_UNLIKELY (ts.tv_nsec < 0))
    ts.tv_nsec = 0;
  while (MOM_UNLIKELY (ts.tv_nsec >= 1000 * 1000 * 1000))
    {
      ts.tv_sec++;
      ts.tv_nsec -= 1000 * 1000 * 1000;
    };
  return ts;
}


double mom_elapsed_real_time (void);    /* relative to start of program */
double mom_process_cpu_time (void);
double mom_thread_cpu_time (void);

extern void *mom_prog_dlhandle;
extern char *mom_dump_dir;
extern char *mom_gtk_style_path;

////////////////////////////////////////////////////////////////
// call strftime on ti, but replace .__ with centiseconds for ti
char *mom_strftime_centi (char *buf, size_t len, const char *fmt, double ti)
__attribute__ ((format (strftime, 3, 0)));
#define mom_now_strftime_centi(Buf,Len,Fmt) mom_strftime_centi((Buf),(Len),(Fmt),mom_clock_time(CLOCK_REALTIME))
#define mom_now_strftime_bufcenti(Buf,Fmt) mom_now_strftime_centi(Buf,sizeof(Buf),(Fmt))

// output with backslashes an UTF8-encoded string str; if len<0 take
// its strlen; without enclosing quotes
void mom_output_utf8_encoded (FILE *f, const char *str, int len);
// output with HTML encoding an UTF8-encoded string str; if len<0 take
// its strlen; without enclosing quotes; if nlisbr is true, newlines are emitted as <br/>
void mom_output_utf8_html (FILE *f, const char *str, int len, bool nlisbr);

typedef void mom_utf8escape_sig_t (FILE *f, gunichar uc,
                                   const char *cescstr, void *clientdata);
void mom_output_utf8_escaped (FILE *f, const char *str, int len,
                              mom_utf8escape_sig_t * rout, void *clientdata);


const char *mom_hexdump_data (char *buf, unsigned buflen,
                              const unsigned char *data, unsigned datalen);


const char *mom_double_to_cstr (double x, char *buf, size_t buflen);

// input and parse and GC-allocate such an UTF-8 quoted string; stop
// en EOL, etc..

struct mom_string_and_size_st
{
  const char *ss_str;
  int ss_len;
};

struct mom_string_and_size_st mom_input_quoted_utf8 (FILE *f);

typedef uint32_t momhash_t;

typedef __int128 mom_int128_t;
typedef unsigned __int128 mom_uint128_t;



momhash_t                       // in main.c
mom_cstring_hash_len (const char *str, int len);

////////////////////////////////////////////////////////////////


enum mo_valkind_en
{
  mo_KNONE,
  mo_KINT,
  mo_KSTRING,
  mo_KTUPLE,
  mo_KSET,
  mo_KOBJECT,
};

#define MOM_FIRST_BOXED_KIND mo_KSTRING
#define MOM_LAST_KIND mo_KOBJECT

enum mo_payloadkind_en
{
  mo_PNONE,
  mo_PASSOVALDATA = (int) MOM_LAST_KIND + 1 /* payload_assoval */ ,
  mo_PVECTVALDATA /* payload_vectval */ ,
  mo_PHASHSET /* payload_hashset */ ,
  mo_PLIST /* payload_list */ ,
  mo_PBUFFER /* payload_buffer */ ,
  mo_PCEMIT /* payload_c_emit */ ,
};

typedef const void *mo_value_t;
typedef struct mo_objectvalue_st mo_objectvalue_ty;
typedef struct mo_assovaldatapayl_st mo_assovaldatapayl_ty;
typedef struct mo_vectvaldatapayl_st mo_vectvaldatapayl_ty;
typedef mo_objectvalue_ty *mo_objref_t;
static inline int mo_objref_cmp (mo_objref_t, mo_objref_t);
static inline mo_value_t mo_dyncast_object (mo_value_t);
static inline mo_objref_t mo_dyncast_objref (mo_value_t);
typedef intptr_t mo_int_t;

typedef uint32_t mo_hid_t;
typedef uint64_t mo_loid_t;

#define MO_INTMAX INTPTR_MAX/2
#define MO_INTMIN INTPTR_MIN/2

static inline bool
mo_valid_pointer_value (const void *p)
{
  return p != NULL && p != MOM_EMPTY_SLOT && ((intptr_t) p % 2 == 0);
}

// the printable string of an object, perhaps GC-strduped; mostly for
// debugging
const char *mo_value_pnamestr (mo_value_t);

static inline bool
mo_value_is_int (mo_value_t p)
{
  return p != NULL && p != MOM_EMPTY_SLOT && ((intptr_t) p % 2 != 0);
}


static inline mo_int_t
mo_value_to_int (mo_value_t p, mo_int_t def)
{
  return mo_value_is_int (p) ? ((((intptr_t) p) & (intptr_t) (~(1ULL))) /
                                2) : def;
}

static inline mo_value_t
mo_int_to_value (mo_int_t i)
{
  MOM_ASSERTPRINTF (i >= MO_INTMIN && i <= MO_INTMAX,
                    "integer %lld out of range", (long long) i);
  return (mo_value_t) (((intptr_t) i * 2) + 1);
}

#define MOM_CSTRIDLEN 18        // used length
#define MOM_CSTRIDSIZ ((MOM_CSTRIDLEN|3)+1)
#define MOM_CSTRIDSCANF "_%17[A-Za-z0-9]"
extern const char *             // in object.c, the buf is either null -then using
// a GC allocated one- or of size MOM_CSTRIDSIZ
mo_cstring_from_hi_lo_ids (char *buf, mo_hid_t hid, mo_loid_t loid);

/* 10 * 60 * 60 so a 3 extendigit thing starting with 0 to 9 */
#define MOM_HID_BUCKETMAX 36000
static inline unsigned
mo_hi_id_bucketnum (mo_hid_t hid)
{
  if (hid == 0)
    return 0;
  unsigned bn = hid >> 16;
  MOM_ASSERTPRINTF (bn > 0 && bn < MOM_HID_BUCKETMAX,
                    "mo_hi_id_bucketnum bad hid %lu (bn=%u)",
                    (unsigned long) hid, bn);
  return bn;
}

// converse operation, fill hid & loid from a valid buffer, or else return false
extern bool
mo_get_hi_lo_ids_from_cstring (mo_hid_t * phid, mo_loid_t * ploid,
                               const char *buf);

extern void                     // in object.c
mo_get_some_random_hi_lo_ids (mo_hid_t * phid, mo_loid_t * ploid);

extern momhash_t                // in object.c
mo_hash_from_hi_lo_ids (mo_hid_t hid, mo_loid_t loid);

////////////////////////////////////////////////////////////////
/////
///// all values have some type & hash
typedef struct mo_hashedvalue_st mo_hashedvalue_ty;
#define MOMFIELDS_hashedvalue     \
  uint16_t mo_va_kind;           \
  uint16_t mo_va_index;          \
  momhash_t mo_va_hash

struct mo_hashedvalue_st
{
  MOMFIELDS_hashedvalue;
};                              // end struct mo_hashedvalue_st

static inline enum mo_valkind_en
mo_kind_of_value (mo_value_t v)
{
  if (mo_value_is_int (v))
    return mo_KINT;
  else if (!mo_valid_pointer_value (v))
    return mo_KNONE;
  else
    {
      mo_hashedvalue_ty *vh = (mo_hashedvalue_ty *) v;
      unsigned k = vh->mo_va_kind;
      MOM_ASSERTPRINTF (k >= MOM_FIRST_BOXED_KIND && k <= MOM_LAST_KIND,
                        "mo_kind_of_value: bad kind #%u @%p", k, v);
      return (enum mo_valkind_en) k;
    }
}

///// sized values have also size
typedef struct mo_sizedvalue_st mo_sizedvalue_ty;
#define MOMFIELDS_sizedvalue       \
  MOMFIELDS_hashedvalue;           \
  uint32_t mo_sva_size
struct mo_sizedvalue_st
{
  MOMFIELDS_sizedvalue;
};

static inline uint32_t
mo_size_of_value (mo_value_t v)
{
  enum mo_valkind_en k = mo_kind_of_value (v);
  if (k == mo_KSTRING || k == mo_KTUPLE || k == mo_KSET)
    {
      mo_sizedvalue_ty *vsz = (mo_sizedvalue_ty *) v;
      return vsz->mo_sva_size;
    }
  return 0;
}                               /* end mo_size_of_value */

///// string values
typedef struct mo_stringvalue_st mo_stringvalue_ty;
#define MOMFIELDS_stringvalue \
  MOMFIELDS_sizedvalue;       \
  char mo_cstr[]                /* allocated size is mo_sva_size+1 */
struct mo_stringvalue_st
{
  MOMFIELDS_stringvalue;
};                              // end struct mo_stringvalue_st
mo_value_t mo_make_string_len (const char *buf, int sz);
static inline mo_value_t
mo_make_string_cstr (const char *buf)
{
  return mo_make_string_len (buf, -1);
};

mo_value_t mo_make_string_sprintf (const char *fmt, ...)
__attribute__ ((format (printf, 1, 2)));

// make a string from the content of some FILE*, skipping some initial
// lines
mo_value_t mo_make_string_from_skipped_textual_file (FILE *fil,
    unsigned skiplines);
static inline mo_value_t
mo_make_string_from_textual_file (FILE *fil)
{
  return mo_make_string_from_skipped_textual_file (fil, 0);
}

static inline mo_value_t
mo_dyncast_string (mo_value_t v)
{
  if (!mo_valid_pointer_value (v))
    return NULL;
  mo_stringvalue_ty *vstr = (mo_stringvalue_ty *) v;
  if (vstr->mo_va_kind != mo_KSTRING)
    return NULL;
  return vstr;
}

static inline const char *
mo_string_cstr (mo_value_t v)
{
  mo_stringvalue_ty *vstr = (mo_stringvalue_ty *) mo_dyncast_string (v);
  if (!vstr)
    return NULL;
  return vstr->mo_cstr;
}

static inline unsigned
mo_string_size (mo_value_t v)
{
  mo_stringvalue_ty *vstr = (mo_stringvalue_ty *) mo_dyncast_string (v);
  if (!vstr)
    return 0;
  return vstr->mo_sva_size;
}                               /* end of mo_string_size */

/******************** SEQUENCEs ****************/
/// They are tuples or sets of (non-nil) objrefs; we might need to add
/// transient sequences later

#define MOM_UNFILLEDFAKESEQKIND 9990
typedef struct mo_sequencevalue_st mo_sequencevalue_ty;
#define MOMFIELDS_sequencevalue \
  MOMFIELDS_sizedvalue;         \
  mo_objref_t mo_seqobj[]       /* size is  mo_sva_size */

struct mo_sequencevalue_st
{
  MOMFIELDS_sequencevalue;
};

/// allocate a sequence which is not a valid object till it is
/// properly filled
static inline mo_sequencevalue_ty *
mo_sequence_allocate (unsigned sz)
{
  if (MOM_UNLIKELY (sz > MOM_SIZE_MAX))
    MOM_FATAPRINTF ("too big size %u for sequence", sz);
  mo_sequencevalue_ty *seq =
    mom_gc_alloc (sizeof (mo_sequencevalue_ty) + sz * sizeof (mo_objref_t));
  // we temporarily put a fake kind
  seq->mo_va_kind = MOM_UNFILLEDFAKESEQKIND;
  seq->mo_sva_size = sz;
  return seq;
}

/// allocate a filled sequence
static inline mo_sequencevalue_ty *
mo_sequence_filled_allocate (unsigned sz, mo_objref_t * arr)
{
  if (MOM_UNLIKELY (sz > MOM_SIZE_MAX))
    MOM_FATAPRINTF ("too big size %u for sequence", sz);
  mo_sequencevalue_ty *seq =
    mom_gc_alloc (sizeof (mo_sequencevalue_ty) + sz * sizeof (mo_objref_t));
  // we temporarily put a fake kind
  seq->mo_va_kind = MOM_UNFILLEDFAKESEQKIND;
  seq->mo_sva_size = sz;
  if (arr && arr != MOM_EMPTY_SLOT)
    memcpy (seq->mo_seqobj, arr, sz * sizeof (mo_objref_t));
  return seq;
}

static inline mo_value_t
mo_dyncast_sequence (mo_value_t v)
{
  if (!mo_valid_pointer_value (v))
    return NULL;
  mo_sequencevalue_ty *vseq = (mo_sequencevalue_ty *) v;
  if (vseq->mo_va_kind != mo_KTUPLE && vseq->mo_va_kind != mo_KSET)
    return NULL;
  return vseq;
}

static inline unsigned
mo_sequence_size (mo_value_t vs)
{
  mo_sequencevalue_ty *seq = (mo_sequencevalue_ty *) mo_dyncast_sequence (vs);
  if (!seq)
    return 0;
  return seq->mo_sva_size;
}

static inline mo_objref_t
mo_sequence_nth (mo_value_t vs, int rk)
{
  mo_sequencevalue_ty *seq = (mo_sequencevalue_ty *) mo_dyncast_sequence (vs);
  if (!seq)
    return NULL;
  unsigned sz = seq->mo_sva_size;
  if (!sz)
    return NULL;
  if (rk < 0)
    rk += sz;
  if (rk < 0 || rk >= (int) sz)
    return NULL;
  return seq->mo_seqobj[rk];
}                               /* end mo_sequence_nth */

// put inside a sequence something during the fill
static inline void
mo_sequence_put (mo_sequencevalue_ty * seq, unsigned ix, mo_objref_t oref)
{
  MOM_ASSERTPRINTF (seq != NULL, "mo_sequence_put: null sequence");
  MOM_ASSERTPRINTF (seq->mo_va_kind ==
                    MOM_UNFILLEDFAKESEQKIND,
                    "mo_sequence_put: non-fake sequence");
  MOM_ASSERTPRINTF (ix < seq->mo_sva_size,
                    "mo_sequence_put: too big index %u (size %u)", ix,
                    seq->mo_sva_size);
  seq->mo_seqobj[ix] = oref;
}

/////// tuples of objrefs
typedef struct mo_tuplevalue_st mo_tuplevalue_ty;
#define MOMFIELDS_tuplevalue MOMFIELDS_sequencevalue
struct mo_tuplevalue_st
{
  MOMFIELDS_tuplevalue;
};
/*** creating a tuple with statement expr
  {( mo_sequencevalue_ty* _sq = mo_sequence_allocate(3);
     mo_sequence_put(_sq, 0, ob0); // or sq->mo_seqobj[0] = ob0;
     mo_sequence_put(_sq, 1, ob1);
     mo_sequence_put(_sq, 2, ob2);
     mo_make_tuple_closeq(sq);
   )}
 ***/

mo_value_t mo_make_tuple_closeq (mo_sequencevalue_ty * seq);

mo_value_t mo_make_empty_tuple (void);
// convenience variadic functions to make a tuple
mo_value_t mom_make_tuple_sized (unsigned siz, /*objref-s */ ...);
mo_value_t mom_make_sentinel_tuple_ (mo_objref_t ob1, ...)
__attribute__ ((sentinel));
#define MOM_MAKE_SENTUPLE(...) mom_make_sentinel_tuple_(__VA_ARGS__,NULL)

static inline mo_value_t
mo_dyncast_tuple (mo_value_t v)
{
  if (!mo_valid_pointer_value (v))
    return NULL;
  mo_tuplevalue_ty *vtup = (mo_tuplevalue_ty *) v;
  if (vtup->mo_va_kind != mo_KTUPLE)
    return NULL;
  return vtup;
}                               /* end mo_dyncast_tuple */


static inline unsigned
mo_tuple_size (mo_value_t vs)
{
  mo_tuplevalue_ty *tup = (mo_tuplevalue_ty *) mo_dyncast_tuple (vs);
  if (!tup)
    return 0;
  return tup->mo_sva_size;
}                               /* end mo_tuple_size */

static inline mo_objref_t
mo_tuple_nth (mo_value_t vs, int rk)
{
  mo_tuplevalue_ty *tup = (mo_tuplevalue_ty *) mo_dyncast_tuple (vs);
  if (!tup)
    return NULL;
  unsigned sz = tup->mo_sva_size;
  if (!sz)
    return NULL;
  if (rk < 0)
    rk += sz;
  if (rk < 0 || rk >= (int) sz)
    return NULL;
  return tup->mo_seqobj[rk];
}

////// ordered sets
typedef struct mo_setvalue_st mo_setvalue_ty;
#define MOMFIELDS_setvalue MOMFIELDS_sequencevalue
struct mo_setvalue_st
{
  MOMFIELDS_setvalue;
};


/*** creating a set with statement expr
  {( mo_sequencevalue_ty* _sq = mo_sequence_allocate(3);
     mo_sequence_put(_sq, 0, ob0); // or sq->mo_seqobj[0] = ob0;
     mo_sequence_put(_sq, 1, ob1);
     mo_sequence_put(_sq, 2, ob2);
     mo_make_set_closeq(sq);
   )}
 ***/

mo_value_t mo_make_set_closeq (mo_sequencevalue_ty * seq);
mo_value_t mo_make_set_closortedseq (mo_sequencevalue_ty * seq);
mo_value_t mo_make_empty_set (void);
// convenience variadic functions to make a set
mo_value_t mom_make_set_sized (unsigned siz, /*objref-s */ ...);
mo_value_t mom_make_sentinel_set_ (mo_objref_t ob1, ...)
__attribute__ ((sentinel));
#define MOM_MAKE_SENSET(...) mom_make_sentinel_set_(__VA_ARGS__,NULL)

static inline mo_value_t
mo_dyncast_set (mo_value_t v)
{
  if (!mo_valid_pointer_value (v))
    return NULL;
  mo_setvalue_ty *set = (mo_setvalue_ty *) v;
  if (set->mo_va_kind != mo_KSET)
    return NULL;
  return set;
}                               /* end of mo_dyncast_set */

mo_value_t mo_set_union (mo_value_t vset1, mo_value_t vset2);
mo_value_t mo_set_intersection (mo_value_t vset1, mo_value_t vset2);
mo_value_t mo_set_difference (mo_value_t vset1, mo_value_t vset2);


static inline unsigned
mo_set_size (mo_value_t vs)
{
  mo_setvalue_ty *set = (mo_setvalue_ty *) mo_dyncast_set (vs);
  if (!set)
    return 0;
  return set->mo_sva_size;
}                               /* end mo_set_size */

static inline mo_objref_t
mo_set_nth (mo_value_t v, int rk)
{
  mo_setvalue_ty *set = (mo_setvalue_ty *) mo_dyncast_set (v);
  if (!set)
    return NULL;
  unsigned sz = set->mo_sva_size;
  if (!sz)
    return NULL;
  if (rk < 0)
    rk += sz;
  if (rk < 0 || rk >= (int) sz)
    return NULL;
  return set->mo_seqobj[rk];
}

static inline bool
mo_set_contains (mo_value_t vs, mo_objref_t ob)
{
  mo_setvalue_ty *set = (mo_setvalue_ty *) mo_dyncast_set (vs);
  if (!set)
    return false;
  if (!mo_dyncast_object (ob))
    return false;
  unsigned card = set->mo_sva_size;
  if (!card)
    return 0;
  unsigned lo = 0, hi = card - 1;
  while (lo + 5 < hi)
    {
      unsigned md = (lo + hi) / 2;
      mo_objref_t midobr = set->mo_seqobj[md];
      MOM_ASSERTPRINTF (mo_dyncast_objref (midobr) != NULL,
                        "corrupted midobr@%p", midobr);
      if (midobr == ob)
        return true;
      int cmp = mo_objref_cmp (ob, midobr);
      if (cmp < 0)
        lo = md;
      else if (cmp > 0)
        hi = md;
      else
        MOM_FATAPRINTF ("corrupted set@%p", set);
    }
  for (unsigned md = lo; md <= hi; md++)
    {
      mo_objref_t midobr = set->mo_seqobj[md];
      MOM_ASSERTPRINTF (mo_dyncast_objref (midobr) != NULL,
                        "corrupted midobr@%p", midobr);
      if (midobr == ob)
        return true;
    }
  return false;
}                               /* end of mo_set_contains */

// given a set, such as an auto-complete set, compute a GC-strduped
// common prefix of them, either using their id or their name
const char *mo_set_common_prefix (mo_value_t setv, bool byid);

/******************** OBJECTs ****************/
enum mo_space_en
{
  mo_SPACE_NONE,
  mo_SPACE_PREDEF,
  mo_SPACE_GLOBAL,
  mo_SPACE_USER,
};

typedef struct mo_objectvalue_st mo_objectvalue_ty;
#define MOMFIELDS_objectvalue                   \
  MOMFIELDS_hashedvalue;                        \
/** don't need mutex before bootstrapping:      \
 ** pthread_mutex_t mo_ob_mtx; **/              \
  time_t mo_ob_mtime;                           \
  mo_hid_t mo_ob_hid;                           \
  mo_loid_t mo_ob_loid;                         \
  mo_objref_t mo_ob_class;                      \
  mo_assovaldatapayl_ty *mo_ob_attrs;           \
  mo_vectvaldatapayl_ty *mo_ob_comps;           \
  /* payload kind & data */                     \
  mo_objref_t mo_ob_paylkind;                   \
  void *mo_ob_payldata
struct mo_objectvalue_st
{
  MOMFIELDS_objectvalue;
};                              /* end struct mo_objectvalue_st */

static inline mo_value_t
mo_dyncast_object (mo_value_t v)
{
  if (!mo_valid_pointer_value (v))
    return NULL;
  mo_objectvalue_ty *ob = (mo_objectvalue_ty *) v;
  if (ob->mo_va_kind != mo_KOBJECT)
    return NULL;
  return ob;
}                               /* end mo_dyncast_object */

static inline mo_objref_t
mo_dyncast_objref (mo_value_t v)
{
  return (mo_objref_t) mo_dyncast_object (v);
}

static inline mo_value_t
mo_objref_get_attr (mo_objref_t ob, mo_objref_t obat);

static inline void
mo_objref_put_attr (mo_objref_t ob, mo_objref_t obat, mo_value_t val);

// find the object of given hid & loid
mo_objref_t mo_objref_find_hid_loid (mo_hid_t hid, mo_loid_t loid);

// create an object of given valid hid & loid; mostly useful at load
// time
mo_objref_t mo_objref_create_hid_loid (mo_hid_t hid, mo_loid_t loid);

// make a fresh transient object of unique hid & loid
mo_objref_t mo_make_object (void);

static inline int
mo_objref_cmp (mo_objref_t obl, mo_objref_t obr)
{
  obl = mo_dyncast_objref (obl);
  obr = mo_dyncast_objref (obr);
  if (obl == obr)
    return 0;
  if (MOM_UNLIKELY (obl == NULL))
    return -1;
  if (MOM_UNLIKELY (obr == NULL))
    return 1;
  if (obl->mo_ob_hid < obr->mo_ob_hid)
    return -1;
  if (obl->mo_ob_hid > obr->mo_ob_hid)
    return 1;
  if (obl->mo_ob_loid < obr->mo_ob_loid)
    return -1;
  if (MOM_LIKELY (obl->mo_ob_loid > obr->mo_ob_loid))
    return 1;
  MOM_FATAPRINTF ("distinct objects @%p & @%p with same id hid=%u lid=%llu",
                  obl, obr,
                  (unsigned) obl->mo_ob_hid,
                  (unsigned long long) obl->mo_ob_loid);
}

static inline momhash_t
mo_objref_hash (mo_objref_t obref)
{
  mo_objectvalue_ty *ob = (mo_dyncast_objref (obref));
  if (!ob)
    return 0;
  return ob->mo_va_hash;
}                               /* end mo_objref_hash */

static inline enum mo_space_en
mo_objref_space (mo_objref_t obref)
{
  mo_objectvalue_ty *ob = (mo_dyncast_objref (obref));
  if (!ob)
    return mo_SPACE_NONE;
  return (enum mo_space_en) (ob->mo_va_index);
}                               /* end mo_objref_space */

void mo_objref_put_space (mo_objref_t obr, enum mo_space_en spa);

// make a global object
static inline mo_objref_t
mo_make_global_object (void)
{
  mo_objref_t obr = mo_make_object ();
  mo_objref_put_space (obr, mo_SPACE_GLOBAL);
  return obr;
}                               /* end mo_make_global_object */

int mom_objref_cmp (const void *, const void *);        // suitable for qsort, in object.c

static inline void
mo_objref_clear_payload (mo_objref_t obr)
{
  extern void mo_objref_really_clear_payload (mo_objref_t);
  if (!mo_dyncast_objref (obr))
    return;
  if (obr->mo_ob_paylkind != NULL)
    mo_objref_really_clear_payload (obr);
  obr->mo_ob_paylkind = NULL;
  obr->mo_ob_payldata = NULL;
}                               /* end of mo_objref_clear_payload */

///// counted payloads have also count
typedef struct mo_countedpayl_st mo_countedpayl_ty;
#define MOMFIELDS_countedpayl   \
  MOMFIELDS_sizedvalue;         \
  uint32_t mo_cpl_count
struct mo_countedpayl_st
{
  MOMFIELDS_countedpayl;
};                              /* end struct mo_countedpayl_st */

/******************** ASSOVALs payload ****************/
#define MOMFIELDS_assoentry    \
  mo_objref_t mo_asso_obr;     \
  mo_value_t mo_asso_val
struct mo_assoentry_st
{
  MOMFIELDS_assoentry;
};                              /* end struct mo_assoentry */

#define MOMFIELDS_assovaldatapayl        \
  MOMFIELDS_countedpayl;                 \
  struct mo_assoentry_st mo_asso_entarr[]
struct mo_assovaldatapayl_st
{
  MOMFIELDS_assovaldatapayl;
};                              /* end struct mo_assovaldatapayl_st */

static inline mo_assovaldatapayl_ty *
mo_dyncastpayl_assoval (const void *p)
{
  if (!mo_valid_pointer_value (p))
    return NULL;
  mo_assovaldatapayl_ty *asso = (mo_assovaldatapayl_ty *) p;
  unsigned k = asso->mo_va_kind;
  if (k != mo_PASSOVALDATA)
    return NULL;
  return asso;
}                               /* end mo_dyncastpayl_assoval */

static inline unsigned
mo_assoval_size (mo_assovaldatapayl_ty * asso)
{
  asso = mo_dyncastpayl_assoval (asso);
  if (!asso)
    return 0;
  return asso->mo_sva_size;
}

static inline unsigned
mo_assoval_count (mo_assovaldatapayl_ty * asso)
{
  asso = mo_dyncastpayl_assoval (asso);
  if (!asso)
    return 0;
  return asso->mo_cpl_count;
}

mo_value_t mo_assoval_get (mo_assovaldatapayl_ty * asso, mo_objref_t ob);
mo_assovaldatapayl_ty *mo_assoval_put (mo_assovaldatapayl_ty * asso,
                                       mo_objref_t ob, mo_value_t va);
mo_assovaldatapayl_ty *mo_assoval_remove (mo_assovaldatapayl_ty * asso,
    mo_objref_t ob);
mo_assovaldatapayl_ty *mo_assoval_reserve (mo_assovaldatapayl_ty * asso,
    unsigned gap);
mo_value_t mo_assoval_keys_set (mo_assovaldatapayl_ty * asso);  // set of keys
void mo_dump_scan_assoval (mo_dumper_ty *, mo_assovaldatapayl_ty *);
mo_json_t mo_dump_json_of_assoval (mo_dumper_ty *, mo_assovaldatapayl_ty *);
mo_assovaldatapayl_ty *mo_assoval_of_json (mo_json_t);

/******************** VECTVALs payload ****************/
#define MOMFIELDS_vectvaldatapayl \
  MOMFIELDS_countedpayl;          \
  mo_value_t mo_vect_arr[]
struct mo_vectvaldatapayl_st
{
  MOMFIELDS_vectvaldatapayl;
};                              /* end struct mo_vectvaldatapayl_st */

static inline mo_vectvaldatapayl_ty *
mo_dyncastpayl_vectval (const void *p)
{
  if (!mo_valid_pointer_value (p))
    return NULL;
  mo_vectvaldatapayl_ty *vect = (mo_vectvaldatapayl_ty *) p;
  unsigned k = vect->mo_va_kind;
  if (k != mo_PVECTVALDATA)
    return NULL;
  return vect;
}

static inline unsigned
mo_vectval_size (mo_vectvaldatapayl_ty * vect)
{
  vect = mo_dyncastpayl_vectval (vect);
  if (!vect)
    return 0;
  return vect->mo_sva_size;
}

static inline unsigned
mo_vectval_count (mo_vectvaldatapayl_ty * vect)
{
  vect = mo_dyncastpayl_vectval (vect);
  if (!vect)
    return 0;
  return vect->mo_cpl_count;
}

static inline mo_value_t
mo_vectval_nth (mo_vectvaldatapayl_ty * vect, int rk)
{
  vect = mo_dyncastpayl_vectval (vect);
  if (!vect)
    return NULL;
  unsigned sz = vect->mo_sva_size;
  unsigned cnt = vect->mo_cpl_count;
  MOM_ASSERTPRINTF (cnt <= sz, "cnt %u larger than sz %u", cnt, sz);
  if (rk < 0)
    rk += (int) cnt;
  if (rk >= 0 && rk < (int) cnt)
    return vect->mo_vect_arr[rk];
  return NULL;
}                               /* end mo_vectval_nth */

static inline void
mo_vectval_put_nth (mo_vectvaldatapayl_ty * vect, int rk, mo_value_t newval)
{
  vect = mo_dyncastpayl_vectval (vect);
  if (!vect)
    return;
  if (newval == MOM_EMPTY_SLOT)
    newval = NULL;
  unsigned sz = vect->mo_sva_size;
  unsigned cnt = vect->mo_cpl_count;
  MOM_ASSERTPRINTF (cnt <= sz, "cnt %u larger than sz %u", cnt, sz);
  if (rk < 0)
    rk += (int) cnt;
  if (rk >= 0 && rk < (int) cnt)
    vect->mo_vect_arr[rk] = newval;
}                               /* end mo_vectval_put_nth */

// the vectval routines are in value.c because they are easy
mo_vectvaldatapayl_ty *mo_vectval_reserve (mo_vectvaldatapayl_ty * vect,
    unsigned newcount);
mo_vectvaldatapayl_ty *mo_vectval_resize (mo_vectvaldatapayl_ty * vect,
    unsigned newlen);
mo_vectvaldatapayl_ty *mo_vectval_append (mo_vectvaldatapayl_ty * vect,
    mo_value_t val);
// make a tuple of all objects in a vector
mo_value_t mo_vectval_objects_tuple (mo_vectvaldatapayl_ty * vect);

void mo_dump_scan_vectval (mo_dumper_ty *, mo_vectvaldatapayl_ty *);
mo_json_t mo_dump_json_of_vectval (mo_dumper_ty *, mo_vectvaldatapayl_ty *);
mo_vectvaldatapayl_ty *mo_vectval_of_json (mo_json_t);
/******************** HASHSETs payload ****************/
typedef struct mo_hashsetpayl_st mo_hashsetpayl_ty;
#define MOMFIELDS_hashsetpayl \
  MOMFIELDS_countedpayl;      \
  mo_objref_t mo_hset_arr[]
struct mo_hashsetpayl_st
{
  MOMFIELDS_hashsetpayl;
};                              /* end struct mo_hashsetpayl_st */

static inline mo_hashsetpayl_ty *
mo_dyncastpayl_hashset (const void *p)
{
  if (!mo_valid_pointer_value (p))
    return NULL;
  mo_hashsetpayl_ty *hset = (mo_hashsetpayl_ty *) p;
  unsigned k = hset->mo_va_kind;
  if (k != mo_PHASHSET)
    return NULL;
  return hset;
}

static inline unsigned
mo_hashset_size (mo_hashsetpayl_ty * hset)
{
  hset = mo_dyncastpayl_hashset (hset);
  if (!hset)
    return 0;
  return hset->mo_sva_size;
}

static inline unsigned
mo_hashset_count (mo_hashsetpayl_ty * hset)
{
  hset = mo_dyncastpayl_hashset (hset);
  if (!hset)
    return 0;
  return hset->mo_cpl_count;
}

bool mo_hashset_contains (mo_hashsetpayl_ty * hset, mo_objref_t obr);
mo_hashsetpayl_ty *mo_hashset_put (mo_hashsetpayl_ty * hset, mo_objref_t ob);
mo_hashsetpayl_ty *mo_hashset_remove (mo_hashsetpayl_ty * hset,
                                      mo_objref_t ob);
mo_hashsetpayl_ty *mo_hashset_reserve (mo_hashsetpayl_ty * hset,
                                       unsigned gap);
mo_value_t mo_hashset_elements_set (mo_hashsetpayl_ty * hset);  // set of elements

void mo_dump_scan_hashset (mo_dumper_ty *, mo_hashsetpayl_ty *);
mo_json_t mo_dump_json_of_hashset (mo_dumper_ty *, mo_hashsetpayl_ty *);
mo_hashsetpayl_ty *mo_hashset_of_json (mo_json_t);

/******************** LISTs payload, also usable as queues ****************/
typedef struct mo_listpayl_st mo_listpayl_ty;
typedef struct mo_listelem_st mo_listelem_ty;

#define MOM_LISTCHUNK_LEN 14
#define MOMFIELDS_listelem      \
  mo_listelem_ty *mo_lie_next;      \
  mo_listelem_ty *mo_lie_prev;      \
  mo_value_t mo_lie_arr[MOM_LISTCHUNK_LEN]
struct mo_listelem_st
{
  MOMFIELDS_listelem;
};                              /* end struct mo_listelem_st */

#define MOMFIELDS_listpayl        \
  MOMFIELDS_hashedvalue;          \
  mo_listelem_ty* mo_lip_first;   \
  mo_listelem_ty* mo_lip_last

struct mo_listpayl_st
{
  MOMFIELDS_listpayl;
};                              /* end struct mo_listpayl_st */

static inline mo_listpayl_ty *
mo_dyncastpayl_list (const void *p)
{
  if (!mo_valid_pointer_value (p))
    return NULL;
  mo_listpayl_ty *lis = (mo_listpayl_ty *) p;
  unsigned k = lis->mo_va_kind;
  if (k != mo_PLIST)
    return NULL;
  return lis;
}                               /* end mo_dyncastpayl_list */

static inline bool
mo_list_non_empty (mo_listpayl_ty * lis)
{
  if (!mo_dyncastpayl_list (lis))
    return false;
  if (lis->mo_lip_first == NULL)
    {
      MOM_ASSERTPRINTF (lis->mo_lip_last == NULL, "corrupted list");
      return false;
    }
  return true;
}                               /* end mo_list_non_empty */

static inline unsigned
mo_list_length (mo_listpayl_ty * lis)
{
  if (!mo_dyncastpayl_list (lis))
    return 0;
  unsigned ln = 0;
  for (mo_listelem_ty * el = lis->mo_lip_first; el != NULL;
       el = el->mo_lie_next)
    {
      if (MOM_UNLIKELY (ln >= MOM_SIZE_MAX))
        MOM_FATAPRINTF ("too long list %u", ln);
      for (int ix = 0; ix < MOM_LISTCHUNK_LEN; ix++)
        if (el->mo_lie_arr[ix])
          ln++;
    }
  return ln;
}                               /* end of mo_list_length */

static inline mo_value_t
mo_list_head (mo_listpayl_ty * lis)
{
  if (!mo_dyncastpayl_list (lis))
    return NULL;
  mo_listelem_ty *hd = lis->mo_lip_first;
  if (!hd)
    return NULL;
  for (int ix = 0; ix < MOM_LISTCHUNK_LEN; ix++)
    if (hd->mo_lie_arr[ix])
      return hd->mo_lie_arr[ix];
  // should never happen
  MOM_FATAPRINTF ("corrupted list@%p with empty head", lis);
}                               /* end of mo_list_head */

static inline mo_value_t
mo_list_tail (mo_listpayl_ty * lis)
{
  if (!mo_dyncastpayl_list (lis))
    return NULL;
  mo_listelem_ty *tl = lis->mo_lip_last;
  if (!tl)
    return NULL;
  for (int ix = MOM_LISTCHUNK_LEN - 1; ix > 0; ix--)
    if (tl->mo_lie_arr[ix])
      return tl->mo_lie_arr[ix];
  // should not happen
  MOM_FATAPRINTF ("corrupted list@%p with empty tail", lis);
}                               /* end of mo_list_tail */

mo_listpayl_ty *mo_list_make (void);
// append and prepend a non-nil value
void mo_list_append (mo_listpayl_ty *, mo_value_t);
void mo_list_prepend (mo_listpayl_ty *, mo_value_t);
// remove head or tail
void mo_list_pop_head (mo_listpayl_ty *);
void mo_list_pop_tail (mo_listpayl_ty *);
// vector of all values in some list
mo_vectvaldatapayl_ty *mo_list_to_vectvaldata (mo_listpayl_ty *);
// tuple of all objects in some list
mo_value_t mo_list_to_tuple (mo_listpayl_ty *);
void mo_dump_scan_list (mo_dumper_ty *, mo_listpayl_ty *);
mo_json_t mo_dump_json_of_list (mo_dumper_ty *, mo_listpayl_ty *);
mo_listpayl_ty *mo_list_of_json (mo_json_t);


////////// FILE & BUFFER-s

/// a file payload has kind payload_file & data the FILE*
/// a buffer payload has kind payload_buffer & data..
typedef struct mo_bufferpayl_st mo_bufferpayl_ty;
#define MOM_BUFFER_MAGIC 0x1af15eb9     /*452026041 buffer_magic */
#define MOMFIELDS_bufferpayl                                    \
  MOMFIELDS_hashedvalue;                                        \
  unsigned mo_buffer_nmagic;    /* always MOM_BUFFER_MAGIC */   \
  char *mo_buffer_zone;                                         \
  size_t mo_buffer_size;                                        \
  FILE *mo_buffer_memstream

struct mo_bufferpayl_st         /*malloced */
{
  MOMFIELDS_bufferpayl;
};                              /* end struct mo_bufferpayl_st */

bool mo_objref_open_file (mo_objref_t obr, const char *path,
                          const char *mods);
bool mo_objref_open_buffer (mo_objref_t obr);
FILE *mo_objref_file (mo_objref_t obr);

///////////////// DUMP support .. in jstate.c
bool mo_dump_scanning (mo_dumper_ty *);
void mo_dump_really_scan_value (mo_dumper_ty *, mo_value_t);

static inline void
mo_dump_scan_value (mo_dumper_ty * du, mo_value_t v)
{
  if (mo_valid_pointer_value (v))
    mo_dump_really_scan_value (du, v);
}                               /* end mo_dump_scan_value */

void mo_dump_really_scan_objref (mo_dumper_ty *, mo_objref_t);
static inline void
mo_dump_scan_objref (mo_dumper_ty * du, mo_objref_t obr)
{
  if (mo_dyncast_objref (obr))
    mo_dump_really_scan_objref (du, obr);
}                               /* end mo_dump_scan_objref */

void mo_dump_scan_inside_object (mo_dumper_ty *, mo_objref_t);
bool mo_dump_emitting (mo_dumper_ty *);
bool mo_dump_is_emitted_objref (mo_dumper_ty *, mo_objref_t);
void mo_dump_emit_object_content (mo_dumper_ty *, mo_objref_t);
void mo_dump_emit_names (mo_dumper_ty *);
FILE *mo_dump_fopen (mo_dumper_ty *, const char *);
void mom_dump_state (const char *dirname);
// for SQLITE_CONFIG_LOG
void mo_dump_errorlog (void *pdata MOM_UNUSED, int errcode, const char *msg);
///////////////// JSON support .. in jstate.c
// get the json for a value
mo_json_t mo_dump_json_of_value (mo_dumper_ty *, mo_value_t);
// get the json for an objref, e.g. an id string or null
mo_json_t mo_dump_jsonid_of_objref (mo_dumper_ty *, mo_objref_t);
// get the value from a json
mo_value_t mo_value_of_json (mo_json_t);
// get the existing objref from a json
mo_objref_t mo_objref_of_jsonid (mo_json_t);
// the loader, called after predefined has been initialized
void mom_load_state (void);
// constant number of loaded objects
unsigned mom_load_nb_objects (void);
// constant number of loaded modules
unsigned mom_load_nb_modules (void);
/************* NAMES ***********/
// the maximal name length is big enough for practical purposes
#define MOM_NAME_MAXLEN 1024
// a name is valid if it is like some C identifier or keyword
// initial and final underscores are not allowed
// consecutive underscores are not allowed
bool mom_valid_name (const char *nam);  // in name.c

// get the name of some object, or else nil
mo_value_t mo_objref_namev (mo_objref_t ob);


// register a name for an anonymous object, return true if successful
bool mo_register_named (mo_objref_t obr, const char *nam);
bool mo_register_name_string (mo_objref_t obr, mo_value_t namv);

// unregister a named object, return true if successful
bool mo_unregister_named_object (mo_objref_t obr);
bool mo_unregister_name_string (const char *nams);
bool mo_unregister_name_vals (mo_value_t namv);

// get an object by its name or else nil
mo_objref_t mo_find_named_cstr (const char *nams);
mo_objref_t mo_find_named_vals (mo_value_t namv);

// reserve space for additional names
void mo_reserve_names (unsigned gap);
// the printable name of an object, perhaps GC-strduped
static inline const char *
mo_objref_pnamestr (mo_objref_t ob)
{
  if (!mo_dyncast_objref (ob))
    return "~";
  mo_value_t namv = mo_objref_namev (ob);
  if (namv)
    return mo_string_cstr (namv);
  return mo_cstring_from_hi_lo_ids (NULL, ob->mo_ob_hid, ob->mo_ob_loid);
}                               /* end mo_objref_pnamestr */

// the short printable name of an object, without the starting underscore if anonymous
static inline const char *
mo_objref_shortnamestr (mo_objref_t ob)
{
  const char *pn = mo_objref_pnamestr (ob);
  if (pn && *pn == '_' && isdigit (pn[1]))
    return pn + 1;
  return pn;
}                               /* end of mo_objref_shortnamestr */



// get the idstr of an object in some given buffer (or else in GC-ed
// heap or literal string for nil)
static inline const char *
mo_objref_idstr (char *bufid, mo_objref_t ob)
{
  if (mo_dyncast_objref (ob))
    {
      return mo_cstring_from_hi_lo_ids (bufid, ob->mo_ob_hid, ob->mo_ob_loid);
    }
  else if (bufid && bufid != MOM_EMPTY_SLOT)
    {
      strcpy (bufid, "~");
      return bufid;
    }
  else
    return "~";
}                               /* end mo_objref_idstr */


/// given a string prefix starting with an underscore and three
/// alphanum, compute the set of objects whose objid start with that
/// prefix; in file object.c
mo_value_t mom_set_complete_objectid (const char *prefix);

// retrieve the set of named objects
mo_value_t mo_named_objects_set (void);

// given a string prefix starting with a letter and made of alphanum
// or underscores, compute the set of objects whose name start with
// that prefix
mo_value_t mo_named_set_of_prefix (const char *prefix);
/************* PREDEFINED ***********/

mo_value_t mo_predefined_objects_set (void);
#define MOM_STRINGIFY(X) #X
#define MOM_VARPREDEF(Nam) mompredef_##Nam
#define MOM_PREDEF(Nam) ((mo_objref_t)(&MOM_VARPREDEF(Nam)))
/* declare them as objects */
#define MOM_HAS_PREDEFINED(Nam,Idstr,Hid,Loid,Hash) \
  extern mo_objectvalue_ty MOM_VARPREDEF(Nam);
#include "_mom_predef.h"

#define MOM_HAS_GLOBAL(Nam,Idstr,Hid,Loid,Hash) \
  extern mo_objref_t momglob_##Nam;
#include "_mom_global.h"

#define MOM_HAS_PREDEFINED(Nam,Idstr,Hid,Loid,Hash) \
  extern const char momidstr_##Nam[];
#include "_mom_predef.h"

static inline mo_value_t
mo_objref_get_attr (mo_objref_t ob, mo_objref_t obat)
{
  mo_objectvalue_ty *pob = (mo_objectvalue_ty *) mo_dyncast_objref (ob);
  if (!pob || !mo_dyncast_objref (obat) || !pob->mo_ob_attrs)
    return NULL;
  return mo_assoval_get (pob->mo_ob_attrs, obat);
}                               /* end mo_objref_get_attr */

static inline void
mo_objref_touch (mo_objref_t ob)
{
  mo_objectvalue_ty *pob = (mo_objectvalue_ty *) mo_dyncast_objref (ob);
  if (!pob)
    return;
  time (&pob->mo_ob_mtime);
}                               /* end mo_objref_touch */

static inline void
mo_objref_remove_attr (mo_objref_t ob, mo_objref_t obat)
{
  mo_objectvalue_ty *pob = (mo_objectvalue_ty *) mo_dyncast_objref (ob);
  if (!pob || !mo_dyncast_objref (obat) || !pob->mo_ob_attrs)
    return;
  pob->mo_ob_attrs = mo_assoval_remove (pob->mo_ob_attrs, obat);
  time (&pob->mo_ob_mtime);
}                               /* end mo_objref_remove_attr */

static inline void
mo_objref_put_attr (mo_objref_t ob, mo_objref_t obat, mo_value_t val)
{
  if (val == NULL || val == MOM_EMPTY_SLOT)
    {
      mo_objref_remove_attr (ob, obat);
      return;
    }
  mo_objectvalue_ty *pob = (mo_objectvalue_ty *) mo_dyncast_objref (ob);
  if (!pob || !mo_dyncast_objref (obat))
    return;
  pob->mo_ob_attrs = mo_assoval_put (pob->mo_ob_attrs, obat, val);
  time (&pob->mo_ob_mtime);
}                               /* end mo_objref_put_attr */

static inline mo_value_t
mo_objref_set_of_attrs (mo_objref_t ob)
{
  mo_objectvalue_ty *pob = (mo_objectvalue_ty *) mo_dyncast_objref (ob);
  if (!pob)
    return NULL;
  return mo_assoval_keys_set (pob->mo_ob_attrs);
}                               /* end of mo_objref_set_of_attrs */

static inline mo_value_t
mo_objref_get_comp (mo_objref_t ob, int rk)
{
  mo_objectvalue_ty *pob = (mo_objectvalue_ty *) mo_dyncast_objref (ob);
  if (!pob)
    return NULL;
  mo_vectvaldatapayl_ty *vecomp = pob->mo_ob_comps;
  if (!vecomp)
    return NULL;
  return mo_vectval_nth (vecomp, rk);
}                               /* end of mo_objref_get_comp */

static inline void
mo_objref_put_comp (mo_objref_t ob, int rk, mo_value_t va)
{
  mo_objectvalue_ty *pob = (mo_objectvalue_ty *) mo_dyncast_objref (ob);
  if (!pob)
    return;
  mo_vectvaldatapayl_ty *vecomp = pob->mo_ob_comps;
  if (!vecomp)
    return;
  mo_vectval_put_nth (vecomp, rk, va);
}                               /* end of mo_objref_put_comp */

static inline unsigned
mo_objref_comp_count (mo_objref_t ob)
{
  mo_objectvalue_ty *pob = (mo_objectvalue_ty *) mo_dyncast_objref (ob);
  if (!pob)
    return 0;
  mo_vectvaldatapayl_ty *vecomp = pob->mo_ob_comps;
  if (!vecomp)
    return 0;
  return mo_vectval_count (vecomp);
}                               /* end mo_objref_comp_count */

static inline void
mo_objref_comp_resize (mo_objref_t ob, unsigned newsiz)
{
  mo_objectvalue_ty *pob = (mo_objectvalue_ty *) mo_dyncast_objref (ob);
  if (!pob)
    return;
  pob->mo_ob_comps = mo_vectval_resize (pob->mo_ob_comps, newsiz);
}                               /* end of mo_objref_comp_resize */

static inline void
mo_objref_comp_reserve (mo_objref_t ob, unsigned gap)
{
  mo_objectvalue_ty *pob = (mo_objectvalue_ty *) mo_dyncast_objref (ob);
  if (!pob)
    return;
  pob->mo_ob_comps = mo_vectval_reserve (pob->mo_ob_comps, gap);
}                               /* end of mo_objref_comp_reserve */

static inline void
mo_objref_comp_append (mo_objref_t ob, mo_value_t va)
{
  mo_objectvalue_ty *pob = (mo_objectvalue_ty *) mo_dyncast_objref (ob);
  if (!pob)
    return;
  pob->mo_ob_comps = mo_vectval_append (pob->mo_ob_comps, va);
}                               /* end of mo_objref_comp_append */

void mo_objref_comp_remove (mo_objref_t ob, int rk);

// the function of id Id has dlsymed symbol mofunId or moId for the code and
// mosigId for the signature
#define MOM_FUNC_PREFIX "mofun_"
#define MOM_CODE_PREFIX "mo_"
#define MOM_SIGNATURE_PREFIX "mosig_"

// the module of id Modid has an initialization function moinit_Modid
#define MOM_MODULEINIT_PREFIX "moinit_"
// put inside an object a [function] payload with a signature, do some checks
void mo_objref_put_signature_payload (mo_objref_t obr, mo_objref_t sigobr);     /* in object.c */

void mo_objref_put_gobject_payload (mo_objref_t obr, GObject * gobj);

static inline GObject *
mo_objref_get_gobject (mo_objref_t obr)
{
  if (!mo_dyncast_objref (obr))
    return NULL;
  if (obr->mo_ob_paylkind != MOM_PREDEF (payload_gobject))
    return NULL;
  return (GObject *) obr->mo_ob_payldata;
}                               /* end mo_objref_get_gobject */

static inline const void *
mo_objref_get_signed_funad (mo_objref_t obr, mo_objref_t obrsig)
{
  if (!mo_dyncast_objref (obr))
    return NULL;
  if (!mo_dyncast_objref (obrsig))
    return NULL;
  if (obrsig->mo_ob_class != MOM_PREDEF (signature_class))
    return NULL;
  if (obr->mo_ob_paylkind == obrsig)
    return obr->mo_ob_payldata;
  return NULL;
}                               /* end mo_objref_get_signed_funad */

/// signature corresponding to signature_object_to_value
typedef mo_value_t mo_signature_object_to_value_sigt (mo_objref_t);
/// signature corresponding to signature_two_objects_to_void
typedef void mo_signature_two_objects_to_void_sigt (mo_objref_t, mo_objref_t);
/// signature corresponding to signature_two_objects_to_object
typedef mo_objref_t mo_signature_two_objects_to_object_sigt (mo_objref_t,
    mo_objref_t);
/// signature corresponding to signature_two_objects_to_value
typedef mo_value_t mo_signature_two_objects_to_value_sigt (mo_objref_t,
    mo_objref_t);
////////////////////////////////////////////////////////////////
/***************** Graphical User Interface (above GTK) *****/
extern bool mom_without_gui;
void mom_run_gtk (int *pargc, char ***pargv, char **dispobjs);
void mo_gui_display_object (mo_objref_t ob);
void mo_gui_undisplay_object (mo_objref_t ob);
void mom_gui_fail_user_action (const char *fmt, ...)
__attribute__ ((format (printf, 1, 2), noreturn));
void mom_gui_set_displayed_nth_value (int ix, mo_value_t curval);

void
mom_gui_cmdstatus_printf (const char *fmt, ...)
__attribute__ ((format (printf, 1, 2)));

// the prefix & suffix of plugins
#define MOM_PLUGIN_PREFIX "momplug_"
#define MOM_PLUGIN_SUFFIX ".so"
// the name of the plugin startup routine and its declaration
#define MOM_PLUGIN_STARTUP "momplugin_startup"
typedef void momplugin_startup_sigt (const char *);
extern momplugin_startup_sigt momplugin_startup;



////////////////////////////////////////////////////////////////
/***************** C code emission *****/
/// in file cemit.c
typedef struct mo_cemitpayl_st mo_cemitpayl_ty; // a private struct

/// initialize a cemit payload for some given module
void mo_objref_put_cemit_payload (mo_objref_t obr, mo_objref_t obmodul);

/// set the suffix in a closed cemit object
void mo_objref_cemit_set_suffix (mo_objref_t obrcem, const char *suffix);
/// set the prefix in a closed cemit object
void mo_objref_cemit_set_prefix (mo_objref_t obrcem, const char *suffix);
// generate and close a cemit object, return an error string on failure
mo_value_t mo_objref_cemit_generate (mo_objref_t obrcem);
// check if an object has a valid c_emit_payload
bool mo_objref_has_valid_cemit_payload (mo_objref_t obr);
// get the cemit payload of an object
mo_cemitpayl_ty *mo_objref_get_cemit (mo_objref_t obr);

// check if an object is an actively emitted
bool mo_objref_active_cemit_payload (mo_objref_t obr);
// dyncast some cemit payload
mo_cemitpayl_ty *mo_dyncastpayl_cemit (const void *p);
// return a GC-strduped string containing some details about a cemit object
const char *mo_objref_cemit_detailstr (mo_objref_t obr);
void
mom_objref_cemit_printf (mo_objref_t obrcem, const char *fmt, ...)
__attribute ((format (printf, 2, 3)));
void mom_objref_cemit_newline (mo_objref_t obrcem);


/// dirty trick to reference something by its id
/// assume we have #define moid_the_system _0BV96V94PJIn9si1K
/// then we want MOM_PREFIXID(mofun_,the_system) to be expanded to mofun__0BV96V94PJIn9si1K
#define MOM_PREFIXID_ADD(Prefix,Id) Prefix##Id
#define MOM_PREFIXID_ADD_AGAIN(Prefix,Id) MOM_PREFIXID_ADD(Prefix,Id)
#define MOM_PREFIXID(Prefix,Name) MOM_PREFIXID_ADD_AGAIN(Prefix,moid_##Name)
#endif /*MONIMELT_HEADER */

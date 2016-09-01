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
#ifndef MONIMELT_INCLUDED_
#define MONIMELT_INCLUDED_ 1


#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif /*_GNU_SOURCE*/

#define GC_THREADS 1
#define HAVE_PTHREADS 1


#include <features.h>   // GNU things
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
// libonion from http://www.coralbits.com/libonion/ &
// https://github.com/davidmoreno/onion
#include <onion/onion.h>
#include <onion/version.h>
#include <onion/low.h>
#include <onion/codecs.h>
#include <onion/request.h>
#include <onion/response.h>
#include <onion/block.h>
#include <onion/handler.h>
#include <onion/dict.h>
#include <onion/log.h>
#include <onion/shortcuts.h>
#include <onion/exportlocal.h>
#include <onion/internal_status.h>
#include <onion/websocket.h>
// jansson, a JSON library in C which is Boehm-GC friendly
// see http://www.digip.org/jansson/
#include <jansson.h>
// Glib & GTK3 see http://gtk.org/
#include <glib.h>
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
#else
#define MOM_UNLIKELY(P) (P)
#define MOM_LIKELY(P) (P)
#define MOM_UNUSED
#endif

void
mom_warnprintf_at (const char *fil, int lin, const char *fmt, ...)
__attribute__ ((format (printf, 3, 4)));


typedef atomic_bool mom_atomic_bool_t;
typedef atomic_int mom_atomic_int_t;
typedef atomic_int_least16_t mom_atomic_int16_t;
typedef FILE *_Atomic mom_atomic_fileptr_t;
typedef json_t *mo_json_t;
typedef struct mo_dumper_st mo_dumper_ty;
#define thread_local _Thread_local


#define MOM_DUMP_VERSIONID "MoniMelt2016B"

// the generated header file
#define MOM_PREDEF_HEADER "_mom_predef.h"
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
extern const char monimelt_cbasesources[];

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

#define MOM_ASSERTPRINTF_AT(Fil,Lin,Cond,Fmt,...) \
  do {if (MOM_UNLIKELY(!(Cond)))      \
      MOM_FATAPRINTF_AT(Fil,Lin,      \
  "MOM_ASSERT FAILURE (" #Cond "): " Fmt,   \
      ##__VA_ARGS__); }while(0)

#define MOM_ASSERTPRINTF_AT_BIS(Fil,Lin,Cond,Fmt,...) \
  MOM_ASSERTPRINTF_AT(Fil,Lin,Cond,Fmt,     \
        ##__VA_ARGS__)

#define MOM_ASSERTPRINTF(Cond,Fmt,...)    \
  MOM_ASSERTPRINTF_AT_BIS(__FILE__,__LINE__,Cond,Fmt, \
      ##__VA_ARGS__)

#endif /*NDEBUG*/
static inline void *
mom_gc_alloc (size_t sz)
{
  void *p = GC_MALLOC (sz);
  if (MOM_UNLIKELY (p == NULL))
    MOM_FATAPRINTF ("failed to allocate %zd bytes", sz);
  memset (p, 0, sz);
  return p;
}

static inline void *
mom_gc_alloc_scalar (size_t sz)
{
  void *p = GC_MALLOC_ATOMIC (sz);
  if (MOM_UNLIKELY (p == NULL))
    MOM_FATAPRINTF ("failed to allocate %zd scalar bytes", sz);
  memset (p, 0, sz);
  return p;
}

static inline void *
mom_gc_alloc_uncollectable (size_t sz)
{
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
  char *p = GC_STRDUP (s);
  if (MOM_UNLIKELY (p == NULL))
    MOM_FATAPRINTF ("failed to gc strdup %s", s);
  return p;
}

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


double mom_elapsed_real_time (void);  /* relative to start of program */
double mom_process_cpu_time (void);
double mom_thread_cpu_time (void);

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



momhash_t     // in main.c
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
  mo_PASSOVALDATA = (int) MOM_LAST_KIND + 1,
  mo_PVECTVALDATA,
  mo_PHASHSET,
  mo_PLIST,
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

static inline bool
mo_value_is_int (mo_value_t p)
{
  return p != NULL && p != MOM_EMPTY_SLOT && ((intptr_t) p % 2 != 0);
}


static inline mo_int_t
mo_value_to_int (mo_value_t p, mo_int_t def)
{
  return mo_value_is_int (p) ? (intptr_t) p / 2 : def;
}

static inline mo_value_t
mo_int_to_value (mo_int_t i)
{
  MOM_ASSERTPRINTF (i >= MO_INTMIN && i <= MO_INTMAX,
                    "integer %lld out of range", (long long) i);
  return (mo_value_t) (((intptr_t) i % 2) + 1);
}

#define MOM_CSTRIDLEN 18
extern const char *   // in object.c, the buf is either null -then using
// a GC allocated one- or of size MOM_CSTRIDLEN+1
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
extern bool     // in object.c


mo_get_hi_lo_ids_from_cstring (mo_hid_t * phid, mo_loid_t * ploid,
                               const char *buf);

extern void     // in object.c
mo_get_some_random_hi_lo_ids (mo_hid_t * phid, mo_loid_t * ploid);

extern momhash_t    // in object.c
mo_hash_from_hi_lo_ids (mo_hid_t hid, mo_loid_t loid);

////////////////////////////////////////////////////////////////
/////
///// all values have some type & hash
typedef struct mo_hashedvalue_st mo_hashedvalue_ty;
struct mo_hashedvalue_st
{
  uint16_t mo_va_kind;
  uint16_t mo_va_index;
  momhash_t mo_va_hash;
};

static inline enum mo_valkind_en
mo_kind_of_value (mo_value_t v)
{
  if (mo_value_is_int (v))
    return mo_KINT;
  else if (!mo_valid_pointer_value (v))
    return mo_KNONE;
  else
    {
      unsigned k = ((mo_hashedvalue_ty *) v)->mo_va_kind;
      MOM_ASSERTPRINTF (k >= MOM_FIRST_BOXED_KIND && k <= MOM_LAST_KIND,
                        "mo_kind_of_value: bad kind #%u @%p", k, v);
      return (enum mo_valkind_en) k;
    }
}

///// sized values have also size
typedef struct mo_sizedvalue_st mo_sizedvalue_ty;
struct mo_sizedvalue_st
{
  struct mo_hashedvalue_st _mo;
  uint32_t mo_sva_size;
};

static inline uint32_t
mo_size_of_value (mo_value_t v)
{
  enum mo_valkind_en k = mo_kind_of_value (v);
  if (k == mo_KSTRING || k == mo_KTUPLE || k == mo_KSET)
    return (((mo_sizedvalue_ty *) v))->mo_sva_size;
  return 0;
}

///// string values
typedef struct mo_stringvalue_st mo_stringvalue_ty;
struct mo_stringvalue_st
{
  struct mo_sizedvalue_st _mo;
  char mo_cstr[];   // allocated size is mo_sva_size+1
};
mo_value_t mo_make_string_len (const char *buf, int sz);
static inline mo_value_t
mo_make_string_cstr (const char *buf)
{
  return mo_make_string_len (buf, -1);
};

mo_value_t mo_make_string_sprintf (const char *fmt, ...)
__attribute__ ((format (printf, 1, 2)));

static inline mo_value_t
mo_dyncast_string (mo_value_t vs)
{
  if (!mo_valid_pointer_value (vs))
    return NULL;
  if (((mo_hashedvalue_ty *) vs)->mo_va_kind != mo_KSTRING)
    return NULL;
  return vs;
}

static inline const char *
mo_string_cstr (mo_value_t v)
{
  mo_value_t vstr = mo_dyncast_string (v);
  if (!vstr)
    return NULL;
  return ((mo_stringvalue_ty *) vstr)->mo_cstr;
}

/******************** SEQUENCEs ****************/
/// They are tuples or sets of (non-nil) objrefs; we might need to add
/// transient sequences later

#define MOM_UNFILLEDFAKESEQKIND 9990
typedef struct mo_sequencevalue_st mo_sequencevalue_ty;
struct mo_sequencevalue_st
{
  struct mo_sizedvalue_st _mo;
  mo_objref_t mo_seqobj[];
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
  ((mo_hashedvalue_ty *) seq)->mo_va_kind = MOM_UNFILLEDFAKESEQKIND;
  ((mo_sizedvalue_ty *) seq)->mo_sva_size = sz;
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
  ((mo_hashedvalue_ty *) seq)->mo_va_kind = MOM_UNFILLEDFAKESEQKIND;
  ((mo_sizedvalue_ty *) seq)->mo_sva_size = sz;
  if (arr && arr != MOM_EMPTY_SLOT)
    memcpy (seq->mo_seqobj, arr, sz * sizeof (mo_objref_t));
  return seq;
}

static inline mo_value_t
mo_dyncast_sequence (mo_value_t vs)
{
  if (!mo_valid_pointer_value (vs))
    return NULL;
  if (((mo_hashedvalue_ty *) vs)->mo_va_kind != mo_KTUPLE
      && ((mo_hashedvalue_ty *) vs)->mo_va_kind != mo_KSET)
    return NULL;
  return vs;
}

static inline unsigned
mo_sequence_size (mo_value_t vs)
{
  mo_sequencevalue_ty *seq = (mo_sequencevalue_ty *) mo_dyncast_sequence (vs);
  if (!seq)
    return 0;
  return ((mo_sizedvalue_ty *) seq)->mo_sva_size;
}

static inline mo_objref_t
mo_sequence_nth (mo_value_t vs, int rk)
{
  mo_sequencevalue_ty *seq = (mo_sequencevalue_ty *) mo_dyncast_sequence (vs);
  if (!seq)
    return NULL;
  unsigned sz = ((mo_sizedvalue_ty *) seq)->mo_sva_size;
  if (!sz)
    return NULL;
  if (rk < 0)
    rk += sz;
  if (rk < 0 || rk >= (int) sz)
    return NULL;
  return seq->mo_seqobj[rk];
}

// put inside a sequence something during the fill
static inline void
mo_sequence_put (mo_sequencevalue_ty * seq, unsigned ix, mo_objref_t oref)
{
  MOM_ASSERTPRINTF (seq != NULL, "mo_sequence_put: null sequence");
  MOM_ASSERTPRINTF (((mo_hashedvalue_ty *) seq)->mo_va_kind ==
                    MOM_UNFILLEDFAKESEQKIND,
                    "mo_sequence_put: non-fake sequence");
  MOM_ASSERTPRINTF (ix < ((mo_sizedvalue_ty *) seq)->mo_sva_size,
                    "mo_sequence_put: too big index %u (size %u)", ix,
                    ((mo_sizedvalue_ty *) seq)->mo_sva_size);
  seq->mo_seqobj[ix] = oref;
}

/////// tuples of objrefs
typedef struct mo_tuplevalue_st mo_tuplevalue_ty;
struct mo_tuplevalue_st
{
  struct mo_sequencevalue_st _mo;
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
#define MOM_MAKE_SENTUPLE(...) mom_make_sentinel_tuple_(##__VA_ARGS__,NULL)

static inline mo_value_t
mo_dyncast_tuple (mo_value_t vs)
{
  if (!mo_valid_pointer_value (vs))
    return NULL;
  if (((mo_hashedvalue_ty *) vs)->mo_va_kind != mo_KTUPLE)
    return NULL;
  return vs;
}


static inline unsigned
mo_tuple_size (mo_value_t vs)
{
  mo_sequencevalue_ty *seq = (mo_sequencevalue_ty *) mo_dyncast_tuple (vs);
  if (!seq)
    return 0;
  return ((mo_sizedvalue_ty *) seq)->mo_sva_size;
}

static inline mo_objref_t
mo_tuple_nth (mo_value_t vs, int rk)
{
  mo_sequencevalue_ty *seq = (mo_sequencevalue_ty *) mo_dyncast_tuple (vs);
  if (!seq)
    return NULL;
  unsigned sz = ((mo_sizedvalue_ty *) seq)->mo_sva_size;
  if (!sz)
    return NULL;
  if (rk < 0)
    rk += sz;
  if (rk < 0 || rk >= (int) sz)
    return NULL;
  return seq->mo_seqobj[rk];
}

////// ordered sets
typedef struct mo_setvalue_st mo_setvalue_ty;
struct mo_setvalue_st
{
  struct mo_sequencevalue_st _mo;
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
#define MOM_MAKE_SENSET(...) mom_make_sentinel_set_(##__VA_ARGS__,NULL)

static inline mo_value_t
mo_dyncast_set (mo_value_t vs)
{
  if (!mo_valid_pointer_value (vs))
    return NULL;
  if (((mo_hashedvalue_ty *) vs)->mo_va_kind != mo_KSET)
    return NULL;
  return vs;
}

mo_value_t mo_set_union (mo_value_t vset1, mo_value_t vset2);
mo_value_t mo_set_intersection (mo_value_t vset1, mo_value_t vset2);
mo_value_t mo_set_difference (mo_value_t vset1, mo_value_t vset2);


static inline unsigned
mo_set_size (mo_value_t vs)
{
  mo_sequencevalue_ty *seq = (mo_sequencevalue_ty *) mo_dyncast_set (vs);
  if (!seq)
    return 0;
  return ((mo_sizedvalue_ty *) seq)->mo_sva_size;
}

static inline mo_objref_t
mo_set_nth (mo_value_t vs, int rk)
{
  mo_sequencevalue_ty *seq = (mo_sequencevalue_ty *) mo_dyncast_set (vs);
  if (!seq)
    return NULL;
  unsigned sz = ((mo_sizedvalue_ty *) seq)->mo_sva_size;
  if (!sz)
    return NULL;
  if (rk < 0)
    rk += sz;
  if (rk < 0 || rk >= (int) sz)
    return NULL;
  return seq->mo_seqobj[rk];
}

static inline bool
mo_set_contains (mo_value_t vset, mo_objref_t ob)
{
  if (!mo_dyncast_set (vset))
    return false;
  if (!mo_dyncast_object (ob))
    return false;
  unsigned card = ((mo_sizedvalue_ty *) vset)->mo_sva_size;
  if (!card)
    return 0;
  unsigned lo = 0, hi = card - 1;
  while (lo + 5 < hi)
    {
      unsigned md = (lo + hi) / 2;
      mo_objref_t midobr = ((mo_sequencevalue_ty *) vset)->mo_seqobj[md];
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
        MOM_FATAPRINTF ("corrupted set@%p", vset);
    }
  for (unsigned md = lo; md < hi; md++)
    {
      mo_objref_t midobr = ((mo_sequencevalue_ty *) vset)->mo_seqobj[md];
      MOM_ASSERTPRINTF (mo_dyncast_objref (midobr) != NULL,
                        "corrupted midobr@%p", midobr);
      if (midobr == ob)
        return true;
    }
  return false;
}

/******************** OBJECTs ****************/
enum mo_space_en
{
  mo_SPACE_NONE,
  mo_SPACE_PREDEF,
  mo_SPACE_GLOBAL,
  mo_SPACE_USER,
};

typedef struct mo_objectvalue_st mo_objectvalue_ty;
struct mo_objectvalue_st
{
  struct mo_hashedvalue_st _mo;
  /// actually, we dont need mutexes before the bootstrap
  /// pthread_mutex_t mo_ob_mtx;
  time_t mo_ob_mtime;
  mo_hid_t mo_ob_hid;
  mo_loid_t mo_ob_loid;
  mo_objref_t mo_ob_class;
  mo_assovaldatapayl_ty *mo_ob_attrs;
  mo_vectvaldatapayl_ty *mo_ob_comps;
  mo_objref_t mo_ob_paylkind;
  void *mo_ob_payload;
};

static inline mo_value_t
mo_dyncast_object (mo_value_t vs)
{
  if (!mo_valid_pointer_value (vs))
    return NULL;
  if (((mo_hashedvalue_ty *) vs)->mo_va_kind != mo_KOBJECT)
    return NULL;
  return vs;
}

static inline mo_objref_t
mo_dyncast_objref (mo_value_t v)
{
  return (mo_objref_t) mo_dyncast_object (v);
}

static inline mo_value_t
mo_objref_get_attr (mo_objref_t ob, mo_objref_t obat);

static inline void
mo_objref_put_attr (mo_objref_t ob, mo_objref_t obat, mo_value_t val);

mo_objref_t mo_objref_find_hid_loid (mo_hid_t hid, mo_loid_t loid);

// create an object of given valid hid & loid; mostly useful at load
// time
mo_objref_t mo_objref_create_hid_loid (mo_hid_t hid, mo_loid_t loid);

// make a fresh object of unique hid & loid
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
mo_objref_hash (mo_objref_t obr)
{
  if (!mo_dyncast_objref (obr))
    return 0;
  return ((mo_hashedvalue_ty *) obr)->mo_va_hash;
}

static inline enum mo_space_en
mo_objref_space (mo_objref_t obr)
{
  if (!mo_dyncast_objref (obr))
    return mo_SPACE_NONE;
  return (enum mo_space_en) ((mo_hashedvalue_ty *) obr)->mo_va_index;
}

void mo_objref_put_space (mo_objref_t obr, enum mo_space_en spa);

int mom_objref_cmp (const void *, const void *);  // suitable for qsort, in object.c

///// counted payloads have also count
typedef struct mo_countedpayl_st mo_countedpayl_ty;
struct mo_countedpayl_st
{
  struct mo_sizedvalue_st _mo;
  uint32_t mo_cpl_count;
};

/******************** ASSOVALs payload ****************/
struct mo_assoentry_st
{
  mo_objref_t mo_asso_obr;
  mo_value_t mo_asso_val;
};
struct mo_assovaldatapayl_st
{
  struct mo_countedpayl_st _mo;
  struct mo_assoentry_st mo_seqent[];
};

static inline mo_assovaldatapayl_ty *
mo_dyncastpayl_assoval (const void *p)
{
  if (!mo_valid_pointer_value (p))
    return NULL;
  unsigned k = ((mo_hashedvalue_ty *) p)->mo_va_kind;
  if (k != mo_PASSOVALDATA)
    return NULL;
  return (mo_assovaldatapayl_ty *) p;
}

static inline unsigned
mo_assoval_size (mo_assovaldatapayl_ty * asso)
{
  asso = mo_dyncastpayl_assoval (asso);
  if (!asso)
    return 0;
  return ((mo_sizedvalue_ty *) asso)->mo_sva_size;
}

static inline unsigned
mo_assoval_count (mo_assovaldatapayl_ty * asso)
{
  asso = mo_dyncastpayl_assoval (asso);
  if (!asso)
    return 0;
  return ((mo_countedpayl_ty *) asso)->mo_cpl_count;
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
struct mo_vectvaldatapayl_st
{
  struct mo_countedpayl_st _mo;
  mo_value_t mo_seqval[];
};

static inline mo_vectvaldatapayl_ty *
mo_dyncastpayl_vectval (const void *p)
{
  if (!mo_valid_pointer_value (p))
    return NULL;
  unsigned k = ((mo_hashedvalue_ty *) p)->mo_va_kind;
  if (k != mo_PVECTVALDATA)
    return NULL;
  return (mo_vectvaldatapayl_ty *) p;
}

static inline unsigned
mo_vectval_size (mo_vectvaldatapayl_ty * vect)
{
  vect = mo_dyncastpayl_vectval (vect);
  if (!vect)
    return 0;
  return ((mo_sizedvalue_ty *) vect)->mo_sva_size;
}

static inline unsigned
mo_vectval_count (mo_vectvaldatapayl_ty * vect)
{
  vect = mo_dyncastpayl_vectval (vect);
  if (!vect)
    return 0;
  return ((mo_countedpayl_ty *) vect)->mo_cpl_count;
}

static inline mo_value_t
mo_vectval_nth (mo_vectvaldatapayl_ty * vect, int rk)
{
  vect = mo_dyncastpayl_vectval (vect);
  if (!vect)
    return NULL;
  unsigned sz = ((mo_sizedvalue_ty *) vect)->mo_sva_size;
  unsigned cnt = ((mo_countedpayl_ty *) vect)->mo_cpl_count;
  MOM_ASSERTPRINTF (cnt <= sz, "cnt %u larger than sz %u", cnt, sz);
  if (rk < 0)
    rk += (int) cnt;
  if (rk >= 0 && rk < (int) cnt)
    return vect->mo_seqval[rk];
  return NULL;
}       /* end mo_vectval_nth */

static inline void
mo_vectval_put_nth (mo_vectvaldatapayl_ty * vect, int rk, mo_value_t newval)
{
  vect = mo_dyncastpayl_vectval (vect);
  if (!vect)
    return;
  if (newval == MOM_EMPTY_SLOT)
    newval = NULL;
  unsigned sz = ((mo_sizedvalue_ty *) vect)->mo_sva_size;
  unsigned cnt = ((mo_countedpayl_ty *) vect)->mo_cpl_count;
  MOM_ASSERTPRINTF (cnt <= sz, "cnt %u larger than sz %u", cnt, sz);
  if (rk < 0)
    rk += (int) cnt;
  if (rk >= 0 && rk < (int) cnt)
    vect->mo_seqval[rk] = newval;
}       /* end mo_vectval_put_nth */

// the vectval routines are in value.c because they are easy
mo_vectvaldatapayl_ty *mo_vectval_reserve (mo_vectvaldatapayl_ty * vect,
    unsigned newcount);
mo_vectvaldatapayl_ty *mo_vectval_resize (mo_vectvaldatapayl_ty * vect,
    unsigned newlen);
mo_vectvaldatapayl_ty *mo_vectval_append (mo_vectvaldatapayl_ty * vect,
    mo_value_t val);

void mo_dump_scan_vectval (mo_dumper_ty *, mo_vectvaldatapayl_ty *);
mo_json_t mo_dump_json_of_vectval (mo_dumper_ty *, mo_vectvaldatapayl_ty *);
mo_vectvaldatapayl_ty *mo_vectval_of_json (mo_json_t);
/******************** HASHSETs payload ****************/
typedef struct mo_hashsetpayl_st mo_hashsetpayl_ty;
struct mo_hashsetpayl_st
{
  struct mo_countedpayl_st _mo;
  mo_objref_t mo_hsetarr[];
};

static inline mo_hashsetpayl_ty *
mo_dyncastpayl_hashset (const void *p)
{
  if (!mo_valid_pointer_value (p))
    return NULL;
  unsigned k = ((mo_hashedvalue_ty *) p)->mo_va_kind;
  if (k != mo_PHASHSET)
    return NULL;
  return (mo_hashsetpayl_ty *) p;
}

static inline unsigned
mo_hashset_size (mo_hashsetpayl_ty * hset)
{
  hset = mo_dyncastpayl_hashset (hset);
  if (!hset)
    return 0;
  return ((mo_sizedvalue_ty *) hset)->mo_sva_size;
}

static inline unsigned
mo_hashset_count (mo_hashsetpayl_ty * hset)
{
  hset = mo_dyncastpayl_hashset (hset);
  if (!hset)
    return 0;
  return ((mo_countedpayl_ty *) hset)->mo_cpl_count;
}

bool mo_hashset_contains (mo_hashsetpayl_ty * hset, mo_objref_t obr);
mo_hashsetpayl_ty *mo_hashset_put (mo_hashsetpayl_ty * hset, mo_objref_t ob);
mo_hashsetpayl_ty *mo_hashset_remove (mo_hashsetpayl_ty * hset,
                                      mo_objref_t ob);
mo_hashsetpayl_ty *mo_hashset_reserve (mo_hashsetpayl_ty * hset,
                                       unsigned gap);
mo_value_t mo_hashset_elements_set (mo_hashsetpayl_ty * hset);  // set of elements

/******************** LISTs payload ****************/
typedef struct mo_listpayl_st mo_listpayl_ty;
typedef struct mo_listelem_st mo_listelem_ty;
#define MOM_LISTCHUNK_LEN 6
struct mo_listelem_st
{
  mo_listelem_ty *mo_lie_next;
  mo_listelem_ty *mo_lie_prev;
  mo_value_t mo_lie_arr[MOM_LISTCHUNK_LEN];
};
struct mo_listpayl_st
{
  mo_hashedvalue_ty _mo;
  mo_listelem_ty *mo_lip_first;
  mo_listelem_ty *mo_lip_last;
};

static inline mo_listpayl_ty *
mo_dyncastpayl_list (const void *p)
{
  if (!mo_valid_pointer_value (p))
    return NULL;
  unsigned k = ((mo_hashedvalue_ty *) p)->mo_va_kind;
  if (k != mo_PLIST)
    return NULL;
  return (mo_listpayl_ty *) p;
}       /* end mo_dyncastpayl_list */

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
}       /* end mo_list_non_empty */

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
}       /* end of mo_list_length */

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
  // should not happen
  MOM_FATAPRINTF ("corrupted list with empty head");
}       /* end of mo_list_head */

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
  MOM_FATAPRINTF ("corrupted list with empty tail");
}       /* end of mo_list_tail */

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

///////////////// DUMP support .. in jstate.c
bool mo_dump_scanning (mo_dumper_ty *);
void mo_dump_really_scan_value (mo_dumper_ty *, mo_value_t);
static inline void
mo_dump_scan_value (mo_dumper_ty * du, mo_value_t v)
{
  if (mo_valid_pointer_value (v))
    mo_dump_really_scan_value (du, v);
}       /* end mo_dump_scan_value */

void mo_dump_really_scan_objref (mo_dumper_ty *, mo_objref_t);
static inline void
mo_dump_scan_objref (mo_dumper_ty * du, mo_objref_t obr)
{
  if (mo_dyncast_objref (obr))
    mo_dump_really_scan_objref (du, obr);
}       /* end mo_dump_scan_objref */

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
/************* NAMES ***********/
// a name is valid if it is like some C identifier or keyword
// initial and final underscores are not allowed
// consecutive underscores are not allowed
bool mom_valid_name (const char *nam);  // in name.c

// get the name of some object, or else nil
mo_value_t mo_get_namev (mo_objref_t ob);


/* currently, the names are never forgotten, old their associated
   object may be removed. */
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
mo_object_pnamestr (mo_objref_t ob)
{
  if (!mo_dyncast_objref (ob))
    return "~";
  mo_value_t namv = mo_get_namev (ob);
  if (namv)
    return mo_string_cstr (namv);
  return mo_cstring_from_hi_lo_ids (NULL, ob->mo_ob_hid, ob->mo_ob_loid);
}       /* end mo_object_pnamestr */

// retrieve the set of names objects
mo_value_t mo_named_objects_set (void);
/************* PREDEFINED ***********/

mo_value_t mo_predefined_objects_set (void);
#define MOM_VARPREDEF(Nam) mompredef_##Nam
#define MOM_PREDEF(Nam) ((mo_objref_t)(&MOM_VARPREDEF(Nam)))
/* declare them as objects */
#define MOM_HAS_PREDEFINED(Nam,Idstr,Hid,Loid,Hash) \
  extern mo_objectvalue_ty MOM_VARPREDEF(Nam);
#include "_mom_predef.h"


static inline mo_value_t
mo_objref_get_attr (mo_objref_t ob, mo_objref_t obat)
{
  if (!mo_dyncast_objref (ob) || !mo_dyncast_objref (obat)
      || !((mo_objectvalue_ty *) ob)->mo_ob_attrs)
    return NULL;
  return mo_assoval_get (((mo_objectvalue_ty *) ob)->mo_ob_attrs, obat);
}       /* end mo_objref_get_attr */

static inline void
mo_objref_remove_attr (mo_objref_t ob, mo_objref_t obat)
{
  if (!mo_dyncast_objref (ob) || !mo_dyncast_objref (obat)
      || !((mo_objectvalue_ty *) ob)->mo_ob_attrs)
    return;
  ((mo_objectvalue_ty *) ob)->mo_ob_attrs
    = mo_assoval_remove (((mo_objectvalue_ty *) ob)->mo_ob_attrs, obat);
  time (&((mo_objectvalue_ty *) ob)->mo_ob_mtime);
}       /* end mo_objref_remove_attr */

static inline void
mo_objref_put_attr (mo_objref_t ob, mo_objref_t obat, mo_value_t val)
{
  if (val == NULL || val == MOM_EMPTY_SLOT)
    {
      mo_objref_remove_attr (ob, obat);
      return;
    }
  if (!mo_dyncast_objref (ob) || !mo_dyncast_objref (obat))
    return;
  ((mo_objectvalue_ty *) ob)->mo_ob_attrs
    = mo_assoval_put (((mo_objectvalue_ty *) ob)->mo_ob_attrs, obat, val);
  time (&((mo_objectvalue_ty *) ob)->mo_ob_mtime);
}       /* end mo_objref_put_attr */

static inline mo_value_t
mo_objref_set_of_attrs (mo_objref_t ob)
{
  if (!mo_dyncast_objref (ob))
    return NULL;
  return mo_assoval_keys_set (((mo_objectvalue_ty *) ob)->mo_ob_attrs);
}       /* end of mo_objref_set_of_attrs */

static inline mo_value_t
mo_objref_get_comp (mo_objref_t ob, int rk)
{
  if (!mo_dyncast_objref (ob))
    return NULL;
  mo_vectvaldatapayl_ty *vecomp = ((mo_objectvalue_ty *) ob)->mo_ob_comps;
  if (!vecomp)
    return NULL;
  return mo_vectval_nth (vecomp, rk);
}       /* end of mo_objref_get_comp */

static inline void
mo_objref_put_comp (mo_objref_t ob, int rk, mo_value_t va)
{
  if (!mo_dyncast_objref (ob))
    return;
  mo_vectvaldatapayl_ty *vecomp = ((mo_objectvalue_ty *) ob)->mo_ob_comps;
  if (!vecomp)
    return;
  mo_vectval_put_nth (vecomp, rk, va);
}       /* end of mo_objref_put_comp */

static inline unsigned
mo_objref_comp_count (mo_objref_t ob)
{
  if (!mo_dyncast_objref (ob))
    return 0;
  mo_vectvaldatapayl_ty *vecomp = ((mo_objectvalue_ty *) ob)->mo_ob_comps;
  if (!vecomp)
    return 0;
  return mo_vectval_count (vecomp);
}       /* end mo_objref_comp_count */

static inline void
mo_objref_comp_resize (mo_objref_t ob, unsigned newsiz)
{
  if (!mo_dyncast_objref (ob))
    return;
  ((mo_objectvalue_ty *) ob)->mo_ob_comps =
    mo_vectval_resize (((mo_objectvalue_ty *) ob)->mo_ob_comps, newsiz);
}       /* end of mo_objref_comp_resize */

static inline void
mo_objref_comp_reserve (mo_objref_t ob, unsigned gap)
{
  if (!mo_dyncast_objref (ob))
    return;
  ((mo_objectvalue_ty *) ob)->mo_ob_comps =
    mo_vectval_reserve (((mo_objectvalue_ty *) ob)->mo_ob_comps, gap);
}       /* end of mo_objref_comp_reserve */

static inline void
mo_objref_comp_append (mo_objref_t ob, mo_value_t va)
{
  if (!mo_dyncast_objref (ob))
    return;
  ((mo_objectvalue_ty *) ob)->mo_ob_comps =
    mo_vectval_append (((mo_objectvalue_ty *) ob)->mo_ob_comps, va);
}       /* end of mo_objref_comp_append */

#endif /*MONIMELT_INCLUDED_ */

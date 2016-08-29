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
typedef const json_t* mo_json_t;
#define thread_local _Thread_local




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

#define MOM_SIZE_MAX (1<<24)

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

static inline void *mom_gc_alloc (size_t sz)
{
  void *p = GC_MALLOC (sz);
  if (MOM_UNLIKELY (p == NULL))
    MOM_FATAPRINTF("failed to allocate %zd bytes", sz);
  memset (p, 0, sz);
  return p;
}
static inline void *mom_gc_alloc_scalar (size_t sz)
{
  void *p = GC_MALLOC_ATOMIC (sz);
  if (MOM_UNLIKELY (p == NULL))
    MOM_FATAPRINTF("failed to allocate %zd scalar bytes", sz);
  memset (p, 0, sz);
  return p;
}

static inline void *mom_gc_alloc_uncollectable (size_t sz)
{
  void *p = GC_MALLOC_UNCOLLECTABLE (sz);
  if (MOM_UNLIKELY (p == NULL))
    MOM_FATAPRINTF("failed to allocate %zd uncollectable bytes", sz);
  memset (p, 0, sz);
  return p;
}

static inline char *mom_gc_strdup (const char *s)
{
  if (!s || s == MOM_EMPTY_SLOT) return NULL;
  char*p =  GC_STRDUP (s);
  if (MOM_UNLIKELY (p == NULL))
    MOM_FATAPRINTF("failed to gc strdup %s", s);
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
extern void mom_init_objects(void);

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

// a name is valid if it is like some C identifier or keyword
// initial and final underscores are not allowed
// consecutive underscores are not allowed
bool mom_valid_name(const char*nam); // in object.c


momhash_t // in main.c
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
  mo_PASSOVALDATA,
  mo_PVECTVALDATA,
};

typedef const void*mo_value_t;
typedef struct mo_objectvalue_st mo_objectvalue_ty;
typedef struct mo_assovaldatapayl_st mo_assovalvadapayl_ty;
typedef struct mo_vectvaldatapayl_st mo_vectvalvadapayl_ty;
typedef mo_objectvalue_ty* mo_objref_t;
static inline int mo_objref_cmp(mo_objref_t, mo_objref_t);
static inline mo_value_t mo_dyncast_object(mo_value_t);
static inline mo_objref_t mo_dyncast_objref(mo_value_t);
typedef intptr_t mo_int_t;

typedef uint32_t mo_hid_t;
typedef uint64_t mo_loid_t;

#define MO_INTMAX INTPTR_MAX/2
#define MO_INTMIN INTPTR_MIN/2

static inline bool
mo_valid_pointer_value(mo_value_t p)
{
  return p != NULL && p != MOM_EMPTY_SLOT && ((intptr_t)p % 2 == 0);
}

static inline bool
mo_value_is_int(mo_value_t p)
{
  return p != NULL && p != MOM_EMPTY_SLOT && ((intptr_t)p % 2 != 0);
}


static inline mo_int_t
mo_value_to_int(mo_value_t p, mo_int_t def)
{
  return mo_value_is_int(p)?(intptr_t)p/2:def;
}

static inline mo_value_t mo_int_to_value(mo_int_t i)
{
  MOM_ASSERTPRINTF(i >= MO_INTMIN && i <= MO_INTMAX,
                   "integer %lld out of range", (long long)i);
  return (mo_value_t)(((intptr_t)i%2)+1);
}

#define MOM_CSTRIDLEN 18
extern const char* // in object.c, the buf is either null -then using
// a GC allocated one- or of size MOM_CSTRIDLEN+1
mo_cstring_from_hi_lo_ids (char*buf, mo_hid_t hid, mo_loid_t loid);

/* 10 * 60 * 60 so a 3 extendigit thing starting with 0 to 9 */
#define MOM_HID_BUCKETMAX 36000
static inline unsigned mo_hi_id_bucketnum(mo_hid_t hid)
{
  if (hid == 0) return 0;
  unsigned bn = hid >> 16;
  MOM_ASSERTPRINTF(bn > 0 && bn < MOM_HID_BUCKETMAX,
                   "mo_hi_id_bucketnum bad hid %lu (bn=%u)",
                   (unsigned long) hid, bn);
  return bn;
}

// converse operation, fill hid & loid from a valid buffer, or else return false
extern bool // in object.c
mo_get_hi_lo_ids_from_cstring(mo_hid_t* phid, mo_loid_t* ploid, const char*buf);

extern void // in object.c
mo_get_some_random_hi_lo_ids (mo_hid_t* phid, mo_loid_t* ploid);

extern momhash_t // in object.c
mo_hash_from_hi_lo_ids(mo_hid_t hid, mo_loid_t loid);

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

static inline enum mo_valkind_en mo_kind_of_value(mo_value_t v)
{
  if (mo_value_is_int(v)) return mo_KINT;
  else if (!mo_valid_pointer_value(v)) return mo_KNONE;
  else
    {
      unsigned k = ((mo_hashedvalue_ty*)v)->mo_va_kind;
      MOM_ASSERTPRINTF(k>=MOM_FIRST_BOXED_KIND && k<=MOM_LAST_KIND,
                       "mo_kind_of_value: bad kind #%u @%p", k, v);
      return (enum mo_valkind_en)k;
    }
}
///// sized values have also size
typedef struct mo_sizedvalue_st mo_sizedvalue_ty;
struct mo_sizedvalue_st
{
  struct mo_hashedvalue_st _mo;
  uint32_t mo_sva_size;
};

static inline uint32_t mo_size_of_value(mo_value_t v)
{
  enum mo_valkind_en k = mo_kind_of_value(v);
  if (k==mo_KSTRING||k==mo_KTUPLE||k==mo_KSET)
    return (((mo_sizedvalue_ty*)v))->mo_sva_size;
  return 0;
}

///// string values
typedef struct mo_stringvalue_st mo_stringvalue_ty;
struct mo_stringvalue_st
{
  struct mo_sizedvalue_st _mo;
  char mo_cstr[]; // allocated size is mo_sva_size+1
};
mo_value_t mo_make_string_len(const char*buf, int sz);
static inline mo_value_t mo_make_string_cstr(const char*buf)
{
  return mo_make_string_len(buf, -1);
};
mo_value_t mo_make_string_sprintf(const char*fmt, ...)
__attribute__((format(printf,1,2)));

static inline mo_value_t mo_dyncast_string(mo_value_t vs)
{
  if (!mo_valid_pointer_value(vs))
    return NULL;
  if (((mo_hashedvalue_ty*)vs)->mo_va_kind != mo_KSTRING)
    return NULL;
  return vs;
}

static inline const char*mo_string_cstr(mo_value_t v)
{
  mo_value_t vstr = mo_dyncast_string(v);
  if (!vstr) return NULL;
  return ((mo_stringvalue_ty*)vstr)->mo_cstr;
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
static inline mo_sequencevalue_ty* mo_sequence_allocate(unsigned sz)
{
  if (MOM_UNLIKELY(sz > MOM_SIZE_MAX))
    MOM_FATAPRINTF("too big size %u for sequence", sz);
  mo_sequencevalue_ty* seq = mom_gc_alloc(sizeof(mo_sequencevalue_ty)+sz*sizeof(mo_objref_t));
  // we temporarily put a fake kind
  ((mo_hashedvalue_ty*)seq)->mo_va_kind =  MOM_UNFILLEDFAKESEQKIND;
  ((mo_sizedvalue_ty*)seq)->mo_sva_size = sz;
  return seq;
}
/// allocate a filled sequence
static inline mo_sequencevalue_ty* mo_sequence_filled_allocate(unsigned sz, mo_objref_t*arr)
{
  if (MOM_UNLIKELY(sz > MOM_SIZE_MAX))
    MOM_FATAPRINTF("too big size %u for sequence", sz);
  mo_sequencevalue_ty* seq = mom_gc_alloc(sizeof(mo_sequencevalue_ty)+sz*sizeof(mo_objref_t));
  // we temporarily put a fake kind
  ((mo_hashedvalue_ty*)seq)->mo_va_kind =  MOM_UNFILLEDFAKESEQKIND;
  ((mo_sizedvalue_ty*)seq)->mo_sva_size = sz;
  if (arr && arr != MOM_EMPTY_SLOT)
    memcpy(seq->mo_seqobj,arr,sz*sizeof(mo_objref_t));
  return seq;
}

static inline mo_value_t mo_dyncast_sequence(mo_value_t vs)
{
  if (!mo_valid_pointer_value(vs))
    return NULL;
  if (((mo_hashedvalue_ty*)vs)->mo_va_kind != mo_KTUPLE
      && ((mo_hashedvalue_ty*)vs)->mo_va_kind != mo_KSET)
    return NULL;
  return vs;
}

// put inside a sequence something during the fill
static inline void mo_sequence_put(mo_sequencevalue_ty*seq, unsigned ix, mo_objref_t oref)
{
  MOM_ASSERTPRINTF(seq != NULL, "mo_sequence_put: null sequence");
  MOM_ASSERTPRINTF(((mo_hashedvalue_ty*)seq)->mo_va_kind == MOM_UNFILLEDFAKESEQKIND,
                   "mo_sequence_put: non-fake sequence");
  MOM_ASSERTPRINTF(ix < ((mo_sizedvalue_ty*)seq)->mo_sva_size,
                   "mo_sequence_put: too big index %u (size %u)",
                   ix, ((mo_sizedvalue_ty*)seq)->mo_sva_size);
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

mo_value_t mo_make_tuple_closeq(mo_sequencevalue_ty*seq);

// convenience variadic functions to make a tuple
mo_value_t mom_make_tuple_sized(unsigned siz, /*objref-s*/ ...);
mo_value_t mom_make_sentinel_tuple_(mo_objref_t ob1, ...) __attribute__((sentinel));
#define MOM_MAKE_SENTUPLE(...) mom_make_sentinel_tuple_(##__VA_ARGS__,NULL)

static inline mo_value_t mo_dyncast_tuple(mo_value_t vs)
{
  if (!mo_valid_pointer_value(vs))
    return NULL;
  if (((mo_hashedvalue_ty*)vs)->mo_va_kind != mo_KTUPLE)
    return NULL;
  return vs;
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

mo_value_t mo_make_set_closeq(mo_sequencevalue_ty*seq);
mo_value_t mo_make_set_closortedseq(mo_sequencevalue_ty*seq);
// convenience variadic functions to make a set
mo_value_t mom_make_set_sized(unsigned siz, /*objref-s*/ ...);
mo_value_t mom_make_sentinel_set_(mo_objref_t ob1, ...) __attribute__((sentinel));
#define MOM_MAKE_SENSET(...) mom_make_sentinel_set_(##__VA_ARGS__,NULL)

static inline mo_value_t mo_dyncast_set(mo_value_t vs)
{
  if (!mo_valid_pointer_value(vs))
    return NULL;
  if (((mo_hashedvalue_ty*)vs)->mo_va_kind != mo_KSET)
    return NULL;
  return vs;
}

mo_value_t mo_set_union (mo_value_t vset1, mo_value_t vset2);
mo_value_t mo_set_intersection (mo_value_t vset1, mo_value_t vset2);
mo_value_t mo_set_difference (mo_value_t vset1, mo_value_t vset2);

static inline bool mo_set_contains(mo_value_t vset, mo_objref_t ob)
{
  if (!mo_dyncast_set(vset)) return false;
  if (!mo_dyncast_object(ob)) return false;
  unsigned card = ((mo_sizedvalue_ty*)vset)->mo_sva_size;
  if (!card) return 0;
  unsigned lo = 0, hi = card - 1;
  while (lo + 5 < hi)
    {
      unsigned md = (lo+hi)/2;
      mo_objref_t midobr = ((mo_sequencevalue_ty*)vset)->mo_seqobj[md];
      MOM_ASSERTPRINTF(mo_dyncast_objref(midobr) != NULL,
                       "corrupted midobr@%p", midobr);
      if (midobr == ob) return true;
      int cmp = mo_objref_cmp(ob, midobr);
      if (cmp<0) lo = md;
      else if (cmp>0) hi = md;
      else
        MOM_FATAPRINTF("corrupted set@%p", vset);
    }
  for (unsigned md=lo; md<hi; md++)
    {
      mo_objref_t midobr = ((mo_sequencevalue_ty*)vset)->mo_seqobj[md];
      MOM_ASSERTPRINTF(mo_dyncast_objref(midobr) != NULL,
                       "corrupted midobr@%p", midobr);
      if (midobr == ob) return true;
    }
  return false;
}

/******************** OBJECTs ****************/
typedef struct mo_objectvalue_st mo_objectvalue_ty;
struct mo_objectvalue_st
{
  struct mo_hashedvalue_st _mo;
  /// actually, we dont need mutexes before the bootstrap
  /// pthread_mutex_t mo_ob_mtx;
  uint32_t mo_nameix;
  clock_t mo_mtime;
  mo_hid_t mo_ob_hid;
  mo_loid_t mo_ob_loid;
  mo_objref_t  mo_ob_class;
  mo_assovalvadapayl_ty *_mo_ob_attrs;
  mo_vectvalvadapayl_ty *_mo_ob_comps;
  mo_objref_t mo_ob_paylkind;
  void* mo_ob_payload;
};

static inline mo_value_t mo_dyncast_object(mo_value_t vs)
{
  if (!mo_valid_pointer_value(vs))
    return NULL;
  if (((mo_hashedvalue_ty*)vs)->mo_va_kind != mo_KOBJECT)
    return NULL;
  return vs;
}

static inline mo_objref_t mo_dyncast_objref(mo_value_t v)
{
  return (mo_objref_t) mo_dyncast_object(v);
}

mo_objref_t mo_objref_find_hid_loid(mo_hid_t hid, mo_loid_t loid);

static inline int mo_objref_cmp(mo_objref_t obl, mo_objref_t obr)
{
  obl = mo_dyncast_objref(obl);
  obr = mo_dyncast_objref(obr);
  if (obl == obr) return 0;
  if (MOM_UNLIKELY(obl == NULL)) return -1;
  if (MOM_UNLIKELY(obr == NULL)) return 1;
  if (obl->mo_ob_hid < obr->mo_ob_hid) return -1;
  if (obl->mo_ob_hid > obr->mo_ob_hid) return 1;
  if (obl->mo_ob_loid < obr->mo_ob_loid) return -1;
  if (MOM_LIKELY(obl->mo_ob_loid > obr->mo_ob_loid)) return 1;
  MOM_FATAPRINTF("distinct objects @%p & @%p with same id hid=%u lid=%llu",
                 obl, obr,
                 (unsigned)obl->mo_ob_hid, (unsigned long long)obl->mo_ob_loid);
}

int mo_value_cmp(mo_value_t vl, mo_value_t vr);

int mom_objref_cmp(const void*,const void*); // suitable for qsort, in object.c


///////////////// JSON support
// get the json for a value
mo_json_t mo_json_of_value(mo_value_t);
// get the json for an objref, e.g. an id string or null
mo_json_t mo_jsonid_of_objref(mo_objref_t);
// get the value from a json
mo_value_t mo_value_of_json(mo_json_t);
// get the existing objref from a json
mo_objref_t mo_objref_of_jsonid(mo_json_t);
#endif /*MONIMELT_INCLUDED_ */

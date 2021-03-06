// file meltmoni.h - common header file to be included everywhere.

/**   Copyright (C)  2015 - 2016 Basile Starynkevitch, later FSF
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

// the C&C++ portable way to have flexible array members is to use a dimension of 1
// see http://stackoverflow.com/a/4413035/841108
#define MOM_FLEXIBLE_DIM 1

#include <features.h>   // GNU things
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <gc/gc.h>


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

extern
#ifdef __cplusplus
"C"
#endif
void
mom_warnprintf_at (const char *fil, int lin, const char *fmt, ...)
__attribute__ ((format (printf, 3, 4)));


#if defined(NDEBUG) || !defined(MOM_GCDEBUG)
#ifdef __cplusplus
extern "C" {
#endif
static inline void *mom_gc_alloc (size_t sz);
static inline void *mom_gc_alloc_scalar (size_t sz);
void *mom_gc_calloc (size_t nmemb, size_t size);
static inline void *mom_gc_alloc_uncollectable (size_t sz);
static inline char *mom_gc_strdup (const char *s);
#ifdef __cplusplus
}; // end extern "C"
#endif
#else // !NDEBUG or MOM_GCDEBUG
#warning garbcoll debugging enabled
extern void *mom_gc_alloc_at (size_t sz, const char *fil, int lin);
#define mom_gc_alloc(Sz) mom_gc_alloc_at((Sz),__FILE__,__LINE__)
extern void *mom_gc_alloc_scalar_at (size_t sz, const char *fil, int lin);
#define mom_gc_alloc_scalar(Sz) mom_gc_alloc_scalar_at((Sz),__FILE__,__LINE__)
extern void *mom_gc_calloc_at (size_t nmemb, size_t size, const char *fil,
                               int lin);
#define mom_gc_calloc(Nb,Siz) mom_gc_calloc_at((Nb),(Siz),__FILE__,__LINE__)
extern void *mom_gc_alloc_uncollectable_at (size_t sz, const char *fil,
    int lin);
#define mom_gc_alloc_uncollectable(Sz) mom_gc_alloc_uncollectable_at((Sz),__FILE__,__LINE__)
extern char *mom_gc_strdup_at (const char *str, const char *fil, int lin);
#define mom_gc_strdup(Str) mom_gc_strdup_at((Str),__FILE__,__LINE__)
#endif // NDEBBUG or MOM_GCDEBUG

#ifdef __cplusplus
#include <atomic>
#include <exception>
#include <stdexcept>
#include <sstream>
#include <gc/gc_allocator.h>

typedef
std::atomic_bool
mom_atomic_bool_t;
typedef
std::atomic_int
mom_atomic_int_t;
typedef
std::atomic_int_least16_t
mom_atomic_int16_t;
typedef
std::atomic < FILE *>
mom_atomic_fileptr_t;

class
  MomRuntimeErrorAt:
  public
  std::runtime_error
{
  const char *
  _mre_file;
  int
  _mre_line;
public:
  const char *
  file () const
  {
    return
      _mre_file;
  };
  int
  lineno () const
  {
    return
      _mre_line;
  };
  MomRuntimeErrorAt (const char *fil, int lin, const char *msg):
    std::runtime_error (std::string (msg)),
    _mre_file (fil),
    _mre_line (lin)
  {
    mom_warnprintf_at (fil, lin, "runtime error: %s", msg);
  };
  MomRuntimeErrorAt (const char *fil, int lin, const std::string & msg):
    std::runtime_error (std::string (msg)),
    _mre_file (fil),
    _mre_line (lin)
  {
    mom_warnprintf_at (fil, lin, "runtime error: %s", msg.c_str ());
  };
  MomRuntimeErrorAt (const char *fil, int lin, std::ostringstream & out):
    std::runtime_error ((out.flush (), std::string (out.str ()))),
    _mre_file (fil),
    _mre_line (lin)
  {
    mom_warnprintf_at (fil, lin, "runtime error: %s", out.str ().c_str ());
  };
  ~MomRuntimeErrorAt ()
  {
    _mre_file = nullptr;
    _mre_line = 0;
  };
};

#define MOM_RUNTIME_ERROR(Msg) MomRuntimeErrorAt(__FILE__,__LINE__,Msg)
#define MOM_RUNTIME_LOG_AT_BIS(Fil,Lin,Log) MomRuntimeErrorAt(Fil,Lin,({std::ostringstream log_#Lin; log_#Lin<<Log; log_#Lin;}))
#define MOM_RUNTIME_LOG_AT(Fil,Lin,Log) \
  MOM_RUNTIME_ERROR_LOG_AT_BIS(Fil,Lin,Log)
#define MOM_RUNTIME_LOG(Log) MOM_RUNTIME_ERROR_LOG_AT(__FILE__,__LINE___,Log)
#define MOM_RUNTIME_PRINTF(Fmt,...) MomRuntimeErrorAt(__FILE__,__LINE__,mom_gc_printf(Fmt,__VA_ARGS__))
extern
"C"
{
#else
#include <stdatomic.h>
typedef atomic_bool mom_atomic_bool_t;
typedef atomic_int mom_atomic_int_t;
typedef atomic_int_least16_t mom_atomic_int16_t;
typedef FILE *_Atomic mom_atomic_fileptr_t;
#define thread_local _Thread_local
#endif /* C++ or C */

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
#if __GLIBC__
#include <execinfo.h>
#endif


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

#include <glib.h>

// in generated _timestamp.c
extern const char monimelt_timestamp[];
extern const char monimelt_lastgitcommit[];
extern const char monimelt_lastgittag[];
extern const char monimelt_compilercommand[];
extern const char monimelt_compilerflags[];
extern const char monimelt_optimflags[];
extern const char monimelt_checksum[];
extern const char monimelt_directory[];
extern const char monimelt_makefile[];


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

typedef const void *momvalue_t;
typedef intptr_t momint_t;
typedef struct mom_item_st*momitemptr_t;

#ifdef NDEBUG
#define MOM_PRIVATE static
#else
#define MOM_PRIVATE   /*nothing */
#endif /*NDEBUG*/
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




#if defined(NDEBUG) || !defined(MOM_GCDEBUG)
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
#endif // NDEBUG || !MOM_GCDEBUG

// for debugging; the color level are user-definable:
#define MOM_DEBUG_LIST_OPTIONS(Dbg)   \
  Dbg(cmd)          \
  Dbg(garbcoll)         \
  Dbg(dump)         \
  Dbg(gencod)         \
  Dbg(item)         \
  Dbg(load)         \
  Dbg(low)          \
  Dbg(mutex)          \
  Dbg(run)          \
  Dbg(boot)         \
  Dbg(blue)         \
  Dbg(green)          \
  Dbg(red)          \
  Dbg(yellow)         \
  Dbg(web)

#define MOM_DEBUG_DEFINE_OPT(Nam) momdbg_##Nam,
enum mom_debug_en
{
  momdbg__none,
  MOM_DEBUG_LIST_OPTIONS (MOM_DEBUG_DEFINE_OPT) momdbg__last
};

unsigned mom_debugflags;

#define MOM_IS_DEBUGGING(Dbg) (mom_debugflags & (1<<momdbg_##Dbg))

void mom_set_debugging (const char *dbgopt);


void
mom_debugprintf_at (const char *fil, int lin, enum mom_debug_en dbg,
                    const char *fmt, ...)
__attribute__ ((format (printf, 4, 5)));

#define MOM_DEBUGPRINTF_AT(Fil,Lin,Dbg,Fmt,...) do {  \
    if (MOM_IS_DEBUGGING(Dbg))        \
      mom_debugprintf_at (Fil,Lin,momdbg_##Dbg,Fmt, \
        ##__VA_ARGS__);   \
  } while(0)

#define MOM_DEBUGPRINTF_AT_BIS(Fil,Lin,Dbg,Fmt,...) \
  MOM_DEBUGPRINTF_AT(Fil,Lin,Dbg,Fmt,     \
         ##__VA_ARGS__)

#define MOM_DEBUGPRINTF(Dbg,Fmt,...)      \
  MOM_DEBUGPRINTF_AT_BIS(__FILE__,__LINE__,Dbg,Fmt, \
       ##__VA_ARGS__)


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
static inline uint32_t
mom_random_uint32 (void)
{
#ifdef __cplusplus
  static thread_local int count;
#else
  static _Thread_local int count;
#endif
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

#include "_mom_header.h"
  
enum momitype_en
{
  MOMITY_NONE,
  MOMITY_INT,     /// see `int` predefined
  MOMITY_BOXDOUBLE,   /// see `double` predefined
  MOMITY_BOXSTRING,   /// see `string` predefined
  MOMITY_TUPLE,     /// see `tuple` predefined
  MOMITY_SET,     /// see `set` predefined
  MOMITY_NODE,      /// see `node` predefined
  MOMITY_ITEM,      /// see `item` predefined
  /// the types above are for genuine values
  MOMITY__LASTHASHED,
  /// here are the payload types, they can only appear as item
  /// payloads if they are persistent (but they might be used as
  /// transient local data in code)
  MOMITY_ASSOVALDATA,
  MOMITY_VECTVALDATA,
  MOMITY_QUEUE,
  MOMITY_HASHSET,
  MOMITY_HASHMAP,
  MOMITY_HASHASSOC,
  MOMITY_LOADER,
  MOMITY_DUMPER,
  MOMITY_JSON,
  MOMITY_WEBEXCH,
  MOMITY_WEBSESSION,
  MOMITY_TASKLET,
  MOMITY_TASKSTEPPER,
  MOMITY_FILE,
  MOMITY_FILEBUFFER,
  MOMITY__LAST
};
struct mom_item_st;
struct mom_loader_st;
struct mom_dumper_st;
struct mom_tasklet_st;
struct mom_filebuffer_st;


#define MOM_PREDEFITM(Nam) (&mompredef_##Nam)

#define MOM_HAS_PREDEFINED(Nam,Hash) extern struct mom_item_st mompredef_##Nam;
#include "_mom_predef.h"

enum
{
#define MOM_HAS_PREDEFINED(Nam,Hash) momhashpredef_##Nam = Hash,
#include "_mom_predef.h"
};

#define MOM_HEADER_PREDEF "_mom_predef.h"
#define MOM_GLOBAL_STATE "global.mom"

struct mom_hashedvalue_st;
const char *mom_item_cstring (const struct mom_item_st *itm);

int mom_item_cmp (const struct mom_item_st *itm1,
                  const struct mom_item_st *itm2);

int mom_hashedvalue_cmp (const struct mom_hashedvalue_st *val1,
                         const struct mom_hashedvalue_st *val2);

bool mom_hashedvalue_equal (const struct mom_hashedvalue_st *val1,
                            const struct mom_hashedvalue_st *val2);
static inline momhash_t mom_hash (const void *p);
static inline momhash_t mom_item_hash (const struct mom_item_st *itm);

#ifdef __cplusplus
struct MomItemLess
{
  bool operator  () (const struct mom_item_st * itm1,
                     const struct mom_item_st * itm2) const
  {
    return::mom_item_cmp (itm1, itm2) < 0;
  }
};

struct MomValueLess
{
  bool operator  () (const void *v1, const void *v2) const
  {
    return::mom_hashedvalue_cmp ((const struct mom_hashedvalue_st *) v1,
                                 (const struct mom_hashedvalue_st *) v2) < 0;
  }
};
struct MomValueEqual
{
  bool operator  () (const void *v1, const void *v2) const
  {
    return::mom_hashedvalue_equal ((const struct mom_hashedvalue_st *) v1,
                                   (const struct mom_hashedvalue_st *) v2);
  }
};
struct MomValueHash
{
  bool operator  () (const void *v) const
  {
    return::mom_hash (v);
  }
};
#endif /*__cplusplus*/

// the common prefix of all values
#define MOM_ANYVALUE_FIELDS     \
  uint16_t va_itype;        \
  mom_atomic_int16_t va_ixv;      \
  uint32_t va_size
struct mom_anyvalue_st
{
  /// field prefix: va_;
  MOM_ANYVALUE_FIELDS;
};

static inline unsigned
mom_itype (const void *p)
{
  if (p && p != MOM_EMPTY_SLOT)
    {
      if ((intptr_t) p % 2 == 0)
        return ((const struct mom_anyvalue_st *) p)->va_itype;
      else
        return MOMITY_INT;
    }
  return 0;
}

const char *mom_itype_str (const void *p);

static inline unsigned
mom_raw_size (const void *p)
{
  assert (p != NULL && p != MOM_EMPTY_SLOT && (intptr_t) p % 2 == 0);
  return (((const struct mom_anyvalue_st *) p)->va_size);
}

static inline unsigned
mom_size (const void *p)
{
  if (mom_itype (p) > MOMITY_INT)
    return mom_raw_size (p);
  return 0;
}

static inline void
mom_put_size (void *p, unsigned sz)
{
  if (mom_itype (p) <= MOMITY_INT)
    return;
  if (sz >= MOM_SIZE_MAX)
    MOM_FATAPRINTF ("too big size %u", sz);
  ((struct mom_anyvalue_st *) p)->va_size = sz;
}


#define MOM_HASHEDVALUE_FIELDS      \
  MOM_ANYVALUE_FIELDS;        \
  momhash_t hva_hash
struct mom_hashedvalue_st
{
  MOM_HASHEDVALUE_FIELDS;
};


static inline momint_t mom_int_val_def (const void *p, momint_t def);
static inline momhash_t
mom_hash (const void *p)
{
  unsigned t = mom_itype (p);
  if (t > MOMITY_INT && t < MOMITY__LASTHASHED)
    return ((const struct mom_hashedvalue_st *) p)->hva_hash;
  else if (t == MOMITY_INT)
    {
      momint_t i = mom_int_val_def (p, 0);
      return i % 1000001137 + 1;
    }
  return 0;
}

static inline void
mom_put_hash (void *p, momhash_t h)
{
  unsigned t = mom_itype (p);
  if (t > MOMITY_INT && t < MOMITY__LASTHASHED)
    {
      if (((struct mom_hashedvalue_st *) p)->hva_hash == 0)
        ((struct mom_hashedvalue_st *) p)->hva_hash = h;
      else if (((const struct mom_hashedvalue_st *) p)->hva_hash != h)
        MOM_FATAPRINTF ("cannot change hash @%p from %u to %u",
                        p,
                        ((const struct mom_hashedvalue_st *) p)->hva_hash, h);
    }
}


struct mom_boxint_st
{
  MOM_HASHEDVALUE_FIELDS;
  // here field
  momint_t boxi_int;
};
static inline momint_t
mom_int_val_def (const void *p, momint_t def)
{
  if (mom_itype (p) == MOMITY_INT)
    {
      if ((momint_t) p % 2 == 0)
        return ((const struct mom_boxint_st *) p)->boxi_int;
      else
        return ((momint_t) p >> 1);
    }
  return def;
}



static inline momhash_t
mom_int_hash (momint_t i)
{
  momhash_t h = (i * 509) ^ (i % 76519);
  if (!h)
    h = (i & 0xffffff) + ((i >> 25) & 0xffffff) + 11;
  assert (h != 0);
  return h;
}


const void *mom_int_make (momint_t i);
static inline const void *
mom_dyncast_int (const void *p)
{
  if (mom_itype (p) == MOMITY_INT)
    return p;
  return NULL;
};

struct mom_boxdouble_st
{
  MOM_HASHEDVALUE_FIELDS;
  // here field
  double boxd_dbl;
};

static inline const struct mom_boxdouble_st *
mom_dyncast_boxdouble (const void *p)
{
  if (mom_itype (p) == MOMITY_BOXDOUBLE)
    return (const struct mom_boxdouble_st *) p;
  return NULL;
}

momhash_t mom_double_hash (double x);
const struct mom_boxdouble_st *mom_boxdouble_make (double x);


static inline double
mom_boxdouble_val_def (const void *p, double def)
{
  const struct mom_boxdouble_st *bd = mom_dyncast_boxdouble (p);
  if (bd)
    return bd->boxd_dbl;
  return def;
}



struct mom_boxstring_st
{
  MOM_HASHEDVALUE_FIELDS;
  // here
  char boxs_cstr[MOM_FLEXIBLE_DIM];  /* actual size mom_raw_size+1 */
};

static inline const struct mom_boxstring_st *
mom_dyncast_boxstring (const void *p)
{
  if (mom_itype (p) == MOMITY_BOXSTRING)
    return (const struct mom_boxstring_st *) p;
  return NULL;
}

static inline const char *
mom_boxstring_cstr (const void *p)
{
  const struct mom_boxstring_st *str = mom_dyncast_boxstring (p);
  if (str)
    return str->boxs_cstr;
  return NULL;
}

const struct mom_boxstring_st *mom_boxstring_make (const char *s);

// make a string of given length or the strlen(s) if len<0 or len bigger
const struct mom_boxstring_st *mom_boxstring_make_len (const char *s,
    int len);

const struct mom_boxstring_st *mom_boxstring_printf (const char *fmt, ...)
__attribute__ ((format (printf, 1, 2)));


const char *mom_gc_printf (const char *fmt, ...)
__attribute__ ((format (printf, 1, 2)));

#define MOM_SEQITEMS_FIELDS           \
  MOM_HASHEDVALUE_FIELDS;           \
  struct mom_item_st* seqitem[MOM_FLEXIBLE_DIM] /* actual size mom_raw_size */
struct mom_seqitems_st
{
  MOM_SEQITEMS_FIELDS;
};

// a tuple can contain NULL items
struct mom_boxtuple_st
{
  MOM_SEQITEMS_FIELDS;
};
// a set does not contain NULL items and they are sorted in ascending order
struct mom_boxset_st
{
  MOM_SEQITEMS_FIELDS;
};

static inline const struct mom_seqitems_st *
mom_dyncast_seqitems (const void *p)
{
  uint8_t ityp = mom_itype (p);
  if (ityp == MOMITY_TUPLE || ityp == MOMITY_SET)
    return (const struct mom_seqitems_st *) p;
  return NULL;
}

static inline struct mom_item_st *const *
mom_seqitems_arr (const void *p)
{
  const struct mom_seqitems_st *si = mom_dyncast_seqitems (p);
  if (si)
    return (struct mom_item_st * const *) si->seqitem;
  return NULL;
}


static inline unsigned
mom_seqitems_length (const void *p)
{
  const struct mom_seqitems_st *si = mom_dyncast_seqitems (p);
  if (si)
    return mom_raw_size (si);
  return 0;
}

static inline struct mom_item_st *
mom_seqitems_nth (const void *p, int rk)
{
  const struct mom_seqitems_st *si = mom_dyncast_seqitems (p);
  if (!si)
    return NULL;
  unsigned sz = mom_raw_size (si);
  if (rk < 0)
    rk += sz;
  if (rk >= 0 && rk < (int) sz)
    return si->seqitem[rk];
  return NULL;
}

static inline const struct mom_boxtuple_st *
mom_dyncast_tuple (const void *p)
{
  if (mom_itype (p) == MOMITY_TUPLE)
    return (const struct mom_boxtuple_st *) p;
  return NULL;
}

static inline const struct mom_boxset_st *
mom_dyncast_set (const void *p)
{
  if (mom_itype (p) == MOMITY_SET)
    return (const struct mom_boxset_st *) p;
  return NULL;
}


static inline unsigned
mom_boxtuple_length (const struct mom_boxtuple_st *btup)
{
  if (mom_itype (btup) != MOMITY_TUPLE)
    return 0;
  return mom_raw_size (btup);
}       /* end of mom_boxtuple_length */

static inline struct mom_item_st *
mom_boxtuple_nth (const struct mom_boxtuple_st *btup, int rk)
{
  if (mom_itype (btup) != MOMITY_TUPLE)
    return NULL;
  unsigned sz = mom_raw_size (btup);
  if (rk < 0)
    rk += sz;
  if (rk >= 0 && rk < (int) sz)
    return (struct mom_item_st *) (btup->seqitem[rk]);
  return NULL;
}       /* end of mom_boxtuple_nth */


const struct mom_boxtuple_st *mom_boxtuple_make_arr2 (unsigned siz1,
    const struct
    mom_item_st *const
    *arr1, unsigned siz2,
    const struct
    mom_item_st *const
    *arr2);

const struct mom_boxtuple_st *mom_boxtuple_make_arr (unsigned siz,
    const struct
    mom_item_st *const *arr);

const struct mom_boxtuple_st *mom_boxtuple_make_va (unsigned siz, ...);

// make a flatten tuple; for every argument: if it is an item or
// nil, add it; if it is a node of connective itm, add the
// items-or-nil sons; if it is a sequence, add its items

const struct mom_boxtuple_st *
mom_boxtuple_flatten_make_va (struct mom_item_st*itm, unsigned siz, ...);

const struct mom_boxtuple_st *mom_boxtuple_make_sentinel_va (struct
    mom_item_st *,
    ...)
__attribute__ ((sentinel));
#define mom_boxtuple_make_sentinel(...) mom_boxtuple_make_sentinel_va(##__VA_ARGS__, NULL)

static inline unsigned
mom_boxset_length (const struct mom_boxset_st *bset)
{
  if (mom_itype (bset) != MOMITY_SET)
    return 0;
  return mom_raw_size (bset);
}       /* end of mom_boxset_length */

static inline const struct mom_item_st *
mom_boxset_nth (const struct mom_boxset_st *bset, int rk)
{
  if (mom_itype (bset) != MOMITY_SET)
    return NULL;
  unsigned sz = mom_raw_size (bset);
  if (rk < 0)
    rk += sz;
  if (rk >= 0 && rk < (int) sz)
    return bset->seqitem[rk];
  return NULL;
}       /* end of mom_boxset_nth */


const struct mom_boxset_st *mom_boxset_make_arr2 (unsigned siz1,
    const struct mom_item_st
    **arr1, unsigned siz2,
    const struct mom_item_st
    **arr2);

static inline const struct mom_boxset_st *
mom_boxset_make_arr (unsigned siz, const struct mom_item_st **arr)
{
  return mom_boxset_make_arr2 (siz, arr, 0, NULL);
}

const struct mom_boxset_st *mom_boxset_make_va (unsigned siz, ...);

// make a flatten set; for every argument: if it is an item, add it;
// if it is a node of connective itm, add the items-or-nil sons; if it
// is a sequence, add its non-nil items

const struct mom_boxset_st *
mom_boxset_flatten_make_va (struct mom_item_st*itm, unsigned siz, ...);

const struct mom_boxset_st *mom_boxset_make_sentinel_va (struct mom_item_st
    *, ...)
__attribute__ ((sentinel));
#define mom_boxset_make_sentinel(...) mom_boxset_make_sentinel_va(##__VA_ARGS__, NULL)


const struct mom_boxset_st *mom_boxset_union (const struct mom_boxset_st
    *set1,
    const struct mom_boxset_st
    *set2);
const struct mom_boxset_st *mom_boxset_intersection (const struct
    mom_boxset_st *set1,
    const struct
    mom_boxset_st *set2);
const struct mom_boxset_st *mom_boxset_difference (const struct
    mom_boxset_st *set1,
    const struct
    mom_boxset_st *set2);

////////////////

struct mom_boxnode_st
{
  MOM_HASHEDVALUE_FIELDS;
  // here prefix nod_
  momint_t nod_metarank;
  struct mom_item_st *nod_metaitem;
  struct mom_item_st *nod_connitm;
  momvalue_t nod_sons[MOM_FLEXIBLE_DIM];  /* actual size is the mom_raw_size */
};
static inline const struct mom_boxnode_st *
mom_dyncast_node (const void *p)
{
  if (mom_itype (p) == MOMITY_NODE)
    return (const struct mom_boxnode_st *) p;
  return NULL;
}

static inline struct mom_item_st *
mom_boxnode_conn (const void *p)
{
  if (mom_itype (p) == MOMITY_NODE)
    return ((const struct mom_boxnode_st *) p)->nod_connitm;
  return NULL;
}

static inline momvalue_t
mom_boxnode_nth (const void *p, int rk)
{
  if (mom_itype (p) == MOMITY_NODE)
    {
      unsigned sz = mom_raw_size (p);
      if (rk < 0)
        rk += sz;
      if (rk >= 0 && rk < (int) sz)
        return ((const struct mom_boxnode_st *) p)->nod_sons[rk];
    }
  return NULL;
}

const struct mom_boxnode_st *mom_boxnode_make_meta
(const struct mom_item_st *conn, int size,
 const momvalue_t *sons, const struct mom_item_st *meta, momint_t metarank);

static inline const struct mom_boxnode_st *
mom_boxnode_make (const struct mom_item_st *conn, int size, momvalue_t *sons)
{
  return mom_boxnode_make_meta (conn, size, sons, NULL, 0);
}

// return a node with each argument, except that if it is a node of
// connective itm, we expand to its sons
const struct mom_boxnode_st *
mom_boxnode_flatten_make_va (const struct mom_item_st *connitm,
                             struct mom_item_st *itm, unsigned siz, ...);

const struct mom_boxnode_st *mom_boxnode_meta_make_va (const struct
    mom_item_st *meta,
    momint_t metarank,
    const struct
    mom_item_st *connitm,
    unsigned size,
    /*momvalue_t-s */ ...);

const struct mom_boxnode_st *mom_boxnode_make_va (const struct mom_item_st
    *connitm, unsigned size,
    /*momvalue_t-s */ ...);

const struct mom_boxnode_st *mom_boxnode_meta_make_sentinel_va (const struct
    mom_item_st
    *meta,
    momint_t
    metarank,
    const struct
    mom_item_st
    *conn, ...)
__attribute__ ((sentinel));

#define mom_boxnode_meta_make_sentinel(MetaItm,MetaRank,Conn,...) \
  mom_boxnode_meta_make_sentinel_va((MetaItm),(MetaRank),(Conn),  \
            ##__VA_ARGS__,NULL)

#define mom_boxnode_make_sentinel(Conn,...)   \
  mom_boxnode_meta_make_sentinel_va(NULL,0,(Conn),  \
            ##__VA_ARGS__,NULL)



#define MOM_COUNTEDATA_FIELDS     \
  MOM_ANYVALUE_FIELDS;        \
  uint32_t cda_count


struct mom_itementry_tu
{
  struct mom_item_st *ient_itm;
  struct mom_hashedvalue_st *ient_val;
};


////////////////

#define MOM_ASSOVALDATA_FIELDS            \
  MOM_COUNTEDATA_FIELDS;            \
  struct mom_itementry_tu ada_ents[MOM_FLEXIBLE_DIM]  /* sorted array of entries */
// allocated size of ada_ents is size; used count is cda_count.
struct mom_assovaldata_st
{
  MOM_ASSOVALDATA_FIELDS;
};

static inline const struct mom_assovaldata_st *
mom_assovaldata_dyncast (const void *p)
{
  if (mom_itype (p) == MOMITY_ASSOVALDATA)
    {
      const struct mom_assovaldata_st *ass =
      (const struct mom_assovaldata_st *) p;
      return ass;
    }
  return NULL;
}       /* end mom_assovaldata_dyncast */

static inline unsigned
mom_assovaldata_count (const struct mom_assovaldata_st *ass)
{
  if (mom_itype (ass) != MOMITY_ASSOVALDATA)
    return 0;
  unsigned cnt = ass->cda_count;
  assert (cnt <= mom_raw_size (ass));
  return cnt;
}       /* end of mom_assovaldata_count */

const struct mom_boxset_st *mom_assovaldata_set_attrs (const struct
    mom_assovaldata_st
    *ass);

struct mom_hashedvalue_st *mom_assovaldata_get (const struct
    mom_assovaldata_st *asso,
    const struct mom_item_st
    *itmat);

struct mom_assovaldata_st *mom_assovaldata_remove (struct mom_assovaldata_st
    *asso,
    const struct mom_item_st
    *itmat);

struct mom_assovaldata_st *mom_assovaldata_put (struct mom_assovaldata_st
    *asso,
    const struct mom_item_st
    *itmat, const void *data);

struct mom_assovaldata_st *mom_assovaldata_reserve (struct
    mom_assovaldata_st
    *asso, unsigned gap);

////////////////

#define MOM_VECTVALDATA_FIELDS          \
  MOM_COUNTEDATA_FIELDS;          \
  struct mom_hashedvalue_st*vecd_valarr[MOM_FLEXIBLE_DIM];
//// mutable vector
struct mom_vectvaldata_st
{
  MOM_VECTVALDATA_FIELDS;
};


static inline struct mom_vectvaldata_st *
mom_vectvaldata_dyncast (void *p)
{
  if (mom_itype (p) == MOMITY_VECTVALDATA)
    return (struct mom_vectvaldata_st *) p;
  return NULL;
}

struct mom_vectvaldata_st *mom_vectvaldata_reserve (struct
    mom_vectvaldata_st *vec,
    unsigned gap);

static inline const struct mom_hashedvalue_st *
mom_vectvaldata_nth (const struct mom_vectvaldata_st *vec, int rk)
{
  if (mom_itype (vec) != MOMITY_VECTVALDATA)
    return NULL;
  unsigned cnt = vec->cda_count;
  assert (cnt <= mom_raw_size (vec));
  if (rk < 0)
    rk += cnt;
  if (rk >= 0 && rk < (int) cnt)
    return vec->vecd_valarr[rk];
  return NULL;
}

static inline void
mom_vectvaldata_put_nth (struct mom_vectvaldata_st *vec, int rk,
                         const void *data)
{
  if (mom_itype (vec) != MOMITY_VECTVALDATA)
    return;
  if (data == MOM_EMPTY_SLOT)
    data = NULL;
  unsigned cnt = vec->cda_count;
  assert (cnt <= mom_raw_size (vec));
  if (rk < 0)
    rk += cnt;
  if (rk >= 0 && rk < (int) cnt)
    vec->vecd_valarr[rk] = (struct mom_hashedvalue_st *) data;
}

static inline unsigned
mom_vectvaldata_count (const struct mom_vectvaldata_st *vec)
{
  if (mom_itype (vec) != MOMITY_VECTVALDATA)
    return 0;
  unsigned cnt = vec->cda_count;
  assert (cnt <= mom_raw_size (vec));
  return cnt;
}

static inline struct mom_anyvalue_st **
mom_vectvaldata_valvect (const struct mom_vectvaldata_st *vec)
{
  if (mom_itype (vec) != MOMITY_VECTVALDATA)
    return NULL;
  assert (vec->cda_count <= mom_raw_size (vec));
  return (struct mom_anyvalue_st **) vec->vecd_valarr;
}

struct mom_vectvaldata_st *mom_vectvaldata_resize (struct mom_vectvaldata_st
    *vec, unsigned count);


struct mom_vectvaldata_st *mom_vectvaldata_append (struct mom_vectvaldata_st
    *vec, const void *data);

////////////////
/// for MOMITY_HASHSET

#define MOM_HASHSET_FIELDS        \
  MOM_COUNTEDATA_FIELDS;        \
  struct mom_item_st*hset_items[MOM_FLEXIBLE_DIM];
//// mutable hashed set
struct mom_hashset_st
{
  MOM_HASHSET_FIELDS;
};

static inline struct mom_hashset_st *
mom_hashset_dyncast (void *p)
{
  if (mom_itype (p) == MOMITY_HASHSET)
    {
      struct mom_hashset_st *hset = (struct mom_hashset_st *) p;
      return hset;
    }
  return NULL;
}       /* end mom_hashset_dyncast */


/// with a 0 gap, will reorganize
struct mom_hashset_st *mom_hashset_reserve (struct mom_hashset_st *hset,
    unsigned gap);


struct mom_hashset_st *mom_hashset_insert (struct mom_hashset_st *hset,
    struct mom_item_st *itm);

struct mom_hashset_st *mom_hashset_remove (struct mom_hashset_st *hset,
    struct mom_item_st *itm);

bool
mom_hashset_contains (const struct mom_hashset_st *hset,
                      const struct mom_item_st *itm);

const struct mom_boxset_st *mom_hashset_to_boxset (const struct
    mom_hashset_st *hset);

static inline unsigned
mom_hashset_count (const struct mom_hashset_st *hset)
{
  if (mom_itype (hset) != MOMITY_HASHSET)
    return 0;
  return hset->cda_count;
}

/// for MOMITY_HASHMAP payload

#define MOM_HASHMAP_FIELDS        \
  MOM_COUNTEDATA_FIELDS;        \
  struct mom_itementry_tu hmap_ents[MOM_FLEXIBLE_DIM]
//// mutable hashed map
struct mom_hashmap_st
{
  MOM_HASHMAP_FIELDS;
};


static inline struct mom_hashmap_st *
mom_hashmap_dyncast (void *p)
{
  if (mom_itype (p) == MOMITY_HASHMAP)
    return (struct mom_hashmap_st *) p;
  return NULL;
}       /* end mom_hashmap_dyncast */



/// with a 0 gap, will reorganize
struct mom_hashmap_st *mom_hashmap_reserve (struct mom_hashmap_st *hmap,
    unsigned gap);

const struct mom_hashedvalue_st *mom_hashmap_get (const struct
    mom_hashmap_st *hmap,
    const struct mom_item_st
    *itm);

struct mom_hashmap_st *mom_hashmap_put (struct mom_hashmap_st *hmap,
                                        const struct mom_item_st *itm,
                                        const struct mom_hashedvalue_st *val);

struct mom_hashmap_st *mom_hashmap_remove (struct mom_hashmap_st *hmap,
    const struct mom_item_st *itm);

const struct mom_boxset_st *mom_hashmap_keyset (const struct mom_hashmap_st
    *hmap);


static inline unsigned
mom_hashmap_count (const struct mom_hashmap_st *hmap)
{
  if (mom_itype (hmap) != MOMITY_HASHMAP)
    return 0;
  return hmap->cda_count;
}

/// for MOMITY_HASHASSOC payload
struct mom_hassocentry_tu
{
  const struct mom_hashedvalue_st *hass_key;
  const struct mom_hashedvalue_st *hass_val;
};

#define MOM_HASHASSOC_FIELDS        \
  MOM_COUNTEDATA_FIELDS;        \
  struct mom_hassocentry_tu hass_ents[MOM_FLEXIBLE_DIM]
//// mutable hashed association
struct mom_hashassoc_st
{
  MOM_HASHASSOC_FIELDS;
};
static inline struct mom_hashassoc_st *
mom_hashassoc_dyncast (void *p)
{
  if (mom_itype (p) == MOMITY_HASHASSOC)
    return (struct mom_hashassoc_st *) p;
  return NULL;
}       /* end mom_hashassoc_dyncast */



// with a 0 gap will reorganize
struct mom_hashassoc_st *mom_hashassoc_reserve (struct mom_hashassoc_st
    *hass, unsigned gap);

const struct mom_hashedvalue_st *mom_hashassoc_get (const struct mom_hashassoc_st *hass,  //
    const struct
    mom_hashedvalue_st *key);

// specialized gets are faster, because they compare specifically the key
const struct mom_hashedvalue_st *mom_hashassoc_get_cstring (const struct mom_hashassoc_st *hass,  //
    const char *cstr);

const struct mom_hashedvalue_st *mom_hashassoc_get_item (const struct mom_hashassoc_st *hass, //
    const struct
    mom_item_st *itm);

const struct mom_hashedvalue_st *mom_hashassoc_get_int (const struct mom_hashassoc_st *hass,  //
    momint_t num);

const struct mom_hashedvalue_st *mom_hashassoc_get_double (const struct mom_hashassoc_st *hass, //
    double d);

struct mom_hashassoc_st *mom_hashassoc_put (struct mom_hashassoc_st *hass,  //
    const struct mom_hashedvalue_st *key, //
    const struct mom_hashedvalue_st
    *val);

struct mom_hashassoc_st *mom_hashassoc_remove (struct mom_hashassoc_st *hass, //
    const struct
    mom_hashedvalue_st *key);
const struct mom_boxnode_st *mom_hashassoc_sorted_key_node (const struct mom_hashassoc_st *hass,  //
    const struct
    mom_item_st
    *connitm);

static inline unsigned
mom_hashassoc_count (const struct mom_hashassoc_st *ha)
{
  if (mom_itype (ha) != MOMITY_HASHASSOC)
    return 0;
  return ha->cda_count;
}

////////////////
struct mom_itemradix_tu
{
  uint32_t itrad_rank;
  struct mom_boxstring_st itrad_string;
};

enum mom_space_en
{
  MOMSPA_NONE,
  MOMSPA_PREDEF,
  MOMSPA_GLOBAL,
};

#define MOM_MODULEDIR "modules"
#define MOM_CPREFIX_PATH "momg_"
#define MOM_FUNC_PREFIX "momf_"
#define MOM_SIGNATURE_PREFIX "momsig_"

#define MOM_DATA_PREFIX "momd_"
#define MOM_DATAKIND_PREFIX "momkind_"

/* inside an item, va_ixv is the space index */
#define MOM_ITEM_FIELDS             \
  MOM_HASHEDVALUE_FIELDS;           \
  struct mom_itemradix_tu* itm_radix;          \
  pthread_mutex_t itm_mtx;            \
  uint32_t itm_hid;             \
  uint64_t itm_lid;             \
  time_t itm_mtime;             \
  struct mom_assovaldata_st* itm_pattr;         \
  struct mom_vectvaldata_st* itm_pcomp;         \
  struct mom_item_st* itm_paylkind;                   \
  void* itm_payldata



struct mom_item_st
{
  MOM_ITEM_FIELDS;
};

static inline momhash_t
mom_item_hash (const struct mom_item_st *itm)
{
  if (itm == NULL || itm == MOM_EMPTY_SLOT || (intptr_t) itm % 4 != 0)
    return 0;
  if (itm->va_itype != MOMITY_ITEM)
    return 0;
  return itm->hva_hash;
}


const struct mom_boxset_st *mom_predefined_items_boxset (void);
void mom_item_put_space (struct mom_item_st *itm, enum mom_space_en spix);
static inline unsigned
mom_item_space (struct mom_item_st *itm)
{
  if (mom_itype (itm) != MOMITY_ITEM)
    return MOMSPA_NONE;
  return atomic_load (&itm->va_ixv);
}       /* end of mom_item_space */

static inline struct mom_item_st *
mom_dyncast_item (const void *p)
{
  if (mom_itype (p) == MOMITY_ITEM)
    return (struct mom_item_st *) p;
  return NULL;
}

void mom_initialize_a_predefined (struct mom_item_st *itm, const char *name,
                                  momhash_t hash);

void mom_initialize_predefined_items (void);

momhash_t mom_cstring_hash_len (const char *str, int len);


bool mom_valid_name_radix_len (const char *str, int len);

static inline bool
mom_valid_name_radix (const char *str)
{
  return str && str != MOM_EMPTY_SLOT
         && mom_valid_name_radix_len (str, strlen (str));
}


const struct mom_itemradix_tu *mom_find_name_radix (const char *str);
static inline const struct mom_itemradix_tu *
mom_find_name_radix_len (const char *str, int len)
{
  if (!str || str == MOM_EMPTY_SLOT || len == 0)
    return NULL;
  if (len < 0)
    return mom_find_name_radix (str);
  if (str[len] == (char) 0 && (int) strlen (str) == len)
    return mom_find_name_radix (str);
  char copybuf[64];
  memset (copybuf, 0, sizeof (copybuf));
  if (len < (int) sizeof (copybuf) - 1)
    {
      strncpy (copybuf, str, len);
      return mom_find_name_radix (copybuf);
    }
  else
    {
      char *cbuf = (char *) mom_gc_alloc_scalar (len + 1);
      strncpy (cbuf, str, len);
      return mom_find_name_radix (cbuf);
    }
}       /* end of mom_find_name_radix_len */


const struct mom_itemradix_tu *mom_make_name_radix (const char *str);
static inline const struct mom_itemradix_tu *
mom_make_name_radix_len (const char *str, int len)
{
  if (!str || str == MOM_EMPTY_SLOT || len == 0)
    return NULL;
  if (len < 0)
    return mom_make_name_radix (str);
  if (str[len] == (char) 0 && (int) strlen (str) == len)
    return mom_make_name_radix (str);
  char copybuf[64];
  memset (copybuf, 0, sizeof (copybuf));
  if (len < (int) sizeof (copybuf) - 1)
    {
      strncpy (copybuf, str, len);
      return mom_make_name_radix (copybuf);
    }
  else
    {
      char *cbuf = (char *) mom_gc_alloc_scalar (len + 1);
      strncpy (cbuf, str, len);
      return mom_make_name_radix (cbuf);
    }
}       /* end of mom_make_name_radix_len */

struct mom_item_st *mom_find_item_from_radix_id (const struct
    mom_itemradix_tu *radix,
    uint16_t hid, uint64_t loid);

static inline struct mom_item_st *
mom_find_item_from_str_id (const char
                           *str, int len, uint16_t hid, uint64_t loid)
{
  const struct mom_itemradix_tu *tu = mom_find_name_radix_len (str, len);
  if (tu)
    return mom_find_item_from_radix_id (tu, hid, loid);
  return NULL;
}


static inline const struct mom_hashedvalue_st *
mom_unsync_item_get_phys_attr (const struct mom_item_st *itm,
                               const struct mom_item_st *itmat)
{
  if (mom_itype (itm) != MOMITY_ITEM)
    return NULL;
  if (mom_itype (itmat) != MOMITY_ITEM)
    return NULL;
  const struct mom_assovaldata_st *attrs =
  mom_assovaldata_dyncast (itm->itm_pattr);
  if (!attrs)
    return NULL;
  return mom_assovaldata_get (attrs, itmat);
}       /* end of mom_unsync_item_get_phys_attr */

static inline struct mom_item_st *
mom_unsync_item_descr (const struct mom_item_st *itm)
{
  return
    mom_dyncast_item (mom_unsync_item_get_phys_attr
                      (itm, MOM_PREDEFITM (descr)));
}

void
mom_unsync_item_output_payload (FILE *fout, const struct mom_item_st *itm);

static inline const struct mom_boxset_st *
mom_unsync_item_phys_set_attrs (const struct mom_item_st *itm)
{
  if (mom_itype (itm) != MOMITY_ITEM)
    return NULL;
  const struct mom_assovaldata_st *attrs =
  mom_assovaldata_dyncast (itm->itm_pattr);
  if (!attrs)
    return NULL;
  return mom_assovaldata_set_attrs (attrs);
}       /* end of mom_unsync_item_phys_set_attrs */


// put a physical attribute inside an item
void mom_unsync_item_put_phys_attr (struct mom_item_st *itm,
                                    const struct mom_item_st *itmat,
                                    const void *data);

// remove a physical attribute from an item
void mom_unsync_item_remove_phys_attr (struct mom_item_st *itm,
                                       const struct mom_item_st *itmat);

// clear the payload in an item, return true iff cleared
bool mom_unsync_item_clear_payload (struct mom_item_st *itm);

///
void mom_initialize_items (void);

static inline void
mom_item_lock_at (struct mom_item_st *itm, const char *fil, int lin)
{
  extern void mom_debug_item_lock_at (struct mom_item_st *itm,
                                      const char *fil, int lin);
  if (MOM_UNLIKELY (mom_itype (itm) != MOMITY_ITEM))
    MOM_FATAPRINTF_AT (fil, lin, "bad itm@%p to lock", itm);
  if (MOM_UNLIKELY (MOM_IS_DEBUGGING (mutex)))
    mom_debug_item_lock_at (itm, fil, lin);
  pthread_mutex_lock (&itm->itm_mtx);
}

#define mom_item_lock(Itm) mom_item_lock_at((Itm),__FILE__,__LINE__)

static inline void
mom_item_unlock_at (struct mom_item_st *itm, const char *fil, int lin)
{
  extern void mom_debug_item_unlock_at (struct mom_item_st *itm,
                                        const char *fil, int lin);
  if (MOM_UNLIKELY (mom_itype (itm) != MOMITY_ITEM))
    MOM_FATAPRINTF_AT (fil, lin, "bad itm@%p to unlock", itm);
  if (MOM_UNLIKELY (MOM_IS_DEBUGGING (mutex)))
    mom_debug_item_unlock_at (itm, fil, lin);
  pthread_mutex_unlock (&itm->itm_mtx);
}

#define mom_item_unlock(Itm) mom_item_unlock_at((Itm),__FILE__,__LINE__)

struct mom_item_st *mom_make_item_from_radix_id (const struct
    mom_itemradix_tu *radix,
    uint16_t hid, uint64_t loid);


static inline struct mom_item_st *
mom_make_item_from_str_id (const char
                           *str, int len, uint16_t hid, uint64_t loid)
{
  const struct mom_itemradix_tu *tu = mom_find_name_radix_len (str, len);
  if (tu)
    return mom_make_item_from_radix_id (tu, hid, loid);
  return NULL;
}

static inline struct mom_item_st *
mom_make_item_from_radix (const struct mom_itemradix_tu *radix)
{
  return mom_make_item_from_radix_id (radix, 0, 0);
}

struct mom_item_st *mom_clone_item_from_radix (const struct mom_itemradix_tu
    *radix);

static inline struct mom_item_st *
mom_clone_item (const struct mom_item_st *itm)
{
  if (mom_itype (itm) == MOMITY_ITEM)
    return mom_clone_item_from_radix (itm->itm_radix);
  return NULL;
}

#define MOM_HI_LO_SUFFIX_LEN 16
#ifdef __cplusplus
#define  MOM_HI_LO_SUFFIX_STATIC_DIM
#else
#define MOM_HI_LO_SUFFIX_STATIC_DIM static MOM_HI_LO_SUFFIX_LEN
#endif
// convert a hi+lo pair to a suffix and return it
const char *mom_hi_lo_suffix (char buf[MOM_HI_LO_SUFFIX_STATIC_DIM],
                              uint16_t hi, uint64_t lo);

// convert a suffix to a hi & lo pair, or return false
bool mom_suffix_to_hi_lo (const char *buf, uint16_t *phi, uint64_t *plo);

static inline const char *
mom_item_hi_lo_suffix (char buf[MOM_HI_LO_SUFFIX_STATIC_DIM],
                       const struct mom_item_st *itm)
{
  memset (buf, 0, MOM_HI_LO_SUFFIX_LEN);
  if (itm && (itm->itm_hid || itm->itm_lid))
    mom_hi_lo_suffix (buf, itm->itm_hid, itm->itm_lid);
  return buf;
}

static inline const char *
mom_item_radix_str (const struct mom_item_st *itm)
{
  if (itm)
    return itm->itm_radix->itrad_string.boxs_cstr;
  else
    return NULL;
}

struct mom_item_st *mom_find_item_from_string (const char *str,
    const char **pend);

static inline struct mom_item_st *
mom_find_item_by_string (const char *str)
{
  return mom_find_item_from_string (str, NULL);
}

/// Return the set of items of some given prefix str.
// if the prefix is invalid, return NULL.
// if the prefix has no __, we give a set of non-suffixed items
// otherwise a set of suffixed items
const struct mom_boxset_st *mom_set_items_prefixed (const char *str,
    int slen);


struct mom_item_st *mom_make_item_from_string (const char *str,
    const char **pend);

static inline struct mom_item_st *
mom_make_item_by_string (const char *str)
{
  return mom_make_item_from_string (str, NULL);
}

static inline bool
mom_set_contains (const struct mom_boxset_st *bs,
                  const struct mom_item_st *const itm)
{
  if (mom_itype (bs) != MOMITY_SET)
    return false;
  if (mom_itype (itm) != MOMITY_ITEM)
    return false;
  unsigned siz = mom_raw_size (bs);
  int lo = 0, hi = (int) siz - 1, md = 0;
  while (lo + 5 < hi)
    {
      md = (lo + hi) / 2;
      struct mom_item_st *curitm = (struct mom_item_st *) bs->seqitem[md];
      assert (curitm);
      if ((struct mom_item_st *) itm == curitm)
        return true;
      int c = mom_item_cmp (itm, curitm);
      if (c < 0)
        hi = md;
      else
        lo = md;
    }
  for (md = lo; md < hi; md++)
    {
      md = (lo + hi) / 2;
      struct mom_item_st *curitm = (struct mom_item_st *) bs->seqitem[md];
      assert (curitm);
      if ((struct mom_item_st *) itm == curitm)
        return true;
    }
  return false;
}


enum mom_statetype_en
{
  MOMSTA_EMPTY, MOMSTA_MARK, MOMSTA_INT, MOMSTA_DBL, MOMSTA_STRING,
  MOMSTA_VAL
};

struct mom_statelem_st
{
  enum mom_statetype_en st_type;
  union
  {
    void *st_nptr;    /* NULL for MOMSTA_EMPTY */
    int st_mark;    /* when MOMSTA_MARK */
    momint_t st_int;    /* when MOMSTA_INT */
    double st_dbl;    /* when MOMSTA_DBL */
    const char *st_str;   /* GC-strduped string for when MOMSTA_STRING */
    struct mom_hashedvalue_st *st_val;  /* when MOMSTA_VAL */
  };
};

void mom_fprint_ldstate (FILE *f, const struct mom_statelem_st se);
const char *mom_ldstate_cstring (const struct mom_statelem_st se);

static inline enum mom_statetype_en
mom_ldstate_type (const struct mom_statelem_st se)
{
  return se.st_type;
};

static inline struct mom_statelem_st
mom_ldstate_empty ()
{
  struct mom_statelem_st el;
  el.st_type = MOMSTA_EMPTY;
  el.st_nptr = NULL;
  return el;
}

static inline int
mom_ldstate_mark (const struct mom_statelem_st se)
{

  return (se.st_type == MOMSTA_MARK) ? se.st_mark : (-1);
};

static inline struct mom_statelem_st
mom_ldstate_make_mark (int m)
{
  struct mom_statelem_st el;
  el.st_type = MOMSTA_MARK;
  el.st_mark = m;
  return el;
}


static inline momint_t
mom_ldstate_int_def (const struct mom_statelem_st se, momint_t def)
{
  return (se.st_type == MOMSTA_INT) ? se.st_int : def;
};

static inline struct mom_statelem_st
mom_ldstate_make_int (momint_t i)
{
  struct mom_statelem_st el;
  el.st_type = MOMSTA_INT;
  el.st_int = i;
  return el;
}



static inline double
mom_ldstate_dbl (const struct mom_statelem_st se)
{
  return (se.st_type == MOMSTA_DBL) ? se.st_int : NAN;
};

static inline struct mom_statelem_st
mom_ldstate_make_dbl (double x)
{
  struct mom_statelem_st el;
  el.st_type = MOMSTA_DBL;
  el.st_dbl = x;
  return el;
}


static inline const char *
mom_ldstate_str (const struct mom_statelem_st se)
{
  return (se.st_type == MOMSTA_STRING) ? se.st_str : NULL;
};

static inline struct mom_statelem_st
mom_ldstate_make_str (const char *s)
{
  struct mom_statelem_st el;
  el.st_type = MOMSTA_STRING;
  el.st_str = s;
  return el;
}


static inline const struct mom_hashedvalue_st *
mom_ldstate_val (const struct mom_statelem_st se)
{
  return (se.st_type == MOMSTA_VAL) ? se.st_val : NULL;
};

static inline struct mom_statelem_st
mom_ldstate_make_val (const struct mom_hashedvalue_st *v)
{
  struct mom_statelem_st el;
  el.st_type = MOMSTA_VAL;
  el.st_val = (struct mom_hashedvalue_st *) v;
  return el;
}


static inline struct mom_item_st *
mom_ldstate_dynitem (const struct mom_statelem_st se)
{
  return (struct mom_item_st *) mom_dyncast_item (mom_ldstate_val (se));
};

static inline struct mom_statelem_st
mom_ldstate_make_item (const struct mom_item_st *itm)
{
  struct mom_statelem_st el;
  el.st_type = MOMSTA_VAL;
  el.st_val = (struct mom_hashedvalue_st *) itm;
  return el;
}


static inline const void *
mom_ldstate_dynboxint (const struct mom_statelem_st se)
{
  return mom_dyncast_int (mom_ldstate_val (se));
};

static inline struct mom_statelem_st
mom_ldstate_make_boxint (const struct mom_boxint_st *bi)
{
  struct mom_statelem_st el;
  el.st_type = MOMSTA_VAL;
  el.st_val = (struct mom_hashedvalue_st *) bi;
  return el;
}

static inline const struct mom_boxstring_st *
mom_ldstate_dynboxstring (const struct mom_statelem_st se)
{
  return mom_dyncast_boxstring (mom_ldstate_val (se));
};

static inline struct mom_statelem_st
mom_ldstate_make_boxstring (const struct mom_boxstring_st *bs)
{
  struct mom_statelem_st el;
  el.st_type = MOMSTA_VAL;
  el.st_val = (struct mom_hashedvalue_st *) bs;
  return el;
}


static inline const struct mom_tuple_st *
mom_ldstate_dyntuple (const struct mom_statelem_st se)
{
  return (const struct mom_tuple_st *)
         mom_dyncast_tuple (mom_ldstate_val (se));
};

static inline struct mom_statelem_st
mom_ldstate_make_tuple (const struct mom_boxtuple_st *tu)
{
  struct mom_statelem_st el;
  el.st_type = MOMSTA_VAL;
  el.st_val = (struct mom_hashedvalue_st *) tu;
  return el;
}


static inline const struct mom_set_st *
mom_ldstate_dynset (const struct mom_statelem_st se)
{
  return (const struct mom_set_st *) mom_dyncast_set (mom_ldstate_val (se));
};

static inline struct mom_statelem_st
mom_ldstate_make_set (const struct mom_boxset_st *se)
{
  struct mom_statelem_st el;
  el.st_type = MOMSTA_VAL;
  el.st_val = (struct mom_hashedvalue_st *) se;
  return el;
}



static inline const struct mom_boxnode_st *
mom_ldstate_dynnode (const struct mom_statelem_st se)
{
  return (const struct mom_boxnode_st *)
         mom_dyncast_node (mom_ldstate_val (se));
};


static inline struct mom_statelem_st
mom_ldstate_make_node (const struct mom_boxnode_st *nd)
{
  struct mom_statelem_st el;
  el.st_type = MOMSTA_VAL;
  el.st_val = (struct mom_hashedvalue_st *) nd;
  return el;
}


#define MOM_LOADER_MAGIC 0x1f3fd30f /*524276495 */


// the mom_raw_size of a loader is the allocatd size of ld_stackarr
/// for MOMITY_LOADER
#define MOM_LOADER_FIELDS     \
  MOM_ANYVALUE_FIELDS;        \
  unsigned ld_stacktop;       \
  int ld_prevmark;                              \
  struct mom_statelem_st *ld_stackarr;    \
  struct mom_hashset_st *ld_hsetitems;    \
  /* ld_magic is always MOM_LOADER_MAGIC */ \
  unsigned ld_magic;        \
  FILE *ld_file;        \
  const char *ld_path;        \
  long ld_kindcount[MOMITY__LAST]

struct mom_loader_st
{
  MOM_LOADER_FIELDS;
};

typedef void mom_loader_caret_sig_t (struct mom_item_st *itm,
                                     struct mom_loader_st *ld);
#define MOM_LOADER_CARET_PREFIX MOM_FUNC_PREFIX "ldc_"

typedef void mom_loader_paren_sig_t (struct mom_item_st *itm,
                                     struct mom_loader_st *ld,
                                     struct mom_statelem_st *elemarr,
                                     unsigned elemsize);
#define MOM_LOADER_PAREN_PREFIX MOM_FUNC_PREFIX "ldp_"

static inline struct mom_statelem_st
mom_loader_top (struct mom_loader_st *ld, unsigned topoff)
{
  if (mom_itype (ld) != MOMITY_LOADER)
    return mom_ldstate_empty ();
  assert (ld->ld_magic == MOM_LOADER_MAGIC);
  assert (ld->ld_stacktop <= mom_raw_size (ld));
  if (topoff >= ld->ld_stacktop)
    return mom_ldstate_empty ();
  return ld->ld_stackarr[ld->ld_stacktop - topoff - 1];
}

#ifdef NDEBUG
void mom_loader_push (struct mom_loader_st *ld,
                      const struct mom_statelem_st el);

// return index of previous mark
int mom_loader_push_mark (struct mom_loader_st *ld);

void mom_loader_pop (struct mom_loader_st *ld, unsigned nb);
#else // no NDEBUG
void mom_loader_push_at (struct mom_loader_st *ld,
                         const struct mom_statelem_st el,
                         const char *fil, int lineno);
#define mom_loader_push(Ld,El)        \
  mom_loader_push_at((Ld),(El),__FILE__,__LINE__)

// return index of previous mark
int mom_loader_push_mark_at (struct mom_loader_st *ld, const char *fil,
                             int lineno);
#define mom_loader_push_mark(Ld) mom_loader_push_mark_at((Ld),__FILE__,__LINE__)

void mom_loader_pop_at (struct mom_loader_st *ld, unsigned nb,
                        const char *fil, int lineno);
#define mom_loader_pop(Ld,Nb) mom_loader_pop_at((Ld),(Nb),__FILE__,__LINE__)
#endif

////////////////
#define MOM_NB_QUELEM 7
struct mom_quelem_st
{
  struct mom_quelem_st *qu_next;
  struct mom_hashedvalue_st *qu_elems[MOM_NB_QUELEM];
};

#define MOM_QUEUE_FIELDS      \
  MOM_ANYVALUE_FIELDS;        \
  struct mom_quelem_st* qu_first;   \
  struct mom_quelem_st* qu_last

struct mom_queue_st
{
  MOM_QUEUE_FIELDS;
};

void mom_queue_prepend (struct mom_queue_st *qu, const void *data);
void mom_queue_append (struct mom_queue_st *qu, const void *data);
void mom_queue_pop_front (struct mom_queue_st *qu);

static inline struct mom_queue_st *
mom_dyncast_queue (const void *p)
{
  if (mom_itype (p) == MOMITY_QUEUE)
    return (struct mom_queue_st *) p;
  return NULL;
}

static inline bool
mom_queue_nonempty (const struct mom_queue_st *qu)
{
  if (mom_itype (qu) != MOMITY_QUEUE)
    return FALSE;
  return qu->qu_first != NULL;
}

static inline const void *
mom_queue_front (const struct mom_queue_st *qu)
{
  if (mom_itype (qu) != MOMITY_QUEUE)
    return NULL;
  struct mom_quelem_st *qfirst = qu->qu_first;
  if (!qfirst)
    return NULL;
  for (unsigned ix = 0; ix < MOM_NB_QUELEM; ix++)
    if (qfirst->qu_elems[ix])
      return qfirst->qu_elems[ix];
  MOM_FATAPRINTF ("corrupted queue @%p", qu);
}

static inline const void *
mom_queue_back (const struct mom_queue_st *qu)
{
  if (mom_itype (qu) != MOMITY_QUEUE)
    return NULL;
  struct mom_quelem_st *qlast = qu->qu_last;
  if (!qlast)
    return NULL;
  for (int ix = MOM_NB_QUELEM - 1; ix >= 0; ix--)
    if (qlast->qu_elems[ix])
      return qlast->qu_elems[ix];
  MOM_FATAPRINTF ("corrupted queue @%p", qu);
}

const struct mom_boxnode_st *mom_queue_node (const struct mom_queue_st *qu,
    const struct mom_item_st
    *connitm);

static inline struct mom_queue_st *
mom_queue_make (void)
{

  struct mom_queue_st *qu =
  (struct mom_queue_st *) mom_gc_alloc (sizeof (struct mom_queue_st));
  qu->va_itype = MOMITY_QUEUE;
  return qu;
}       /* end mom_queue_make */

/// initialize a local queue on the call stack
static inline void
mom_queue_init (struct mom_queue_st *qu)
{
  memset (qu, 0, sizeof (struct mom_queue_st));
  qu->va_itype = MOMITY_QUEUE;
}

/// for MOMITY_JSON payload
#define MOM_JSON_FIELDS       \
  MOM_ANYVALUE_FIELDS;        \
  json_t *json

struct mom_json_st
{
  MOM_JSON_FIELDS;
};

/// for MOMITY_WEBEXCH payload

enum mom_webmethod_en
{
  MOMWEBM_NONE,
  MOMWEBM_HEAD,
  MOMWEBM_GET,
  MOMWEBM_POST
};
char *mom_webmethod_name (unsigned);

#define MOM_WEBEXCH_FIELDS      \
  MOM_ANYVALUE_FIELDS;        \
  enum mom_webmethod_en webx_meth;              \
  double webx_time;       \
  long webx_count;        \
  const struct mom_hashedvalue_st*webx_key; \
  const struct mom_boxnode_st*webx_clos;  \
  const struct mom_boxstring_st*webx_restpath;  \
  onion_request* webx_requ;     \
  onion_response* webx_resp;      \
  struct mom_item_st*webx_sessitm;    \
  char webx_mimetype[48];     \
  volatile mom_atomic_int_t webx_code;    \
  char* webx_outbuf;        \
  size_t webx_outsiz;       \
  FILE* webx_outfil;        \
  pthread_cond_t webx_donecond;     \
  long webx__spare

struct mom_webexch_st
{
  MOM_WEBEXCH_FIELDS;
};

static inline struct mom_webexch_st *
mom_item_unsync_webexch (struct mom_item_st *itm)
{
  struct mom_webexch_st *wex = NULL;
  if (mom_itype (itm) == MOMITY_ITEM
      && itm->itm_paylkind == MOM_PREDEFITM (web_exchange)
      && (wex = (struct mom_webexch_st *) itm->itm_payldata)
      && mom_itype (wex) == MOMITY_WEBEXCH)
    return wex;
  return NULL;
}


#define MOM_WEXCH_PRINTF_AT(Lin,Wex,...) do {   \
    struct mom_webexch_st*wex_##Lin = (Wex);    \
    if (mom_itype(wex_##Lin) == MOMITY_WEBEXCH    \
  && wex_##Lin->webx_outfil)      \
      fprintf(wex_##Lin->webx_outfil, __VA_ARGS__); \
  }while(0)
// MOM_WEXCH_PRINTF should be used with the owning webexchange item locked
#define MOM_WEXCH_PRINTF(Wex,...) MOM_WEXCH_PRINTF_AT(__LINE__,(Wex),__VA_ARGS__)

// mom_wexch_write should be used with the owning webexchange item locked
static inline void
mom_wexch_write (struct mom_webexch_st *wex, const char *buf, size_t size)
{
  if (mom_itype (wex) == MOMITY_WEBEXCH && wex->webx_outfil)
    fwrite (buf, size, 1, wex->webx_outfil);
}

static inline void
mom_wexch_puts (struct mom_webexch_st *wex, const char *buf)
{
  if (mom_itype (wex) == MOMITY_WEBEXCH && wex->webx_outfil)
    fputs (buf, wex->webx_outfil);
}

static inline void
mom_wexch_flush (struct mom_webexch_st *wex)
{
  if (mom_itype (wex) == MOMITY_WEBEXCH && wex->webx_outfil)
    fflush (wex->webx_outfil);
}


// mom_wexch_reply should be used with the owning wexitm locked
void
mom_unsync_wexch_reply (struct mom_item_st *wexitm,
                        struct mom_webexch_st *wex, int httpcode,
                        const char *mimetype);

#define MOM_HTTPCODE_WEBSOCKET 999

void mom_webexch_payload_cleanup (struct mom_item_st *itm,
                                  struct mom_webexch_st *payl);


/// for signature_webhandler
typedef void mom_webhandler_sig_t (struct mom_item_st *wexitm,
                                   struct mom_webexch_st *wexch,
                                   const struct mom_boxnode_st *wexclos);



/// for MOMITY_WEBSESSION payload
#define MOM_WEBSESSION_FIELDS     \
  MOM_ANYVALUE_FIELDS;        \
  uint32_t wbss_rand1, wbss_rand2;              \
  time_t wbss_obstime;        \
  onion_websocket* wbss_websock;    \
  const struct mom_boxnode_st*wbss_wsnod;       \
  char* wbss_inbuf;       \
  unsigned wbss_insiz;        \
  unsigned wbss_inoff;        \
  long wbss__spare

struct mom_websession_st
{
  MOM_WEBSESSION_FIELDS;
};


void mom_unsync_websocket_printf (struct mom_item_st *itm,
                                  const char *fmt,
                                  ...)
__attribute__ ((format (printf, 2, 3)));


void mom_unsync_websocket_puts (struct mom_item_st *itm, const char *str);

void mom_unsync_websocket_send_json (struct mom_item_st *itm,
                                     const json_t *js);

void mom_websession_payload_cleanup (struct mom_item_st *itm,
                                     struct mom_websession_st *payl);

////////////////
enum mom_dumpstate_en
{ MOMDUMP_NONE, MOMDUMP_SCAN, MOMDUMP_EMIT };

#define MOM_DUMPER_FIELDS       \
  MOM_ANYVALUE_FIELDS;          \
  enum mom_dumpstate_en du_state;     \
  const struct mom_boxset_st*du_predefset;    \
  struct mom_hashset_st*du_itemset;     \
  const struct mom_boxstring_st*du_predefhtmpath; \
  const struct mom_boxstring_st*du_globaltmpath;  \
  struct mom_queue_st*du_itemque;     \
  FILE*du_emitfile

extern bool mom_skip_dump_hooks;
extern bool mom_dont_make_after_dump;

struct mom_dumper_st
{
  MOM_DUMPER_FIELDS;
};

static inline bool
mom_dumpable_item (const struct mom_item_st *itm)
{
  return itm && itm != MOM_EMPTY_SLOT
         && itm->va_itype == MOMITY_ITEM
         && mom_item_space ((struct mom_item_st *)itm) > MOMSPA_NONE;
}

static inline bool
mom_dumped_item (struct mom_dumper_st *du, const struct mom_item_st *itm)
{
  assert (du && du->va_itype == MOMITY_DUMPER);
  if (!itm || itm == MOM_EMPTY_SLOT || itm->va_itype != MOMITY_ITEM)
    return false;
  return mom_hashset_contains (du->du_itemset, itm);
}


static inline bool
mom_dumped_value (struct mom_dumper_st *du,
                  const struct mom_hashedvalue_st *val)
{
  assert (du && du->va_itype == MOMITY_DUMPER);
  if (!val || val == MOM_EMPTY_SLOT)
    return true;
  if (val->va_itype == MOMITY_ITEM)
    return mom_dumped_item (du, ((const struct mom_item_st *) val));
  else if (val->va_itype == MOMITY_NODE)
    return mom_dumped_item (du,
                            ((const struct mom_boxnode_st *)
                             val)->nod_connitm);
  return true;
}

void mom_dumpscan_item (struct mom_dumper_st *du,
                        const struct mom_item_st *itm);
void mom_dumpscan_value (struct mom_dumper_st *du,
                         const struct mom_hashedvalue_st *val);

void
mom_dumpscan_queue (struct mom_dumper_st *du, const struct mom_queue_st *qu);

void
mom_dumpscan_assovaldata (struct mom_dumper_st *du,
                          struct mom_assovaldata_st *asso);

void
mom_dumpscan_vectvaldata (struct mom_dumper_st *du,
                          struct mom_vectvaldata_st *vec);

void
mom_dumpscan_hashset (struct mom_dumper_st *du, struct mom_hashset_st *hset);

void
mom_dumpscan_hashmap (struct mom_dumper_st *du, struct mom_hashmap_st *hmap);

void
mom_dumpscan_hashassoc (struct mom_dumper_st *du,
                        struct mom_hashassoc_st *hass);

void
mom_dumpscan_tasklet (struct mom_dumper_st *du, struct mom_tasklet_st *tklet);
/////

void mom_dumpemit_refitem (struct mom_dumper_st *du,
                           const struct mom_item_st *itm);

void
mom_dumpemit_item_content (struct mom_dumper_st *du,
                           const struct mom_item_st *itm);

void mom_dumpemit_value (struct mom_dumper_st *du,
                         const struct mom_hashedvalue_st *val);

void
mom_dumpemit_queue (struct mom_dumper_st *du, const struct mom_queue_st *qu);

void
mom_dumpemit_assovaldata (struct mom_dumper_st *du,
                          struct mom_assovaldata_st *asso);

void
mom_dumpemit_vectvaldata (struct mom_dumper_st *du,
                          struct mom_vectvaldata_st *vec);

void
mom_dumpemit_hashassoc_payload (struct mom_dumper_st *du,
                                struct mom_hashassoc_st *hass);

void
mom_dumpemit_filebuffer_payload (struct mom_dumper_st *du,
                                 struct mom_filebuffer_st *fb);

void mom_dumpemit_hashmap_payload (struct mom_dumper_st *du,
                                   struct mom_hashmap_st *hmap);

void mom_dumpemit_tasklet_payload (struct mom_dumper_st *du,
                                   struct mom_tasklet_st *tsklet);


void mom_load_state (const char *statepath);
void mom_dump_state (void);
const char *mom_value_cstring (const void *val);

// output on stdout, with a message
void mom_print_value(const char*msg, const void*val);
// if val is an item print its content, otherwise print the value
void mom_print_content(const char*msg, const void*val);

void
mom_output_value (FILE *f, long *plastnl, int depth, const momvalue_t val);

void
mom_output_item_content (FILE *f, long *plastnl, struct mom_item_st *itm);

const char *mom_item_content_cstring (struct mom_item_st *itm);

#define MOM_LOAD_WEBDIR "webroot"
#define MOM_MAX_WEBDIR 8
extern const char *mom_webdir[MOM_MAX_WEBDIR];

// we dont accept very long paths, notably URLs
#define MOM_PATH_MAX 384

void mom_start_web (const char *webservice);
void mom_stop_web (void);

extern volatile mom_atomic_bool_t mom_should_run;

static inline void
mom_stop (void)
{
  atomic_store (&mom_should_run, false);
}

void mom_stop_and_dump (void);

// for signature_displayer
typedef void mom_displayer_sig_t (const struct mom_boxnode_st *closnod,
                                  struct mom_filebuffer_st *fb,
                                  struct mom_item_st *wexitm,
                                  struct mom_item_st *thistatitm,
                                  const void *pval, int depth);


// for signature_func_item_to_void
typedef void mom_func_item_to_void_sig_t (const struct mom_boxnode_st *closnod,
    struct mom_item_st*itm0);
////////////////
/**** for MOMITY_FILE & MOMITY_FILEBUFFER *****/
#define MOM_FILE_FIELDS       \
  MOM_ANYVALUE_FIELDS;        \
  mom_atomic_fileptr_t mom_filp;    \
  long mom_flastnloff;        \
  int mom_findent

//// file payload for MOMITY_FILE
struct mom_file_st
{
  MOM_FILE_FIELDS;
};


#define MOM_FILEBUFFER_FIELDS         \
  MOM_FILE_FIELDS; /* the mom_file is from open_memstream */  \
  char*mom_filbuf;            \
  size_t mom_filbufsiz

//// filebuffer payload for MOMITY_FILEBUFFER
struct mom_filebuffer_st
{
  MOM_FILEBUFFER_FIELDS;
};

static inline FILE *
mom_file (void *mfil)
{
  if (!mfil || mfil == MOM_EMPTY_SLOT)
    return NULL;
  unsigned ty = mom_itype (mfil);
  if (ty == MOMITY_FILEBUFFER || ty == MOMITY_FILE)
    {
      struct mom_file_st *mf = (struct mom_file_st *) mfil;
      FILE *f = atomic_load (&mf->mom_filp);
      return f;
    }
  else if (ty == MOMITY_WEBEXCH)
    {
      struct mom_webexch_st *we = (struct mom_webexch_st *) mfil;
      return we->webx_outfil;
    }
  return NULL;
}

void mom_file_indent (void *mfil);
void mom_file_outdent (void *mfil);
void mom_file_newline (void *mfil);

static inline int
mom_file_indentation (const void *mfil)
{
  if (!mfil || mfil == MOM_EMPTY_SLOT)
    return 0;
  struct mom_file_st *mf = (struct mom_file_st *) mfil;
  if (mf->va_itype != MOMITY_FILEBUFFER && mf->va_itype != MOMITY_FILE)
    return 0;
  return mf->mom_findent;
}

static inline long
mom_file_last_line_width (const void *mfil)
{
  struct mom_file_st *mf = (struct mom_file_st *) mfil;
  if (mf->va_itype != MOMITY_FILEBUFFER && mf->va_itype != MOMITY_FILE)
    return 0;
  FILE *f = atomic_load (&mf->mom_filp);
  if (!f)
    return 0;
  return ftell (f) - mf->mom_flastnloff;
}       /* end of mom_file_last_line_width */

void mom_file_set_indentation (void *mfil, int ind);

void mom_file_close (void *mfil);

// put a string, if it is terminated by a newline, with indentation
static inline void
mom_file_puts (void *mfil, const char *str)
{
  if (!mfil || mfil == MOM_EMPTY_SLOT || !str || str == MOM_EMPTY_SLOT
      || !str[0])
    return;
  struct mom_file_st *mf = (struct mom_file_st *) mfil;
  if (mf->va_itype != MOMITY_FILEBUFFER && mf->va_itype != MOMITY_FILE)
    return;
  FILE *f = atomic_load (&mf->mom_filp);
  if (!f)
    return;
  size_t slen = strlen (str);
  fputs (str, f);
  if (slen > 0 && str[slen - 1] == '\n' && mf->mom_findent > 0)
    {
      mf->mom_flastnloff = ftell (f);
      for (int i = mf->mom_findent % 16; i >= 0; i--)
        fputc (' ', f);
    }
}

/// a terminating newline in the format is interpreted with
/// indentation; of course, internal newlines and the newlines in data
/// string are output verbatim
void mom_file_printf (void *mfil, const char *fmt, ...)
__attribute__ ((format (printf, 2, 3)));

// allocate a GC-finalized file buffer
struct mom_filebuffer_st *mom_make_filebuffer (void);
// allocate a GC-finalized file
struct mom_file_st *mom_make_file (FILE *);

// give its boxedstring content, closing it iff close is set
const struct mom_boxstring_st *mom_filebuffer_boxstring (struct
    mom_filebuffer_st
    *mf, bool close);
// give its GC_strdup content, closing it iff close is set
const char *mom_filebuffer_strdup (struct mom_filebuffer_st *mf, bool close);
enum
{ MOM_FILEBUFFER_KEEPOPEN, MOM_FILEBUFFER_CLOSE };

// put the content of a filebuffer inside some output file
void mom_puts_filebuffer (FILE *outf, struct mom_filebuffer_st *fb,
                          bool close);


////////////////
/// for MOMITY_TASKSTEPPER payload, sitting in the code segment
#define MOM_TASKSTEPPER_FIELDS          \
  /* the mom_size is the number of values in the frame */ \
  MOM_ANYVALUE_FIELDS;            \
  /* number of integers and doubles in the frame */   \
  uint16_t tksp_nbint, tksp_nbdbl
struct mom_taskstepper_st
{
  MOM_TASKSTEPPER_FIELDS;
};

void mom_dumpemit_taskstepper (struct mom_dumper_st *du,
                               struct mom_item_st *itm,
                               struct mom_taskstepper_st *tstep);

#define MOM_TASKSTEPPER_PREFIX "momtaskstepper_"

#define MOM_TASKSTEPPER_DEFINE(Itm,NbVal,NbInt,NbDbl) \
  static_assert ((NbVal)>=0 && (NbVal)<UINT16_MAX,  \
                 "too many values " #NbVal    \
     " for taskstepper " #Itm);   \
  static_assert ((NbInt)>=0 && (NbInt)<UINT16_MAX,  \
     "too many integers " #NbInt    \
     " for taskstepper " #Itm);   \
  static_assert ((NbDbl)>=0 && (NbDbl)<UINT16_MAX,  \
     "too many doubles " #NbDbl   \
     " for taskstepper " #Itm);   \
  const struct mom_taskstepper_st     \
  momtaskstepper_##Itm =        \
    { .va_itype= MOMITY_TASKSTEPPER,      \
      .va_hsiz=0, .va_lsiz= (NbVal),      \
      .tksp_nbint=(NbInt), .tksp_nbdbl=(NbDbl) }

static inline const struct mom_taskstepper_st *
mom_dyncast_taskstepper (const void *p)
{
  if (mom_itype (p) == MOMITY_TASKSTEPPER)
    return (const struct mom_taskstepper_st *) p;
  return NULL;
}       /* end of mom_taskstepper_dyncast */


static inline unsigned
mom_taskstepper_nbval (const struct mom_taskstepper_st *tstep)
{
  if (tstep && tstep != MOM_EMPTY_SLOT
      && tstep->va_itype == MOMITY_TASKSTEPPER)
    return mom_raw_size (tstep);
  return 0;
}

static inline unsigned
mom_taskstepper_nbint (const struct mom_taskstepper_st *tstep)
{
  if (tstep && tstep != MOM_EMPTY_SLOT
      && tstep->va_itype == MOMITY_TASKSTEPPER)
    return tstep->tksp_nbint;
  return 0;
}

static inline unsigned
mom_taskstepper_nbdbl (const struct mom_taskstepper_st *tstep)
{
  if (tstep && tstep != MOM_EMPTY_SLOT
      && tstep->va_itype == MOMITY_TASKSTEPPER)
    return tstep->tksp_nbdbl;
  return 0;
}

////////////////
/// for MOMITY_TASKLET payload
/* a tasklet contains mini-call-frames, we have one scalar zone for
   scalar data (integers, doubles) and one "pointer" zone for GC-ed
   data to be Boehm-GC friendly */

struct mom_frameoffsets_st
{
  unsigned fo_ptroff;   /* offset in pointer zone */
  unsigned fo_scaoff;   /* offset in scalar zone */
};

#define MOM_TASKLET_FIELDS            \
  /* the mom_size is the allocated size in        \
     #elements of tlk_froffsets */          \
  MOM_ANYVALUE_FIELDS;              \
  unsigned tkl_scasiz; /* scalar zone size */       \
  unsigned tkl_scatop; /* scalar zone top */        \
  intptr_t *tkl_scalars; /* scalar zone  */       \
  unsigned tkl_ptrsiz; /* pointer zone size */        \
  unsigned tkl_ptrtop; /* pointer zone top */       \
  void **tkl_pointers; /* pointer zone */       \
  struct mom_frameoffsets_st*tkl_froffsets; /* array of offset pairs */ \
  unsigned tkl_frametop;        /* top frame index */     \
  struct mom_item_st*tkl_statitm; /* state item for nanev_thistatitm */ \
  const struct mom_boxnode_st*tkl_excnod  /*node to handle nanoexceptions */


struct mom_tasklet_st
{
  MOM_TASKLET_FIELDS;
};



struct mom_framescalar_st
{
  momint_t tfs_state;
  momint_t tfs_scalars[];
};

struct mom_framepointer_st
{
  const struct mom_boxnode_st *tfp_node;
  void *tfp_pointers[];
};

struct mom_frame_st
{
  struct mom_framescalar_st *fr_sca;
  struct mom_framepointer_st *fr_ptr;
};


struct mom_tasklet_st *mom_unsync_item_initialize_tasklet (struct
    mom_item_st *itm,
    unsigned frasiz,
    unsigned scasiz,
    unsigned ptrsiz);
void mom_tasklet_reserve (struct mom_tasklet_st *tkl, unsigned nbframes,
                          unsigned nbscalars, unsigned nbpointers);

static inline struct mom_tasklet_st *
mom_dyncast_tasklet (const void *p)
{
  if (mom_itype (p) == MOMITY_TASKLET)
    return (struct mom_tasklet_st *) p;
  return NULL;
}       /* mom_dyncast_tasklet */


static inline struct mom_frame_st
mom_tasklet_nth_frame (const struct mom_tasklet_st *tkl, int rk)
{
  if (!tkl || tkl == MOM_EMPTY_SLOT || tkl->va_itype != MOMITY_TASKLET)
    return (struct mom_frame_st)
    {
      NULL, NULL
    };
  unsigned frsiz = mom_raw_size (tkl);
  unsigned frtop = tkl->tkl_frametop;
  assert (frtop <= frsiz);
  if (rk < 0)
    rk += (int) frtop;
  if (rk >= 0 && rk < (int) frtop)
    {
      struct mom_frameoffsets_st fo = tkl->tkl_froffsets[rk];
      assert (fo.fo_ptroff <= tkl->tkl_ptrtop);
      assert (tkl->tkl_ptrtop <= tkl->tkl_ptrsiz);
      assert (fo.fo_scaoff <= tkl->tkl_scatop);
      assert (tkl->tkl_scatop <= tkl->tkl_scasiz);
      return ((struct mom_frame_st)
      {
        .fr_sca =   //
          (struct mom_framescalar_st *) (tkl->tkl_scalars + fo.fo_scaoff),.fr_ptr = //
            (struct mom_framepointer_st *) (tkl->tkl_pointers +
                                            fo.fo_ptroff)
      });
    }
  return (struct mom_frame_st)
  {
    NULL, NULL
  };
}       /* end of mom_tasklet_nth_frame */

static inline struct mom_item_st *
mom_tasklet_statitm (const struct mom_tasklet_st *tklet)
{
  if (!tklet || tklet == MOM_EMPTY_SLOT || tklet->va_itype != MOMITY_TASKLET)
    return NULL;
  return tklet->tkl_statitm;
}       /* end mom_tasklet_statitm */

void mom_tasklet_put_statitm (struct mom_tasklet_st *tklet,
                              struct mom_item_st *itm);

static inline const struct mom_boxnode_st *
mom_tasklet_excnode (const struct mom_tasklet_st *tklet)
{
  if (!tklet || tklet == MOM_EMPTY_SLOT || tklet->va_itype != MOMITY_TASKLET)
    return NULL;
  return tklet->tkl_excnod;
}       /* end mom_tasklet_excnode */

void mom_tasklet_put_excnode (struct mom_tasklet_st *tklet,
                              const struct mom_boxnode_st *nod);

static inline unsigned
mom_tasklet_frame_depth (const struct mom_tasklet_st *tklet)
{
  if (!tklet || tklet == MOM_EMPTY_SLOT || tklet->va_itype != MOMITY_TASKLET)
    return 0;
  return tklet->tkl_frametop;
}       /* end mom_tasklet_frame_depth */

////////////////

void mom_agenda_add_tasklet_front (const struct mom_item_st *tkletitm);
void mom_agenda_add_tasklet_back (const struct mom_item_st *tkletitm);
void mom_agenda_remove_tasklet (const struct mom_item_st *tkletitm);
void mom_agenda_remove_set_tasklets (const struct mom_boxset_st *set);
const struct mom_boxtuple_st *mom_agenda_tuple_tasklets (void);
void mom_agenda_changing (void);
void mom_start_agenda (void);
extern unsigned mom_nbjobs;
// a tasklet item has a node payload, whose connective should have a function of signature_tasklet;
// we run the tasklet function with the tkitm locked
typedef void mom_tasklet_sig_t (struct mom_item_st *tkitm);


//// for signature_closure_1int_to_void
typedef void mom_closure_1int_to_void_sig_t (const struct mom_boxnode_st
    *clonod, momint_t num);
//// for signature_closure_void_to_void
typedef void mom_closure_void_to_void_sig_t (const struct mom_boxnode_st
    *clonod);


#define NANOEVAL_MAGIC_MOM 617373733  /*0x24cc6025 */
struct mom_nanoeval_st
{
  unsigned nanev_magic;   /* always NANOEVAL_MAGIC_MOM */
  jmp_buf nanev_jb;
  bool nanev_transient;   /* true to create transient items */
  long nanev_count;
  long nanev_maxstep;
  struct mom_item_st *nanev_tkitm;
  const void *nanev_fail;
  const void *nanev_expr;
  const char *nanev_errfile;
};
void mom_bind_nanoev (struct mom_nanoeval_st *nev,
                      struct mom_item_st *envitm,
                      const struct mom_item_st *varitm, const void *val);
#define MOM_NANOEVAL_FAILURE(Ne,Expr,Fail) do {     \
    struct mom_nanoeval_st*_ne = (struct mom_nanoeval_st*)(Ne); \
    assert (_ne && _ne->nanev_magic == NANOEVAL_MAGIC_MOM); \
    _ne->nanev_fail = (Fail);         \
    _ne->nanev_errfile = __FILE__;        \
    _ne->nanev_expr = (Expr);         \
    MOM_DEBUGPRINTF           \
      (run,"nanoeval failing %s\n.. expr %s",     \
       mom_value_cstring((void*)(_ne->nanev_fail)),   \
       mom_value_cstring((void*)(_ne->nanev_expr)));    \
    longjmp(_ne->nanev_jb,__LINE__);        \
  } while(0)

#define MOM_NANOTASKSTEP_MAGIC 421788551  /*0x1923fb87 */
#define MOM_TASKSTEP_MAX_DATA 8

struct mom_nanotaskstep_st
{
  unsigned nats_magic;    /* always MOM_NANOTASKSTEP_MAGIC */
  unsigned nats_nblevels; // number of frame levels to push or pop
  struct mom_nanoeval_st nats_nanev;
  double nats_startim;
  struct mom_tasklet_st *nats_tasklet;
  void *nats_ptrs[MOM_TASKSTEP_MAX_DATA];
  momint_t nats_nums[MOM_TASKSTEP_MAX_DATA];
  double nats_dbls[MOM_TASKSTEP_MAX_DATA];
};

const void *mom_nanoeval (struct mom_nanoeval_st *nev,
                          struct mom_item_st *envitm, const void *exprv,
                          int depth);

const void *mom_nanoapply (struct mom_nanoeval_st *nev,
                           struct mom_item_st *envitm,
                           const struct mom_boxnode_st *nodfun,
                           const struct mom_boxnode_st *nodexp,
                           unsigned nbargs, const void **argv, int depth);

/// for signature_nanoeval0
typedef const void *mom_nanoeval0_sig_t (struct mom_nanoeval_st *nev,
    struct mom_item_st *envitm,
    int depth,
    const struct mom_boxnode_st
    *expnod,
    const struct mom_boxnode_st
    *closnod);

/// for signature_nanoeval1
typedef const void *mom_nanoeval1_sig_t (struct mom_nanoeval_st *nev,
    struct mom_item_st *envitm,
    int depth,
    const struct mom_boxnode_st
    *expnod,
    const struct mom_boxnode_st
    *closnod, const void *arg0);

/// for signature_nanoeval2
typedef const void *mom_nanoeval2_sig_t (struct mom_nanoeval_st *nev,
    struct mom_item_st *envitm,
    int depth,
    const struct mom_boxnode_st
    *expnod,
    const struct mom_boxnode_st
    *closnod, const void *arg0,
    const void *arg1);

/// for signature_nanoeval3
typedef const void *mom_nanoeval3_sig_t (struct mom_nanoeval_st *nev,
    struct mom_item_st *envitm,
    int depth,
    const struct mom_boxnode_st
    *expnod,
    const struct mom_boxnode_st
    *closnod, const void *arg0,
    const void *arg1, const void *arg2);

/// for signature_nanoeval4
typedef const void *mom_nanoeval4_sig_t (struct mom_nanoeval_st *nev,
    struct mom_item_st *envitm,
    int depth,
    const struct mom_boxnode_st
    *expnod,
    const struct mom_boxnode_st
    *closnod, const void *arg0,
    const void *arg1, const void *arg2,
    const void *arg3);

/// for signature_nanoevalany
typedef const void *mom_nanoevalany_sig_t (struct mom_nanoeval_st *nev,
    struct mom_item_st *envitm,
    int depth,
    const struct mom_boxnode_st
    *expnod,
    const struct mom_boxnode_st
    *closnod, unsigned nbval,
    const void **valarr);

enum mom_taskstep_en
{
  MOMTKS_NOP,     /* no effect on tasklet stack */
  MOMTKS_POP_ONE,   /* pop one frame, so "return" */
  MOMTKS_REPLACE,   /* replace top frame, à la tail-call */
  MOMTKS_POP_MANY,    /* pop several frames */
  MOMTKS_PUSH_ONE,    /* push one frame, so "call" */
  // perhaps we need a push-slice and a replace-slice
};

static inline uint32_t
mom_tasklet_what (enum mom_taskstep_en ts, unsigned char nbval,
                  unsigned char nbint, unsigned char nbdbl)
{
  assert (nbval + nbint + nbdbl <= 6);
  return (uint32_t) ((nbdbl & 0xf) << 24) | (uint32_t) ((nbint & 0xf) << 16)
         | (uint32_t) ((nbval & 0xf) << 8) | (uint32_t) ts;
}       /* end of mom_tasklet_what */

static inline unsigned char
mom_what_nbdbl (uint32_t w)
{
  return (w & 0xff000000U) >> 24;
}

static inline unsigned char
mom_what_nbint (uint32_t w)
{
  return (w & 0xff0000U) >> 16;
}

static inline unsigned char
mom_what_nbval (uint32_t w)
{
  return (w & 0xff00U) >> 8;
}

static inline enum mom_taskstep_en
mom_what_taskstep (uint32_t w)
{
  return (enum mom_taskstep_en) (w & 0xff);
};

struct mom_result_tasklet_st
{
  uint32_t r_what;
  /* when r_what's taskstep is MOMTKS_REPLACE, r_val is the "tail-called"
     node; when the taskstep is MOMTKS_PUSH_ONE, r_val is the
     "called" node. */
  const void *r_val;
};
// for signature_nanotaskstep
typedef struct mom_result_tasklet_st
mom_nanotaskstep_sig_t (struct mom_nanotaskstep_st *nats,
                        const struct mom_boxnode_st *clos,
                        struct mom_framescalar_st *fscal,
                        struct mom_framepointer_st *fptr);

// for signature_nanotaskexception
typedef void
mom_nanotaskexception_sig_t (const struct mom_boxnode_st *clos,
                             struct mom_item_st *tskitm,
                             const void *failv,
                             const void *exprv,
                             const char *errfil, int errlin);

/// tests could be run after load
typedef void mom_test_sig_t (const char *);

// miniedit related

#define MOM_MINEDJS_MAGIC 506568011 /*0x1e319d4b */
struct mom_minedjs_st
{
  unsigned miejs_magic;   /* always MOM_MINEDJS_MAGIC */
  jmp_buf miejs_jb;
  long miejs_lastnl;
  const char *miejs_errfile;
  const void *miejs_fail;
  const void *miejs_expr;
  struct mom_item_st *miejs_wexitm;
  struct mom_webexch_st *miejs_wexch;
};
#define MOM_MINEDJS_FAILURE(Mj,Expr,Fail) do {        \
    struct mom_minedjs_st*_mj = (Mj);         \
    assert (_mj && _mj->miejs_magic==MOM_MINEDJS_MAGIC);    \
    _mj->miejs_fail = (Fail);           \
    _mj->miejs_expr = (Expr);           \
    _mj->miejs_errfile = __FILE__;          \
    MOM_DEBUGPRINTF(run,"minedjs failing %s\n.. expr %s",   \
        mom_value_cstring((void*) _mj->miejs_fail),   \
        mom_value_cstring((void*) _mj->miejs_expr));  \
    longjmp(_mj->miejs_jb,__LINE__);          \
  } while(0)

void mom_boot_file (const char *path);

// return true on successful emission of C code
bool mom_emit_c_code (struct mom_item_st *itm);
// return true on successful emission of C header code
bool mom_emit_header_code (struct mom_item_st *itm);

// return true on successful emission of JavaScript code
bool mom_emit_javascript_code (struct mom_item_st *itm, FILE *out);

#ifdef __cplusplus
};        // end extern "C"
#endif /*__cplusplus*/

#endif /*MONIMELT_INCLUDED_ */

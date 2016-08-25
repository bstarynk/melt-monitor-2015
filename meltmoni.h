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
#include <glib.h>
#include <gtk/gtk.h>
#if __GLIBC__
#include <execinfo.h>
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
#define thread_local _Thread_local





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
extern const char monimelt_sqlite[];

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

#define MOM_ASSERTPRINTF_AT(Fil,Lin,Cond,Fmt,...)	\
  do {if (MOM_UNLIKELY(!(Cond)))			\
      MOM_FATAPRINTF_AT(Fil,Lin,			\
	"MOM_ASSERT FAILURE (" #Cond "): " Fmt,		\
			##__VA_ARGS__); }while(0)

#define MOM_ASSERTPRINTF_AT_BIS(Fil,Lin,Cond,Fmt,...)	\
  MOM_FATAPRINTF_AT(Fil,Lin,Cond,Fmt,			\
        ##__VA_ARGS__)

#define MOM_ASSERTPRINTF(Cond,Fmt,...)		\
  MOM_ASSERTPRINTF_AT_BIS(__FILE__,__LINE__,Cond,Fmt,	\
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



////////////////////////////////////////////////////////////////


#endif /*MONIMELT_INCLUDED_ */

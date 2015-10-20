// file monimelt.h - common header file to be included everywhere.

/**   Copyright (C)  2015 Basile Starynkevitch, later FSF
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

#include <features.h>           // GNU things
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdatomic.h>
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

#include <gc/gc.h>

// libonion from http://www.coralbits.com/libonion/ &
// https://github.com/davidmoreno/onion
#include <onion/onion.h>
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

#include <glib.h>

// in generated _timestamp.c
extern const char monimelt_timestamp[];
extern const char monimelt_lastgitcommit[];
extern const char monimelt_lastgittag[];
extern const char monimelt_compilercommand[];
extern const char monimelt_compilerflags[];
extern const char monimelt_checksum[];


// increasing array of primes and its size
extern const int64_t mom_primes_tab[];
extern const unsigned mom_primes_num;
/// give a prime number above or below a given n, or else 0
int64_t mom_prime_above (int64_t n);
int64_t mom_prime_below (int64_t n);

// mark unlikely conditions to help optimization
#ifdef __GNUC__
#define MOM_UNLIKELY(P) __builtin_expect((P),0)
#define MOM_LIKELY(P) !__builtin_expect(!(P),0)
#define MOM_UNUSED __attribute__((unused))
#else
#define MOM_UNLIKELY(P) (P)
#define MOM_LIKELY(P) (P)
#define MOM_UNUSED
#endif

#define MOM_EMPTY_SLOT ((void*)-1)

#define MOM_SIZE_MAX (1<<24)

static inline pid_t
mom_gettid (void)
{
  return syscall (SYS_gettid, 0L);
}

/// generate a GPLv3 notice
void mom_output_gplv3_notice (FILE *out, const char *prefix,
                              const char *suffix, const char *filename);

static inline void *
mom_gc_alloc (size_t sz)
{
  void *p = GC_MALLOC (sz);
  if (p)
    memset (p, 0, sz);
  return p;
}

void *mom_gc_calloc (size_t nmemb, size_t size);

static inline void *
mom_gc_alloc_atomic (size_t sz)
{
  void *p = GC_MALLOC_ATOMIC (sz);
  if (p)
    memset (p, 0, sz);
  return p;
}


static inline void *
mom_gc_alloc_uncollectable (size_t sz)
{
  void *p = GC_MALLOC_UNCOLLECTABLE (sz);
  if (p)
    memset (p, 0, sz);
  return p;
}

static inline char *
mom_gc_strdup (const char *s)
{
  return GC_STRDUP (s);
}

void
mom_fataprintf_at (const char *fil, int lin, const char *fmt, ...)
__attribute__ ((format (printf, 3, 4), noreturn));

#define MOM_FATAPRINTF_AT(Fil,Lin,Fmt,...) do {	\
      mom_fataprintf_at (Fil,Lin,Fmt,		\
		    ##__VA_ARGS__);		\
  } while(0)

#define MOM_FATAPRINTF_AT_BIS(Fil,Lin,Fmt,...)	\
  MOM_FATAPRINTF_AT(Fil,Lin,Fmt,		\
		    ##__VA_ARGS__)

#define MOM_FATAPRINTF(Fmt,...)			\
  MOM_FATAPRINTF_AT_BIS(__FILE__,__LINE__,Fmt,	\
			##__VA_ARGS__)

// for debugging:
#define MOM_DEBUG_LIST_OPTIONS(Dbg)		\
  Dbg(item)					\
  Dbg(dump)					\
  Dbg(load)					\
  Dbg(run)					\
  Dbg(gencod)					\
  Dbg(cmd)					\
  Dbg(low)					\
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

#define MOM_DEBUGPRINTF_AT(Fil,Lin,Dbg,Fmt,...) do {	\
    if (MOM_IS_DEBUGGING(Dbg))				\
      mom_debugprintf_at (Fil,Lin,momdbg_##Dbg,Fmt,	\
		    ##__VA_ARGS__);			\
  } while(0)

#define MOM_DEBUGPRINTF_AT_BIS(Fil,Lin,Dbg,Fmt,...)	\
  MOM_DEBUGPRINTF_AT(Fil,Lin,Dbg,Fmt,			\
		    ##__VA_ARGS__)

#define MOM_DEBUGPRINTF(Dbg,Fmt,...)			\
  MOM_DEBUGPRINTF_AT_BIS(__FILE__,__LINE__,Dbg,Fmt,	\
			##__VA_ARGS__)


void
mom_informprintf_at (const char *fil, int lin, const char *fmt, ...)
__attribute__ ((format (printf, 3, 4)));

#define MOM_INFORMPRINTF_AT(Fil,Lin,Fmt,...) do {	\
      mom_informprintf_at (Fil,Lin,Fmt,		\
		    ##__VA_ARGS__);		\
  } while(0)

#define MOM_INFORMPRINTF_AT_BIS(Fil,Lin,Fmt,...)	\
  MOM_INFORMPRINTF_AT(Fil,Lin,Fmt,		\
		    ##__VA_ARGS__)

#define MOM_INFORMPRINTF(Fmt,...)			\
  MOM_INFORMPRINTF_AT_BIS(__FILE__,__LINE__,Fmt,	\
			##__VA_ARGS__)


void
mom_warnprintf_at (const char *fil, int lin, const char *fmt, ...)
__attribute__ ((format (printf, 3, 4)));

#define MOM_WARNPRINTF_AT(Fil,Lin,Fmt,...) do {	\
      mom_warnprintf_at (Fil,Lin,Fmt,		\
		    ##__VA_ARGS__);		\
  } while(0)

#define MOM_WARNPRINTF_AT_BIS(Fil,Lin,Fmt,...)	\
  MOM_WARNPRINTF_AT(Fil,Lin,Fmt,		\
		    ##__VA_ARGS__)

#define MOM_WARNPRINTF(Fmt,...)			\
  MOM_WARNPRINTF_AT_BIS(__FILE__,__LINE__,Fmt,	\
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
  if (count % 4096 == 0)
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

double mom_elapsed_real_time (void);    /* relative to start of program */
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

enum momitype_en
{
  MOMITY_NONE,
  MOMITY_BOXINT,
  MOMITY_BOXDOUBLE,
  MOMITY_BOXSTRING,
  MOMITY_TUPLE,
  MOMITY_SET,
  MOMITY_NODE,
  MOMITY_ITEM,
  MOMITY_ASSOVALDATA,
  MOMITY_VECTVALDATA,
};
struct mom_item_st;


// the common prefix of all values
#define MOM_ANYVALUE_FIELDS			\
  uint8_t va_itype;				\
  uint8_t va_hsiz;				\
  uint16_t va_lsiz
struct mom_anyvalue_st
{                               /// field prefix: va_;
  MOM_ANYVALUE_FIELDS;
};

#define MOM_HASHEDVALUE_FIELDS			\
  MOM_ANYVALUE_FIELDS;				\
  momhash_t hva_hash
struct mom_hashedvalue_st
{
  MOM_HASHEDVALUE_FIELDS;
};

struct mom_boxint_st
{
  MOM_HASHEDVALUE_FIELDS;
  // here field 
  intptr_t boxi_int;
};


static inline const struct mom_boxint_st *
mom_dyncast_boxint (const void *p)
{
  if (p && p != MOM_EMPTY_SLOT
      && ((struct mom_anyvalue_st *) p)->va_itype == MOMITY_BOXINT)
    return (const struct mom_boxint_st *) p;
  return NULL;
}

static inline momhash_t
mom_int_hash (intptr_t i)
{
  momhash_t h = (i * 509) ^ (i % 76519);
  if (!h)
    h = (i & 0xffffff) + ((i >> 25) & 0xffffff) + 11;
  assert (h != 0);
  return h;
}

static inline intptr_t
mom_boxint_val_def (const void *p, intptr_t def)
{
  const struct mom_boxint_st *bi = mom_dyncast_boxint (p);
  if (bi)
    return bi->boxi_int;
  return def;
}

const struct mom_boxint_st *mom_boxint_make (intptr_t i);

struct mom_boxdouble_st
{
  MOM_HASHEDVALUE_FIELDS;
  // here field
  double boxd_dbl;
};

static inline const struct mom_boxdouble_st *
mom_dyncast_boxdouble (const void *p)
{
  if (p && p != MOM_EMPTY_SLOT
      && ((struct mom_anyvalue_st *) p)->va_itype == MOMITY_BOXDOUBLE)
    return (const struct mom_boxdouble_st *) p;
  return NULL;
}



/// sized values have sva_ prefix, with sva_size
#define MOM_SIZEDVALUE_FIELDS			\
  MOM_HASHEDVALUE_FIELDS;			\
  uint32_t sva_size
struct mom_sizedvalue_st
{
  MOM_SIZEDVALUE_FIELDS;
};


struct mom_boxstring_st
{
  MOM_HASHEDVALUE_FIELDS;
  // here
  char cstr[];                  /* actual size sva_size+1 */
};

static inline const struct mom_boxstring_st *
mom_dyncast_boxstring (const void *p)
{
  if (p && p != MOM_EMPTY_SLOT
      && ((const struct mom_anyvalue_st *) p)->va_itype == MOMITY_BOXSTRING)
    return (const struct mom_boxstring_st *) p;
  return NULL;
}

static inline const char *
mom_boxstring_cstr (const void *p)
{
  const struct mom_boxstring_st *str = mom_dyncast_boxstring (p);
  if (str)
    return str->cstr;
  return NULL;
}

const struct mom_boxstring_st *mom_boxstring_make (const char *s);

#define MOM_SEQITEMS_FIELDS			\
  MOM_SIZEDVALUE_FIELDS;			\
  struct mom_item_st* seqitem[] /* actual size sva_size */
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
  if (p && p != MOM_EMPTY_SLOT)
    {
      uint8_t ityp = ((const struct mom_anyvalue_st *) p)->va_itype;
      if (ityp == MOMITY_TUPLE || ityp == MOMITY_SET)
        return (const struct mom_seqitems_st *) p;
    }
  return NULL;
}

static inline const struct mom_item_st **
mom_seqitems_arr (const void *p)
{
  const struct mom_seqitems_st *si = mom_dyncast_seqitems (p);
  if (si)
    return (const struct mom_item_st **) si->seqitem;
  return NULL;
}

static inline const struct mom_item_st *
mom_seqitem_nth (const void *p, int rk)
{
  const struct mom_seqitems_st *si = mom_dyncast_seqitems (p);
  if (!si)
    return NULL;
  unsigned sz = (si->va_hsiz << 16) + si->va_lsiz;
  if (rk < 0)
    rk += sz;
  if (rk >= 0 && rk < (int) sz)
    return si->seqitem[rk];
  return NULL;
}

static inline const struct mom_boxtuple_st *
mom_dyncast_tuple (const void *p)
{
  if (p && p != MOM_EMPTY_SLOT
      && ((const struct mom_anyvalue_st *) p)->va_itype == MOMITY_TUPLE)
    return (const struct mom_boxtuple_st *) p;
  return NULL;
}

static inline const struct mom_boxset_st *
mom_dyncast_set (const void *p)
{
  if (p && p != MOM_EMPTY_SLOT
      && ((const struct mom_anyvalue_st *) p)->va_itype == MOMITY_SET)
    return (const struct mom_boxset_st *) p;
  return NULL;
}

const struct mom_boxtuple_st *mom_boxtuple_make_arr2 (unsigned siz1,
                                                      const struct mom_item_st
                                                      **arr1, unsigned siz2,
                                                      const struct mom_item_st
                                                      **arr2);

const struct mom_boxtuple_st *mom_boxtuple_make_arr (unsigned siz,
                                                     const struct mom_item_st
                                                     **arr);

const struct mom_boxtuple_st *mom_boxtuple_make_va (unsigned siz, ...);

const struct mom_boxtuple_st *mom_boxtuple_make_sentinel_va (struct
                                                             mom_item_st *,
                                                             ...)
  __attribute__ ((sentinel));
#define mom_boxtuple_make_sentinel(...) mom_boxtuple_make_sentinel_va(##__VA_ARGS__, NULL)

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

const struct mom_boxset_st *mom_boxset_make_sentinel_va (struct mom_item_st *,
                                                         ...)
  __attribute__ ((sentinel));
#define mom_boxset_make_sentinel(...) mom_boxset_make_sentinel_va(##__VA_ARGS__, NULL)


struct mom_boxnode_st
{
  MOM_HASHEDVALUE_FIELDS;
  // here prefix nod_
  intptr_t nod_metarank;
  struct mom_item_st *nod_metaitem;
  struct mom_item_st *nod_connitm;
  struct mom_hashedvalue_st *nod_sons[];        /* actual size sva_size */
};
static inline const struct mom_boxnode_st *
mom_dyncast_node (const void *p)
{
  if (p && p != MOM_EMPTY_SLOT
      && ((const struct mom_anyvalue_st *) p)->va_itype == MOMITY_NODE)
    return (const struct mom_boxnode_st *) p;
  return NULL;
}

const struct mom_boxnode_st *mom_boxnode_make_meta (const struct mom_item_st
                                                    *conn, unsigned size,
                                                    const struct
                                                    mom_hashedvalue_st **sons,
                                                    const struct mom_item_st
                                                    *meta, intptr_t metarank);

static inline const struct mom_boxnode_st *
mom_boxnode_make (const struct mom_item_st *conn,
                  unsigned size, const struct mom_hashedvalue_st **sons)
{
  return mom_boxnode_make_meta (conn, size, sons, NULL, 0);
}

const struct mom_boxnode_st *mom_boxnode_meta_make_va (const struct
                                                       mom_item_st *meta,
                                                       intptr_t metarank,
                                                       const struct
                                                       mom_item_st *conn,
                                                       unsigned size, ...);

#define mom_boxnode_make_va(Conn,Siz,...) mom_boxnode_meta_make_va(NULL,0,Conn,Siz,__VA_ARGS__)


const struct mom_boxnode_st *mom_boxnode_meta_make_sentinel_va (const struct
                                                                mom_item_st
                                                                *meta,
                                                                intptr_t
                                                                metarank,
                                                                const struct
                                                                mom_item_st
                                                                *conn, ...)
  __attribute__ ((sentinel));

#define mom_boxnode_meta_make_sentinel(MetaItm,MetaRank,Conn,...) \
  mom_boxnode_meta_make_sentinel_va((MetaItm),(MetaRank),(Conn),\
				    ##__VA_ARGS__,NULL)

#define mom_boxnode_make_sentinel(Conn,...) \
  mom_boxnode_meta_make_sentinel_va(NULL,0,(Conn),\
				    ##__VA_ARGS__,NULL)



#define MOM_COUNTEDATA_FIELDS			\
  MOM_ANYVALUE_FIELDS;				\
  uint32_t cda_size;				\
  uint32_t cda_count


struct mom_itementry_tu
{
  struct mom_item_st *ient_itm;
  struct mom_anyvalue_st *ient_val;
};

#define MOM_ASSOVALDATA_FIELDS			\
  MOM_COUNTEDATA_FIELDS;			\
  struct mom_itementry_tu ada_ents[]    /* sorted array of entries */
// allocated size of ada_ents is cda_size; used count is cda_count.
struct mom_assovaldata_st
{
  MOM_ASSOVALDATA_FIELDS;
};


#define MOM_VECTVALDATA_FIELDS			\
  MOM_COUNTEDATA_FIELDS;			\
  struct mom_anyvalue_st*vecd_valarr[];
//// mutable vector
struct mom_vectvaldata_st
{
  MOM_VECTVALDATA_FIELDS;
};

struct mom_itemname_tu
{
  uint32_t itname_rank;
  struct mom_boxstring_st itname_string;
};

#define MOM_ITEM_FIELDS				\
  MOM_HASHEDVALUE_FIELDS;			\
  struct mom_itemname_tu* itm_radix;		\
  pthread_mutex_t itm_mtx;			\
  atomic_uint_least16_t itm_spacix;		\
  uint16_t itm_xtra;				\
  uint32_t itm_hid;				\
  uint64_t itm_lid;				\
  struct mom_assovaldata_st* itm_pattr;		\
  struct mom_vectvaldata_st* itm_pcomp;		\
  struct mom_item_st* itm_kinditm;		\
  struct mom_anyvalue_st* itm_payload


struct mom_item_st
{
  MOM_ITEM_FIELDS;
};


momhash_t mom_cstring_hash_len (const char *str, int len);


bool mom_valid_name_radix (const char *str, int len);

const struct mom_itemname_tu *mom_find_name_radix (const char *str, int len);
const struct mom_itemname_tu *mom_make_name_radix (const char *str, int len);
struct mom_item_st *mom_find_item_from_radix_id (const struct mom_itemname_tu
                                                 *radix, uint16_t hid,
                                                 uint64_t loid);

static inline struct mom_item_st *
mom_find_item_from_str_id (const char *str, int len, uint16_t hid,
                           uint64_t loid)
{
  const struct mom_itemname_tu *tu = mom_find_name_radix (str, len);
  if (tu)
    return mom_find_item_from_radix_id (tu, hid, loid);
  return NULL;
}

void mom_initialize_items (void);

struct mom_item_st *mom_make_item_from_radix_id (const struct mom_itemname_tu
                                                 *radix, uint16_t hid,
                                                 uint64_t loid);


static inline struct mom_item_st *
mom_make_item_from_str_id (const char *str, int len,
                           uint16_t hid, uint64_t loid)
{
  const struct mom_itemname_tu *tu = mom_find_name_radix (str, len);
  if (tu)
    return mom_make_item_from_radix_id (tu, hid, loid);
  return NULL;
}

static inline struct mom_item_st *
mom_make_item_from_radix (const struct mom_itemname_tu *radix)
{
  return mom_make_item_from_radix_id (radix, 0, 0);
}

struct mom_item_st *mom_clone_item_from_radix (const struct mom_itemname_tu
                                               *radix);

static inline struct mom_item_st *
mom_clone_item (const struct mom_item_st *itm)
{
  if (itm)
    return mom_clone_item_from_radix (itm->itm_radix);
  return NULL;
}

#define MOM_HI_LO_SUFFIX_LEN 16
const char *mom_hi_lo_suffix (char buf[static MOM_HI_LO_SUFFIX_LEN],
                              uint16_t hi, uint64_t lo);

static inline const char *
mom_item_hi_lo_suffix (char buf[static MOM_HI_LO_SUFFIX_LEN],
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
    return itm->itm_radix->itname_string.cstr;
  else
    return NULL;
}

const char *mom_item_cstring (const struct mom_item_st *itm);

int mom_item_cmp (const struct mom_item_st *itm1,
                  const struct mom_item_st *itm2);



struct mom_anyvalue_st *mom_assovaldata_get (const struct mom_assovaldata_st
                                             *asso,
                                             const struct mom_item_st *itmat);

struct mom_assovaldata_st *mom_assovaldata_remove (struct mom_assovaldata_st
                                                   *asso,
                                                   const struct mom_item_st
                                                   *itmat);

struct mom_assovaldata_st *mom_assovaldata_put (struct mom_assovaldata_st
                                                *asso,
                                                const struct mom_item_st
                                                *itmat, const void *data);

struct mom_assovaldata_st *mom_assovaldata_reserve (struct mom_assovaldata_st
                                                    *asso, unsigned gap);


struct mom_vectvaldata_st *mom_vectvaldata_reserve (struct mom_vectvaldata_st
                                                    *vec, unsigned gap);

static inline struct mom_anyvalue_st *
mom_vectvaldata_nth (const struct mom_vectvaldata_st *vec, int rk)
{
  if (!vec || vec == MOM_EMPTY_SLOT || vec->va_itype != MOMITY_VECTVALDATA)
    return NULL;
  unsigned cnt = vec->cda_count;
  assert (cnt <= vec->cda_size);
  if (rk < 0)
    rk += cnt;
  if (rk >= 0 && rk < (int) cnt)
    return vec->vecd_valarr[rk];
  return NULL;
}

static inline unsigned
mom_vectvaldata_count (const struct mom_vectvaldata_st *vec)
{
  if (!vec || vec == MOM_EMPTY_SLOT || vec->va_itype != MOMITY_VECTVALDATA)
    return 0;
  unsigned cnt = vec->cda_count;
  assert (cnt <= vec->cda_size);
  return cnt;
}

static inline struct mom_anyvalue_st **
mom_vectvaldata_valvect (const struct mom_vectvaldata_st *vec)
{
  if (!vec || vec == MOM_EMPTY_SLOT || vec->va_itype != MOMITY_VECTVALDATA)
    return NULL;
  assert (vec->cda_count <= vec->cda_size);
  return (struct mom_anyvalue_st **) vec->vecd_valarr;
}

struct mom_vectvaldata_st *mom_vectvaldata_resize (struct mom_vectvaldata_st
                                                   *vec, unsigned count);

#endif /*MONIMELT_INCLUDED_ */

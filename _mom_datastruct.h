// we want this file to be generated, but it is not yet

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
{				/// field prefix: va_;
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
  char cstr[];			/* actual size sva_size+1 */
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
  struct mom_item_st* seqitem[]	/* actual size sva_size */
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
  struct mom_hashedvalue_st *nod_sons[];	/* actual size sva_size */
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
  struct mom_itementry_tu ada_ents[]	/* sorted array of entries */
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

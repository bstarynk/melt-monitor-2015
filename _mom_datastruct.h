// we want this file to be generated, but it is not yet

struct mom_item_st;


/* when va_itype is 0, use va_flag & va_ltype; when va_itype is >0,
   use va_hsiz & va_lsiz */
// the common prefix of all values
#define MOM_ANYVALUE_FIELDS			\
  uint8_t va_itype;				\
  union {					\
    uint8_t va_hsiz;				\
    atomic_uint_least8_t va_flag;		\
  };						\
  union {					\
    uint16_t va_lsiz;				\
    int16_t va_ltype;				\
  }
struct mom_anyvalue_st { /// field prefix: va_;
  MOM_ANYVALUE_FIELDS;
};

#define MOM_HASHEDVALUE_FIELDS			\
  MOM_ANYVALUE_FIELDS;				\
  momhash_t hva_hash

enum { MOM_BOXINT_LTYPE=1 };
struct mom_boxint_st {
  MOM_HASHEDVALUE_FIELDS;
  // here field 
  intptr_t boxi_int;
};


static inline const struct mom_boxint_st*
mom_dyncast_boxint(const void*p)
{
  if (p && ((struct mom_anyvalue_st*)p)->va_itype==0
      &&  ((struct mom_anyvalue_st*)p)->va_ltype == MOM_BOXINT_LTYPE)
    return (const struct mom_boxint_st*)p;
  return NULL;
}

enum { MOM_BOXDOUBLE_LTYPE=2 };
struct mom_boxdouble_st {
  MOM_HASHEDVALUE_FIELDS;
  // here field
  double boxd_dbl;
};

static inline const struct mom_boxdouble_st*
mom_dyncast_boxdouble(const void*p)
{
  if (p && ((struct mom_anyvalue_st*)p)->va_itype == 0
      &&  ((struct mom_anyvalue_st*)p)->va_ltype == MOM_BOXDOUBLE_LTYPE)
    return (const struct mom_boxdouble_st*)p;
  return NULL;
}

/// sized values have sva_ prefix, with sva_size
#define MOM_SIZEDVALUE_FIELDS			\
  MOM_HASHEDVALUE_FIELDS;			\
  uint32_t sva_size
struct mom_sizedvalue_st {
  MOM_SIZEDVALUE_FIELDS;
};

  
enum { MOM_BOXSTRING_ITYPE=1 };
struct mom_boxstring_st {
  MOM_HASHEDVALUE_FIELDS;
  // here
  char cstr[];			/* actual size sva_size+1 */
};
  
static inline const struct mom_boxstring_st*
mom_dyncast_boxstring(const void*p)
{
  if (p && ((const struct mom_anyvalue_st*)p)->va_itype == MOM_BOXSTRING_ITYPE)
    return (const struct mom_boxstring_st*)p;
  return NULL;
}

#define MOM_SEQITEMS_FIELDS			\
  MOM_SIZEDVALUE_FIELDS;			\
  struct mom_item_st* seqitem[]	/* actual size sva_size */
struct mom_seqitems_st {
  MOM_SEQITEMS_FIELDS;
};

enum { MOM_TUPLE_ITYPE=2 };
// a tuple can contain NULL items 
struct mom_boxtuple_st {
  MOM_SEQITEMS_FIELDS;
};

enum { MOM_SET_ITYPE=3 };
// a set does not contain NULL items and they are sorted in ascending order
struct mom_boxset_st {
  MOM_SEQITEMS_FIELDS;
};

static inline const struct mom_seqitems_st*
mom_dyncast_seqitems(const void*p)
{
  if (p) {
    uint8_t ityp = ((const struct mom_anyvalue_st*)p)->va_itype;
    if (ityp==MOM_TUPLE_ITYPE || ityp==MOM_SET_ITYPE)
      return (const struct mom_seqitems_st*)p;
  }
  return NULL;
}

static inline const struct mom_boxtuple_st*
mom_dyncast_tuple(const void*p)
{
  if (p && ((const struct mom_anyvalue_st*)p)->va_itype==MOM_TUPLE_ITYPE)
    return (const struct mom_boxtuple_st*)p;
  return NULL;
}

static inline const struct mom_boxset_st*
mom_dyncast_set(const void*p)
{
  if (p && ((const struct mom_anyvalue_st*)p)->va_itype==MOM_TUPLE_ITYPE)
    return (const struct mom_boxset_st*)p;
  return NULL;
}

enum { MOM_NODE_ITYPE=4 };
struct mom_boxnode_st {
  MOM_SIZEDVALUE_FIELDS;
  // here prefix nod_
  intptr_t nod_metarank;
  struct mom_item_st* nod_metaitem;
  struct mom_item_st* nod_connitm;
  struct mom_anyvalue_st* nod_sons[]; /* actual size sva_size */
};
static inline const struct mom_boxnode_st*
mom_dyncast_node(const void*p)
{
  if (p && ((const struct mom_anyvalue_st*)p)->va_itype==MOM_NODE_ITYPE)
    return (const struct mom_boxnode_st*)p;
  return NULL;
}

#define MOM_COUNTEDATA_FIELDS			\
  MOM_ANYVALUE_FIELDS;				\
  uint32_t cda_size;				\
  uint32_t cda_count


struct mom_itementry_tu {
  struct mom_item_st*ient_itm;
  struct mom_anyvalue_st*ient_val;
};

enum { MOM_ASSOVALDATA_LTYPE=3 };
#define MOM_ASSOVALDATA_FIELDS			\
  MOM_COUNTEDATA_FIELDS;			\
  struct mom_itementry_tu ada_ents[] /* sorted array of entries */
// allocated size of ada_ents is cda_size; used count is cda_count.
struct mom_assovaldata_st {
  MOM_ASSOVALDATA_FIELDS;
};


enum { MOM_VECTVALDATA_LTYPE=4 };
#define MOM_VECTVALDATA_FIELDS			\
  MOM_COUNTEDATA_FIELDS;			\
  struct mom_anyvalue_st**vecd_valarr
//// mutable vector
struct mom_vectvaldata_st {
  MOM_VECTVALDATA_FIELDS;
};

struct mom_itemname_tu {
  uint32_t itname_rank;
  struct mom_boxstring_st itname_string;
};

enum { MOM_ITEM_LTYPE=3 };
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

int mom_item_cmp(const struct mom_item_st*, const struct mom_item_st*);

#define MOM_ITEM_MAXFIELDS 32768
struct mom_item_st {
  MOM_ITEM_FIELDS;
};

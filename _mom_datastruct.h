// we want this file to be generated, but it is not yet

struct mom_item_st;

// the common project of all objects
struct mom_anyobject_st { /// field prefix: ob_
  int16_t obtype;
  atomic_uint_least8_t ob_flag;
  uint8_t ob_more;
  uint32_t ob_hash;
};

struct mom_boxint_st {
  // from mom_anyobject_st
  int16_t obtype;
  atomic_uint_least8_t ob_flag;
  uint8_t ob_more;
  uint32_t ob_hash;
  // here field 
  intptr_t boxint;
};

struct mom_boxdouble_st {
  // from mom_anyobject_st
  int16_t obtype;
  atomic_uint_least8_t ob_flag;
  uint8_t ob_more;
  uint32_t ob_hash;
  // here field 
  double boxdouble;
};

struct mom_sizedobject_st {
  // from mom_anyobject_st
  int16_t obtype;
  atomic_uint_least8_t ob_flag;
  uint8_t ob_more;
  uint32_t ob_hash;
  // here
  uint32_t sob_size;
};

struct mom_countedobject_st {
  // from mom_anyobject_st
  int16_t obtype;
  atomic_uint_least8_t ob_flag;
  uint8_t ob_more;
  uint32_t ob_hash;
  // from mom_sizedobject_st
  uint32_t sob_size;
  // here
  uint32_t cob_count;
};
  
struct mom_boxstring_st {
  // from mom_sizedobject_st
  int16_t obtype;
  atomic_uint_least8_t ob_flag;
  uint8_t ob_more;
  uint32_t ob_hash;
  // from mom_sizedobject_st
  uint32_t sob_size;
  // here
  char cstr[];			/* actual size sob_size+1 */
};
  


struct mom_boxseqitem_st {
  // from mom_sizedobject_st
  int16_t obtype;
  atomic_uint_least8_t ob_flag;
  uint8_t ob_more;
  uint32_t ob_hash;
  // from mom_sizedobject_st
  uint32_t sob_size;
  // here
  struct mom_item_st* seqitem[];			/* actual size sob_size */
};
  
struct mom_boxnode_st {
  // from mom_sizedobject_st
  int16_t obtype;
  atomic_uint_least8_t ob_flag;
  uint8_t ob_more;
  uint32_t ob_hash;
  // from mom_sizedobject_st
  uint32_t sob_size;
  // here prefix nod_
  struct mom_item_st* nod_connitm;
  struct mom_anyobject_st* nod_sons[]; /* actual size sob_size */
};

//// mutable association
struct mom_mutassoc_st {
  // from mom_sizedobject_st
  int16_t obtype;
  atomic_uint_least8_t ob_flag;
  uint8_t ob_more;
  uint32_t ob_hash;
  // from mom_sizedobject_st
  uint32_t sob_size;
  // from mom_countedobject_st
  uint32_t cob_count;
  // here prefix is muass_
  struct {
    struct mom_item_st*ent_itm;
    struct mom_anyobject_st*ent_val;
  } *muass_entarr; // allocated size is sob_size, used count is uas_count, entries are sorted
};

//// mutable vector
struct mom_mutvect_st {
  // from mom_sizedobject_st
  int16_t obtype;
  atomic_uint_least8_t ob_flag;
  uint8_t ob_more;
  uint32_t ob_hash;
  // from mom_sizedobject_st
  uint32_t sob_size;
  // from mom_countedobject_st
  uint32_t cob_count;
  // here prefix is muvec
  struct mom_anyobject_st* muvec_arr; // allocated size is sob_size, used count is cob_count
};

struct mom_item_st {
  // from mom_sizedobject_st
  int16_t obtype;
  atomic_uint_least8_t ob_flag;
  uint8_t ob_more;
  uint32_t ob_hash;
  // from mom_sizedobject_st
  uint32_t sob_size;
  /// here prefix is itm_
  struct mom_boxstring_st* itm_radixstr;
  pthread_mutex_t itm_mtx;
  uint16_t itm_hid;
  uint64_t itm_lid;
  struct mom_mutassoc_st* itm_attr;
  struct mom_mutvect_st* itm_comp;
  struct mom_anyobject_st* itm_rest[]; /* length is sob_size */
};

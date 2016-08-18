// EmitC-generated header file momg_header_module.h ** DO NOT EDIT

// *** generated file momg_header_module.h - DO NOT EDIT 
// Copyright (C) 2015 - 2016 Free Software Foundation, Inc. 
// MONIMELT is a monitor for MELT - see http://gcc-melt.org/ 
// This generated file momg_header_module.h is part of MONIMELT, part of GCC 
//
// GCC is free software; you can redistribute it and/or modify 
// it under the terms of the GNU General Public License as published by 
// the Free Software Foundation; either version 3, or (at your option) 
// any later version. 
//
//  GCC is distributed in the hope that it will be useful, 
//  but WITHOUT ANY WARRANTY; without even the implied warranty of 
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
//  GNU General Public License for more details. 
//  You should have received a copy of the GNU General Public License 
//  along with GCC; see the file COPYING3.   If not see 
//  <http://www.gnu.org/licenses/>. 
//


#ifndef MONIMELT_INCLUDED_
#error meltmoni.h should be included before momg_header_module.h
#endif /*MONIMELT_INCLUDED_ */



/// 51 declarations:

typedef enum momenum_space_en
{ momenuva_spa_none = 0, momenuva_spa_predef = 1,
  momenuva_spa_global = 2, momenuva_spa_user = 3,
} momty_space_en;


typedef momitemptr_t momsigty_signature_space_to_item (momty_space_en);


extern momsigty_signature_space_to_item momf_make_item;


typedef struct momstruct_anyvalue_ty momty_anyvalue_ty;


struct momstruct_anyvalue_ty
{
  uint16_t momfi_va_itype;
  mom_atomic_int16_t momfi_va_ixv;
  uint32_t momfi_va_size;	/*ending struct anyvalue_ty */
};


typedef struct momstruct_hashedvalue_ty momty_hashedvalue_ty;


struct momstruct_hashedvalue_ty
{ /*extending struct anyvalue_ty */ ;
  uint16_t momfi_va_itype;
  mom_atomic_int16_t momfi_va_ixv;
  uint32_t momfi_va_size;
  /*extended struct anyvalue_ty in hashedvalue_ty */ ;
  momhash_t momfi_hva_hash;
  /*ending struct hashedvalue_ty */ };


typedef struct momstruct_boxint_ty momty_boxint_ty;


struct momstruct_boxint_ty
{ /*extending struct anyvalue_ty */ ;
  uint16_t momfi_va_itype;
  mom_atomic_int16_t momfi_va_ixv;
  uint32_t momfi_va_size; /*extended struct anyvalue_ty in boxint_ty */ ;
  long momfi_boxi_int;		/*ending struct boxint_ty */
};


typedef struct momstruct_boxdouble_ty momty_boxdouble_ty;


struct momstruct_boxdouble_ty
{ /*extending struct hashedvalue_ty */ ;
  uint16_t momfi_va_itype;
  mom_atomic_int16_t momfi_va_ixv;
  uint32_t momfi_va_size;
  momhash_t momfi_hva_hash;
    /*extended struct hashedvalue_ty in boxdouble_ty */ ;
  double momfi_boxd_dbl;	/*ending struct boxdouble_ty */
};


typedef struct momstruct_boxstring_ty momty_boxstring_ty;


struct momstruct_boxstring_ty
{ /*extending struct hashedvalue_ty */ ;
  uint16_t momfi_va_itype;
  mom_atomic_int16_t momfi_va_ixv;
  uint32_t momfi_va_size;
  momhash_t momfi_hva_hash;
    /*extended struct hashedvalue_ty in boxstring_ty */ ;
  char momfi_boxs_cstr[MOM_FLEXIBLE_DIM];	/*ending struct boxstring_ty */
};


typedef struct momstruct_seqitem_ty momty_seqitem_ty;


struct momstruct_seqitem_ty
{ /*extending struct hashedvalue_ty */ ;
  uint16_t momfi_va_itype;
  mom_atomic_int16_t momfi_va_ixv;
  uint32_t momfi_va_size;
  momhash_t momfi_hva_hash;
    /*extended struct hashedvalue_ty in seqitem_ty */ ;
  momitemptr_t momfi_seqitem[MOM_FLEXIBLE_DIM];	/*ending struct seqitem_ty */
};


typedef struct momstruct_boxtuple_ty momty_boxtuple_ty;


struct momstruct_boxtuple_ty
{ /*extending struct seqitem_ty */ ;
  uint16_t momfi_va_itype;
  mom_atomic_int16_t momfi_va_ixv;
  uint32_t momfi_va_size;
  momhash_t momfi_hva_hash;
  momitemptr_t momfi_seqitem[MOM_FLEXIBLE_DIM];
    /*extended struct seqitem_ty in boxtuple_ty */ ;
  /*ending struct boxtuple_ty */ };


typedef struct momstruct_boxset_ty momty_boxset_ty;


struct momstruct_boxset_ty
{ /*extending struct seqitem_ty */ ;
  uint16_t momfi_va_itype;
  mom_atomic_int16_t momfi_va_ixv;
  uint32_t momfi_va_size;
  momhash_t momfi_hva_hash;
  momitemptr_t momfi_seqitem[MOM_FLEXIBLE_DIM];
    /*extended struct seqitem_ty in boxset_ty */ ;
  /*ending struct boxset_ty */ };


typedef struct momstruct_countedata_ty momty_countedata_ty;


struct momstruct_countedata_ty
{ /*extending struct hashedvalue_ty */ ;
  uint16_t momfi_va_itype;
  mom_atomic_int16_t momfi_va_ixv;
  uint32_t momfi_va_size;
  momhash_t momfi_hva_hash;
    /*extended struct hashedvalue_ty in countedata_ty */ ;
  uint32_t momfi_cda_count;	/*ending struct countedata_ty */
};


typedef struct momstruct_assovaldata_ty momty_assovaldata_ty;


typedef struct momstruct_itementry_tu momty_itementry_tu;


struct momstruct_itementry_tu
{
  momitemptr_t momfi_ient_item;
  momvalue_t momfi_ient_value;
  /*ending struct itementry_tu */ };


struct momstruct_assovaldata_ty
{ /*extending struct countedata_ty */ ;
  uint16_t momfi_va_itype;
  mom_atomic_int16_t momfi_va_ixv;
  uint32_t momfi_va_size;
  momhash_t momfi_hva_hash;
  uint32_t momfi_cda_count;
    /*extended struct countedata_ty in assovaldata_ty */ ;
  momty_itementry_tu momfi_ada_ents[MOM_FLEXIBLE_DIM];	/*ending struct assovaldata_ty */
};


typedef struct momstruct_vectvaldata_ty momty_vectvaldata_ty;


struct momstruct_vectvaldata_ty
{ /*extending struct countedata_ty */ ;
  uint16_t momfi_va_itype;
  mom_atomic_int16_t momfi_va_ixv;
  uint32_t momfi_va_size;
  momhash_t momfi_hva_hash;
  uint32_t momfi_cda_count;
    /*extended struct countedata_ty in vectvaldata_ty */ ;
  momvalue_t momfi_vecd_valarr[MOM_FLEXIBLE_DIM];	/*ending struct vectvaldata_ty */
};


typedef struct momstruct_item_ty momty_item_ty;


struct momstruct_item_ty
{ /*extending struct hashedvalue_ty */ ;
  uint16_t momfi_va_itype;
  mom_atomic_int16_t momfi_va_ixv;
  uint32_t momfi_va_size;
  momhash_t momfi_hva_hash;
  /*extended struct hashedvalue_ty in item_ty */ ;
  pthread_mutex_t momfi_itm_mtx;
  uint32_t momfi_itm_namix;
  uint32_t momfi_itm_hid;
  uint64_t momfi_itm_lid;
  time_t momfi_itm_mtime;
  momty_assovaldata_ty *momfi_itm_pattr;
  momty_vectvaldata_ty *momfi_itm_pcomp;
  momitemptr_t momfi_itm_paylkind;
  void *momfi_itm_payldata;	/*ending struct item_ty */
};


typedef struct momstruct_node_ty momty_node_ty;


struct momstruct_node_ty
{ /*extending struct hashedvalue_ty */ ;
  uint16_t momfi_va_itype;
  mom_atomic_int16_t momfi_va_ixv;
  uint32_t momfi_va_size;
  momhash_t momfi_hva_hash;
  /*extended struct hashedvalue_ty in node_ty */ ;
  long momfi_nod_metarank;
  momitemptr_t momfi_nod_metaitem;
  momitemptr_t momfi_nod_connitm;
  momvalue_t momfi_nod_sons[MOM_FLEXIBLE_DIM];	/*ending struct node_ty */
};


typedef bool momsigty_signature_value_to_bool (momvalue_t);


static inline momsigty_signature_value_to_bool momf_valid_value_pointer;


typedef enum momenum_itype_en
{ momenuva_ity_none = 0, momenuva_ity_int = 1,
  momenuva_ity_double = 2, momenuva_ity_string = 3, momenuva_ity_item = 4,
  momenuva_ity_tuple = 5, momenuva_ity_set = 6, momenuva_ity_node = 7,
} momty_itype_en;


typedef momty_itype_en momsigty_signature_value_to_itype_en (momvalue_t);


static inline momsigty_signature_value_to_itype_en momf_itype;


typedef bool momsigty_signature_item_to_bool (momitemptr_t);


static inline momsigty_signature_item_to_bool momf_valid_item_pointer;


typedef long momsigty_signature_value_int_to_int (momvalue_t, long);


static inline momsigty_signature_value_int_to_int momf_int_val_def;


typedef momhash_t momsigty_signature_value_to_hash_t (momvalue_t);


static inline momsigty_signature_value_to_hash_t momf_value_hash;


typedef momvalue_t momsigty_signature_value_to_value (momvalue_t);


static inline momsigty_signature_value_to_value momf_dyncast_int;


static inline momsigty_signature_value_to_value momf_dyncast_boxdouble;


static inline momsigty_signature_value_to_value momf_dyncast_boxstring;


static inline momsigty_signature_value_to_value momf_dyncast_seqitem;


static inline momsigty_signature_value_to_value momf_dyncast_tuple;


static inline momsigty_signature_value_to_value momf_dyncast_set;


static inline momsigty_signature_value_to_value momf_dyncast_node;


static inline momsigty_signature_value_to_value momf_dyncast_item;


/// 1 definitions:





/**type anyvalue_ty **/




/**type hashedvalue_ty **/




/**type boxint_ty **/




/**type boxdouble_ty **/




/**type boxstring_ty **/




/**type seqitem_ty **/




/**type boxtuple_ty **/




/**type boxset_ty **/




/**type countedata_ty **/




/**type assovaldata_ty **/




/**type vectvaldata_ty **/




/**type item_ty **/




/**type node_ty **/




/**inline:valid_value_pointer **/

static inline bool
momf_valid_value_pointer (momvalue_t momarg_val0)
{
   /**block valid_value_pointer__1oT81X9WV7H8nt **/
  ;
  {
     /**cond cond__0el26bylTcPsue **/
      /**test:test__6SMrbLRgJCHMxm **/
    if ( /*is_null */ ((momarg_val0) == NULL))
      {
	return (false);;
      }
      /**test:test__7ConxCTpHWpuKH **/
    else if ( /*same_value_prim */
      ((momarg_val0) == ( /*empty_slot_prim */ MOM_EMPTY_SLOT)))
      {
	return (false);;
      }
      /**test:test__0DYkv46mig7fUI **/
    else if ( /*unsafe_int_mod_prim */
      (( /*cast_pointer_to_int_prim */ (intptr_t) (momarg_val0)) % (2)))
      {
	return (false);;
      }
      /**lastest:test__8rM9KAhvcF8Cgo **/
    else
      {
	return (true);;
      }
  };
  ;
   /**endblock valid_value_pointer__1oT81X9WV7H8nt **/ }

/**endinline:valid_value_pointer **/




/**inline:itype **/

static inline momty_itype_en
momf_itype (momvalue_t momarg_val0)
{
   /**block itype__0rR1W9JEWIxCd3 **/
  ;
  {
     /**cond cond__8V9frM9jRoLFYx **/
      /**test:test__3Y3tix83PBX0RZ **/
    if ( /*is_null */ ((momarg_val0) == NULL))
      {
	return (momenuva_ity_none);;
      }
      /**test:test__4ZeFNsVjHR6HA9 **/
    else if ( /*same_value_prim */
      ((momarg_val0) == ( /*empty_slot_prim */ MOM_EMPTY_SLOT)))
      {
	return (momenuva_ity_none);;
      }
      /**test:test__0G1BK7SeWkVM6J **/
    else if ( /*unsafe_int_mod_prim */
      (( /*cast_pointer_to_int_prim */ (intptr_t) (momarg_val0)) % (2)))
      {
	return (momenuva_ity_int);;
      }
      /**lastest:test__4CMwmIgphxmlWz **/
    else
      {
	return ((momty_itype_en)
		(( /*field */
		 ((momty_anyvalue_ty *) (momarg_val0))->momfi_va_itype)));;
      }
  };
  ;
   /**endblock itype__0rR1W9JEWIxCd3 **/ }

/**endinline:itype **/




/**inline:valid_item_pointer **/

static inline bool
momf_valid_item_pointer (momitemptr_t momarg_itm0)
{
   /**block valid_item_pointer__7cP5WnFAXpNaiJ **/
  ;
  {
     /**cond cond__6S44bo3qcKen3V **/
      /**test:test__7mzdyMB1Nyvmq3 **/
    if ( /*not_bool_prim */ !(momf_valid_value_pointer ((  /**cast value
         **/
							  momvalue_t)
							(momarg_itm0))))
      {
	return (false);;
      }
      /**test:test__6qUXAj7cSfHHjp **/
    else if ( /*equal_int_prim */ ((( /**cast int **/ long) (( /*field */ ((momty_anyvalue_ty *) (momarg_itm0))->momfi_va_itype))) == ((
																   /**cast int
        **/
																	 long)
																       (momenuva_ity_item))))
      {
	return (true);;
      }
      /**lastest:test__91zWA1ibJx1DiX **/
    else
      {
	return (false);;
      }
  };
  ;
   /**endblock valid_item_pointer__7cP5WnFAXpNaiJ **/ }

/**endinline:valid_item_pointer **/




/**inline:int_val_def **/

static inline long
momf_int_val_def (momvalue_t momarg_val0, long momarg_num1)
{
   /**block int_val_def__1GcVAR4TvWkkf8 **/
  ;
  {
     /**cond cond__4qX17nIvqUTu3p **/
      /**test:test__0vyHrvuSKGg3lH **/
    if ( /*is_null */ ((momarg_val0) == NULL))
      {
	return (momarg_num1);;
      }
      /**test:test__6brWpLAMzPq2dp **/
    else if ( /*same_value_prim */
      ((momarg_val0) == ( /*empty_slot_prim */ MOM_EMPTY_SLOT)))
      {
	return (momarg_num1);;
      }
      /**test:test__4gRWkpiaj6NXcp **/
    else if ( /*unsafe_int_mod_prim */
      (( /*cast_pointer_to_int_prim */ (intptr_t) (momarg_val0)) % (2)))
      {
	return ((( /*cast_pointer_to_int_prim */ (intptr_t) (momarg_val0)) /
		 (2)));;
      }
      /**test:test__5D0pkeHAJtZBuj **/
    else if ( /*equal_int_prim */ ((( /**cast int **/ long) (( /*field */ ((momty_anyvalue_ty *) (momarg_val0))->momfi_va_itype))) == ((
																   /**cast int
        **/
																	 long)
																       (momenuva_ity_int))))
      {
	return (( /*field */
		((momty_boxint_ty *) (momarg_val0))->momfi_boxi_int));;
      }
      /**lastest:test__1pjNx9CaNB5JWZ **/
    else
      {
	return (momarg_num1);;
      }
  };
  ;
   /**endblock int_val_def__1GcVAR4TvWkkf8 **/ }

/**endinline:int_val_def **/




/**inline:value_hash **/

static inline momhash_t
momf_value_hash (momvalue_t momarg_val0)
{
   /**block value_hash__2gvgl2u88BdTkh **/
  momty_itype_en momloc_typ0loc = (momty_itype_en /*nothing */ )0;

  ;
  momloc_typ0loc = momf_itype (momarg_val0);
  ;
  {
     /**cond cond__5o2KRTCqM2yTXz **/
      /**test:test__0BLnhg1hYShgY5 **/
    if ( /*equal_int_prim */ ((( /**cast int **/ long) (momloc_typ0loc)) == ((
									     /**cast int
        **/
									       long)
									     (momenuva_ity_none))))
      {
	return ((				   /**cast hash_t
        **/ momhash_t) (0));;
      }
      /**test:test__713o3vm3Dvfszy **/
    else if ( /*equal_int_prim */ ((( /**cast int **/ long) (momloc_typ0loc)) == ((
										  /**cast int
        **/
										    long)
										  (momenuva_ity_int))))
      {
	return ((				  /**cast hash_t
        **/
		  momhash_t) (((((momf_int_val_def (momarg_val0, 0)
				 ) % (1000001137))) + (1))));;
      }
      /**test:test__7cXlTU7FwgxiMr **/
    else if ( /*equal_int_prim */ ((( /**cast int **/ long) (momloc_typ0loc)) <= ((
										  /**cast int
        **/
										    long)
										  (momenuva_ity_node))))
      {
	return ((				   /**cast hash_t
		          **/ momhash_t) (( /*field */
					  ((momty_hashedvalue_ty
					    *) (momarg_val0))->
			      momfi_hva_hash)));;
      }
      /**lastest:test__90uqA5HK7isacr **/
    else
      {
	return ((					 /**cast hash_t
		          **/ momhash_t) (((1) +
					   (( /**cast int **/
					     long) (momloc_typ0loc)))));;
  }};
  ;
   /**endblock value_hash__2gvgl2u88BdTkh **/ }

/**endinline:value_hash **/




/**inline:dyncast_int **/

static inline momvalue_t
momf_dyncast_int (momvalue_t momarg_val0)
{
   /**block dyncast_int__59nm7lpB8c3IHt **/
  ;
  {
     /**cond cond__7wfVmhwEW1RKTz **/
      /**test:test__6meaCZywvL62gG **/
    if ( /*equal_int_prim */ ((( /**cast int **/ long) (momf_itype
							(momarg_val0))) ==
			      (( /**cast int **/ long) (momenuva_ity_int))))
      {
	return (momarg_val0);;
      }
      /**lastest:test__3Svn0h5lKwx8FP **/
    else
      {
	return (NULL);;
      }
  };
  ;
   /**endblock dyncast_int__59nm7lpB8c3IHt **/ }

/**endinline:dyncast_int **/




/**inline:dyncast_boxdouble **/

static inline momvalue_t
momf_dyncast_boxdouble (momvalue_t momarg_val0)
{
   /**block dyncast_boxdouble__2Fh2govIuMauTf **/
  ;
  {
     /**cond cond__621k6z731BTub8 **/
      /**test:test__5l56bBbAHSumoG **/
    if ( /*equal_int_prim */ ((( /**cast int **/ long) (momf_itype
							(momarg_val0))) ==
			      (( /**cast int **/
				long) (momenuva_ity_double))))
      {
	return (momarg_val0);;
      }
      /**lastest:test__6vcCpLsAL58zVN **/
    else
      {
	return (NULL);;
      }
  };
  ;
   /**endblock dyncast_boxdouble__2Fh2govIuMauTf **/ }

/**endinline:dyncast_boxdouble **/




/**inline:dyncast_boxstring **/

static inline momvalue_t
momf_dyncast_boxstring (momvalue_t momarg_val0)
{
   /**block dyncast_boxstring__8CaXgeaK2SDhSU **/
  ;
  {
     /**cond cond__1o8ZG3gakElfwI **/
      /**test:test__4Xqsj0P9EzT7jk **/
    if ( /*equal_int_prim */ ((( /**cast int **/ long) (momf_itype
							(momarg_val0))) ==
			      (( /**cast int **/
				long) (momenuva_ity_string))))
      {
	return (momarg_val0);;
      }
      /**lastest:test__42xXEXjFg2IRY8 **/
    else
      {
	return (NULL);;
      }
  };
  ;
   /**endblock dyncast_boxstring__8CaXgeaK2SDhSU **/ }

/**endinline:dyncast_boxstring **/




/**inline:dyncast_seqitem **/

static inline momvalue_t
momf_dyncast_seqitem (momvalue_t momarg_val0)
{
   /**block dyncast_seqitem__2vMWx9eAaq1tzd **/
  ;
  {
     /**cond cond__2JekxKFGMqFbv3 **/
      /**test:test__4f6vWWLlFUnXz7 **/
    if ((( /*equal_int_prim */ ((( /**cast int **/ long) (momf_itype
							  (momarg_val0))) == (( /**cast int **/ long) (momenuva_ity_tuple)))) || ( /*equal_int_prim */ (((
													 /**cast int
							         **/ long) (momf_itype (momarg_val0))) == ((
							/**cast int
         **/
													     long)
													   (momenuva_ity_set))))))
      {
	return (momarg_val0);;
      }
      /**lastest:test__1MNeRDvl1mKU0v **/
    else
      {
	return (NULL);;
      }
  };
  ;
   /**endblock dyncast_seqitem__2vMWx9eAaq1tzd **/ }

/**endinline:dyncast_seqitem **/




/**inline:dyncast_tuple **/

static inline momvalue_t
momf_dyncast_tuple (momvalue_t momarg_val0)
{
   /**block dyncast_tuple__6hj75E8rs4aeyG **/
  ;
  {
     /**cond cond__6hJcJoiI8KuVZz **/
      /**test:test__5uaARG20KEm2Ws **/
    if ( /*equal_int_prim */ ((( /**cast int **/ long) (momf_itype
							(momarg_val0))) ==
			      (( /**cast int **/ long) (momenuva_ity_tuple))))
      {
	return (momarg_val0);;
      }
      /**lastest:test__2rDJNxTIZDJSbr **/
    else
      {
	return (NULL);;
      }
  };
  ;
   /**endblock dyncast_tuple__6hj75E8rs4aeyG **/ }

/**endinline:dyncast_tuple **/




/**inline:dyncast_set **/

static inline momvalue_t
momf_dyncast_set (momvalue_t momarg_val0)
{
   /**block dyncast_set__7EhL3TDea7S3Wk **/
  ;
  {
     /**cond cond__99DS1VL4P7lkIA **/
      /**test:test__3Ed4Hvg5L6YL46 **/
    if ( /*equal_int_prim */ ((( /**cast int **/ long) (momf_itype
							(momarg_val0))) ==
			      (( /**cast int **/ long) (momenuva_ity_set))))
      {
	return (momarg_val0);;
      }
      /**lastest:test__8h4gTGzFmcB2LK **/
    else
      {
	return (NULL);;
      }
  };
  ;
   /**endblock dyncast_set__7EhL3TDea7S3Wk **/ }

/**endinline:dyncast_set **/




/**inline:dyncast_node **/

static inline momvalue_t
momf_dyncast_node (momvalue_t momarg_val0)
{
   /**block dyncast_node__0ddbCINeUfNAqj **/
  ;
  {
     /**cond cond__1KZkFGspqonw7y **/
      /**test:test__0MmVhhxfreNjrU **/
    if ( /*equal_int_prim */ ((( /**cast int **/ long) (momf_itype
							(momarg_val0))) ==
			      (( /**cast int **/ long) (momenuva_ity_node))))
      {
	return (momarg_val0);;
      }
      /**lastest:test__4t5UITMDeEhzkg **/
    else
      {
	return (NULL);;
      }
  };
  ;
   /**endblock dyncast_node__0ddbCINeUfNAqj **/ }

/**endinline:dyncast_node **/




/**inline:dyncast_item **/

static inline momvalue_t
momf_dyncast_item (momvalue_t momarg_val0)
{
   /**block dyncast_item__0dPsyfqPuPhCue **/
  ;
  {
     /**cond cond__3sl7hm2xP0LJkc **/
      /**test:test__2MHUCZFFHzMg9Y **/
    if (momf_valid_item_pointer
	(( /**cast item **/ momitemptr_t) (momarg_val0)
	))
      {
	return (momarg_val0);;
      }
      /**lastest:test__8TeGI0SdPnpWtL **/
    else
      {
	return (NULL);;
      }
  };
  ;
   /**endblock dyncast_item__0dPsyfqPuPhCue **/ }

/**endinline:dyncast_item **/


/// end of generated header file momg_header_module.h

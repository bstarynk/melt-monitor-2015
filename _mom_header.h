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



/// 52 declarations:

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


typedef enum momenum_itype_en
{ momenuva_ity_none = 0, momenuva_ity_int = 1,
  momenuva_ity_double = 2, momenuva_ity_string = 3, momenuva_ity_item = 4,
  momenuva_ity_tuple = 5, momenuva_ity_set = 6, momenuva_ity_node = 7,
} momty_itype_en;


typedef enum momenum_xtype_en
{ /*extending enum itype_en */ momenuva_ity_none_momenfr_itype_en = 0,
  momenuva_ity_int_momenfr_itype_en =
    1, momenuva_ity_double_momenfr_itype_en = 2,
  momenuva_ity_string_momenfr_itype_en =
    3, momenuva_ity_item_momenfr_itype_en = 4,
  momenuva_ity_tuple_momenfr_itype_en = 5, momenuva_ity_set_momenfr_itype_en =
    6,
  momenuva_ity_node_momenfr_itype_en = 7,	/*extended enum itype_en */
  momenuva_xty_lastvalue = 8, momenuva_xty_assovaldata =
    9, momenuva_xty_vectvaldata = 10,
  momenuva_xty_queue = 11, momenuva_xty_hashset = 12, momenuva_xty_hashmap =
    13,
  momenuva_xty_hashassoc = 14, momenuva_xty_loader = 15, momenuva_xty_dumper =
    16,
  momenuva_xty_json = 17, momenuva_xty_file = 18, momenuva_xty_filebuffer =
    19,
} momty_xtype_en;


typedef bool momsigty_signature_value_to_bool (momvalue_t);


static inline momsigty_signature_value_to_bool momf_valid_value_pointer;


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




/**type itype_en **/




/**type xtype_en **/




/**inline:valid_value_pointer **/

static inline bool
momf_valid_value_pointer (momvalue_t momarg_val0)
{
   /**block valid_value_pointer__0CtV3nTFD2Xe2B **/
  ;
  {
     /**cond cond__4o19NkHMe5zkME **/
      /**test:test__7n2bzgbne6oYZG **/
    if ( /*is_null */ ((momarg_val0) == NULL))
      {
	return (false);;
      }
      /**test:test__2v3VLbW6xEKPCG **/
    else if ( /*same_value_prim */
      ((momarg_val0) == ( /*empty_slot_prim */ MOM_EMPTY_SLOT)))
      {
	return (false);;
      }
      /**test:test__7btFZAAMHMubIa **/
    else if ( /*unsafe_int_mod_prim */
      (( /*cast_pointer_to_int_prim */ (intptr_t) (momarg_val0)) % (2)))
      {
	return (false);;
      }
      /**lastest:test__8l2RyzH7GE6Pl8 **/
    else
      {
	return (true);;
      }
  };
  ;
   /**endblock valid_value_pointer__0CtV3nTFD2Xe2B **/ }

/**endinline:valid_value_pointer **/




/**inline:itype **/

static inline momty_itype_en
momf_itype (momvalue_t momarg_val0)
{
   /**block itype__2MXjxUpFPqY5he **/
  ;
  {
     /**cond cond__6utoMjutG3H3ag **/
      /**test:test__8mtLPf8q6bVmwG **/
    if ( /*is_null */ ((momarg_val0) == NULL))
      {
	return (momenuva_ity_none);;
      }
      /**test:test__2AhrYtUsUo1bWf **/
    else if ( /*same_value_prim */
      ((momarg_val0) == ( /*empty_slot_prim */ MOM_EMPTY_SLOT)))
      {
	return (momenuva_ity_none);;
      }
      /**test:test__2NIw7xJrmohCVi **/
    else if ( /*unsafe_int_mod_prim */
      (( /*cast_pointer_to_int_prim */ (intptr_t) (momarg_val0)) % (2)))
      {
	return (momenuva_ity_int);;
      }
      /**lastest:test__1nueH0n0j0RfH3 **/
    else
      {
	return ((momty_itype_en)
		(( /*field */
		 ((momty_anyvalue_ty *) (momarg_val0))->momfi_va_itype)));;
      }
  };
  ;
   /**endblock itype__2MXjxUpFPqY5he **/ }

/**endinline:itype **/




/**inline:valid_item_pointer **/

static inline bool
momf_valid_item_pointer (momitemptr_t momarg_itm0)
{
   /**block valid_item_pointer__8uNTGdwpsyT9gR **/
  ;
  {
     /**cond cond__4AohkCJtvmasLN **/
      /**test:test__1B0IosvWxE3bKr **/
    if ( /*not_bool_prim */ !(momf_valid_value_pointer ((  /**cast value
         **/
							  momvalue_t)
							(momarg_itm0))))
      {
	return (false);;
      }
      /**test:test__1Xc58JaBlnsJ2S **/
    else if ( /*equal_int_prim */ ((( /**cast int **/ long) (( /*field */ ((momty_anyvalue_ty *) (momarg_itm0))->momfi_va_itype))) == ((
																   /**cast int
        **/
																	 long)
																       (momenuva_ity_item))))
      {
	return (true);;
      }
      /**lastest:test__5JxFuUSJS0IJdw **/
    else
      {
	return (false);;
      }
  };
  ;
   /**endblock valid_item_pointer__8uNTGdwpsyT9gR **/ }

/**endinline:valid_item_pointer **/




/**inline:int_val_def **/

static inline long
momf_int_val_def (momvalue_t momarg_val0, long momarg_num1)
{
   /**block int_val_def__17CIW0Ztt0b06M **/
  ;
  {
     /**cond cond__5SMyg6ISYqns0E **/
      /**test:test__7g6giR7LaVc7sK **/
    if ( /*is_null */ ((momarg_val0) == NULL))
      {
	return (momarg_num1);;
      }
      /**test:test__6MeitjnVsLw6bS **/
    else if ( /*same_value_prim */
      ((momarg_val0) == ( /*empty_slot_prim */ MOM_EMPTY_SLOT)))
      {
	return (momarg_num1);;
      }
      /**test:test__1IWI0RtB0UR2pM **/
    else if ( /*unsafe_int_mod_prim */
      (( /*cast_pointer_to_int_prim */ (intptr_t) (momarg_val0)) % (2)))
      {
	return ((( /*cast_pointer_to_int_prim */ (intptr_t) (momarg_val0)) /
		 (2)));;
      }
      /**test:test__8aeLD7my1vamjy **/
    else if ( /*equal_int_prim */ ((( /**cast int **/ long) (( /*field */ ((momty_anyvalue_ty *) (momarg_val0))->momfi_va_itype))) == ((
																   /**cast int
        **/
																	 long)
																       (momenuva_ity_int))))
      {
	return (( /*field */
		((momty_boxint_ty *) (momarg_val0))->momfi_boxi_int));;
      }
      /**lastest:test__1YSznZVWhgxdnx **/
    else
      {
	return (momarg_num1);;
      }
  };
  ;
   /**endblock int_val_def__17CIW0Ztt0b06M **/ }

/**endinline:int_val_def **/




/**inline:value_hash **/

static inline momhash_t
momf_value_hash (momvalue_t momarg_val0)
{
   /**block value_hash__1yDCiRiIR6Xh3h **/
  momty_itype_en momloc_typ0loc = (momty_itype_en /*nothing */ )0;

  ;
  momloc_typ0loc = momf_itype (momarg_val0);
  ;
  {
     /**cond cond__8AlEltlaEyE1VF **/
      /**test:test__7KLtzhIb7mBfRj **/
    if ( /*equal_int_prim */ ((( /**cast int **/ long) (momloc_typ0loc)) == ((
									     /**cast int
        **/
									       long)
									     (momenuva_ity_none))))
      {
	return ((				   /**cast hash_t
        **/ momhash_t) (0));;
      }
      /**test:test__1R33GIVivvBN9t **/
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
      /**test:test__8zfFRIxwy8hrei **/
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
      /**lastest:test__0fymbry9gvuzzr **/
    else
      {
	return ((					 /**cast hash_t
		          **/ momhash_t) (((1) +
					   (( /**cast int **/
					     long) (momloc_typ0loc)))));;
  }};
  ;
   /**endblock value_hash__1yDCiRiIR6Xh3h **/ }

/**endinline:value_hash **/




/**inline:dyncast_int **/

static inline momvalue_t
momf_dyncast_int (momvalue_t momarg_val0)
{
   /**block dyncast_int__0HpFWcuzkdIeam **/
  ;
  {
     /**cond cond__7FPfwr5EDxULt9 **/
      /**test:test__6Ktm9pFTNzNW8z **/
    if ( /*equal_int_prim */ ((( /**cast int **/ long) (momf_itype
							(momarg_val0))) ==
			      (( /**cast int **/ long) (momenuva_ity_int))))
      {
	return (momarg_val0);;
      }
      /**lastest:test__1z5gJyEU6UPeof **/
    else
      {
	return (NULL);;
      }
  };
  ;
   /**endblock dyncast_int__0HpFWcuzkdIeam **/ }

/**endinline:dyncast_int **/




/**inline:dyncast_boxdouble **/

static inline momvalue_t
momf_dyncast_boxdouble (momvalue_t momarg_val0)
{
   /**block dyncast_boxdouble__2oWZgehkrvRa0M **/
  ;
  {
     /**cond cond__6V13ymDXyZUuS3 **/
      /**test:test__019RjGz1rRjLGt **/
    if ( /*equal_int_prim */ ((( /**cast int **/ long) (momf_itype
							(momarg_val0))) ==
			      (( /**cast int **/
				long) (momenuva_ity_double))))
      {
	return (momarg_val0);;
      }
      /**lastest:test__4tjdgotl9Lzab8 **/
    else
      {
	return (NULL);;
      }
  };
  ;
   /**endblock dyncast_boxdouble__2oWZgehkrvRa0M **/ }

/**endinline:dyncast_boxdouble **/




/**inline:dyncast_boxstring **/

static inline momvalue_t
momf_dyncast_boxstring (momvalue_t momarg_val0)
{
   /**block dyncast_boxstring__20KSPnMBcxk6ag **/
  ;
  {
     /**cond cond__3fAfrecVAbibh9 **/
      /**test:test__8rBzJje7rdka1a **/
    if ( /*equal_int_prim */ ((( /**cast int **/ long) (momf_itype
							(momarg_val0))) ==
			      (( /**cast int **/
				long) (momenuva_ity_string))))
      {
	return (momarg_val0);;
      }
      /**lastest:test__0wcXuTEP6f9ciY **/
    else
      {
	return (NULL);;
      }
  };
  ;
   /**endblock dyncast_boxstring__20KSPnMBcxk6ag **/ }

/**endinline:dyncast_boxstring **/




/**inline:dyncast_seqitem **/

static inline momvalue_t
momf_dyncast_seqitem (momvalue_t momarg_val0)
{
   /**block dyncast_seqitem__1SF7P1BUzwbmMJ **/
  ;
  {
     /**cond cond__24BlkFKCugmYmC **/
      /**test:test__38aUot1ymUum3f **/
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
      /**lastest:test__0PbPLD6ZHrYCbA **/
    else
      {
	return (NULL);;
      }
  };
  ;
   /**endblock dyncast_seqitem__1SF7P1BUzwbmMJ **/ }

/**endinline:dyncast_seqitem **/




/**inline:dyncast_tuple **/

static inline momvalue_t
momf_dyncast_tuple (momvalue_t momarg_val0)
{
   /**block dyncast_tuple__3NXJH8KHBUtAol **/
  ;
  {
     /**cond cond__72nVYMZtLlBVIB **/
      /**test:test__8JwPzTVoPRUI3A **/
    if ( /*equal_int_prim */ ((( /**cast int **/ long) (momf_itype
							(momarg_val0))) ==
			      (( /**cast int **/ long) (momenuva_ity_tuple))))
      {
	return (momarg_val0);;
      }
      /**lastest:test__7tsaBCPbbdX8Xr **/
    else
      {
	return (NULL);;
      }
  };
  ;
   /**endblock dyncast_tuple__3NXJH8KHBUtAol **/ }

/**endinline:dyncast_tuple **/




/**inline:dyncast_set **/

static inline momvalue_t
momf_dyncast_set (momvalue_t momarg_val0)
{
   /**block dyncast_set__9faFKy5lFSevXL **/
  ;
  {
     /**cond cond__7VZnGZKxXgvWaN **/
      /**test:test__0j2Sl2m22oeyBY **/
    if ( /*equal_int_prim */ ((( /**cast int **/ long) (momf_itype
							(momarg_val0))) ==
			      (( /**cast int **/ long) (momenuva_ity_set))))
      {
	return (momarg_val0);;
      }
      /**lastest:test__53AxVdjEK0hqgR **/
    else
      {
	return (NULL);;
      }
  };
  ;
   /**endblock dyncast_set__9faFKy5lFSevXL **/ }

/**endinline:dyncast_set **/




/**inline:dyncast_node **/

static inline momvalue_t
momf_dyncast_node (momvalue_t momarg_val0)
{
   /**block dyncast_node__5p8yNLtoAd4x4C **/
  ;
  {
     /**cond cond__0UN4is6gUfNUXG **/
      /**test:test__1bg5xaPNb09fAl **/
    if ( /*equal_int_prim */ ((( /**cast int **/ long) (momf_itype
							(momarg_val0))) ==
			      (( /**cast int **/ long) (momenuva_ity_node))))
      {
	return (momarg_val0);;
      }
      /**lastest:test__2wazDux3H58lwT **/
    else
      {
	return (NULL);;
      }
  };
  ;
   /**endblock dyncast_node__5p8yNLtoAd4x4C **/ }

/**endinline:dyncast_node **/




/**inline:dyncast_item **/

static inline momvalue_t
momf_dyncast_item (momvalue_t momarg_val0)
{
   /**block dyncast_item__2Ew5puaKy1vUqY **/
  ;
  {
     /**cond cond__0iGsobsnrIAY3V **/
      /**test:test__2LTEmcKNArLHeV **/
    if (momf_valid_item_pointer
	(( /**cast item **/ momitemptr_t) (momarg_val0)
	))
      {
	return (momarg_val0);;
      }
      /**lastest:test__1BnFu0KFq23K9y **/
    else
      {
	return (NULL);;
      }
  };
  ;
   /**endblock dyncast_item__2Ew5puaKy1vUqY **/ }

/**endinline:dyncast_item **/


/// end of generated header file momg_header_module.h
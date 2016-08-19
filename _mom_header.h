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



/// 56 declarations:

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


typedef momvalue_t momsigty_signature_int_to_value (long);


static inline momsigty_signature_int_to_value momf_int_make;


typedef momitemptr_t momsigty_signature_value_int_to_item (momvalue_t, long);


static inline momsigty_signature_value_int_to_item momf_seqitem_nth;


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
   /**block valid_value_pointer__0K3vxlNHLKzp1p **/
  ;
  {
     /**cond cond__7mu2mYzsGkMTeH **/
      /**test:test__6hl9b1lhJzHLVh **/
    if ( /*is_null */ ((momarg_val0) == NULL))
      {
	return (false);;
      }
      /**test:test__1MwjJhMFVF5V7P **/
    else if ( /*same_value_prim */
      ((momarg_val0) == ( /*empty_slot_prim */ MOM_EMPTY_SLOT)))
      {
	return (false);;
      }
      /**test:test__41j3eojFCh5zhu **/
    else if ( /*unsafe_int_mod_prim */
      (( /*cast_pointer_to_int_prim */ (intptr_t) (momarg_val0)) % (2)))
      {
	return (false);;
      }
      /**lastunitest:test__0lD7PaTvkX9ZK1 **/
    else
      {
	return (true);;
      }
  };
  ;
   /**endblock valid_value_pointer__0K3vxlNHLKzp1p **/ }

/**endinline:valid_value_pointer **/




/**inline:itype **/

static inline momty_itype_en
momf_itype (momvalue_t momarg_val0)
{
   /**block itype__4ng6DLaXempkJG **/
  ;
  {
     /**cond cond__0GACL0nCUAo1Ne **/
      /**test:test__0liYe8YykXBrWb **/
    if ( /*is_null */ ((momarg_val0) == NULL))
      {
	return (momenuva_ity_none);;
      }
      /**test:test__49Ey3Yk0putI62 **/
    else if ( /*same_value_prim */
      ((momarg_val0) == ( /*empty_slot_prim */ MOM_EMPTY_SLOT)))
      {
	return (momenuva_ity_none);;
      }
      /**test:test__4zEy98m08TADW1 **/
    else if ( /*unsafe_int_mod_prim */
      (( /*cast_pointer_to_int_prim */ (intptr_t) (momarg_val0)) % (2)))
      {
	return (momenuva_ity_int);;
      }
      /**lastunitest:test__4uYHPy0I1zxzPg **/
    else
      {
	return ((momty_itype_en)
		(( /*field */
		 ((momty_anyvalue_ty *) (momarg_val0))->momfi_va_itype)));;
      }
  };
  ;
   /**endblock itype__4ng6DLaXempkJG **/ }

/**endinline:itype **/




/**inline:valid_item_pointer **/

static inline bool
momf_valid_item_pointer (momitemptr_t momarg_itm0)
{
   /**block valid_item_pointer__1tTnuqhhVwG81p **/
  ;
  {
     /**cond cond__3bD9LGHVF2u0H2 **/
      /**test:test__93jlC1m6mzUN6q **/
    if ( /*not_bool_prim */ !(momf_valid_value_pointer ((  /**cast value
         **/
							  momvalue_t)
							(momarg_itm0))))
      {
	return (false);;
      }
      /**test:test__0JsdHo6raURNLT **/
    else if ( /*equal_int_prim */ ((( /**cast int **/ long) (( /*field */ ((momty_anyvalue_ty *) (momarg_itm0))->momfi_va_itype))) == ((
																   /**cast int
        **/
																	 long)
																       (momenuva_ity_item))))
      {
	return (true);;
      }
      /**lastunitest:test__4qBFT2X1gshppK **/
    else
      {
	return (false);;
      }
  };
  ;
   /**endblock valid_item_pointer__1tTnuqhhVwG81p **/ }

/**endinline:valid_item_pointer **/




/**inline:int_val_def **/

static inline long
momf_int_val_def (momvalue_t momarg_val0, long momarg_num1)
{
   /**block int_val_def__7MvK2wtkprmW0K **/
  ;
  {
     /**cond cond__8deKjL0VXsIdwK **/
      /**test:test__03GazxE5CNhiq7 **/
    if ( /*is_null */ ((momarg_val0) == NULL))
      {
	return (momarg_num1);;
      }
      /**test:test__2XvZz2NYur3AxV **/
    else if ( /*same_value_prim */
      ((momarg_val0) == ( /*empty_slot_prim */ MOM_EMPTY_SLOT)))
      {
	return (momarg_num1);;
      }
      /**test:test__6dhdACJ3CDqHSK **/
    else if ( /*unsafe_int_mod_prim */
      (( /*cast_pointer_to_int_prim */ (intptr_t) (momarg_val0)) % (2)))
      {
	return ((( /*cast_pointer_to_int_prim */ (intptr_t) (momarg_val0)) /
		 (2)));;
      }
      /**test:test__1XJJgeEkmHtl4G **/
    else if ( /*equal_int_prim */ ((( /**cast int **/ long) (( /*field */ ((momty_anyvalue_ty *) (momarg_val0))->momfi_va_itype))) == ((
																   /**cast int
        **/
																	 long)
																       (momenuva_ity_int))))
      {
	return (( /*field */
		((momty_boxint_ty *) (momarg_val0))->momfi_boxi_int));;
      }
      /**lastunitest:test__4qtdJBc3nMTI8l **/
    else
      {
	return (momarg_num1);;
      }
  };
  ;
   /**endblock int_val_def__7MvK2wtkprmW0K **/ }

/**endinline:int_val_def **/




/**inline:value_hash **/

static inline momhash_t
momf_value_hash (momvalue_t momarg_val0)
{
   /**block value_hash__58npNVtYY45TXT **/
  momty_itype_en momloc_typ0loc = (momty_itype_en /*nothing */ )0;

  ;
  momloc_typ0loc = momf_itype (momarg_val0);
  ;
  {
     /**cond cond__4ycCRe9jL7Rtrn **/
      /**test:test__3UnhsCA7SUAZUt **/
    if ( /*equal_int_prim */ ((( /**cast int **/ long) (momloc_typ0loc)) == ((
									     /**cast int
        **/
									       long)
									     (momenuva_ity_none))))
      {
	return ((				   /**cast hash_t
        **/ momhash_t) (0));;
      }
      /**test:test__0pWd9D47LXAZDA **/
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
      /**test:test__7B6aN7xzxIsYGA **/
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
      /**lastunitest:test__2BrqtzzpAjTrJF **/
    else
      {
	return ((					     /**cast hash_t
		          **/ momhash_t) (((1) +
					   (( /**cast int **/
					     long) (momloc_typ0loc)))));;
  }};
  ;
   /**endblock value_hash__58npNVtYY45TXT **/ }

/**endinline:value_hash **/




/**inline:dyncast_int **/

static inline momvalue_t
momf_dyncast_int (momvalue_t momarg_val0)
{
   /**block dyncast_int__4D96VPaj8eim9n **/
  ;
  {
     /**cond cond__5nI0EE0IjwHR6u **/
      /**test:test__36yV4E2bL0oLR3 **/
    if ( /*equal_int_prim */ ((( /**cast int **/ long) (momf_itype
							(momarg_val0))) ==
			      (( /**cast int **/ long) (momenuva_ity_int))))
      {
	return (momarg_val0);;
      }
      /**lastunitest:test__6vgJhCjCClXkqC **/
    else
      {
	return (NULL);;
      }
  };
  ;
   /**endblock dyncast_int__4D96VPaj8eim9n **/ }

/**endinline:dyncast_int **/




/**inline:dyncast_boxdouble **/

static inline momvalue_t
momf_dyncast_boxdouble (momvalue_t momarg_val0)
{
   /**block dyncast_boxdouble__2G4YZRMsX3aScK **/
  ;
  {
     /**cond cond__43CLvppIj0AY1k **/
      /**test:test__4ZiGh1AjxV4jjv **/
    if ( /*equal_int_prim */ ((( /**cast int **/ long) (momf_itype
							(momarg_val0))) ==
			      (( /**cast int **/
				long) (momenuva_ity_double))))
      {
	return (momarg_val0);;
      }
      /**lastunitest:test__3jGZ3AJpJWbLqu **/
    else
      {
	return (NULL);;
      }
  };
  ;
   /**endblock dyncast_boxdouble__2G4YZRMsX3aScK **/ }

/**endinline:dyncast_boxdouble **/




/**inline:dyncast_boxstring **/

static inline momvalue_t
momf_dyncast_boxstring (momvalue_t momarg_val0)
{
   /**block dyncast_boxstring__0Lvftz4X7T4f9a **/
  ;
  {
     /**cond cond__6ijgNm8dZM8jmR **/
      /**test:test__2c6TpALyMKahEG **/
    if ( /*equal_int_prim */ ((( /**cast int **/ long) (momf_itype
							(momarg_val0))) ==
			      (( /**cast int **/
				long) (momenuva_ity_string))))
      {
	return (momarg_val0);;
      }
      /**lastunitest:test__6ixMupJ8eBkwVw **/
    else
      {
	return (NULL);;
      }
  };
  ;
   /**endblock dyncast_boxstring__0Lvftz4X7T4f9a **/ }

/**endinline:dyncast_boxstring **/




/**inline:dyncast_seqitem **/

static inline momvalue_t
momf_dyncast_seqitem (momvalue_t momarg_val0)
{
   /**block dyncast_seqitem__7JrfVnlafuUuSm **/
  ;
  {
     /**cond cond__91fWd5RL8cJZcU **/
      /**test:test__21Fmiq6KHcf4ci **/
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
      /**lastunitest:test__8TZ87d9iEgwMZu **/
    else
      {
	return (NULL);;
      }
  };
  ;
   /**endblock dyncast_seqitem__7JrfVnlafuUuSm **/ }

/**endinline:dyncast_seqitem **/




/**inline:dyncast_tuple **/

static inline momvalue_t
momf_dyncast_tuple (momvalue_t momarg_val0)
{
   /**block dyncast_tuple__6Lz56XmCYkjDbw **/
  ;
  {
     /**cond cond__951q7MTTLHzd1R **/
      /**test:test__7qL72bETZ31MrE **/
    if ( /*equal_int_prim */ ((( /**cast int **/ long) (momf_itype
							(momarg_val0))) ==
			      (( /**cast int **/ long) (momenuva_ity_tuple))))
      {
	return (momarg_val0);;
      }
      /**lastunitest:test__11YhJHC63u7txf **/
    else
      {
	return (NULL);;
      }
  };
  ;
   /**endblock dyncast_tuple__6Lz56XmCYkjDbw **/ }

/**endinline:dyncast_tuple **/




/**inline:dyncast_set **/

static inline momvalue_t
momf_dyncast_set (momvalue_t momarg_val0)
{
   /**block dyncast_set__5nF5qWTtUtzH8K **/
  ;
  {
     /**cond cond__6Rzerp0GrZlHGs **/
      /**test:test__0Xz79U0AA4t3Ic **/
    if ( /*equal_int_prim */ ((( /**cast int **/ long) (momf_itype
							(momarg_val0))) ==
			      (( /**cast int **/ long) (momenuva_ity_set))))
      {
	return (momarg_val0);;
      }
      /**lastunitest:test__6zvWyzoyWcuz4F **/
    else
      {
	return (NULL);;
      }
  };
  ;
   /**endblock dyncast_set__5nF5qWTtUtzH8K **/ }

/**endinline:dyncast_set **/




/**inline:dyncast_node **/

static inline momvalue_t
momf_dyncast_node (momvalue_t momarg_val0)
{
   /**block dyncast_node__68V8kfxgZfCJSl **/
  ;
  {
     /**cond cond__5TUme4u8Dl1dCE **/
      /**test:test__4Rkdw4afF8etSp **/
    if ( /*equal_int_prim */ ((( /**cast int **/ long) (momf_itype
							(momarg_val0))) ==
			      (( /**cast int **/ long) (momenuva_ity_node))))
      {
	return (momarg_val0);;
      }
      /**lastunitest:test__0iFzqdYKjLJiwM **/
    else
      {
	return (NULL);;
      }
  };
  ;
   /**endblock dyncast_node__68V8kfxgZfCJSl **/ }

/**endinline:dyncast_node **/




/**inline:dyncast_item **/

static inline momvalue_t
momf_dyncast_item (momvalue_t momarg_val0)
{
   /**block dyncast_item__2ZyBbdWWAXnjcH **/
  ;
  {
     /**cond cond__5m49oytX2m75A4 **/
      /**test:test__1bknMBo2oEq7NL **/
    if (momf_valid_item_pointer
	(( /**cast item **/ momitemptr_t) (momarg_val0)
	))
      {
	return (momarg_val0);;
      }
      /**lastunitest:test__6sI8VtYkIcKoDh **/
    else
      {
	return (NULL);;
      }
  };
  ;
   /**endblock dyncast_item__2ZyBbdWWAXnjcH **/ }

/**endinline:dyncast_item **/




/**inline:int_make **/

static inline momvalue_t
momf_int_make (long momarg_num0)
{
   /**block int_make__8AtttY24fI5ubn **/
  ;
  {
     /**cond cond__3U61YU8PT1uLcK **/
      /**test:test__5bnCLKyvPNvcsR **/
    if ( /*small_int_prim */
      (momarg_num0 > -INTPTR_MAX / 4 && momarg_num0 < INTPTR_MAX / 4))
      {
	return (( /**cast value **/
		 momvalue_t) (((((2) * (momarg_num0))) + (1))));;
      }
      /**lastunitest:test__7NnjLVhYH4YIEP **/
    else
      {
      /**block sequence__8yA1Ztd7gBMnNH **/
	momvalue_t momloc_locval = (momvalue_t /*nothing */ ) 0;
	;
	momloc_locval =
	  /*gc_alloc_scalar */ mom_gc_alloc_scalar (sizeof (momty_boxint_ty));

	;
	( /*get */ ((momty_anyvalue_ty *) (momloc_locval))->momfi_va_itype) = (
									   /**cast uint16_t
         **/
										uint16_t)
	  (momenuva_ity_int);
	;
	( /*get */ ((momty_boxint_ty *) (momloc_locval))->momfi_boxi_int) =
	  momarg_num0;

	;
	return (momloc_locval);;;
      /**endblock sequence__8yA1Ztd7gBMnNH **/ }
  };;
   /**endblock int_make__8AtttY24fI5ubn **/ }

/**endinline:int_make **/




/**inline:seqitem_nth **/

static inline momitemptr_t
momf_seqitem_nth (momvalue_t momarg_val0, long momarg_num0)
{
   /**block seqitem_nth__7rdkEIplBfs4gy **/
  ;
  {
     /**cond cond__2Z8K4sXT5RC341 **/
      /**latest:test__0Sa8YKx7k90vFy **/
    if (momf_dyncast_seqitem (momarg_val0))
      {
      /**block sequence__1Sc1C6sxt8Zbt3 **/
	long momloc_nloc = (long /*nothing */ ) 0;
	;
	momloc_nloc =
	  ( /**cast int **/ long) (( /*get */
				   ((momty_anyvalue_ty *) (momarg_val0))->
			 momfi_va_size));

	;
	{
	/**cond cond__1YEEitzucqldLX **/
	 /**latest:test__6rZKVneZJcaLg4 **/
	  if ( /*equal_int_prim */ ((momarg_num0) < (0)))
	    {
	      momarg_num0 = ((momloc_nloc) + (momarg_num0));
	    }
	};

	;
	{
	/**cond cond__6NZEmKT6uGjCxx **/
	 /**latest:test__1bHvsDGewAs1xD **/
	  if ((( /*equal_int_prim */ ((momarg_num0) >= (0)))
	       && ( /*equal_int_prim */ ((momarg_num0) < (momloc_nloc)))))
	    {
	      return (( /*at */
		      ((momty_seqitem_ty *) (momarg_val0))->
	     momfi_seqitem[momarg_num0]));;
	    }
	};
	;
      /**endblock sequence__1Sc1C6sxt8Zbt3 **/ }
  };
  ;
  return (NULL);;;
   /**endblock seqitem_nth__7rdkEIplBfs4gy **/ }

/**endinline:seqitem_nth **/


/// end of generated header file momg_header_module.h

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



/// 73 declarations:

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
{				/*extending enum itype_en */
  momenuva_ity_none_momenfr_itype_en = 0,
  momenuva_ity_int_momenfr_itype_en = 1,
  momenuva_ity_double_momenfr_itype_en = 2,
  momenuva_ity_string_momenfr_itype_en = 3,
  momenuva_ity_item_momenfr_itype_en = 4,
  momenuva_ity_tuple_momenfr_itype_en = 5,
  momenuva_ity_set_momenfr_itype_en = 6,
  momenuva_ity_node_momenfr_itype_en = 7,	/*extended enum itype_en */

  momenuva_xty_lastvalue = 8,
  momenuva_xty_assovaldata = 9,
  momenuva_xty_vectvaldata = 10,
  momenuva_xty_queue = 11,
  momenuva_xty_hashset = 12,
  momenuva_xty_hashmap = 13,
  momenuva_xty_hashassoc = 14,
  momenuva_xty_loader = 15,
  momenuva_xty_dumper = 16,
  momenuva_xty_json = 17,
  momenuva_xty_file = 18,
  momenuva_xty_filebuffer = 19,
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


typedef momhash_t momsigty_signature_item_to_hash_t (momitemptr_t);


static inline momsigty_signature_item_to_hash_t momf_item_hash;


typedef long momsigty_signature_2items_to_int (momitemptr_t, momitemptr_t);


static inline momsigty_signature_2items_to_int momf_item_cmp;


typedef momvalue_t momsigty_signature_value_to_value (momvalue_t);


static inline momsigty_signature_value_to_value momf_dyncast_item;


static inline momsigty_signature_value_to_value momf_dyncast_int;


static inline momsigty_signature_value_to_value momf_dyncast_boxdouble;


static inline momsigty_signature_value_to_value momf_dyncast_boxstring;


static inline momsigty_signature_value_to_value momf_dyncast_seqitem;


static inline momsigty_signature_value_to_value momf_dyncast_tuple;


static inline momsigty_signature_value_to_value momf_dyncast_set;


static inline momsigty_signature_value_to_value momf_dyncast_node;


typedef momvalue_t momsigty_signature_int_to_value (long);


static inline momsigty_signature_int_to_value momf_int_make;


typedef momitemptr_t momsigty_signature_value_int_to_item (momvalue_t, long);


static inline momsigty_signature_value_int_to_item momf_seqitem_nth;


static inline momsigty_signature_value_int_to_item momf_tuple_nth;


static inline momsigty_signature_value_int_to_item momf_set_nth;


typedef long momsigty_signature_value_to_int (momvalue_t);


static inline momsigty_signature_value_to_int momf_seqitem_size;


static inline momsigty_signature_value_to_int momf_tuple_size;


static inline momsigty_signature_value_to_int momf_set_size;


static inline momsigty_signature_value_to_int momf_node_size;


typedef momitemptr_t momsigty_signature_value_to_item (momvalue_t);


static inline momsigty_signature_value_to_item momf_node_conn;


typedef momvalue_t momsigty_signature_value_int_to_value (momvalue_t, long);


static inline momsigty_signature_value_int_to_value momf_node_nth;


static inline momsigty_signature_value_to_int momf_node_metarank;


static inline momsigty_signature_value_to_item momf_node_metaitem;


/// 1 definitions:





/**signature signature_2int64t_to_bool **/




/**signature signature_2uint64t_to_bool **/




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
   /**block valid_value_pointer__81fyTjDDcNkf3J **/
  ;
  {
     /**cond cond__2je2bFhsDpT86L **/
      /**test:test__32aLFoPeZdx1Cs **/
    if ( /*is_null */ ((momarg_val0) == NULL))
      {
	return (false);;
      }
      /**test:test__38jHaTHrltJryu **/
    else if ( /*same_value_prim */
      ((momarg_val0) == ( /*empty_slot_prim */ MOM_EMPTY_SLOT)))
      {
	return (false);;
      }
      /**test:test__2WSuPK151m39fG **/
    else if ( /*unsafe_int_mod_prim */
      (( /*cast_pointer_to_int_prim */ (intptr_t) (momarg_val0)) % (2)))
      {
	return (false);;
      }
      /**lastunitest:test__8lslFtrlBehI0R **/
    else
      {
	return (true);;
      }
  };
  ;
   /**endblock valid_value_pointer__81fyTjDDcNkf3J **/ }

/**endinline:valid_value_pointer **/




/**inline:itype **/

static inline momty_itype_en
momf_itype (momvalue_t momarg_val0)
{
   /**block itype__5ZXYEZby5SDzaN **/
  ;
  {
     /**cond cond__8Y4hc7XFfYq8NM **/
      /**test:test__3fd05KierInJtF **/
    if ( /*is_null */ ((momarg_val0) == NULL))
      {
	return (momenuva_ity_none);;
      }
      /**test:test__4TXGvnHsvamPh1 **/
    else if ( /*same_value_prim */
      ((momarg_val0) == ( /*empty_slot_prim */ MOM_EMPTY_SLOT)))
      {
	return (momenuva_ity_none);;
      }
      /**test:test__0031wop58EVNPP **/
    else if ( /*unsafe_int_mod_prim */
      (( /*cast_pointer_to_int_prim */ (intptr_t) (momarg_val0)) % (2)))
      {
	return (momenuva_ity_int);;
      }
      /**lastunitest:test__8IUirFuqr1x420 **/
    else
      {
	return ((momty_itype_en)
		(( /*field */
		 ((momty_anyvalue_ty *) (momarg_val0))->momfi_va_itype)));;
      }
  };
  ;
   /**endblock itype__5ZXYEZby5SDzaN **/ }

/**endinline:itype **/




/**inline:valid_item_pointer **/

static inline bool
momf_valid_item_pointer (momitemptr_t momarg_itm0)
{
   /**block valid_item_pointer__2xrtjZN0fTXAVN **/
  ;
  {
     /**cond cond__6qJm05vw0r935c **/
      /**test:test__4uufJofKRKuNro **/
    if ( /*not_bool_prim */ !(momf_valid_value_pointer ((  /**cast value
         **/
							  momvalue_t)
							(momarg_itm0))))
      {
	return (false);;
      }
      /**test:test__39viBZygTEi9IX **/
    else if ( /*equal_int_prim */ ((( /**cast int **/ long) (( /*field */ ((momty_anyvalue_ty *) (momarg_itm0))->momfi_va_itype))) == ((
																   /**cast int
        **/
																	 long)
																       (momenuva_ity_item))))
      {
	return (true);;
      }
      /**lastunitest:test__5ifyiSeavbn6i2 **/
    else
      {
	return (false);;
      }
  };
  ;
   /**endblock valid_item_pointer__2xrtjZN0fTXAVN **/ }

/**endinline:valid_item_pointer **/




/**inline:int_val_def **/

static inline long
momf_int_val_def (momvalue_t momarg_val0, long momarg_num1)
{
   /**block int_val_def__7ngu6jieMzKhxd **/
  ;
  {
     /**cond cond__4z3W8owTLED3qD **/
      /**test:test__7tcay7jHglSr3B **/
    if ( /*is_null */ ((momarg_val0) == NULL))
      {
	return (momarg_num1);;
      }
      /**test:test__2jobdKJ3h8zwIr **/
    else if ( /*same_value_prim */
      ((momarg_val0) == ( /*empty_slot_prim */ MOM_EMPTY_SLOT)))
      {
	return (momarg_num1);;
      }
      /**test:test__2HjoDl66r5LsLb **/
    else if ( /*unsafe_int_mod_prim */
      (( /*cast_pointer_to_int_prim */ (intptr_t) (momarg_val0)) % (2)))
      {
	return ((( /*cast_pointer_to_int_prim */ (intptr_t) (momarg_val0)) /
		 (2)));;
      }
      /**test:test__0dVwnFElnicZ0P **/
    else if ( /*equal_int_prim */ ((( /**cast int **/ long) (( /*field */ ((momty_anyvalue_ty *) (momarg_val0))->momfi_va_itype))) == ((
																   /**cast int
        **/
																	 long)
																       (momenuva_ity_int))))
      {
	return (( /*field */
		((momty_boxint_ty *) (momarg_val0))->momfi_boxi_int));;
      }
      /**lastunitest:test__5CGML1k75sNs6e **/
    else
      {
	return (momarg_num1);;
      }
  };
  ;
   /**endblock int_val_def__7ngu6jieMzKhxd **/ }

/**endinline:int_val_def **/




/**inline:value_hash **/

static inline momhash_t
momf_value_hash (momvalue_t momarg_val0)
{
   /**block value_hash__3X8CfLq055h966 **/
  /**1 locals in block:value_hash__3X8CfLq055h966 **/ momty_itype_en
    momloc_typ0loc = (momty_itype_en /*nothing */ )0;;

  ;
  momloc_typ0loc = momf_itype (momarg_val0);
  ;
  {
     /**cond cond__0sy5zWU7ef8m8m **/
      /**test:test__89XIlU60oSmGyb **/
    if ( /*equal_int_prim */ ((( /**cast int **/ long) (momloc_typ0loc)) == ((
									     /**cast int
        **/
									       long)
									     (momenuva_ity_none))))
      {
	return ((				   /**cast hash_t
        **/ momhash_t) (0));;
      }
      /**test:test__3DjnbCBktz9ukf **/
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
      /**test:test__0Ped7vIB0Sk27a **/
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
      /**lastunitest:test__1Vxx2fz9r8R8WS **/
    else
      {
	return ((					     /**cast hash_t
		          **/ momhash_t) (((1) +
					   (( /**cast int **/
					     long) (momloc_typ0loc)))));;
  }};
  ;
   /**endblock value_hash__3X8CfLq055h966 **/ }

/**endinline:value_hash **/




/**inline:item_hash **/

static inline momhash_t
momf_item_hash (momitemptr_t momarg_itm0)
{
   /**block item_hash__5V8eTDTMWRH60F **/
  ;
  {
     /**cond cond__8bjVVK6X00kKz9 **/
      /**test:test__3phL9nJnz6UTe6 **/
    if ( /*not_bool_prim */ !(momf_valid_value_pointer ((  /**cast value
         **/
							  momvalue_t)
							(momarg_itm0))))
      {
	return ((				      /**cast hash_t
        **/ momhash_t) (0));;
      }
      /**test:test__8FtPntJ9hPCCvf **/
    else if ( /*not_bool_prim */ !( /*equal_int_prim */ (((
							 /**cast int
							             **/ long) (( /*get */ ((momty_anyvalue_ty *) (momarg_itm0))->momfi_va_itype))) == ((
											/**cast int
         **/
																			  long)
																			(momenuva_ity_item)))))
      {
	return ((				     /**cast hash_t
        **/ momhash_t) (0));;
      }
      /**lastunitest:test__8cs58KiUutyD8Z **/
    else
      {
	return (( /*get */
		((momty_hashedvalue_ty *) (momarg_itm0))->momfi_hva_hash));;
      }
  };
  ;
   /**endblock item_hash__5V8eTDTMWRH60F **/ }

/**endinline:item_hash **/




/**inline:item_cmp **/

static inline long
momf_item_cmp (momitemptr_t momarg_itm0, momitemptr_t momarg_itm1)
{
   /**block item_cmp__5RimGd2eJ1nW44 **/
  /**2 locals in block:item_cmp__5RimGd2eJ1nW44 **/ momitemptr_t
    momloc_itmvar_l = (momitemptr_t /*nothing */ ) 0;
  momitemptr_t momloc_itmvar_r = (momitemptr_t /*nothing */ ) 0;;

  ;
  momloc_itmvar_l = ( /**cast item **/ momitemptr_t) (momf_dyncast_item
						      (momarg_itm0));
  ;
  momloc_itmvar_r = ( /**cast item **/ momitemptr_t) (momf_dyncast_item
						      (momarg_itm1));
  ;
  {
     /**cond cond__3L2S1VHGoWLIXc **/
      /**test:test__47CqySX0JkJBT5 **/
    if ( /*same_item_prim */ ((momloc_itmvar_l) == (momloc_itmvar_r)))
      {
	return (0);;
      }
      /**test:test__2ip8BDzsjspys7 **/
    else if ( /*is_null */
      ((( /**cast value **/ momvalue_t) (momloc_itmvar_l)) == NULL))
      {
	return (-1);;
      }
      /**test:test__7FjmYlYKiw7Ujq **/
    else if ( /*is_null */
      ((( /**cast value **/ momvalue_t) (momloc_itmvar_r)) == NULL))
      {
	return (1);;
      }
      /**test:test__0NxKJj6kjALpml **/
    else if ( /*equal_int_prim */ ((( /**cast int **/ long) (( /*get */ ((momty_item_ty *) (momloc_itmvar_l))->momfi_itm_hid))) < ((
															      /**cast int
																             **/
																     long)
																   (( /*get */ ((momty_item_ty *) (momloc_itmvar_r))->momfi_itm_hid)))))
      {
	return (-1);;
      }
      /**test:test__58ruCnSKrmuNwP **/
    else if ( /*equal_int_prim */ ((( /**cast int **/ long) (( /*get */ ((momty_item_ty *) (momloc_itmvar_l))->momfi_itm_hid))) > ((
															      /**cast int
																             **/
																     long)
																   (( /*get */ ((momty_item_ty *) (momloc_itmvar_r))->momfi_itm_hid)))))
      {
	return (1);;
      }
      /**test:test__2JxxU3Ge3pWiIY **/
    else if ( /*equal_uint64_prim */
      ((( /*get */ ((momty_item_ty *) (momloc_itmvar_l))->momfi_itm_lid)) <
       (( /*get */ ((momty_item_ty *) (momloc_itmvar_r))->momfi_itm_lid))))
      {
	return (-1);;
      }
      /**test:test__0onTK5HXDD5JwH **/
    else if ( /*greater_uint64_prim */
      ((( /*get */ ((momty_item_ty *) (momloc_itmvar_l))->momfi_itm_lid)) >
       (( /*get */ ((momty_item_ty *) (momloc_itmvar_r))->momfi_itm_lid))))
      {
	return (1);;
      }
      /**lastunitest:test__9c6cECWTuBe7oo **/
    else
      {
	return (0);;
      }
  };
  ;
   /**endblock item_cmp__5RimGd2eJ1nW44 **/ }

/**endinline:item_cmp **/




/**inline:dyncast_int **/

static inline momvalue_t
momf_dyncast_int (momvalue_t momarg_val0)
{
   /**block dyncast_int__7LqIUP0dP7q12C **/
  ;
  {
     /**cond cond__7Gd5VgTJhXN08I **/
      /**test:test__1PhoxuAqjxKhTR **/
    if ( /*equal_int_prim */ ((( /**cast int **/ long) (momf_itype
							(momarg_val0))) ==
			      (( /**cast int **/ long) (momenuva_ity_int))))
      {
	return (momarg_val0);;
      }
      /**lastunitest:test__8yuRFc83wrudyR **/
    else
      {
	return (NULL);;
      }
  };
  ;
   /**endblock dyncast_int__7LqIUP0dP7q12C **/ }

/**endinline:dyncast_int **/




/**inline:dyncast_boxdouble **/

static inline momvalue_t
momf_dyncast_boxdouble (momvalue_t momarg_val0)
{
   /**block dyncast_boxdouble__34En55KkHih3dm **/
  ;
  {
     /**cond cond__1AtPK8msAZVVic **/
      /**test:test__8oUvS8w8Lt3tWD **/
    if ( /*equal_int_prim */ ((( /**cast int **/ long) (momf_itype
							(momarg_val0))) ==
			      (( /**cast int **/
				long) (momenuva_ity_double))))
      {
	return (momarg_val0);;
      }
      /**lastunitest:test__4mAs673KV0LdZT **/
    else
      {
	return (NULL);;
      }
  };
  ;
   /**endblock dyncast_boxdouble__34En55KkHih3dm **/ }

/**endinline:dyncast_boxdouble **/




/**inline:dyncast_boxstring **/

static inline momvalue_t
momf_dyncast_boxstring (momvalue_t momarg_val0)
{
   /**block dyncast_boxstring__7eZyeG82vtCR5G **/
  ;
  {
     /**cond cond__1LIILB4MPviVhy **/
      /**test:test__94cr1avzqVxvA7 **/
    if ( /*equal_int_prim */ ((( /**cast int **/ long) (momf_itype
							(momarg_val0))) ==
			      (( /**cast int **/
				long) (momenuva_ity_string))))
      {
	return (momarg_val0);;
      }
      /**lastunitest:test__2xvy97EAAVLVgz **/
    else
      {
	return (NULL);;
      }
  };
  ;
   /**endblock dyncast_boxstring__7eZyeG82vtCR5G **/ }

/**endinline:dyncast_boxstring **/




/**inline:dyncast_seqitem **/

static inline momvalue_t
momf_dyncast_seqitem (momvalue_t momarg_val0)
{
   /**block dyncast_seqitem__3JKmmyTwSehypL **/
  ;
  {
     /**cond cond__8ssj6kRPJETBAr **/
      /**test:test__7rm6hmvXjFJGuh **/
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
      /**lastunitest:test__6zV4aa3GX2o92e **/
    else
      {
	return (NULL);;
      }
  };
  ;
   /**endblock dyncast_seqitem__3JKmmyTwSehypL **/ }

/**endinline:dyncast_seqitem **/




/**inline:dyncast_tuple **/

static inline momvalue_t
momf_dyncast_tuple (momvalue_t momarg_val0)
{
   /**block dyncast_tuple__8aowuFCPprqTE2 **/
  ;
  {
     /**cond cond__3ck1YiXouWk3na **/
      /**test:test__0BqaAtCotMYr6G **/
    if ( /*equal_int_prim */ ((( /**cast int **/ long) (momf_itype
							(momarg_val0))) ==
			      (( /**cast int **/ long) (momenuva_ity_tuple))))
      {
	return (momarg_val0);;
      }
      /**lastunitest:test__970ph6zdukCYcn **/
    else
      {
	return (NULL);;
      }
  };
  ;
   /**endblock dyncast_tuple__8aowuFCPprqTE2 **/ }

/**endinline:dyncast_tuple **/




/**inline:dyncast_set **/

static inline momvalue_t
momf_dyncast_set (momvalue_t momarg_val0)
{
   /**block dyncast_set__42ckZKL70TTKHP **/
  ;
  {
     /**cond cond__4dShfI1KBbEF9Z **/
      /**test:test__8mF5kAWenhmlEa **/
    if ( /*equal_int_prim */ ((( /**cast int **/ long) (momf_itype
							(momarg_val0))) ==
			      (( /**cast int **/ long) (momenuva_ity_set))))
      {
	return (momarg_val0);;
      }
      /**lastunitest:test__4mDY1aqrYNAzF1 **/
    else
      {
	return (NULL);;
      }
  };
  ;
   /**endblock dyncast_set__42ckZKL70TTKHP **/ }

/**endinline:dyncast_set **/




/**inline:dyncast_node **/

static inline momvalue_t
momf_dyncast_node (momvalue_t momarg_val0)
{
   /**block dyncast_node__3E2vmdYyTHoMfR **/
  ;
  {
     /**cond cond__1npo1V5zzxtEtA **/
      /**test:test__8sl0BK55fMXmVX **/
    if ( /*equal_int_prim */ ((( /**cast int **/ long) (momf_itype
							(momarg_val0))) ==
			      (( /**cast int **/ long) (momenuva_ity_node))))
      {
	return (momarg_val0);;
      }
      /**lastunitest:test__0Iv8GB86bMXYkJ **/
    else
      {
	return (NULL);;
      }
  };
  ;
   /**endblock dyncast_node__3E2vmdYyTHoMfR **/ }

/**endinline:dyncast_node **/




/**inline:dyncast_item **/

static inline momvalue_t
momf_dyncast_item (momvalue_t momarg_val0)
{
   /**block dyncast_item__7NFvWke2ptiCnk **/
  ;
  {
     /**cond cond__6lr3xL66lI2Z5i **/
      /**test:test__0hNdxJqfDhwIAd **/
    if (momf_valid_item_pointer
	(( /**cast item **/ momitemptr_t) (momarg_val0)
	))
      {
	return (momarg_val0);;
      }
      /**lastunitest:test__1ekHharbwEGA6N **/
    else
      {
	return (NULL);;
      }
  };
  ;
   /**endblock dyncast_item__7NFvWke2ptiCnk **/ }

/**endinline:dyncast_item **/




/**inline:int_make **/

static inline momvalue_t
momf_int_make (long momarg_num0)
{
   /**block int_make__0JFxLmVVS2zgNW **/
  ;
  {
     /**cond cond__0JYeqaDeIYaZGL **/
      /**test:test__2kUM0g7KYR5eM2 **/
    if ( /*small_int_prim */
      (momarg_num0 > -INTPTR_MAX / 4 && momarg_num0 < INTPTR_MAX / 4))
      {
	return (( /**cast value **/
		 momvalue_t) (((((2) * (momarg_num0))) + (1))));;
      }
      /**lastunitest:test__0awihFol5jgjqg **/
    else
      {
      /**block sequence__5oTybmSy8WCmPC **/
	/**1 locals in block:sequence__5oTybmSy8WCmPC **/ momvalue_t
	  momloc_locval = (momvalue_t /*nothing */ ) 0;;

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
      /**endblock sequence__5oTybmSy8WCmPC **/ }
  };;
   /**endblock int_make__0JFxLmVVS2zgNW **/ }

/**endinline:int_make **/




/**inline:seqitem_nth **/

static inline momitemptr_t
momf_seqitem_nth (momvalue_t momarg_val0, long momarg_num0)
{
   /**block seqitem_nth__4kcMjtxDDTGBNI **/
  ;
  {
     /**cond cond__6i2EVmDm2EmPaY **/
      /**latest:test__7AAc2AtapIdiDb **/
    if (momf_dyncast_seqitem (momarg_val0))
      {
      /**block sequence__4IBtXhcbtmo8Ip **/
	/**1 locals in block:sequence__4IBtXhcbtmo8Ip **/ long momloc_nloc =
	  (long /*nothing */ ) 0;;

	;
	momloc_nloc =
	  ( /**cast int **/ long) (( /*get */
				   ((momty_anyvalue_ty *) (momarg_val0))->
			 momfi_va_size));

	;
	{
	/**cond cond__8xyMT0a9MDGdiG **/
	 /**latest:test__6jDXekXmnAYXUC **/
	  if ( /*equal_int_prim */ ((momarg_num0) < (0)))
	    {
	      momarg_num0 = ((momloc_nloc) + (momarg_num0));
	    }
	};

	;
	{
	/**cond cond__7RAACMlkZ7Ex8C **/
	 /**latest:test__2jIHu8z0uETFxh **/
	  if ((( /*greater_int_prim */ ((momarg_num0) >= (0)))
	       && ( /*equal_int_prim */ ((momarg_num0) < (momloc_nloc)))))
	    {
	      return (( /*at */
		      ((momty_seqitem_ty *) (momarg_val0))->
	     momfi_seqitem[momarg_num0]));;
	    }
	};
	;
      /**endblock sequence__4IBtXhcbtmo8Ip **/ }
  };
  ;
  return (NULL);;;
   /**endblock seqitem_nth__4kcMjtxDDTGBNI **/ }

/**endinline:seqitem_nth **/




/**inline:tuple_nth **/

static inline momitemptr_t
momf_tuple_nth (momvalue_t momarg_val0, long momarg_num0)
{
   /**block tuple_nth__6Db598XrW0cWi7 **/
  ;
  {
     /**cond cond__6PqvmjTNGteezP **/
      /**latest:test__57l8TNBdnT6u2t **/
    if (momf_dyncast_tuple (momarg_val0))
      {
      /**block sequence__22ukS5jpdBDwaW **/
	/**1 locals in block:sequence__22ukS5jpdBDwaW **/ long momloc_nloc =
	  (long /*nothing */ ) 0;;

	;
	momloc_nloc =
	  ( /**cast int **/ long) (( /*get */
				   ((momty_anyvalue_ty *) (momarg_val0))->
			 momfi_va_size));

	;
	{
	/**cond cond__7CGCIr9pwyWKNN **/
	 /**latest:test__1uAS2wI4JUBFWE **/
	  if ( /*equal_int_prim */ ((momarg_num0) < (0)))
	    {
	      momarg_num0 = ((momloc_nloc) + (momarg_num0));
	    }
	};

	;
	{
	/**cond cond__4i6fgoM1VWoiqS **/
	 /**latest:test__5BWKwLaEKwCtNL **/
	  if ((( /*greater_int_prim */ ((momarg_num0) >= (0)))
	       && ( /*equal_int_prim */ ((momarg_num0) < (momloc_nloc)))))
	    {
	      return (( /*at */
		      ((momty_seqitem_ty *) (momarg_val0))->
	     momfi_seqitem[momarg_num0]));;
	    }
	};
	;
      /**endblock sequence__22ukS5jpdBDwaW **/ }
  };
  ;
  return (NULL);;;
   /**endblock tuple_nth__6Db598XrW0cWi7 **/ }

/**endinline:tuple_nth **/




/**inline:set_nth **/

static inline momitemptr_t
momf_set_nth (momvalue_t momarg_val0, long momarg_num0)
{
   /**block set_nth__651AnxnVZjyVnY **/
  ;
  {
     /**cond cond__7aEBJpqghrj3p6 **/
      /**latest:test__0LUggoJVidVncD **/
    if (momf_dyncast_set (momarg_val0))
      {
      /**block sequence__08ipPa0U033aha **/
	/**1 locals in block:sequence__08ipPa0U033aha **/ long momloc_nloc =
	  (long /*nothing */ ) 0;;

	;
	momloc_nloc =
	  ( /**cast int **/ long) (( /*get */
				   ((momty_anyvalue_ty *) (momarg_val0))->
			 momfi_va_size));

	;
	{
	/**cond cond__8pL7AjA15XJJHD **/
	 /**latest:test__857Cp7oIjAwSmv **/
	  if ( /*equal_int_prim */ ((momarg_num0) < (0)))
	    {
	      momarg_num0 = ((momloc_nloc) + (momarg_num0));
	    }
	};

	;
	{
	/**cond cond__2w2BZKyv0VIwh2 **/
	 /**latest:test__6xY0GFlalcuYgr **/
	  if ((( /*greater_int_prim */ ((momarg_num0) >= (0)))
	       && ( /*equal_int_prim */ ((momarg_num0) < (momloc_nloc)))))
	    {
	      return (( /*at */
		      ((momty_seqitem_ty *) (momarg_val0))->
	     momfi_seqitem[momarg_num0]));;
	    }
	};
	;
      /**endblock sequence__08ipPa0U033aha **/ }
  };
  ;
  return (NULL);;;
   /**endblock set_nth__651AnxnVZjyVnY **/ }

/**endinline:set_nth **/




/**inline:seqitem_size **/

static inline long
momf_seqitem_size (momvalue_t momarg_val0)
{
   /**block seqitem_size__3nrDmuFEU9g3vE **/
  ;
  {
     /**cond cond__62FFGEjxgBuBX5 **/
      /**test:test__2VgFVWTdXD9Nwf **/
    if (momf_dyncast_seqitem (momarg_val0))
      {
	return ((					    /**cast int
		          **/ long) (( /*get */
				     ((momty_anyvalue_ty *) (momarg_val0))->
			   momfi_va_size)));;
      }
      /**lastunitest:test__3kzpissfqqHxYu **/
    else
      {
	return (0);;
      }
  };
  ;
   /**endblock seqitem_size__3nrDmuFEU9g3vE **/ }

/**endinline:seqitem_size **/




/**inline:tuple_size **/

static inline long
momf_tuple_size (momvalue_t momarg_val0)
{
   /**block tuple_size__1toDjyoxE1oA5H **/
  ;
  {
     /**cond cond__6fZlsgd7HFNetw **/
      /**test:test__7cqmxrGh9A2JlW **/
    if (momf_dyncast_tuple (momarg_val0))
      {
	return ((					  /**cast int
		          **/ long) (( /*get */
				     ((momty_anyvalue_ty *) (momarg_val0))->
			   momfi_va_size)));;
      }
      /**lastunitest:test__4CAXHIX7tP4B9F **/
    else
      {
	return (0);;
      }
  };
  ;
   /**endblock tuple_size__1toDjyoxE1oA5H **/ }

/**endinline:tuple_size **/




/**inline:set_size **/

static inline long
momf_set_size (momvalue_t momarg_val0)
{
   /**block set_size__5gJV37ruDuiA4e **/
  ;
  {
     /**cond cond__7DddF1pR4Jh5bo **/
      /**test:test__2pY3G0JLVDLXt2 **/
    if (momf_dyncast_set (momarg_val0))
      {
	return ((					/**cast int
		          **/ long) (( /*get */
				     ((momty_anyvalue_ty *) (momarg_val0))->
			   momfi_va_size)));;
      }
      /**lastunitest:test__3ykHLDi1eohUSL **/
    else
      {
	return (0);;
      }
  };
  ;
   /**endblock set_size__5gJV37ruDuiA4e **/ }

/**endinline:set_size **/




/**inline:node_size **/

static inline long
momf_node_size (momvalue_t momarg_val0)
{
   /**block node_size__8vCyxZJmYDsueN **/
  ;
  {
     /**cond cond__0cl2BRWlUjSYMu **/
      /**test:test__0EwUGdrJxFieFG **/
    if (momf_dyncast_node (momarg_val0))
      {
	return ((					 /**cast int
		          **/ long) (( /*get */
				     ((momty_anyvalue_ty *) (momarg_val0))->
			   momfi_va_size)));;
      }
      /**lastunitest:test__1NqDlMcEiCjEMh **/
    else
      {
	return (0);;
      }
  };
  ;
   /**endblock node_size__8vCyxZJmYDsueN **/ }

/**endinline:node_size **/




/**inline:node_conn **/

static inline momitemptr_t
momf_node_conn (momvalue_t momarg_val0)
{
   /**block node_conn__4pmMlB7A8WweG9 **/
  ;
  {
     /**cond cond__2lcX7EgCwtCplz **/
      /**test:test__0eck1UsWcgS6qL **/
    if (momf_dyncast_node (momarg_val0))
      {
	return (( /*get */
		((momty_node_ty *) (momarg_val0))->momfi_nod_connitm));;
      }
      /**lastunitest:test__78NTnXZaYMcZ6X **/
    else
      {
	return (NULL);;
      }
  };
  ;
   /**endblock node_conn__4pmMlB7A8WweG9 **/ }

/**endinline:node_conn **/




/**inline:node_nth **/

static inline momvalue_t
momf_node_nth (momvalue_t momarg_val0, long momarg_num0)
{
   /**block node_nth__1MhWJv6ouTX3is **/
  ;
  {
     /**cond cond__3EhHvheFGo6Nfp **/
      /**latest:test__2bsf2h5d1xq2IA **/
    if (momf_dyncast_node (momarg_val0))
      {
      /**block sequence__1MZwzdDLAy9q5q **/
	/**1 locals in block:sequence__1MZwzdDLAy9q5q **/ long momloc_nloc =
	  (long /*nothing */ ) 0;;

	;
	momloc_nloc =
	  ( /**cast int **/ long) (( /*get */
				   ((momty_anyvalue_ty *) (momarg_val0))->
			 momfi_va_size));

	;
	{
	/**cond cond__3jVIGHdp1xa1af **/
	 /**latest:test__6Gztdz2wUd2iTz **/
	  if ( /*equal_int_prim */ ((momarg_num0) < (0)))
	    {
	      momarg_num0 = ((momloc_nloc) + (momarg_num0));
	    }
	};

	;
	{
	/**cond cond__1vVlrMp1DFfT2v **/
	 /**latest:test__40HKktz9h4Sf09 **/
	  if ((( /*greater_int_prim */ ((momarg_num0) >= (0)))
	       && ( /*equal_int_prim */ ((momarg_num0) < (momloc_nloc)))))
	    {
	      return (( /*at */
		      ((momty_node_ty *) (momarg_val0))->
	     momfi_nod_sons[momarg_num0]));;
	    }
	};
	;
      /**endblock sequence__1MZwzdDLAy9q5q **/ }
  };
  ;
  return (NULL);;;
   /**endblock node_nth__1MhWJv6ouTX3is **/ }

/**endinline:node_nth **/




/**inline:node_metarank **/

static inline long
momf_node_metarank (momvalue_t momarg_val0)
{
   /**block node_metarank__0ykiFoLZ3aMhVu **/
  ;
  {
     /**cond cond__0JECp9hpYYWaM7 **/
      /**test:test__36xViyfSAeumVS **/
    if (momf_dyncast_node (momarg_val0))
      {
	return (( /*get */
		((momty_node_ty *) (momarg_val0))->momfi_nod_metarank));;
      }
      /**lastunitest:test__8CqMzrxPos7k7S **/
    else
      {
	return (0);;
      }
  };
  ;
   /**endblock node_metarank__0ykiFoLZ3aMhVu **/ }

/**endinline:node_metarank **/




/**inline:node_metaitem **/

static inline momitemptr_t
momf_node_metaitem (momvalue_t momarg_val0)
{
   /**block node_metaitem__2fNZC3x1WjCaaa **/
  ;
  {
     /**cond cond__3dqYT3FIRtT2xF **/
      /**test:test__7NKNmay6UpWXa1 **/
    if (momf_dyncast_node (momarg_val0))
      {
	return (( /*get */
		((momty_node_ty *) (momarg_val0))->momfi_nod_metaitem));;
      }
      /**lastunitest:test__5UM2RTrIgUeEou **/
    else
      {
	return (NULL);;
      }
  };
  ;
   /**endblock node_metaitem__2fNZC3x1WjCaaa **/ }

/**endinline:node_metaitem **/


/// end of generated header file momg_header_module.h

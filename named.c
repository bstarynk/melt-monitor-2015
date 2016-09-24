// file named.c - name of objects

/**   Copyright (C) 2016  Basile Starynkevitch and later the FSF
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

#include "meltmoni.h"

// we use a binary balanced tree (similar to red-black tree)
// shamelessly copied from Ocaml's stdlib/map.ml by X.Leroy
// actually, this code should look very close to
// https://github.com/bstarynk/misc-basile/blob/master/basilemap.ml
// which is itself a slight rewriting of X.Leroy's map.ml
#define MOM_NN_MAX_HEIGHT 256
#define MOM_NNAME_MAGIC 0x18f1  /*6385 */
typedef struct mom_nnode_st mom_nnode_ty;
struct mom_nnode_st
{
  uint16_t nn_magic;            /* always MOM_NNAME_MAGIC */
  uint16_t nn_height;
  mom_nnode_ty *nn_left;
  mo_stringvalue_ty *nn_name;
  mo_objref_t nn_objref;
  mom_nnode_ty *nn_right;
};
static mom_nnode_ty *mom_rootnnode;
// we also use an association from named-objects to their names
static mo_assovaldatapayl_ty *mom_nameassop;

bool
mom_valid_name (const char *nam)
{
  if (!nam || nam == MOM_EMPTY_SLOT)
    return false;
  if (!((nam[0] >= 'a' && nam[0] <= 'z') || (nam[0] >= 'A' && nam[0] <= 'Z')))
    return false;
  for (const char *p = nam + 1; *p; p++)
    {
      if (p > nam + MOM_NAME_MAXLEN)
        return false;
      if (isalnum (*p))
        continue;
      if (*p == '_')
        {
          if (p[-1] == '_')
            return false;
          if (!isalnum (p[1]))
            return false;
        }
    }
  return true;
}                               // end mom_valid_name


mo_value_t
mo_objref_namev (mo_objref_t obr)
{
  if (!mo_dyncast_objref (obr))
    return NULL;
  mo_value_t r = mo_assoval_get (mom_nameassop, obr);
  MOM_ASSERTPRINTF (r == NULL || mo_dyncast_string (r), "bad name");
  return r;
}                               /* end mo_objref_namev */

void
mo_reserve_names (unsigned gap)
{
  mom_nameassop = mo_assoval_reserve (mom_nameassop, gap);
}                               /* end mo_reserve_names */

mo_value_t
mo_named_objects_set (void)
{
  return mo_assoval_keys_set (mom_nameassop);
}                               /* end mo_named_objects_set */


static inline bool
mom_nnode_exists (const mom_nnode_ty * n)
{
  if (!n)
    return false;
  MOM_ASSERTPRINTF (n->nn_magic == MOM_NNAME_MAGIC, "bad n@%p", n);
  MOM_ASSERTPRINTF (mo_dyncast_string (n->nn_name), "bad name in n@%p", n);
  MOM_ASSERTPRINTF (n->nn_objref == NULL
                    || mo_dyncast_objref (n->nn_objref), "bad objref in n@%p",
                    n);
  return true;
}                               /* end mom_nnode_exists */

static inline unsigned
mom_nnode_height (const mom_nnode_ty * n)
{
  if (!n)
    return 0;
  MOM_ASSERTPRINTF (n->nn_magic == MOM_NNAME_MAGIC && n->nn_height > 0,
                    "bad n@%p", n);
  return n->nn_height;
}

static inline mom_nnode_ty *
mom_nnode_left (const mom_nnode_ty * n)
{
  if (!n)
    return NULL;
  MOM_ASSERTPRINTF (n->nn_magic == MOM_NNAME_MAGIC, "bad n@%p", n);
  return n->nn_left;
}                               /* end mom_nnode_left */

static inline mom_nnode_ty *
mom_nnode_right (const mom_nnode_ty * n)
{
  if (!n)
    return NULL;
  MOM_ASSERTPRINTF (n->nn_magic == MOM_NNAME_MAGIC, "bad n@%p", n);
  return n->nn_right;
}                               /* end mom_nnode_right */

static inline mo_stringvalue_ty *
mom_nnode_name (const mom_nnode_ty * n)
{
  if (!n)
    return NULL;
  MOM_ASSERTPRINTF (n->nn_magic == MOM_NNAME_MAGIC, "bad n@%p", n);
  MOM_ASSERTPRINTF (mo_dyncast_string (n->nn_name), "bad name in n@%p", n);
  return n->nn_name;
}                               /* end mom_nnode_name */

static inline mo_objref_t
mom_nnode_objref (const mom_nnode_ty * n)
{
  if (!n)
    return NULL;
  MOM_ASSERTPRINTF (n->nn_magic == MOM_NNAME_MAGIC, "bad n@%p", n);
  return n->nn_objref;
}                               /* end of mom_nnode_objref */

static inline mom_nnode_ty *
mom_nnode_create (mom_nnode_ty * left, mo_stringvalue_ty * namev,
                  mo_objref_t objr, mom_nnode_ty * right)
{
  mom_nnode_ty *n = mom_gc_alloc (sizeof (mom_nnode_ty));
  MOM_ASSERTPRINTF (!left
                    || (left->nn_magic == MOM_NNAME_MAGIC
                        && left->nn_height < MOM_NN_MAX_HEIGHT),
                    "mom_nnode_create: bad left@%p", left);
  MOM_ASSERTPRINTF (!right
                    || (right->nn_magic == MOM_NNAME_MAGIC
                        && right->nn_height < MOM_NN_MAX_HEIGHT),
                    "mom_nnode_create: bad right@%p", right);
  MOM_ASSERTPRINTF (mo_dyncast_string (namev) != NULL,
                    "mom_nnode_create: nil namev");
  MOM_ASSERTPRINTF (!objr || mo_dyncast_objref (objr),
                    "mom_nnode_create: bad objr");
  n->nn_magic = MOM_NNAME_MAGIC;
  n->nn_left = left;
  n->nn_right = right;
  n->nn_name = namev;
  n->nn_objref = objr;
  unsigned lefth = mom_nnode_height (left);
  unsigned righth = mom_nnode_height (right);
  n->nn_height = (lefth > righth) ? (lefth + 1) : (righth + 1);
  return n;
}                               /* end mom_nnode_create */

static inline mom_nnode_ty *
mom_nnode_singleton (mo_stringvalue_ty * name, mo_objref_t objr)
{
  return mom_nnode_create (NULL, name, objr, NULL);
}

/* end mom_nnode_singleton */

static mom_nnode_ty *
mom_nnode_balance (mom_nnode_ty * left,
                   mo_stringvalue_ty * name, mo_objref_t objr,
                   mom_nnode_ty * right)
{
  MOM_ASSERTPRINTF (!left || mom_nnode_exists (left), "bad left");
  MOM_ASSERTPRINTF (!right || mom_nnode_exists (right), "bad right");
  MOM_ASSERTPRINTF (mo_dyncast_string (name), "bad name");
  MOM_ASSERTPRINTF (!objr || mo_dyncast_objref (objr), "bad objr");
  unsigned lefth = mom_nnode_height (left);
  unsigned righth = mom_nnode_height (right);
  if (lefth > righth + 2)
    {
      MOM_ASSERTPRINTF (mom_nnode_exists (left), "nil left");
      if (mom_nnode_height (left->nn_left) >=
          mom_nnode_height (left->nn_right))
        return mom_nnode_create (left->nn_left,
                                 left->nn_name, left->nn_objref,
                                 mom_nnode_create (left->nn_right,
                                                   name, objr, right));
      else
        {
          mom_nnode_ty *lr = left->nn_right;
          MOM_ASSERTPRINTF (mom_nnode_exists (lr), "nil right of left");
          return
            mom_nnode_create (mom_nnode_create (left->nn_left,
                                                left->nn_name,
                                                left->nn_objref, lr->nn_left),
                              lr->nn_name, lr->nn_objref,
                              mom_nnode_create (lr->nn_right, name, objr,
                                                right));
        }
    }
  else if (righth > lefth + 2)
    {
      MOM_ASSERTPRINTF (mom_nnode_exists (right), "nil right");
      mom_nnode_ty *rl = right->nn_left;
      mom_nnode_ty *rr = right->nn_right;
      if (mom_nnode_height (rr) >= mom_nnode_height (rl))
        return mom_nnode_create (mom_nnode_create (left,
                                                   name, objr,
                                                   rl),
                                 right->nn_name, right->nn_objref, rr);
      else
        {
          MOM_ASSERTPRINTF (mom_nnode_exists (rl), "nil rl");
          mom_nnode_ty *rll = rl->nn_left;
          mom_nnode_ty *rlr = rl->nn_right;
          return mom_nnode_create (mom_nnode_create (left,
                                                     name, objr,
                                                     rll),
                                   rl->nn_name, rl->nn_objref,
                                   mom_nnode_create (rlr,
                                                     right->nn_name,
                                                     right->nn_objref, rr));
        }
    }
  else
    return mom_nnode_create (left, name, objr, right);
}                               /* end mom_nnode_balance */

static mom_nnode_ty *
mom_nnode_add (mo_stringvalue_ty * nam, mo_objref_t obr, mom_nnode_ty * n)
{
  MOM_ASSERTPRINTF (mo_dyncast_string (nam) != NULL, "bad nam");
  MOM_ASSERTPRINTF (!obr || mo_dyncast_objref (obr), "bad obr");
  if (!n)
    return mom_nnode_create (NULL, nam, obr, NULL);
  MOM_ASSERTPRINTF (mom_nnode_exists (n), "bad x");
  mom_nnode_ty *l = mom_nnode_left (n);
  mom_nnode_ty *r = mom_nnode_right (n);
  mo_stringvalue_ty *d = mom_nnode_name (n);
  MOM_ASSERTPRINTF (mo_dyncast_string (d), "bad name d in x");
  int cmp = strcmp (nam->mo_cstr, d->mo_cstr);
  if (!cmp)
    return n;
  else if (cmp < 0)
    {
      mom_nnode_ty *l = mom_nnode_left (n);
      mom_nnode_ty *r = mom_nnode_right (n);
      mom_nnode_ty *ll = mom_nnode_add (nam, obr, l);
      if (l == ll)
        return n;
      else
        return mom_nnode_balance (ll, n->nn_name, n->nn_objref, r);
    }
  else                          /*cmp>0 */
    {
      mom_nnode_ty *rr = mom_nnode_add (nam, obr, r);
      if (r == rr)
        return n;
      else
        return mom_nnode_balance (l, n->nn_name, n->nn_objref, rr);
    }
}                               /* end of mom_nnode_add */

static mom_nnode_ty *
mom_nnode_find (mo_stringvalue_ty * x, mom_nnode_ty * n)
{
  if (!mo_dyncast_string (x))
    return NULL;
  if (!n)
    return NULL;
  MOM_ASSERTPRINTF (mom_nnode_exists (n), "bad n");
  int cmp = strcmp (x->mo_cstr, n->nn_name->mo_cstr);
  if (!cmp)
    return n;
  if (cmp < 0)
    return mom_nnode_find (x, n->nn_left);
  else
    return mom_nnode_find (x, n->nn_right);
}

static mom_nnode_ty *
mom_nnode_strfind (const char *s, mom_nnode_ty * n)
{
  if (!s)
    return NULL;
  if (!n)
    return NULL;
  MOM_ASSERTPRINTF (mom_nnode_exists (n), "bad n");
  int cmp = strcmp (s, n->nn_name->mo_cstr);
  if (!cmp)
    return n;
  if (cmp < 0)
    return mom_nnode_strfind (s, n->nn_left);
  else
    return mom_nnode_strfind (s, n->nn_right);
}                               /* end mom_nnode_strfind */


static mom_nnode_ty *
mom_nnode_min_binding (mom_nnode_ty * n)
{
  if (!n)
    return NULL;
  MOM_ASSERTPRINTF (mom_nnode_exists (n), "bad n");
  if (n->nn_left == NULL)
    return n;
  return mom_nnode_min_binding (n->nn_left);
}                               /* end mom_nnode_min_binding */

static mom_nnode_ty *
mom_nnode_max_binding (mom_nnode_ty * n)
{
  if (!n)
    return NULL;
  MOM_ASSERTPRINTF (mom_nnode_exists (n), "bad n");
  if (n->nn_right == NULL)
    return n;
  return mom_nnode_max_binding (n->nn_right);
}                               /* end mom_nnode_max_binding */

static mom_nnode_ty *
mom_nnode_remove_min_binding (mom_nnode_ty * n)
{
  if (!n)
    return NULL;
  MOM_ASSERTPRINTF (mom_nnode_exists (n), "bad n");
  if (n->nn_left == NULL)
    return n->nn_right;
  return mom_nnode_balance (mom_nnode_remove_min_binding (n->nn_left),
                            n->nn_name, n->nn_objref, n->nn_right);
}                               /* end mom_nnode_remove_min_binding */

/* in X.Leroy's code there are two merge functions -one internal,
   another public-, this is the internal first */
static mom_nnode_ty *
mom_nnode_merge_trees (mom_nnode_ty * t1, mom_nnode_ty * t2)
{
  if (t1 == NULL && t2 == NULL)
    return NULL;
  if (t1 == NULL)
    {
      MOM_ASSERTPRINTF (mom_nnode_exists (t2), "bad t2");
      return t2;
    };
  if (t2 == NULL)
    {
      MOM_ASSERTPRINTF (mom_nnode_exists (t1), "bad t1");
      return t1;
    };
  mom_nnode_ty *mint2 = mom_nnode_min_binding (t2);
  MOM_ASSERTPRINTF (mom_nnode_exists (mint2), "bad mint2");
  return mom_nnode_balance (t1, mint2->nn_name, mint2->nn_objref,
                            mom_nnode_remove_min_binding (t2));
}                               /* end of mom_nnode_merge_trees */


static mom_nnode_ty *
mom_nnode_remove (mo_stringvalue_ty * xn, mom_nnode_ty * nd)
{
  MOM_ASSERTPRINTF (mo_dyncast_string (xn), "bad xn");
  if (!nd)
    return NULL;
  MOM_ASSERTPRINTF (mom_nnode_exists (nd), "bad nd");
  int cmp = strcmp (xn->mo_cstr, nd->nn_name->mo_cstr);
  if (cmp == 0)
    return mom_nnode_merge_trees (nd->nn_left, nd->nn_right);
  else if (cmp < 0)
    return mom_nnode_balance (mom_nnode_remove (xn, nd->nn_left),
                              nd->nn_name, nd->nn_objref, nd->nn_right);
  else
    return mom_nnode_balance (nd->nn_left,
                              nd->nn_name, nd->nn_objref,
                              mom_nnode_remove (xn, nd->nn_right));
}                               /* end of mom_nnode_remove */

// to iterate on every node
typedef void mom_nnode_iterfun_sigt (const mom_nnode_ty * nd, void *data);

static void
mom_nnode_iter (mom_nnode_iterfun_sigt * fun, void *data, mom_nnode_ty * nd)
{
  if (!fun)
    return;
  if (nd == NULL)
    return;
  MOM_ASSERTPRINTF (mom_nnode_exists (nd), "bad nd");
  if (nd->nn_left)
    mom_nnode_iter (fun, data, nd->nn_left);
  (*fun) (nd, data);
  if (nd->nn_right)
    mom_nnode_iter (fun, data, nd->nn_right);
}                               /* end mom_nnode_iter */

// to repeat on every node, till the function gives true to stop or
// false to continue; return true if it has been stopped
typedef bool mom_nnode_repeatfun_sigt (const mom_nnode_ty * nd, void *data);
static bool
mom_nnode_repeat (mom_nnode_repeatfun_sigt * fun, void *data,
                  mom_nnode_ty * nd)
{
  if (!fun)
    return true;
  if (nd == NULL)
    return false;
  MOM_ASSERTPRINTF (mom_nnode_exists (nd), "bad nd");
  if (nd->nn_left)
    {
      if (mom_nnode_repeat (fun, data, nd->nn_left))
        return true;
    }
  if ((*fun) (nd, data))
    return true;
  if (nd->nn_right)
    {
      return mom_nnode_repeat (fun, data, nd->nn_right);
    }
  return false;
}                               /* end of mom_nnode_repeat */

/** The below comment was originally by X.Leroy in his stdlib/map.ml:

  (* Beware: those two functions assume that the added k is *strictly*
       smaller (or bigger) than all the present keys in the tree; it
       does not test for equality with the current min (or max) key.

       Indeed, they are only used during the "join" operation which
       respects this precondition.
  *)

 **/
static mom_nnode_ty *
mom_nnode_add_min_binding_internal (mo_stringvalue_ty * kn, mo_objref_t vo,
                                    mom_nnode_ty * nd)
{
  if (!nd)
    return mom_nnode_singleton (kn, vo);
  MOM_ASSERTPRINTF (mom_nnode_exists (nd), "bad nd");
  return
    mom_nnode_balance (mom_nnode_add_min_binding_internal
                       (kn, vo, nd->nn_left), nd->nn_name, nd->nn_objref,
                       nd->nn_right);
}                               /* end of mom_nnode_add_min_binding_internal */

static mom_nnode_ty *
mom_nnode_add_max_binding_internal (mo_stringvalue_ty * kn, mo_objref_t vo,
                                    mom_nnode_ty * nd)
{
  if (!nd)
    return mom_nnode_singleton (kn, vo);
  MOM_ASSERTPRINTF (mom_nnode_exists (nd), "bad nd");
  return mom_nnode_balance (nd->nn_left,
                            nd->nn_name, nd->nn_objref,
                            mom_nnode_add_max_binding_internal (kn, vo,
                                                                nd->nn_right));
}                               /* end mom_nnode_add_max_binding_internal */

/** Comment by X.Leroy:
  (* Same as create and bal, but no assumptions are made on the
       relative heights of l and r. *)
**/
static mom_nnode_ty *
mom_nnode_join (mom_nnode_ty * left, mo_stringvalue_ty * name,
                mo_objref_t objr, mom_nnode_ty * right)
{
  if (!left)
    return mom_nnode_add_min_binding_internal (name, objr, right);
  else if (!right)
    return mom_nnode_add_max_binding_internal (name, objr, left);
  MOM_ASSERTPRINTF (mom_nnode_exists (left), "bad left");
  MOM_ASSERTPRINTF (mom_nnode_exists (right), "bad right");
  if (left->nn_height > right->nn_height + 2)
    return mom_nnode_balance (left->nn_left,
                              left->nn_name, left->nn_objref,
                              mom_nnode_join (left->nn_right, name, objr,
                                              right));
  else if (right->nn_height > left->nn_height + 2)
    return
      mom_nnode_balance (mom_nnode_join (left, name, objr, right->nn_left),
                         right->nn_name, right->nn_objref, right->nn_right);
  else
    return mom_nnode_create (left, name, objr, right);
}                               /* end mom_nnode_join */


/** Comment by X.Leroy:
 (* Merge two trees l and r into one.
       All elements of l must precede the elements of r.
       No assumption on the heights of l and r. *)
**/
static mom_nnode_ty *
mom_nnode_concat (mom_nnode_ty * t1, mom_nnode_ty * t2)
{
  if (!t1)
    return t2;
  if (!t2)
    return t1;
  MOM_ASSERTPRINTF (mom_nnode_exists (t1), "bad t1");
  MOM_ASSERTPRINTF (mom_nnode_exists (t2), "bad t2");
  mom_nnode_ty *minb2 = mom_nnode_min_binding (t2);
  return mom_nnode_join (t1,
                         minb2->nn_name, minb2->nn_objref,
                         mom_nnode_remove_min_binding (t2));
}                               /* end of mom_nnode_concat */

// the split function gives the following structure:
struct mom_splitnnode_st
{
  mom_nnode_ty *nsp_below;      // the subtree below the key
  mom_nnode_ty *nsp_found;      // the node containing the key, if any
  mom_nnode_ty *nsp_above;      // the subtree above the key
};
struct mom_splitnnode_st
mom_nnode_split (mo_stringvalue_ty * namx, mom_nnode_ty * nd)
{
  MOM_ASSERTPRINTF (mo_dyncast_string (namx), "bad namx");
  if (!nd)
    return (struct mom_splitnnode_st)
    {
    NULL, NULL, NULL};
  MOM_ASSERTPRINTF (mom_nnode_exists (nd), "bad nd");
  int cmp = strcmp (namx->mo_cstr, nd->nn_name->mo_cstr);
  if (!cmp)
    return (struct mom_splitnnode_st)
    {
    .nsp_below = nd->nn_left,.nsp_found = nd,.nsp_above = nd->nn_right};
  if (cmp < 0)
    {
      struct mom_splitnnode_st leftsplit =
        mom_nnode_split (namx, nd->nn_left);
      return (struct mom_splitnnode_st)
      {
      .nsp_below = leftsplit.nsp_below,.nsp_found =
          leftsplit.nsp_found,.nsp_above =
          mom_nnode_join (leftsplit.nsp_above, nd->nn_name, nd->nn_objref,
                            nd->nn_right)};
    }
  else                          /* cmp > 0 */
    {
      struct mom_splitnnode_st rightsplit =
        mom_nnode_split (namx, nd->nn_right);
      return (struct mom_splitnnode_st)
      {
      .nsp_below = mom_nnode_join (nd->nn_left,
                                     nd->nn_name, nd->nn_objref,
                                     rightsplit.nsp_below),.nsp_found =
          rightsplit.nsp_found,.nsp_above = rightsplit.nsp_above};
    }
}                               /* end mom_nnode_split */

// probably useless:
//static  mom_nnode_ty*
//mom_nnode_concat_or_join(mom_nnode_ty * t1,
//                       mo_stringvalue_ty * name, mo_objref_t objr,
//                       mom_nnode_ty * t2)
//{
//}      /* end of mom_nnode_concat_or_join */

// register a name for an anonymous object, return true if successful
bool
mo_register_named (mo_objref_t obr, const char *nam)
{
  if (!mo_dyncast_objref (obr))
    return false;
  if (!mom_valid_name (nam))
    return false;
  mo_value_t oldnamv = mo_assoval_get (mom_nameassop, obr);
  if (oldnamv != NULL)
    return false;
  return mo_register_name_string (obr, mo_make_string_cstr (nam));
}                               /* end mo_register_named */

bool
mo_register_name_string (mo_objref_t obr, mo_value_t namv)
{
  if (!mo_dyncast_objref (obr))
    return false;
  if (!mom_valid_name (mo_string_cstr (namv)))
    return false;
  mo_value_t oldnamv = mo_assoval_get (mom_nameassop, obr);
  if (oldnamv != NULL)
    return false;
  mom_rootnnode =
    mom_nnode_add ((mo_stringvalue_ty *) namv, obr, mom_rootnnode);
  mom_nameassop = mo_assoval_put (mom_nameassop, obr, namv);
  return true;
}                               /* end mo_register_name_string */


bool
mo_unregister_named_object (mo_objref_t obr)
{
  if (!mo_dyncast_objref (obr))
    return false;
  mo_value_t oldnamv = mo_assoval_get (mom_nameassop, obr);
  if (oldnamv == NULL)
    return false;
  MOM_ASSERTPRINTF (mo_dyncast_string (oldnamv),
                    "bad oldnamv for obr@%p", obr);
  mom_nameassop = mo_assoval_remove (mom_nameassop, obr);
  mom_rootnnode =
    mom_nnode_remove ((mo_stringvalue_ty *) oldnamv, mom_rootnnode);
  return true;
}                               /* end mo_unregister_named_object */


bool
mo_unregister_name_string (const char *nams)
{
  if (!mom_valid_name (nams))
    return false;
  mom_nnode_ty *nd = mom_nnode_strfind (nams, mom_rootnnode);
  if (!nd)
    return false;
  MOM_ASSERTPRINTF (mom_nnode_exists (nd), "bad node nd@%p", nd);
  MOM_ASSERTPRINTF (!strcmp (nams, mo_string_cstr (nd->nn_name)),
                    "wrong name nd@%p", nd);
  mo_objref_t oldobjv = nd->nn_objref;
  if (!oldobjv)
    return false;
  MOM_ASSERTPRINTF (mo_assoval_get (mom_nameassop, oldobjv) == nd->nn_name,
                    "strange name nd@%p", nd);
  nd->nn_objref = NULL;
  mom_nameassop = mo_assoval_remove (mom_nameassop, oldobjv);
  mom_rootnnode = mom_nnode_remove (nd->nn_name, mom_rootnnode);
  return true;
}                               /* end mo_unregister_name_string */

bool
mo_unregister_name_vals (mo_value_t namv)
{
  if (!mom_valid_name (mo_string_cstr (namv)))
    return false;
  mom_nnode_ty *nd = mom_nnode_strfind (mo_string_cstr (namv), mom_rootnnode);
  if (!nd)
    return false;
  MOM_ASSERTPRINTF (mom_nnode_exists (nd), "bad node nd@%p", nd);
  MOM_ASSERTPRINTF (!strcmp (mo_string_cstr (namv),
                             mo_string_cstr (nd->nn_name)),
                    "wrong name nd@%p", nd);
  nd->nn_objref = NULL;
  mo_objref_t oldobjv = nd->nn_objref;
  if (!oldobjv)
    return false;
  MOM_ASSERTPRINTF (mo_assoval_get (mom_nameassop, oldobjv) == nd->nn_name,
                    "strange name nd@%p", nd);
  mom_nameassop = mo_assoval_remove (mom_nameassop, oldobjv);
  mom_rootnnode =
    mom_nnode_remove ((mo_stringvalue_ty *) namv, mom_rootnnode);
  return true;
}                               /* end mo_unregister_name_vals */


mo_objref_t
mo_find_named_cstr (const char *nams)
{
  if (!mom_valid_name (nams))
    return NULL;
  mom_nnode_ty *nd = mom_nnode_strfind (nams, mom_rootnnode);
  if (!nd)
    return NULL;
  return nd->nn_objref;
}                               /* end mo_find_named_cstr */

mo_objref_t
mo_find_named_vals (mo_value_t namv)
{
  if (!mom_valid_name (mo_string_cstr (namv)))
    return NULL;
  mom_nnode_ty *nd =
    mom_nnode_find ((mo_stringvalue_ty *) namv, mom_rootnnode);
  if (!nd)
    return NULL;
  return nd->nn_objref;
}                               /* end mo_find_named_vals */

// locally allocated on the stack
#define MOM_NPREFIXDATA_MAGIC 0x3cd92803        /* 1020864515 nprefixdata magic */
struct mom_nprefixdata_st
{
  unsigned mnpd_magic;          /* always MOM_NPREFIXDATA_MAGIC */
  unsigned mnpd_count;          /* used count */
  mo_sequencevalue_ty *mnpd_seq;
  const char *mnpd_prefix;
  unsigned mnpd_lenprefix;
};

static bool
mom_nameprefixrepeat (const mom_nnode_ty * nd, void *data)
{                               // return true to stop the repetition;
  struct mom_nprefixdata_st *pd = (struct mom_nprefixdata_st *) data;
  MOM_ASSERTPRINTF (pd && pd->mnpd_magic == MOM_NPREFIXDATA_MAGIC,
                    "bad data");
  if (!nd)
    return true;
  MOM_ASSERTPRINTF (mom_nnode_exists (nd), "bad nd");
  if (!strncmp (nd->nn_name->mo_cstr, pd->mnpd_prefix, pd->mnpd_lenprefix))
    {
      unsigned siz = pd->mnpd_seq->mo_sva_size;
      if (MOM_UNLIKELY (pd->mnpd_count + 1 >= siz))
        {
          unsigned newsiz = mom_prime_above (3 * pd->mnpd_count / 2 + 30);
          mo_sequencevalue_ty *newseq = mo_sequence_allocate (newsiz);
          memcpy (newseq->mo_seqobj, pd->mnpd_seq->mo_seqobj,
                  pd->mnpd_count * sizeof (mo_objref_t));
          pd->mnpd_seq = newseq;
        };
      pd->mnpd_seq->mo_seqobj[pd->mnpd_count++] = nd->nn_objref;
      return false;             // continue the repetition
    }
  return true;                  // stop the repetition
}                               /* end mom_nameprefixrepeat */

mo_value_t
mo_named_set_of_prefix (const char *prefix)
{
  if (!prefix || prefix == MOM_EMPTY_SLOT)
    return NULL;
  if (!isalpha (prefix[0]))
    return NULL;
  int ln = 1;
  for (const char *pc = prefix + 1; *pc; pc++)
    {
      if (!isalnum (*pc) && *pc != '_')
        return NULL;
      ln++;
    };
  struct mom_nprefixdata_st prefixdata = { };
  prefixdata.mnpd_magic = MOM_NPREFIXDATA_MAGIC;
  prefixdata.mnpd_count = 0;
  prefixdata.mnpd_seq = mo_sequence_allocate ((ln > 2) ? 20 : 60);
  prefixdata.mnpd_prefix = prefix;
  prefixdata.mnpd_lenprefix = (unsigned) ln;
  struct mom_splitnnode_st split =
    mom_nnode_split ((mo_stringvalue_ty *) mo_make_string_cstr (prefix),
                     mom_rootnnode);
  if (split.nsp_found)
    {
      MOM_ASSERTPRINTF (mom_nnode_exists (split.nsp_found), "bad nspfound");
      prefixdata.mnpd_seq->mo_seqobj[0] = split.nsp_found->nn_objref;
      prefixdata.mnpd_count = 1;
    };
  mom_nnode_repeat (mom_nameprefixrepeat, &prefixdata, split.nsp_above);
  if (prefixdata.mnpd_count == 0)
    return mo_make_empty_set ();
  mo_sequencevalue_ty *newseq = mo_sequence_allocate (prefixdata.mnpd_count);
  memcpy (newseq->mo_seqobj, prefixdata.mnpd_seq->mo_seqobj,
          prefixdata.mnpd_count * sizeof (mo_objref_t));
  return mo_make_set_closeq (newseq);
}                               /* end of mo_named_set_of_prefix */

/* end of file named.c */

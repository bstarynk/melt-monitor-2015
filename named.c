// file name.c - name of objects

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

#define MOM_NAME_MAGIC 0x18f1   /*6385 */
// we use a red-black tree with the following nodes
struct mom_namednode_st
{
  uint16_t nn_magic;            /* always MOM_NAME_MAGIC */
  bool nn_black;
  intptr_t nn_npar;             // negation of pointer to parent, to be GC-friendly
  struct mom_namednode_st *nn_left;
  struct mom_namednode_st *nn_right;
  mo_stringvalue_ty *nn_name;   /* the name, it is the key */
  mo_objref_t nn_objref;
};
static struct mom_namednode_st *mom_rootnamenode;

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


struct mom_namednode_st *
mom_name_node_parent (struct mom_namednode_st *nd)
{
  if (!nd)
    return NULL;
  MOM_ASSERTPRINTF (nd->nn_magic == MOM_NAME_MAGIC, "bad magic nd@%p", nd);
  if (nd->nn_npar)
    return (struct mom_namednode_st *) (~(nd->nn_npar));
  else
    return NULL;
}                               /* end mom_name_node_parent */

struct mom_namednode_st *
mom_find_name_node_cstr (const char *namstr)
{
  if (!mom_valid_name (namstr))
    return NULL;
  struct mom_namednode_st *nd = mom_rootnamenode;
  while (nd)
    {
      MOM_ASSERTPRINTF (nd->nn_magic == MOM_NAME_MAGIC, "bad magic nd@%p",
                        nd);
      MOM_ASSERTPRINTF (mo_dyncast_string (nd->nn_name), "bad name nd@%p",
                        nd);
      int cmp = strcmp (namstr, mo_string_cstr (nd->nn_name));
      if (!cmp)
        return nd;
      if (cmp < 0)
        nd = nd->nn_left;
      else
        nd = nd->nn_right;
    }
  return nd;
}


struct mom_namednode_st *
mom_find_name_node_value (const mo_stringvalue_ty * namv)
{
  if (!mom_valid_name (mo_string_cstr (namv)))
    return NULL;
  struct mom_namednode_st *nd = mom_rootnamenode;
  while (nd)
    {
      MOM_ASSERTPRINTF (nd->nn_magic == MOM_NAME_MAGIC, "bad magic nd@%p",
                        nd);
      MOM_ASSERTPRINTF (mo_dyncast_string (nd->nn_name), "bad name nd@%p",
                        nd);
      int cmp = strcmp (namv->mo_cstr, mo_string_cstr (nd->nn_name));
      if (!cmp)
        return nd;
      if (cmp < 0)
        nd = nd->nn_left;
      else
        nd = nd->nn_right;
    }
  return nd;
}

struct mom_namednode_st *
mom_find_after_equal_name_node (const char *namstr)
{
  if (!namstr)
    namstr = "";
  else if (namstr[0] != 0 && !mom_valid_name (namstr))
    return NULL;
  struct mom_namednode_st *nd = mom_rootnamenode;
  while (nd)
    {
      MOM_ASSERTPRINTF (nd->nn_magic == MOM_NAME_MAGIC, "bad magic nd@%p",
                        nd);
      MOM_ASSERTPRINTF (mo_dyncast_string (nd->nn_name), "bad name nd@%p",
                        nd);
      int cmp = strcmp (namstr, mo_string_cstr (nd->nn_name));
      if (cmp == 0)
        return nd;
      if (cmp < 0)
        {
          if (nd->nn_left)
            {
              nd = nd->nn_left;
              continue;
            }
          else
            return nd;
        }
      else
        {                       // cmp>0
          if (nd->nn_right)
            {
              nd = nd->nn_right;
              continue;
            }
          else
            {
              struct mom_namednode_st *npar = NULL;
              while ((npar = mom_name_node_parent (nd)) != NULL)
                {
                  MOM_ASSERTPRINTF (npar->nn_magic == MOM_NAME_MAGIC,
                                    "bad magic npar@%p", npar);
                  MOM_ASSERTPRINTF (mo_dyncast_string (npar->nn_name),
                                    "bad name npar@%p", npar);
                  int pacmp = strcmp (namstr, mo_string_cstr (npar->nn_name));
                  if (pacmp <= 0)
                    return npar;
                  nd = npar;
                }
              return nd;
            }
        }
    }                           /* end while nd */
  return NULL;
}                               /* end mom_find_after_equal_name_node */



struct mom_namednode_st *
mom_find_after_name_node (const char *namstr)
{
  if (!namstr)
    namstr = "";
  else if (namstr[0] != 0 && !mom_valid_name (namstr))
    return NULL;
  struct mom_namednode_st *nd = mom_rootnamenode;
  while (nd)
    {
      MOM_ASSERTPRINTF (nd->nn_magic == MOM_NAME_MAGIC, "bad magic nd@%p",
                        nd);
      MOM_ASSERTPRINTF (mo_dyncast_string (nd->nn_name), "bad name nd@%p",
                        nd);
      int cmp = strcmp (namstr, mo_string_cstr (nd->nn_name));
      if (cmp < 0)
        {
          if (nd->nn_left)
            {
              nd = nd->nn_left;
              continue;
            }
          else
            return nd;
        }
      else
        {                       // cmp>=0
          if (nd->nn_right)
            {
              nd = nd->nn_right;
              continue;
            }
          else
            {
              struct mom_namednode_st *npar = NULL;
              while ((npar = mom_name_node_parent (nd)) != NULL)
                {
                  MOM_ASSERTPRINTF (npar->nn_magic == MOM_NAME_MAGIC,
                                    "bad magic npar@%p", npar);
                  MOM_ASSERTPRINTF (mo_dyncast_string (npar->nn_name),
                                    "bad name npar@%p", npar);
                  int pacmp = strcmp (namstr, mo_string_cstr (npar->nn_name));
                  if (pacmp < 0)
                    return npar;
                  nd = npar;
                }
              return nd;
            }
        }
    }                           /* end while nd */
  return NULL;
}                               /* end mom_find_after_name_node */


struct mom_namednode_st *
mom_find_before_equal_name_node (const char *namstr)
{
  if (!namstr)
    namstr = "~";               /* it is after z in UTF8 & ASCII */
  else if (namstr[0] != 0 && !mom_valid_name (namstr))
    return NULL;
  struct mom_namednode_st *nd = mom_rootnamenode;
  while (nd)
    {
      MOM_ASSERTPRINTF (nd->nn_magic == MOM_NAME_MAGIC, "bad magic nd@%p",
                        nd);
      MOM_ASSERTPRINTF (mo_dyncast_string (nd->nn_name), "bad name nd@%p",
                        nd);
      int cmp = strcmp (namstr, mo_string_cstr (nd->nn_name));
      if (cmp == 0)
        return nd;
      if (cmp > 0)
        {
          if (nd->nn_right)
            {
              nd = nd->nn_right;
              continue;
            }
          else
            return nd;
        }
      else
        {                       // cmp<0
          if (nd->nn_left)
            {
              nd = nd->nn_left;
              continue;
            }
          else
            {
              struct mom_namednode_st *npar = NULL;
              while ((npar = mom_name_node_parent (nd)) != NULL)
                {
                  MOM_ASSERTPRINTF (npar->nn_magic == MOM_NAME_MAGIC,
                                    "bad magic npar@%p", npar);
                  MOM_ASSERTPRINTF (mo_dyncast_string (npar->nn_name),
                                    "bad name npar@%p", npar);
                  int pacmp = strcmp (namstr, mo_string_cstr (npar->nn_name));
                  if (pacmp >= 0)
                    return npar;
                  nd = npar;
                }
              return nd;
            }
        }
    }                           /* end while nd */
  return NULL;
}                               /* end mom_find_before_equal_name_node */



struct mom_namednode_st *
mom_find_before_name_node (const char *namstr)
{
  if (!namstr)
    namstr = "~";               /* it is after z in UTF8 & ASCII */
  else if (namstr[0] != 0 && !mom_valid_name (namstr))
    return NULL;
  struct mom_namednode_st *nd = mom_rootnamenode;
  while (nd)
    {
      MOM_ASSERTPRINTF (nd->nn_magic == MOM_NAME_MAGIC, "bad magic nd@%p",
                        nd);
      MOM_ASSERTPRINTF (mo_dyncast_string (nd->nn_name), "bad name nd@%p",
                        nd);
      int cmp = strcmp (namstr, mo_string_cstr (nd->nn_name));
      if (cmp > 0)
        {
          if (nd->nn_right)
            {
              nd = nd->nn_right;
              continue;
            }
          else
            return nd;
        }
      else
        {                       // cmp<=0
          if (nd->nn_left)
            {
              nd = nd->nn_left;
              continue;
            }
          else
            {
              struct mom_namednode_st *npar = NULL;
              while ((npar = mom_name_node_parent (nd)) != NULL)
                {
                  MOM_ASSERTPRINTF (npar->nn_magic == MOM_NAME_MAGIC,
                                    "bad magic npar@%p", npar);
                  MOM_ASSERTPRINTF (mo_dyncast_string (npar->nn_name),
                                    "bad name npar@%p", npar);
                  int pacmp = strcmp (namstr, mo_string_cstr (npar->nn_name));
                  if (pacmp < 0)
                    return npar;
                  nd = npar;
                }
              return nd;
            }
        }
    }                           /* end while nd */
  return NULL;
}                               /* end mom_find_before_name_node */


static void
mom_name_node_left_rotation (struct mom_namednode_st *nx)
{
  MOM_ASSERTPRINTF (nx && nx->nn_magic == MOM_NAME_MAGIC, "bad nx@%p", nx);
  struct mom_namednode_st *ny = nx->nn_right;
  MOM_ASSERTPRINTF (ny && ny->nn_magic == MOM_NAME_MAGIC, "bad ny@%p", ny);
  nx->nn_right = ny->nn_left;
  if (ny->nn_left != NULL)
    ny->nn_left->nn_npar = ~(intptr_t) nx;
  ny->nn_npar = nx->nn_npar;
  if (nx->nn_npar == 0)
    mom_rootnamenode = ny;
  else if (nx == mom_name_node_parent (nx)->nn_left)
    mom_name_node_parent (nx)->nn_left = ny;
  else
    mom_name_node_parent (nx)->nn_right = ny;
  ny->nn_left = nx;
  nx->nn_npar = ~(intptr_t) ny;
}                               /* end mom_name_node_left_rotation */


static void
mom_name_node_right_rotation (struct mom_namednode_st *nx)
{
  MOM_ASSERTPRINTF (nx && nx->nn_magic == MOM_NAME_MAGIC, "bad nx@%p", nx);
  struct mom_namednode_st *ny = nx->nn_left;
  MOM_ASSERTPRINTF (ny && ny->nn_magic == MOM_NAME_MAGIC, "bad ny@%p", ny);
  nx->nn_left = ny->nn_right;
  if (ny->nn_right != NULL)
    ny->nn_right->nn_npar = ~(intptr_t) nx;
  ny->nn_npar = nx->nn_npar;
  if (nx->nn_npar == 0)
    mom_rootnamenode = ny;
  else if (nx == mom_name_node_parent (nx)->nn_right)
    mom_name_node_parent (nx)->nn_right = ny;
  else
    mom_name_node_parent (nx)->nn_left = ny;
  ny->nn_right = nx;
  nx->nn_npar = ~(intptr_t) ny;
}                               /* end mom_name_node_right_rotation */

static struct mom_namednode_st *
mom_create_name_node (mo_stringvalue_ty * vstr)
{
  MOM_ASSERTPRINTF (mom_valid_name (mo_string_cstr (vstr)), "invalid vstr");
  struct mom_namednode_st *nd = //
    mom_gc_alloc (sizeof (struct mom_namednode_st *));
  nd->nn_magic = MOM_NAME_MAGIC;
  nd->nn_black = false;
  nd->nn_npar = 0;
  nd->nn_left = NULL;
  nd->nn_right = NULL;
  nd->nn_name = vstr;
  nd->nn_objref = NULL;
  return nd;
}                               /* end of mom_create_name_node */

static void mom_correct_name_node_after_insertion (struct mom_namednode_st *);

static struct mom_namednode_st *
mom_name_node_insert_nameval (mo_stringvalue_ty * vstr)
{
  MOM_ASSERTPRINTF (mom_valid_name (mo_string_cstr (vstr)), "invalid vstr");
  struct mom_namednode_st *nx = mom_rootnamenode;
  struct mom_namednode_st *ny = NULL;
  int cmp = 0;
  while (nx != NULL)
    {
      MOM_ASSERTPRINTF (nx->nn_magic == MOM_NAME_MAGIC,
                        "bad magic nx@%p", nx);
      MOM_ASSERTPRINTF (mo_dyncast_string (nx->nn_name),
                        "bad name nx@%p", nx);
      ny = nx;
      cmp = strcmp (vstr->mo_cstr, nx->nn_name->mo_cstr);
      if (!cmp)
        return nx;
      else if (cmp < 0)
        nx = nx->nn_left;
      else
        nx = nx->nn_right;
    }
  struct mom_namednode_st *nz = mom_create_name_node (vstr);
  MOM_ASSERTPRINTF (nz && nz->nn_magic == MOM_NAME_MAGIC, "bad nz@%p", nz);
  if (ny)
    {
      nz->nn_npar = ~(intptr_t) ny;
      if ((cmp = strcmp (vstr->mo_cstr, ny->nn_name->mo_cstr)) < 0)
        ny->nn_left = nz;
      else if (cmp > 0)
        ny->nn_right = nz;
      else
        MOM_FATAPRINTF
          ("mom_name_node_insert_nameval corruption: impossible ny@%p", ny);
    }
  else
    mom_rootnamenode = nz;
  mom_correct_name_node_after_insertion (nz);
  return nz;
}                               /* end mom_name_node_insert_nameval  */

static struct mom_namednode_st *
mom_name_node_insert_namestr (const char *nams)
{
  MOM_ASSERTPRINTF (mom_valid_name (nams), "invalid nams=%s", nams);
  struct mom_namednode_st *nx = mom_rootnamenode;
  struct mom_namednode_st *ny = NULL;
  int cmp = 0;
  while (nx != NULL)
    {
      MOM_ASSERTPRINTF (nx->nn_magic == MOM_NAME_MAGIC,
                        "bad magic nx@%p", nx);
      MOM_ASSERTPRINTF (mo_dyncast_string (nx->nn_name),
                        "bad name nx@%p", nx);
      ny = nx;
      cmp = strcmp (nams, nx->nn_name->mo_cstr);
      if (!cmp)
        return nx;
      else if (cmp < 0)
        nx = nx->nn_left;
      else
        nx = nx->nn_right;
    }
  struct mom_namednode_st *nz = //
    mom_create_name_node ((mo_stringvalue_ty *) mo_make_string_cstr (nams));
  MOM_ASSERTPRINTF (nz && nz->nn_magic == MOM_NAME_MAGIC, "bad nz@%p", nz);
  if (ny)
    {
      nz->nn_npar = ~(intptr_t) ny;
      if ((cmp = strcmp (nams, ny->nn_name->mo_cstr)) < 0)
        ny->nn_left = nz;
      else if (cmp > 0)
        ny->nn_right = nz;
      else
        MOM_FATAPRINTF
          ("mom_name_node_insert_namestr corruption: impossible ny@%p", ny);
    }
  else
    mom_rootnamenode = nz;
  mom_correct_name_node_after_insertion (nz);
  return nz;
}                               /* end mom_name_node_insert_namestr  */


static void
mom_correct_name_node_after_insertion (struct mom_namednode_st *nz)
{
  MOM_ASSERTPRINTF (nz && nz->nn_magic == MOM_NAME_MAGIC, "bad nz@%p", nz);
  struct mom_namednode_st *nparz = NULL;
  while ((nparz = mom_name_node_parent (nz)) != NULL && !nparz->nn_black)
    {
      MOM_ASSERTPRINTF (nparz->nn_magic == MOM_NAME_MAGIC,
                        "bad nparz@%p", nz);
      struct mom_namednode_st *ngrparz = mom_name_node_parent (nparz);
      if (ngrparz != NULL && nparz == ngrparz->nn_left)
        {
          struct mom_namednode_st *ny = ngrparz->nn_right;
          MOM_ASSERTPRINTF (!ny || ny->nn_magic == MOM_NAME_MAGIC,
                            "bad ny@%p", ny);
          if (ny && !ny->nn_black)
            {
              nparz->nn_black = true;
              ny->nn_black = true;
              ngrparz->nn_black = false;
              nz = ngrparz;
            }
          else
            {
              if (nz == nparz->nn_right)
                {
                  nz = nparz;
                  mom_name_node_left_rotation (nz);
                  nparz = mom_name_node_parent (nz);
                  ngrparz = mom_name_node_parent (nparz);
                  MOM_ASSERTPRINTF (ngrparz
                                    && ngrparz->nn_magic == MOM_NAME_MAGIC,
                                    "bad ngrparz@%p", ny);
                  ngrparz->nn_black = false;
                  mom_name_node_right_rotation (ngrparz);
                };
              nz->nn_black = true;
            }
        }
      else if (ngrparz != NULL && nparz == ngrparz->nn_right)
        {
          struct mom_namednode_st *ny = ngrparz->nn_left;
          MOM_ASSERTPRINTF (!ny || ny->nn_magic == MOM_NAME_MAGIC,
                            "bad ny@%p", ny);
          if (ny && !ny->nn_black)
            {
              nz->nn_black = true;
              ny->nn_black = true;
              ngrparz->nn_black = false;
              nz = ngrparz;
            }
          else
            {
              if (nz == nparz->nn_left)
                {
                  nz = nparz;
                  mom_name_node_right_rotation (nz);
                  nparz = mom_name_node_parent (nz);
                  ngrparz = mom_name_node_parent (nparz);
                };
              nz->nn_black = true;
              MOM_ASSERTPRINTF (ngrparz
                                && ngrparz->nn_magic == MOM_NAME_MAGIC,
                                "bad ngrparz");
              mom_name_node_left_rotation (ngrparz);
            }
        }
      else if (ngrparz != NULL) // should never happen
        MOM_FATAPRINTF ("nametree corrupted");
      else
        break;
    }
  if (mom_rootnamenode)
    mom_rootnamenode->nn_black = true;
}                               /* end mom_correct_name_node_after_insertion */

mo_value_t
mo_get_namev (mo_objref_t obr)
{
  if (!mo_dyncast_objref (obr))
    return NULL;
  mo_value_t r = mo_assoval_get (mom_nameassop, obr);
  MOM_ASSERTPRINTF (r == NULL || mo_dyncast_string (r), "bad name");
  return r;
}                               /* end mo_get_namev */

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
  struct mom_namednode_st *nd = mom_name_node_insert_namestr (nam);
  MOM_ASSERTPRINTF (nd && nd->nn_magic == MOM_NAME_MAGIC,
                    "bad nd@%p for nam %s", nd, nam);
  MOM_ASSERTPRINTF (nd->nn_objref == NULL, "already used nd@%p", nd);
  MOM_ASSERTPRINTF (!strcmp (mo_string_cstr (nd->nn_name), nam),
                    "wrong name at nd@%p for %s", nd, nam);
  mom_nameassop = mo_assoval_put (mom_nameassop, obr, nd->nn_name);
  nd->nn_objref = obr;
  return true;
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
  struct mom_namednode_st *nd = //
    mom_name_node_insert_nameval ((mo_stringvalue_ty *) namv);
  MOM_ASSERTPRINTF (nd && nd->nn_magic == MOM_NAME_MAGIC,
                    "bad nd@%p for nam %s", nd, mo_string_cstr (namv));
  MOM_ASSERTPRINTF (nd->nn_objref == NULL, "already used nd@%p", nd);
  MOM_ASSERTPRINTF (!strcmp
                    (mo_string_cstr (nd->nn_name), mo_string_cstr (namv)),
                    "wrong name at nd@%p", nd);
  mom_nameassop = mo_assoval_put (mom_nameassop, obr, nd->nn_name);
  nd->nn_objref = obr;
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
  struct mom_namednode_st *nd =
    mom_find_name_node_value ((const mo_stringvalue_ty *) oldnamv);
  MOM_ASSERTPRINTF (nd && nd->nn_magic == MOM_NAME_MAGIC, "bad nd");
  MOM_ASSERTPRINTF (!strcmp
                    (mo_string_cstr (oldnamv), mo_string_cstr (nd->nn_name)),
                    "bad name in nd");
  MOM_ASSERTPRINTF (nd->nn_objref == obr, "wrong obj in nd");
  nd->nn_objref = NULL;
  return true;
}                               /* end mo_unregister_named_object */


bool
mo_unregister_name_string (const char *nams)
{
  if (!mom_valid_name (nams))
    return false;
  struct mom_namednode_st *nd = mom_find_name_node_cstr (nams);
  if (!nd)
    return false;
  MOM_ASSERTPRINTF (nd->nn_magic == MOM_NAME_MAGIC, "bad magic nd@%p", nd);
  MOM_ASSERTPRINTF (mo_dyncast_string (nd->nn_name), "bad name nd@%p", nd);
  MOM_ASSERTPRINTF (!strcmp (nams, mo_string_cstr (nd->nn_name)),
                    "wrong name nd@%p", nd);
  mo_objref_t oldobjv = nd->nn_objref;
  if (!oldobjv)
    return false;
  MOM_ASSERTPRINTF (mo_assoval_get (mom_nameassop, oldobjv) == nd->nn_name,
                    "strange name nd@%p", nd);
  mom_nameassop = mo_assoval_remove (mom_nameassop, oldobjv);
  nd->nn_objref = NULL;
  return true;
}                               /* end mo_unregister_name_string */

bool
mo_unregister_name_vals (mo_value_t namv)
{
  if (!mom_valid_name (mo_string_cstr (namv)))
    return false;
  struct mom_namednode_st *nd = mom_find_name_node_value (namv);
  if (!nd)
    return false;
  MOM_ASSERTPRINTF (nd->nn_magic == MOM_NAME_MAGIC, "bad magic nd@%p", nd);
  MOM_ASSERTPRINTF (mo_dyncast_string (nd->nn_name), "bad name nd@%p", nd);
  mo_objref_t oldobjv = nd->nn_objref;
  if (!oldobjv)
    return false;
  MOM_ASSERTPRINTF (mo_assoval_get (mom_nameassop, oldobjv) == nd->nn_name,
                    "strange name nd@%p", nd);
  mom_nameassop = mo_assoval_remove (mom_nameassop, oldobjv);
  nd->nn_objref = NULL;
  return true;
}                               /* end mo_unregister_name_vals */


mo_objref_t
mo_find_named_cstr (const char *nams)
{
  if (!mom_valid_name (nams))
    return NULL;
  struct mom_namednode_st *nd = mom_find_name_node_cstr (nams);
  if (!nd)
    return NULL;
  return nd->nn_objref;
}                               /* end mo_find_named_cstr */

mo_objref_t
mo_find_named_vals (mo_value_t vals)
{
  if (!mom_valid_name (mo_string_cstr (vals)))
    return NULL;
  struct mom_namednode_st *nd = mom_find_name_node_value (vals);
  if (!nd)
    return NULL;
  return nd->nn_objref;
}                               /* end mo_find_named_vals */

void
mo_reserve_names (unsigned gap)
{
  mom_nameassop = mo_assoval_reserve (mom_nameassop, gap);
}                               /* end mo_reserve_names */


/* eof named.c */
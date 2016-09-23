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
  struct mom_namednode_st *nn_par;      // pointer to parent
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


struct mom_namednode_st *
mom_name_node_parent (struct mom_namednode_st *nd)
{
  if (!nd)
    return NULL;
  MOM_ASSERTPRINTF (nd->nn_magic == MOM_NAME_MAGIC, "bad magic nd@%p", nd);
  if (nd->nn_par)
    return (struct mom_namednode_st *) ((nd->nn_par));
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
    ny->nn_left->nn_par = nx;
  ny->nn_par = nx->nn_par;
  if (nx->nn_par == NULL)
    mom_rootnamenode = ny;
  else if (nx == mom_name_node_parent (nx)->nn_left)
    mom_name_node_parent (nx)->nn_left = ny;
  else
    mom_name_node_parent (nx)->nn_right = ny;
  ny->nn_left = nx;
  nx->nn_par = ny;
}                               /* end mom_name_node_left_rotation */


static void
mom_name_node_right_rotation (struct mom_namednode_st *nx)
{
  MOM_ASSERTPRINTF (nx && nx->nn_magic == MOM_NAME_MAGIC, "bad nx@%p", nx);
  struct mom_namednode_st *ny = nx->nn_left;
  MOM_ASSERTPRINTF (ny && ny->nn_magic == MOM_NAME_MAGIC, "bad ny@%p", ny);
  nx->nn_left = ny->nn_right;
  if (ny->nn_right != NULL)
    ny->nn_right->nn_par = nx;
  ny->nn_par = nx->nn_par;
  if (nx->nn_par == 0)
    mom_rootnamenode = ny;
  else if (nx == mom_name_node_parent (nx)->nn_right)
    mom_name_node_parent (nx)->nn_right = ny;
  else
    mom_name_node_parent (nx)->nn_left = ny;
  ny->nn_right = nx;
  nx->nn_par = ny;
}                               /* end mom_name_node_right_rotation */

static struct mom_namednode_st *
mom_create_name_node (mo_stringvalue_ty * vstr)
{
  MOM_ASSERTPRINTF (mom_valid_name (mo_string_cstr (vstr)), "invalid vstr");
  struct mom_namednode_st *nd = //
    mom_gc_alloc (sizeof (struct mom_namednode_st));
  nd->nn_magic = MOM_NAME_MAGIC;
  nd->nn_black = false;
  nd->nn_par = NULL;
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
      nz->nn_par = ny;
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
  mo_stringvalue_ty *nameval =
    (mo_stringvalue_ty *) mo_make_string_cstr (nams);
  MOM_ASSERTPRINTF (mo_dyncast_string (nameval), "bad nameval");
  struct mom_namednode_st *nz = mom_create_name_node (nameval);
  MOM_ASSERTPRINTF (nz && nz->nn_magic == MOM_NAME_MAGIC, "bad nz@%p", nz);
  if (ny)
    {
      nz->nn_par = ny;
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
mo_objref_namev (mo_objref_t obr)
{
  if (!mo_dyncast_objref (obr))
    return NULL;
  mo_value_t r = mo_assoval_get (mom_nameassop, obr);
  MOM_ASSERTPRINTF (r == NULL || mo_dyncast_string (r), "bad name");
  return r;
}                               /* end mo_objref_namev */

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
  MOM_ASSERTPRINTF (nd->nn_name != NULL, "unnamed nd@%p", nd);
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

mo_value_t
mo_named_objects_set (void)
{
  return mo_assoval_keys_set (mom_nameassop);
}                               /* end mo_named_objects_set */

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
  mo_sequencevalue_ty *seq = mo_sequence_allocate ((ln > 2) ? 20 : 60);
  struct mom_namednode_st *nod = mom_find_after_equal_name_node (prefix);
  unsigned cnt = 0;
  while (nod != NULL)
    {
      MOM_ASSERTPRINTF (nod->nn_magic == MOM_NAME_MAGIC,
                        "bad magic nod@%p", nod);
      mo_objref_t curobj = nod->nn_objref;
      if (MOM_UNLIKELY (cnt + 1 >= seq->mo_sva_size))
        {
          unsigned newsiz = mom_prime_above (3 * cnt / 2 + 40);
          mo_sequencevalue_ty *newseq = mo_sequence_allocate (newsiz);
          memcpy (newseq->mo_seqobj, seq->mo_seqobj,
                  cnt * sizeof (mo_objref_t));
          seq = newseq;
        };
      MOM_ASSERTPRINTF (mo_dyncast_string (nod->nn_name),
                        "bad name in nod@%p", nod);
      if (!strncmp (prefix, nod->nn_name->mo_cstr, ln))
        {
          if (mo_dyncast_objref (curobj))
            seq->mo_seqobj[cnt++] = curobj;
        }
      else
        break;
      nod = mom_find_after_name_node (nod->nn_name->mo_cstr);
    }
  if (cnt == 0)
    return mo_make_empty_set ();
  {
    mo_sequencevalue_ty *newseq = mo_sequence_allocate (cnt);
    memcpy (newseq->mo_seqobj, seq->mo_seqobj, cnt * sizeof (mo_objref_t));
    seq = newseq;
    return mo_make_set_closeq (newseq);
  }
  return NULL;
}                               /* end of mo_named_set_of_prefix */

#if 0

// we use a binary balanced tree (similar to red-black tree)
// shamelessly copied from Ocaml's stdlib/map.ml by X.Leroy
// actually, this code should look very close to
// https://github.com/bstarynk/misc-basile/blob/master/basilemap.ml
// which is itself a slight rewriting of X.Leroy's map.ml
#define MOM_NN_MAX_HEIGHT 256
typedef struct mom_nnode_st mom_nnode_ty;
struct mom_nnode_st
{
  uint16_t nn_magic;
  uint16_t nn_height;
  mom_nnode_ty *nn_left;
  mo_stringvalue_ty *nn_name;
  mo_objref_t nn_objref;
  mom_nnode_ty *nn_right;
};

static inline bool
mom_nnode_exists (mom_nnode_ty * n)
{
  if (!n)
    return false;
  MOM_ASSERTPRINTF (n->nn_magic == MOM_NAME_MAGIC, "bad n@%p", n);
  MOM_ASSERTPRINTF (mo_dyncast_string (n->nn_name), "bad name in n@%p", n);
  MOM_ASSERTPRINTF (n->nn_objref == NULL
                    || mo_dyncast_objref (n->nn_objref), "bad objref in n@%p",
                    n);
  return true;
}                               /* end mom_nnode_exists */

static inline unsigned
mom_nnode_height (mom_nnode_ty * n)
{
  if (!n)
    return 0;
  MOM_ASSERTPRINTF (n->nn_magic == MOM_NAME_MAGIC && n->nn_height > 0,
                    "bad n@%p", n);
  return n->nn_height;
}

static inline mom_nnode_ty *
mom_nnode_left (mom_nnode_ty * n)
{
  if (!n)
    return NULL;
  MOM_ASSERTPRINTF (n->nn_magic == MOM_NAME_MAGIC, "bad n@%p", n);
  return n->nn_left;
}                               /* end mom_nnode_left */

static inline mom_nnode_ty *
mom_nnode_right (mom_nnode_ty * n)
{
  if (!n)
    return NULL;
  MOM_ASSERTPRINTF (n->nn_magic == MOM_NAME_MAGIC, "bad n@%p", n);
  return n->nn_right;
}                               /* end mom_nnode_right */

static inline mo_stringvalue_ty *
mom_nnode_name (mom_nnode_ty * n)
{
  if (!n)
    return NULL;
  MOM_ASSERTPRINTF (n->nn_magic == MOM_NAME_MAGIC, "bad n@%p", n);
  MOM_ASSERTPRINTF (mo_dyncast_string (n->nn_name), "bad name in n@%p", n);
  return n->nn_name;
}                               /* end mom_nnode_name */

static inline mo_objref_t
mom_nnode_objref (mom_nnode_ty * n)
{
  if (!n)
    return NULL;
  MOM_ASSERTPRINTF (n->nn_magic == MOM_NAME_MAGIC, "bad n@%p", n);
  return n->nn_objref;
}                               /* end of mom_nnode_objref */

static inline mom_nnode_ty *
mom_nnode_create (mom_nnode_ty * left, mo_stringvalue_ty * name,
                  mo_objref_t objr, mom_nnode_ty * right)
{
  mom_nnode_ty *n = mom_gc_alloc (sizeof (mom_nnode_ty));
  MOM_ASSERTPRINTF (!left
                    || (left->nn_magic == MOM_NAME_MAGIC
                        && left->nn_height < MOM_NN_MAX_HEIGHT),
                    "mom_nnode_create: bad left@%p");
  MOM_ASSERTPRINTF (!right
                    || (right->nn_magic == MOM_NAME_MAGIC
                        && right->nn_height < MOM_NN_MAX_HEIGHT),
                    "mom_nnode_create: bad left@%p");
  MOM_ASSERTPRINTF (mo_dyncast_string (name) != NULL,
                    "mom_nnode_create: nil name");
  MOM_ASSERTPRINTF (!objr || mo_dyncast_objref (objr),
                    "mom_nnode_create: bad objr");
  n->nn_magic = MOM_NAME_MAGIC;
  n->nn_left = left;
  n->nn_right = right;
  n->nn_name = nam;
  n->nn_objr = objr;
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
  MOM_ASSERTPRINTF (!objr || mo_dyncast_objref (obr), "bad obr");
  unsigned lefth = mom_nnode_height (left);
  unsigned righth = mom_nnode_height (right);
  if (lefth > righth + 2)
    {
      MOM_ASSERTPRINTF (mom_nnode_exists (left), "nil left");
      if (mom_nnode_height (left->nn_left) >=
          mom_nnode_height (left->nn_right))
        return mom_nnode_create (left->nn_left,
                                 left->nn_name, left->nn_objr,
                                 mom_nnode_create (left->nn_right,
                                                   name, objr, right));
      else
        {
          mom_node_ty *lr = left->nn_right;
          MOM_ASSERTPRINTF (mom_nnode_exists (lr), "nil right of left");
          return
            mom_nnode_create (mom_nnode_create (left->nn_left,
                                                left->nn_name, left->nn_objr,
                                                lr->nn_left),
                              lr->nn_name, lr->nn_objr,
                              mom_nnode_create (lr->right,
                                                name, objr, right));
        }
    }
  else if (righth > lefth + 2)
    {
      MOM_ASSERTPRINTF (mom_nnode_exists (right), "nil right");
      mom_node_ty *rl = right->nn_left;
      mom_node_ty *rr = right->nn_right;
      if (mom_nnode_height (rr) >= mom_nnode_height (rl))
        return mom_nnode_create (mom_nnode_create (left,
                                                   name, objr,
                                                   rl),
                                 right->nn_name, right->nn_objr, rr);
      else
        {
          MOM_ASSERTPRINTF (mom_nnode_exists (rl), "nil rl");
          mom_node_ty *rll = rl->nn_left;
          mom_node_ty *rlr = rl->nn_right;
          return mom_nnode_create (mom_nnode_create (left,
                                                     name, objr,
                                                     rll),
                                   rl->nn_name, rl->nn_objr,
                                   mom_nnode_create (rlr,
                                                     right->nn_name,
                                                     right->nn_objr, rr));
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
  mo_stringvalue_ty *d = mom_nnode_name (x);
  MOM_ASSERTPRINTF (mo_dyncast_string (d), "bad name d in x");
  int cmp = strcmp (x->mo_cstr, d->mo_cstr);
  if (!cmp)
    return x;
  else if (cmp < 0)
    {
      mom_nnode_ty *l = mom_nnode_left (n);
      mom_nnode_ty *r = mom_nnode_right (n);
      mom_nnode_ty *ll = mom_nnode_add (x, l);
      if (l == ll)
        return n;
      else
        return mom_nnode_balance (ll, n->nn_name, r);
    }
  else                          /*cmp>0 */
    {
      mom_nnode_ty *rr = mom_nnode_add (x, r);
      if (r == rr)
        return n;
      else
        return mom_nnode_balance (l, n->nn_name, rr);
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
  int cmp = strcmp (s n->nn_name->mo_cstr);
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
                            n->nn_name, n->nn_objr, n->nn_right);
}                               /* end mom_nnode_remove_min_binding */

static mom_nnode_ty *
mom_nnode_merge (mom_nnode_ty * t1, mom_nnode_ty * t2)
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
  mom_nnode_ty *mb = mom_nnode_min_binding (t2);
  MOM_ASSERTPRINTF (mom_nnode_exists (mb), "bad mb2");
  return mom_nnode_balance ();
}                               /* end of mom_nnode_merge */


static mom_nnode_ty *
mom_nnode_remove (mo_stringvalue_ty * xn, mom_nnode_ty * nd)
{
  MOM_ASSERTPRINTF (mo_dyncast_string (xn), "bad xn");
  if (!nd)
    return NULL;
  MOM_ASSERTPRINTF (mom_nnode_exists (nd), "bad nd");
  int cmp = strcmp (xn->mo_cstr, nd->nn_name->mo_cstr);
  if (cmp == 0)
    return mom_nnode_merge (nd->nn_left, nd->nn_right);
  else if (cmp < 0)
    return mom_nnode_balance (mom_nnode_remove (xn, nd->nn_left),
                              nd->nn_name, nd->nn_objr, nd->nn_right);
  else
    return mom_nnode_balance (nd->nn_left,
                              nd->nn_name, nd->nn_objr,
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
                       (kn, vo, nd->nn_left), nd->nn_name, nd->nn_objr,
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
                            nd->nn_name, nd->nn_objr,
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
                              left->nn_name, left->nn_objr,
                              mom_nnode_join (left->nn_right, name, objr,
                                              right));
  else if (right->nn_height > left->nn_height + 2)
    return
      mom_nnode_balance (mom_nnode_join (left, name, objr, right->nn_left),
                         right->nn_name, right->nn_objr, right->nn_right);
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
                         minb2->nn_name, minb2->nn_objr,
                         mom_nnode_remove_min_binding (t2));
}                               /* end of mom_nnode_concat */


// probably useless:
//static  mom_nnode_ty*
//mom_nnode_concat_or_join(mom_nnode_ty * t1,
//                       mo_stringvalue_ty * name, mo_objref_t objr,
//                       mom_nnode_ty * t2)
//{
//}      /* end of mom_nnode_concat_or_join */
#endif /*0 because new code */
/* eof named.c */

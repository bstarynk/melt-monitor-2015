// file emitc.cc - Emit C code

/**   Copyright (C)  2016  Basile Starynkevitch and later the FSF
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

#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <functional>
#include <gc/gc_allocator.h>


class MomCEmitter
{
  friend bool mom_emit_c_code(struct mom_item_st*itm);
  static unsigned constexpr MAGIC = 508723037 /*0x1e527f5d*/;
  const unsigned _ce_magic;			      // always MAGIC
  struct mom_item_st* _ce_topitm;
  std::vector<struct mom_item_st*,traceable_allocator<struct mom_item_st*>> _ce_vecitems;
  std::set<struct mom_item_st*,MomItemLess,traceable_allocator<struct mom_item_st*>> _ce_setitems;
  void lock_item(struct mom_item_st*itm)
  {
    if (itm && itm != MOM_EMPTY_SLOT && itm->va_itype == MOMITY_ITEM
        && _ce_setitems.find(itm)==_ce_setitems.end())
      {
        _ce_vecitems.push_back(itm);
        _ce_setitems.insert(itm);
        mom_item_lock(itm);
      }
  };
public:
  MomCEmitter(struct mom_item_st*itm);
  MomCEmitter(const MomCEmitter&) = delete;
  ~MomCEmitter();
};				// end class MomCEmitter



bool mom_emit_c_code(struct mom_item_st*itm)
{
  if (!itm || itm==MOM_EMPTY_SLOT || itm->va_itype != MOMITY_ITEM)
    {
      MOM_WARNPRINTF("invalid item for mom_emit_c_code");
      return false;
    }
  MOM_DEBUGPRINTF(gencod, "mom_emit_c_code start itm=%s", mom_item_cstring(itm));
  errno = 0;
  try
    {
      MomCEmitter cemit {itm};
      auto descitm = mom_unsync_item_descr(itm);
      MOM_DEBUGPRINTF(gencod, "mom_emit_c_code descitm=%s", mom_item_cstring(descitm));
      if (descitm != MOM_PREDEFITM(module))
        throw MOM_RUNTIME_PRINTF("item %s has non-module descr: %s",
                                 mom_item_cstring(itm), mom_item_cstring(descitm));
#warning mom_emit_c_code incomplete
      MOM_FATAPRINTF("unimplemented mom_emit_c_code %s",
                     mom_item_cstring(itm));
      return true;
    }
  catch (const MomRuntimeErrorAt& e)
    {
      MOM_WARNPRINTF_AT(e.file(), e.lineno(),
                        "mom_emit_c_code %s failed with MOM runtime exception %s",
                        mom_item_cstring(itm), e.what());
      return false;
    }
  catch (const std::exception& e)
    {
      MOM_WARNPRINTF("mom_emit_c_code %s failed with exception %s",
                     mom_item_cstring(itm), e.what());
      return false;
    }
  catch (...)
    {
      MOM_WARNPRINTF("mom_emit_c_code %s failed",
                     mom_item_cstring(itm));
      return false;
    }
} // end of mom_emit_c_code


MomCEmitter::MomCEmitter(struct mom_item_st*itm)
  : _ce_magic(MAGIC),
    _ce_topitm(itm),
    _ce_vecitems {},
_ce_setitems {}
{
  if (!itm || itm==MOM_EMPTY_SLOT || itm->va_itype != MOMITY_ITEM)
    throw MOM_RUNTIME_ERROR("non item");
  lock_item(itm);
} // end MomCEmitter::MomCEmitter

MomCEmitter::~MomCEmitter()
{
  int nbit = _ce_vecitems.size();
  for (int ix=nbit-1; ix>=0; ix--)
    mom_item_unlock(_ce_vecitems[ix]);
  _ce_vecitems.clear();
  _ce_setitems.clear();
} // end MomCEmitter::~MomCEmitter

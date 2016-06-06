// file emitc.cc - Emit code

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
#include <deque>
#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <functional>
#include <gc/gc_allocator.h>

class MomEmitter
{
public:
  typedef std::function<void(MomEmitter*)> todofun_t;
  typedef std::set<const struct mom_item_st*,
    MomItemLess,
    traceable_allocator<struct mom_item_st*>>
                                           traced_set_items_t;
  struct vardef_st
  {
    struct mom_item_st* vd_rolitm;
    const void* vd_what;
    const void* vd_detail;
    long vd_rank;
  };
  typedef std::map<const struct mom_item_st*,
    vardef_st,
    MomItemLess,
    traceable_allocator<struct mom_item_st*>>
                                           traced_varmap_t;
private:
  const unsigned _ce_magic;
  struct mom_item_st* _ce_topitm;
  std::vector<struct mom_item_st*,traceable_allocator<struct mom_item_st*>> _ce_veclockeditems;
  traced_set_items_t _ce_setlockeditems;
  traced_set_items_t _ce_sigitems;
  traced_set_items_t _ce_typitems;
  std::deque<todofun_t,traceable_allocator<todofun_t>> _ce_todoque;
  traced_varmap_t _ce_globalvarmap;
  traced_varmap_t _ce_localvarmap;
protected:
  MomEmitter(unsigned magic, struct mom_item_st*itm);
  MomEmitter(const MomEmitter&) = delete;
  virtual ~MomEmitter();
  virtual void scan_data_element(struct mom_item_st*itm);
  virtual void scan_func_element(struct mom_item_st*itm);
  virtual void scan_routine_element(struct mom_item_st*itm);
  void scan_signature(struct mom_item_st*sigitm, struct mom_item_st*initm);
  void scan_block(struct mom_item_st*blockitm, struct mom_item_st*initm);
  void scan_type(struct mom_item_st*typitm);
  void scan_instr(struct mom_item_st*insitm, int rk, struct mom_item_st*blkitm);
  void scan_expr(const void*expv, struct mom_item_st*insitm, int depth, struct mom_item_st*typitm=0);
  void scan_node_expr(const struct mom_boxnode_st*expnod, struct mom_item_st*insitm, int depth, struct mom_item_st*typitm=0);
  void scan_item_expr(struct mom_item_st*expitm, struct mom_item_st*insitm, int depth, struct mom_item_st*typitm=0);
  void scan_var(struct mom_item_st*varitm, struct mom_item_st*insitm, struct mom_item_st*typitm=0);
public:
  static int constexpr MAX_DEPTH_EXPR=128;
  vardef_st*get_binding(const struct mom_item_st*itm) const
{
    if (mom_itype(itm) != MOMITY_ITEM) return nullptr;
    auto it = _ce_localvarmap.find(itm);
    if (it != _ce_localvarmap.end())
      return const_cast<vardef_st*>(&it->second);
    it = _ce_globalvarmap.find(itm);
    if (it != _ce_globalvarmap.end())
      return const_cast<vardef_st*>(&it->second);
    return nullptr;
  }
  bool is_bound(const struct mom_item_st*itm) const
  {
    return _ce_localvarmap.find(itm) != _ce_localvarmap.end()
           || _ce_globalvarmap.find(itm) != _ce_globalvarmap.end();
  };
  void bind_global(const struct mom_item_st*itm, const vardef_st& vd)
  {
    if (mom_itype(itm) != MOMITY_ITEM)
      throw MOM_RUNTIME_ERROR("global binding non-item");
    if (vd.vd_rolitm == MOM_PREDEFITM(data))
      MOM_DEBUGPRINTF(gencod, "global binding %s to data @%p rank#%ld",
                      mom_item_cstring(itm), vd.vd_what, vd.vd_rank);

    else
      MOM_DEBUGPRINTF(gencod, "global binding %s to role %s what %s detail %s rank#%ld",
                      mom_item_cstring(itm), mom_item_cstring(vd.vd_rolitm),
                      mom_value_cstring(vd.vd_what),
                      mom_value_cstring(vd.vd_detail),
                      vd.vd_rank);
    _ce_globalvarmap[itm] = vd;
  }
  void bind_global(const struct mom_item_st*itm, struct mom_item_st*rolitm, const void*what,
                   const void*detail=nullptr, long rank=0)
  {
    bind_global(itm,vardef_st {rolitm,what,detail,rank});
  }
  void bind_local(const struct mom_item_st*itm, const vardef_st& vd)
  {
    if (mom_itype(itm) != MOMITY_ITEM)
      throw MOM_RUNTIME_ERROR("local binding non-item");
    if (vd.vd_rolitm == MOM_PREDEFITM(data))
      MOM_DEBUGPRINTF(gencod, "local binding %s to data @%p rank#%ld",
                      mom_item_cstring(itm), vd.vd_what, vd.vd_rank);

    else
      MOM_DEBUGPRINTF(gencod, "local binding %s to role %s what %s detail %s rank#%ld",
                      mom_item_cstring(itm), mom_item_cstring(vd.vd_rolitm),
                      mom_value_cstring(vd.vd_what),
                      mom_value_cstring(vd.vd_detail),
                      vd.vd_rank);
    _ce_localvarmap[itm] = vd;
  }
  void bind_local(const struct mom_item_st*itm, struct mom_item_st*rolitm, const void*what, const void*detail=nullptr, long rank=0)
  {
    bind_local(itm,vardef_st {rolitm,what,detail,rank});
  }
  void unbind(const struct mom_item_st*itm)
  {
    if (mom_itype(itm) != MOMITY_ITEM)
      throw MOM_RUNTIME_ERROR("unbinding non-item");
    auto itglo = _ce_globalvarmap.find(itm);
    if (itglo != _ce_globalvarmap.end())
      {
        MOM_DEBUGPRINTF(gencod, "unbinding global %s role %s",
                        mom_item_cstring(itm), mom_item_cstring(itglo->second.vd_rolitm));
        _ce_globalvarmap.erase(itglo);
        return;
      };
    auto itloc = _ce_localvarmap.find(itm);
    if (itloc != _ce_localvarmap.end())
      {
        MOM_DEBUGPRINTF(gencod, "unbinding local %s role %s",
                        mom_item_cstring(itm), mom_item_cstring(itglo->second.vd_rolitm));
        _ce_localvarmap.erase(itloc);
        return;
      }
    throw MOM_RUNTIME_PRINTF("unbinding unbound item %s", mom_item_cstring(itm));
  }
  virtual const char*kindname() const =0;
  void lock_item(struct mom_item_st*itm)
  {
    if (mom_itype(itm) == MOMITY_ITEM
        && _ce_setlockeditems.find(itm)==_ce_setlockeditems.end())
      {
        _ce_veclockeditems.push_back(itm);
        _ce_setlockeditems.insert(itm);
        mom_item_lock(itm);
      }
  };
  bool is_locked_item(struct  mom_item_st*itm)
  {
    if (mom_itype(itm) == MOMITY_ITEM)
      return _ce_setlockeditems.find(itm) != _ce_setlockeditems.end();
    return false;
  }
  void scan_top_module(void);
  void scan_module_element(struct mom_item_st*itm);
  void todo(const todofun_t& tf)
  {
    _ce_todoque.push_back(tf);
  };
  unsigned long nb_todos() const
  {
    return _ce_todoque.size();
  };
  void flush_todo_list(int lin=0)
  {
    if (lin)
      MOM_DEBUGPRINTF_AT(__FILE__,lin,gencod,"flush_todo_list %s with %ld todos",
                         kindname(), nb_todos());
    else
      MOM_DEBUGPRINTF(gencod,"flush_todo_list %s with %ld todos",
                      kindname(), nb_todos());
    while (!_ce_todoque.empty())
      {
        auto tf = _ce_todoque.front();
        _ce_todoque.pop_front();
        tf(this);
      }
    _ce_todoque.shrink_to_fit();
  };
  unsigned magic() const
  {
    return _ce_magic;
  };
};				// end of MomEmitter


////////////////
class MomCEmitter final :public MomEmitter
{
  friend bool mom_emit_c_code(struct mom_item_st*itm);
  static unsigned constexpr MAGIC = 508723037 /*0x1e527f5d*/;
public:
  MomCEmitter(struct mom_item_st*itm) : MomEmitter(MAGIC, itm) {};
  MomCEmitter(const MomCEmitter&) = delete;
  virtual ~MomCEmitter();
  virtual const char*kindname() const
  {
    return "C-emitter";
  };
};				// end class MomCEmitter


////////////////
class MomJavascriptEmitter final : public MomEmitter
{
  friend bool mom_emit_javascript_code(struct mom_item_st*itm, FILE*out);
  static unsigned constexpr MAGIC = 852389659 /*0x32ce6f1b*/;
public:
  MomJavascriptEmitter(struct mom_item_st*itm) : MomEmitter(MAGIC, itm) {};
  MomJavascriptEmitter(const MomJavascriptEmitter&) = delete;
  virtual ~MomJavascriptEmitter();
  virtual void scan_routine_element(struct mom_item_st*elitm)
  {
    throw MOM_RUNTIME_PRINTF("routine element %s unsupported for JavaScript", mom_item_cstring(elitm));
  };
  virtual const char*kindname() const
  {
    return "JavaScript-emitter";
  };
};



////////////////////////////////////////////////////////////////
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
      cemit.scan_top_module();
      cemit.flush_todo_list(__LINE__);
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




////////////////////////////////////////////////////////////////
bool mom_emit_javascript_code(struct mom_item_st*itm, FILE*fil)
{
  if (!itm || itm==MOM_EMPTY_SLOT || itm->va_itype != MOMITY_ITEM)
    {
      MOM_WARNPRINTF("invalid item for mom_emit_javascript_code");
      return false;
    }
  if (!fil)
    {
      MOM_WARNPRINTF("no file for mom_emit_javascript_code");
      return false;
    }
  MOM_DEBUGPRINTF(gencod, "mom_emit_javascript_code start itm=%s", mom_item_cstring(itm));
  errno = 0;
  try
    {
      MomJavascriptEmitter jsemit {itm};
      jsemit.scan_top_module();
      jsemit.flush_todo_list(__LINE__);
#warning mom_emit_javascript_code incomplete
      MOM_FATAPRINTF("unimplemented mom_emit_javascript_code %s",
                     mom_item_cstring(itm));
    }
  catch (const MomRuntimeErrorAt& e)
    {
      MOM_WARNPRINTF_AT(e.file(), e.lineno(),
                        "mom_emit_javascript_code %s failed with MOM runtime exception %s",
                        mom_item_cstring(itm), e.what());
      return false;
    }
  catch (const std::exception& e)
    {
      MOM_WARNPRINTF("mom_emit_javascript_code %s failed with exception %s",
                     mom_item_cstring(itm), e.what());
      return false;
    }
  catch (...)
    {
      MOM_WARNPRINTF("mom_emit_javascript_code %s failed",
                     mom_item_cstring(itm));
      return false;
    }
} // end mom_emit_javascript_code

////////////////////////////////////////////////////////////////

MomEmitter::MomEmitter(unsigned magic, struct mom_item_st*itm)
  : _ce_magic(magic),
    _ce_topitm(itm),
    _ce_veclockeditems {},
                     _ce_setlockeditems {},
                     _ce_sigitems {},
                     _ce_typitems {},
                     _ce_todoque {},
                     _ce_globalvarmap {},
_ce_localvarmap {}
{
  if (!itm || itm==MOM_EMPTY_SLOT || itm->va_itype != MOMITY_ITEM)
    throw MOM_RUNTIME_ERROR("non item");
  lock_item(itm);
} // end MomEmitter::MomEmitter



void MomEmitter::scan_top_module(void)
{
  auto descitm = mom_unsync_item_descr(_ce_topitm);
  MOM_DEBUGPRINTF(gencod, "scan_top_module %s topitm=%s descitm=%s",
                  kindname(),
                  mom_item_cstring(_ce_topitm),
                  mom_item_cstring(descitm));
  if (descitm != MOM_PREDEFITM(module))
    throw MOM_RUNTIME_PRINTF("item %s has non-module descr: %s",
                             mom_item_cstring(_ce_topitm), mom_item_cstring(descitm));
  auto modulev = mom_unsync_item_get_phys_attr (_ce_topitm,  MOM_PREDEFITM(module));
  MOM_DEBUGPRINTF(gencod, "scan_top_module %s modulev %s",
                  kindname(),
                  mom_value_cstring(modulev));
  auto modulseq = mom_dyncast_seqitems (modulev);
  if (!modulseq)
    throw MOM_RUNTIME_PRINTF("item %s with bad module:%s",
                             mom_item_cstring(_ce_topitm), mom_value_cstring(modulev));
  bind_global(MOM_PREDEFITM(module),MOM_PREDEFITM(module),_ce_topitm);
  unsigned nbmodelem = mom_seqitems_length(modulseq);
  auto itemsarr = mom_seqitems_arr(modulseq);
  for (unsigned ix=0; ix<nbmodelem; ix++)
    {
      auto curitm = itemsarr[ix];
      if (curitm==nullptr) continue;
      assert (mom_itype(curitm)==MOMITY_ITEM);
      lock_item(curitm);
      todo([=](MomEmitter*em)
      {
        em->scan_module_element(curitm);
      });
      flush_todo_list(__LINE__);
    }
} // end of MomEmitter::scan_top_module


void MomEmitter::scan_module_element(struct mom_item_st*elitm)
{
  struct mom_item_st*descitm = mom_unsync_item_descr(elitm);
  MOM_DEBUGPRINTF(gencod, "scan_module_element %s topitm=%s elitm=%s descitm=%s",
                  kindname(),
                  mom_item_cstring(_ce_topitm),
                  mom_item_cstring(elitm), mom_item_cstring(descitm));
  if (!descitm)
    throw MOM_RUNTIME_PRINTF("module element %s without descr", mom_item_cstring(elitm));
#define NBMODELEMDESC_MOM 31
#define CASE_DESCR_MOM(Nam) momhashpredef_##Nam % NBMODELEMDESC_MOM:	\
	  if (descitm == MOM_PREDEFITM(Nam)) goto foundcase_##Nam;	\
	  goto defaultcasedesc; foundcase_##Nam
  switch (descitm->hva_hash % NBMODELEMDESC_MOM)
    {
    case CASE_DESCR_MOM (data):
      todo([=](MomEmitter*thisemit)
      {
        thisemit->scan_data_element(elitm);
      });
      break;
    case CASE_DESCR_MOM (func):
      todo([=](MomEmitter*thisemit)
      {
        thisemit->scan_func_element(elitm);
      });
      break;
    case CASE_DESCR_MOM (routine):
      todo([=](MomEmitter*thisemit)
      {
        thisemit->scan_routine_element(elitm);
      });
      break;
defaultcasedesc:
    default:
      throw MOM_RUNTIME_PRINTF("module element %s strange desc %s",
                               mom_item_cstring(elitm), mom_item_cstring(descitm));
    };
#undef NBMODELEMDESC_MOM
#undef CASE_DESCR_MOM
} // end of MomEmitter::scan_module_element



////////////////
void
MomEmitter::scan_data_element(struct mom_item_st*daitm)
{
  MOM_DEBUGPRINTF(gencod, "scan_data_element start daitm=%s", mom_item_cstring(daitm));
#warning MomEmitter::scan_data_element unimplemented
  MOM_FATAPRINTF("unimplemented scan_data_element daitm=%s", mom_item_cstring(daitm));
} // end  MomEmitter::scan_data_element



void
MomEmitter::scan_func_element(struct mom_item_st*fuitm)
{
  MOM_DEBUGPRINTF(gencod, "scan_func_element start fuitm=%s", mom_item_cstring(fuitm));
  assert (is_locked_item(fuitm));
  bind_global(MOM_PREDEFITM(func),MOM_PREDEFITM(func),fuitm);
  bind_global(fuitm,MOM_PREDEFITM(func),fuitm);
  auto sigitm = mom_dyncast_item(mom_unsync_item_get_phys_attr (fuitm, MOM_PREDEFITM(signature)));
  auto bdyitm = mom_dyncast_item(mom_unsync_item_get_phys_attr (fuitm, MOM_PREDEFITM(body)));
  MOM_DEBUGPRINTF(gencod, "scan_func_element fuitm=%s sigitm=%s bdyitm=%s",
                  mom_item_cstring(fuitm), mom_item_cstring(sigitm), mom_item_cstring(bdyitm));
  if (sigitm == nullptr)
    throw MOM_RUNTIME_PRINTF("missing signature in func %s", mom_item_cstring(fuitm));
  scan_signature(sigitm,fuitm);
  if (bdyitm == nullptr)
    throw MOM_RUNTIME_PRINTF("missing body in func %s", mom_item_cstring(fuitm));
  scan_block(bdyitm,fuitm);
#warning MomEmitter::scan_func_element unimplemented
  MOM_FATAPRINTF("unimplemented scan_func_element fuitm=%s", mom_item_cstring(fuitm));
} // end  MomEmitter::scan_func_element


////////////////
void
MomEmitter::scan_type(struct mom_item_st*typitm)
{
  MOM_DEBUGPRINTF(gencod, "scan_type start typitm=%s", mom_item_cstring(typitm));
  assert (is_locked_item(typitm));
  struct mom_item_st*desitm = mom_unsync_item_descr(typitm);
  if (desitm != MOM_PREDEFITM(type))
    throw MOM_RUNTIME_PRINTF("type %s has bad descr %s",
                             mom_item_cstring(typitm), mom_item_cstring(desitm));
  _ce_typitems.insert(typitm);
} // end  MomEmitter::scan_type



void
MomEmitter::scan_signature(struct mom_item_st*sigitm, struct mom_item_st*initm)
{
  MOM_DEBUGPRINTF(gencod, "scan_signature start sigitm=%s initm=%s",
                  mom_item_cstring(sigitm), mom_item_cstring(initm));
  assert (is_locked_item(sigitm));
  struct mom_item_st*desitm = mom_unsync_item_descr(sigitm);
  MOM_DEBUGPRINTF(gencod, "scan_signature desitm=%s", mom_item_cstring(desitm));
  if (desitm != MOM_PREDEFITM(signature))
    throw MOM_RUNTIME_PRINTF("in %s signature %s of bad descr %s",
                             mom_item_cstring(initm), mom_item_cstring(sigitm), mom_item_cstring(desitm));
  bind_local(MOM_PREDEFITM(signature),MOM_PREDEFITM(signature),sigitm);
  auto formtup =
    mom_dyncast_tuple(mom_unsync_item_get_phys_attr(sigitm, MOM_PREDEFITM(formals)));
  MOM_DEBUGPRINTF(gencod, "scan_signature sigitm=%s formtup=%s",
                  mom_item_cstring(sigitm), mom_value_cstring(formtup));
  if (formtup == nullptr)
    throw MOM_RUNTIME_PRINTF("missing formals in signature %s", mom_item_cstring(sigitm));
  bind_local(MOM_PREDEFITM(formals),MOM_PREDEFITM(formals),formtup);
  unsigned nbformals = mom_boxtuple_length(formtup);
  for (unsigned ix=0; ix<nbformals; ix++)
    {
      struct mom_item_st*curformitm = mom_boxtuple_nth(formtup,ix);
      if (!curformitm)
        throw MOM_RUNTIME_PRINTF("missing formal#%d in signature %s",
                                 ix, mom_item_cstring(sigitm));
      lock_item(curformitm);
      if (is_bound(curformitm))
        throw MOM_RUNTIME_PRINTF("already bound formal#%d %s in signature %s",
                                 ix, mom_item_cstring(curformitm), mom_item_cstring(sigitm));
      MOM_DEBUGPRINTF(gencod, "formal#%d in signature %s is %s",
                      ix, mom_item_cstring(sigitm),
                      mom_item_cstring(curformitm));
      if (mom_unsync_item_descr(curformitm) != MOM_PREDEFITM(formal))
        throw MOM_RUNTIME_PRINTF("bad formal#%d %s in signature %s",
                                 ix, mom_item_cstring(curformitm),
                                 mom_item_cstring(sigitm));
      struct mom_item_st*typfitm =
      mom_dyncast_item(mom_unsync_item_get_phys_attr(curformitm,MOM_PREDEFITM(type)));
      MOM_DEBUGPRINTF(gencod, "formal#%d %s has type %s",
                      ix, mom_item_cstring(curformitm), mom_item_cstring(typfitm));
      if (!typfitm)
        throw MOM_RUNTIME_PRINTF("untyped formal#%d %s in signature %s",
                                 ix, mom_item_cstring(curformitm),
                                 mom_item_cstring(sigitm));
      lock_item(typfitm);
      scan_type(typfitm);
      bind_local(curformitm, MOM_PREDEFITM(formal), sigitm, typfitm, ix);
    }
  auto resv = mom_unsync_item_get_phys_attr(sigitm, MOM_PREDEFITM(result));
  MOM_DEBUGPRINTF(gencod, "scan_signature sigitm=%s resv=%s",
                  mom_item_cstring(sigitm), mom_value_cstring(resv));
  unsigned restyp = mom_itype(resv);
  if (restyp == MOMITY_ITEM)
    {
      auto restypitm = reinterpret_cast<struct mom_item_st*>((void*)resv);
      lock_item(restypitm);
      scan_type(restypitm);
    }
  else if (restyp == MOMITY_TUPLE)
    {
      auto restytup = reinterpret_cast<const struct mom_boxtuple_st*>(resv);
      unsigned nbresults = mom_boxtuple_length(restytup);
      for (unsigned rix=0; rix<nbresults; rix++)
        {
          struct mom_item_st*curtyitm = mom_boxtuple_nth(restytup,rix);
          if (!curtyitm || curtyitm==MOM_PREDEFITM(unit))
            throw MOM_RUNTIME_PRINTF("missing type#%d in result %s of signature %s",
                                     rix, mom_value_cstring(resv), mom_item_cstring(sigitm));
          lock_item(curtyitm);
          scan_type(curtyitm);
        }
    }
  else if (restyp != MOMITY_NONE)
    throw  MOM_RUNTIME_PRINTF("invalid result %s for signature %s",
                              mom_value_cstring(resv), mom_item_cstring(sigitm));
  MOM_DEBUGPRINTF(gencod, "scan_signature sigitm=%s done",
                  mom_item_cstring(sigitm));
} // end MomEmitter::scan_signature


void
MomEmitter::scan_block(struct mom_item_st*blkitm, struct mom_item_st*initm)
{
  MOM_DEBUGPRINTF(gencod, "scan_block start blkitm=%s initm=%s",
                  mom_item_cstring(blkitm), mom_item_cstring(initm));
  assert (is_locked_item(blkitm));
  struct mom_item_st*desitm = mom_unsync_item_descr(blkitm);
  MOM_DEBUGPRINTF(gencod, "scan_block desitm=%s", mom_item_cstring(desitm));
  if (is_bound(blkitm))
    throw MOM_RUNTIME_PRINTF("in %s block %s already bound",
                             mom_item_cstring(initm),
                             mom_item_cstring(blkitm));
  auto whilexpv =  mom_unsync_item_get_phys_attr (blkitm, MOM_PREDEFITM(while));
  auto bodytup = mom_dyncast_tuple(mom_unsync_item_get_phys_attr (blkitm, MOM_PREDEFITM(body)));
  MOM_DEBUGPRINTF(gencod, "scan_block blkitm=%s whilexpv=%s bodytup=%s",
                  mom_item_cstring(blkitm), mom_value_cstring(whilexpv), mom_value_cstring(bodytup));
  if (bodytup == nullptr)
    throw  MOM_RUNTIME_PRINTF("in %s block %s without body",
                              mom_item_cstring(initm),
                              mom_item_cstring(blkitm));
  if (desitm ==  MOM_PREDEFITM(sequence))
    {
      if (whilexpv != nullptr)
        throw MOM_RUNTIME_PRINTF("in %s sequence %s with while:%s",
                                 mom_item_cstring(initm),
                                 mom_item_cstring(blkitm),
                                 mom_value_cstring(whilexpv));
      bind_local(blkitm, MOM_PREDEFITM(sequence), initm);
      auto localseq = mom_dyncast_seqitems(mom_unsync_item_get_phys_attr (blkitm, MOM_PREDEFITM(locals)));
      unsigned nblocals = mom_seqitems_length(localseq);
      MOM_DEBUGPRINTF(gencod, "scan_block blkitm=%s localseq=%s", mom_item_cstring(blkitm), mom_value_cstring(localseq));
      for (unsigned locix=0; locix<nblocals; locix++)
        {
          auto curlocitm = const_cast<struct mom_item_st*>(mom_seqitems_nth(localseq, locix));
          MOM_DEBUGPRINTF(gencod, "local#%d in sequence %s is %s", locix, mom_item_cstring(blkitm), mom_item_cstring(curlocitm));
          if (mom_itype(curlocitm) != MOMITY_ITEM)
            throw MOM_RUNTIME_PRINTF("in %s sequence %s with bad local#%d from %s",
                                     mom_item_cstring(initm),
                                     mom_item_cstring(blkitm),
                                     locix, mom_value_cstring(localseq));
          lock_item(curlocitm);
          if (is_bound(curlocitm))
            throw MOM_RUNTIME_PRINTF("in %s sequence %s with already bound local#%d %s",
                                     mom_item_cstring(initm),
                                     mom_item_cstring(blkitm),
                                     locix, mom_item_cstring(curlocitm));
          auto deslocitm =  mom_unsync_item_descr(curlocitm);
          if (deslocitm != MOM_PREDEFITM(variable))
            throw MOM_RUNTIME_PRINTF("in %s sequence %s with local#%d %s non-variable",
                                     mom_item_cstring(initm),
                                     mom_item_cstring(blkitm),
                                     locix, mom_item_cstring(curlocitm));
          struct mom_item_st*typlocitm =
          mom_dyncast_item(mom_unsync_item_get_phys_attr(curlocitm,MOM_PREDEFITM(type)));
          MOM_DEBUGPRINTF(gencod, "local %s has type %s", mom_item_cstring(curlocitm), mom_item_cstring(typlocitm));
          if (!typlocitm)
            throw  MOM_RUNTIME_PRINTF("in %s sequence %s with local#%d %s without type",
                                      mom_item_cstring(initm),
                                      mom_item_cstring(blkitm),
                                      locix, mom_item_cstring(curlocitm));
          lock_item(typlocitm);
          scan_type(typlocitm);
          bind_local(curlocitm, MOM_PREDEFITM(variable), blkitm, typlocitm, locix);
        }
    }
  else if (desitm == MOM_PREDEFITM(loop))
    {
      auto localsv = mom_unsync_item_get_phys_attr (blkitm, MOM_PREDEFITM(locals));
      if (localsv)
        throw MOM_RUNTIME_PRINTF("in %s loop %s with locals %s",
                                 mom_item_cstring(initm),
                                 mom_item_cstring(blkitm),
                                 mom_value_cstring(localsv));
      bind_local(blkitm, MOM_PREDEFITM(loop), initm);
      if (whilexpv)
        scan_expr(whilexpv,blkitm,0);
    }
  else
    throw MOM_RUNTIME_PRINTF("in %s block %s of bad descr %s",
                             mom_item_cstring(initm),
                             mom_item_cstring(blkitm), mom_item_cstring(desitm));
  flush_todo_list(__LINE__);
  unsigned bodylen = mom_boxtuple_length(bodytup);
  for (unsigned ix=0; ix<bodylen; ix++)
    {
      struct mom_item_st*insitm = mom_boxtuple_nth(bodytup,ix);
      if (mom_itype(insitm) != MOMITY_ITEM)
        throw  MOM_RUNTIME_PRINTF("in %s block %s has bad instr#%d",
                                  mom_item_cstring(initm),
                                  mom_item_cstring(blkitm), ix);
      lock_item(insitm);
      scan_instr(insitm, (int)ix, blkitm);
    }
  flush_todo_list(__LINE__);
} // end MomEmitter::scan_block


void MomEmitter::scan_instr(struct mom_item_st*insitm, int rk, struct mom_item_st*blkitm)
{
  MOM_DEBUGPRINTF(gencod, "scan_instr start insitm=%s rk#%d blkitm=%s",
                  mom_item_cstring(insitm), rk, mom_item_cstring(blkitm));
  assert (is_locked_item(insitm));
  struct mom_item_st*desitm = mom_unsync_item_descr(insitm);
  MOM_DEBUGPRINTF(gencod, "scan_instr insitm=%s desitm=%s", mom_item_cstring(insitm),  mom_item_cstring(desitm));
  if (!desitm)
    throw MOM_RUNTIME_PRINTF("instr %s #%d of %s without descr",
                             mom_item_cstring(insitm), rk, mom_item_cstring(blkitm));
#define NBMODOPER_MOM 97
#define CASE_OPER_MOM(Nam) momhashpredef_##Nam % NBMODOPER_MOM:		\
	  if (desitm == MOM_PREDEFITM(Nam)) goto foundcase_##Nam;	\
	  goto defaultcasedesc; foundcase_##Nam
  switch (desitm->hva_hash % NBMODOPER_MOM)
    {
    case CASE_OPER_MOM(assign):
      {
      }
    break;
    default:
defaultcasedesc:
      throw MOM_RUNTIME_PRINTF("instr %s #%d of %s with unexpected descr %s",
                               mom_item_cstring(insitm), rk, mom_item_cstring(blkitm),
                               mom_item_cstring(desitm));
#undef CASE_OPER_MOM
#undef NBMODOPER_MOM
    }

#warning MomEmitter::scan_instr unimplemented
  MOM_FATAPRINTF("unimplemented scan_instr insitm=%s rk#%d blkitm=%s",
                 mom_item_cstring(insitm), rk, mom_item_cstring(blkitm));
} // end of MomEmitter::scan_instr


void MomEmitter::scan_expr(const void*expv, struct mom_item_st*insitm, int depth, struct mom_item_st*typitm)
{
  MOM_DEBUGPRINTF(gencod, "scan_expr start expv=%s insitm=%s depth#%d typitm=%s",
                  mom_value_cstring(expv), mom_item_cstring(insitm), depth, mom_item_cstring(typitm));
  if (depth >= MAX_DEPTH_EXPR)
    throw  MOM_RUNTIME_PRINTF("expr %s in instr %s is too deep (%d)",
                              mom_value_cstring(expv), mom_item_cstring(insitm), depth);
  if (typitm)
    {
      lock_item(typitm);
      scan_type(typitm);
    }
  unsigned typexp = mom_itype(expv);
  switch (typexp)
    {
    case MOMITY_NONE:
      break;
    case MOMITY_INT:
      if (typitm && typitm != MOM_PREDEFITM(int) && typitm != MOM_PREDEFITM(value))
        throw MOM_RUNTIME_PRINTF("int.expr %s in instr %s with type mismatch for %s",
                                 mom_value_cstring(expv), mom_item_cstring(insitm), mom_item_cstring(typitm));
      break;
    case MOMITY_BOXSTRING:
      if (typitm && typitm != MOM_PREDEFITM(string) && typitm != MOM_PREDEFITM(value))
        throw MOM_RUNTIME_PRINTF("string.expr %s in instr %s with type mismatch for %s",
                                 mom_value_cstring(expv), mom_item_cstring(insitm), mom_item_cstring(typitm));
      break;
    case MOMITY_BOXDOUBLE:
      if (typitm && typitm != MOM_PREDEFITM(double) && typitm != MOM_PREDEFITM(value))
        throw MOM_RUNTIME_PRINTF("double.expr %s in instr %s with type mismatch for %s",
                                 mom_value_cstring(expv), mom_item_cstring(insitm), mom_item_cstring(typitm));
      break;
    case MOMITY_ITEM:
    {
      auto itmexp = (struct mom_item_st*)expv;
      lock_item(itmexp);
      scan_item_expr(itmexp,insitm,depth,typitm);
      break;
    }
    case MOMITY_NODE:
    {
      auto nodexp = (const struct mom_boxnode_st*)expv;
      scan_node_expr(nodexp,insitm,depth,typitm);
      break;
    }
    default:
      throw MOM_RUNTIME_PRINTF("unexpected expr %s in instr %s with type %s",
                               mom_value_cstring(expv), mom_item_cstring(insitm), mom_item_cstring(typitm));
    }
#warning MomEmitter::scan_expr unimplemented
  MOM_FATAPRINTF("unimplemented scan_expr expv=%s insitm=%s depth#%d typitm=%s",
                 mom_value_cstring(expv), mom_item_cstring(insitm), depth, mom_item_cstring(typitm));
} // end of MomEmitter::scan_expr


void
MomEmitter::scan_node_expr(const struct mom_boxnode_st*expnod, struct mom_item_st*insitm, int depth, struct mom_item_st*typitm)
{
  MOM_DEBUGPRINTF(gencod, "scan_node_expr start expnod=%s insitm=%s depth#%d typitm=%s",
                  mom_value_cstring(expnod), mom_item_cstring(insitm), depth, mom_item_cstring(typitm));
#warning MomEmitter::scan_var unimplemented
  MOM_FATAPRINTF("unimplemented scan_node_expr expnod=%s insitm=%s depth#%d typitm=%s",
                 mom_value_cstring(expnod), mom_item_cstring(insitm), depth, mom_item_cstring(typitm));
} // end of MomEmitter::scan_node_expr


void
MomEmitter::scan_item_expr(struct mom_item_st*expitm, struct mom_item_st*insitm, int depth, struct mom_item_st*typitm)
{
  MOM_DEBUGPRINTF(gencod, "scan_item_expr start expitm=%s insitm=%s depth#%d typitm=%s",
                  mom_item_cstring(expitm), mom_item_cstring(insitm), depth, mom_item_cstring(typitm));
#warning MomEmitter::scan_item_expr unimplemented
  MOM_FATAPRINTF("unimplemented scan_item_expr expitm=%s insitm=%s depth#%d typitm=%s",
                 mom_item_cstring(expitm), mom_item_cstring(insitm), depth, mom_item_cstring(typitm));
} // end of MomEmitter::scan_node_expr


void MomEmitter::scan_var(struct mom_item_st*varitm, struct mom_item_st*insitm, struct mom_item_st*typitm)
{
  MOM_DEBUGPRINTF(gencod, "scan_var start varitm=%s insitm=%s typitm=%s",
                  mom_item_cstring(varitm), mom_item_cstring(insitm), mom_item_cstring(typitm));
#warning MomEmitter::scan_var unimplemented
  MOM_FATAPRINTF("unimplemented scan_var varitm=%s insitm=%s",
                 mom_item_cstring(varitm), mom_item_cstring(insitm), mom_item_cstring(typitm));
} // end of MomEmitter::scan_var

void
MomEmitter::scan_routine_element(struct mom_item_st*rtitm)
{
  MOM_DEBUGPRINTF(gencod, "scan_routine_element start rtitm=%s", mom_item_cstring(rtitm));
} // end  MomEmitter::scan_routine_element

MomEmitter::~MomEmitter()
{
  int nbit = _ce_veclockeditems.size();
  for (int ix=nbit-1; ix>=0; ix--)
    mom_item_unlock(_ce_veclockeditems[ix]);
  _ce_veclockeditems.clear();
  _ce_setlockeditems.clear();
} // end MomEmitter::~MomEmitter


////////////////////////////////////////////////////////////////
MomCEmitter::~MomCEmitter()
{
  MOM_DEBUGPRINTF(gencod, "end %s for this@%p", kindname(), this);
} // end MomCEmitter::~MomCEmitter

MomJavascriptEmitter::~MomJavascriptEmitter()
{
  MOM_DEBUGPRINTF(gencod, "end %s for this@%p", kindname(), this);
} // end MomJavascriptEmitter::~MomJavascriptEmitter

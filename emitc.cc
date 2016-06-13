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
  typedef std::set<const void*,
          MomValueLess,
          traceable_allocator<struct mom_item_st*>>
            traced_set_values_t;
  struct vardef_st
  {
    struct mom_item_st* vd_rolitm;
    const void* vd_what;
    const void* vd_detail;
    long vd_rank;
  };
  struct sigdef_st
  {
    const struct mom_boxtuple_st* sig_formals;
    const void*sig_result;
  };
  typedef std::map<const struct mom_item_st*,
    vardef_st,
    MomItemLess,
    traceable_allocator<struct mom_item_st*>>
                                           traced_varmap_t;
  static const unsigned constexpr NB_MAX_BLOCKS = 65536;
  static const unsigned constexpr NB_MAX_INSTRS = 1048576;
private:
  const unsigned _ce_magic;
  struct mom_item_st* _ce_topitm;
  std::vector<struct mom_item_st*,traceable_allocator<struct mom_item_st*>> _ce_veclockeditems;
  traced_set_items_t _ce_setlockeditems;
  traced_set_items_t _ce_sigitems;
  traced_set_items_t _ce_typitems;
  traced_set_items_t _ce_blockitems;
  traced_set_items_t _ce_instritems;
  std::deque<todofun_t,traceable_allocator<todofun_t>> _ce_todoque;
  traced_varmap_t _ce_globalvarmap;
  traced_varmap_t _ce_localvarmap;
  traced_set_values_t _ce_localvalueset;
  traced_set_items_t _ce_localcloseditems;
  struct mom_item_st*_ce_curfunctionitm;
protected:
  MomEmitter(unsigned magic, struct mom_item_st*itm);
  MomEmitter(const MomEmitter&) = delete;
  virtual ~MomEmitter();
  virtual void scan_data_element(struct mom_item_st*itm);
  virtual void scan_func_element(struct mom_item_st*itm);
  virtual void scan_routine_element(struct mom_item_st*itm);
struct sigdef_st scan_signature(struct mom_item_st*sigitm, struct mom_item_st*initm, bool nobind=false);
  struct sigdef_st scan_nonbinding_signature(struct mom_item_st*sigitm, struct mom_item_st*initm)
  {
    return scan_signature(sigitm,initm,true);
  }
  void scan_block(struct mom_item_st*blockitm, struct mom_item_st*initm);
  void scan_instr(struct mom_item_st*insitm, int rk, struct mom_item_st*blkitm);
  virtual void scan_special_instr(struct mom_item_st*insitm, struct mom_item_st*desitm, int rk, struct mom_item_st*blkitm)
  {
    throw MOM_RUNTIME_PRINTF("unexpected special instruction %s of descriptor %s rank#%d in block %s",
                             mom_item_cstring(insitm),
                             mom_item_cstring(desitm),
                             rk,
                             mom_item_cstring(blkitm));
  }
  void scan_type(struct mom_item_st*typitm);
  struct mom_item_st*current_function(void) const
  {
    return _ce_curfunctionitm;
  };
  // scanning of expressions & variables return their type item
  struct mom_item_st* scan_expr(const void*expv, struct mom_item_st*insitm, int depth, struct mom_item_st*typitm=nullptr);
  struct mom_item_st* scan_node_expr(const struct mom_boxnode_st*expnod, struct mom_item_st*insitm, int depth, struct mom_item_st*typitm=nullptr);
  struct mom_item_st* scan_node_descr_conn_expr(const struct mom_boxnode_st*expnod,
      struct mom_item_st*desconnitm,
      struct mom_item_st*insitm,
      int depth, struct mom_item_st*typitm=nullptr);
  struct mom_item_st* scan_item_expr(struct mom_item_st*expitm, struct mom_item_st*insitm, int depth, struct mom_item_st*typitm=nullptr);
  struct mom_item_st* scan_var(struct mom_item_st*varitm, struct mom_item_st*insitm, struct mom_item_st*typitm=nullptr);
  struct mom_item_st* scan_closed(struct mom_item_st*varitm, struct mom_item_st*insitm);
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
  vardef_st*get_local_binding(const struct mom_item_st*itm) const
  {
    if (mom_itype(itm) != MOMITY_ITEM) return nullptr;
    auto it = _ce_localvarmap.find(itm);
    if (it != _ce_localvarmap.end())
      return const_cast<vardef_st*>(&it->second);
    return nullptr;
  }
  vardef_st*get_global_binding(const struct mom_item_st*itm) const
  {
    if (mom_itype(itm) != MOMITY_ITEM) return nullptr;
    auto it = _ce_globalvarmap.find(itm);
    if (it != _ce_globalvarmap.end())
      return const_cast<vardef_st*>(&it->second);
    return nullptr;
  }
  bool is_bound(const struct mom_item_st*itm) const
  {
    return _ce_localvarmap.find(itm) != _ce_localvarmap.end()
           || _ce_globalvarmap.find(itm) != _ce_globalvarmap.end();
  };
  bool is_locally_bound(const struct mom_item_st*itm) const
  {
    return _ce_localvarmap.find(itm) != _ce_localvarmap.end();
  }
  bool is_globally_bound(const struct mom_item_st*itm) const
  {
    return _ce_globalvarmap.find(itm) != _ce_globalvarmap.end();
  }
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
    long count = 0;
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
        count++;
      }
    _ce_todoque.shrink_to_fit();
    if (lin)
      MOM_DEBUGPRINTF_AT(__FILE__,lin,gencod,"flush_todo_list %s done %ld todos",
                         kindname(), count);
    else
      MOM_DEBUGPRINTF(gencod,"flush_todo_list %s done %ld todos",
                      kindname(), count);
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
public:
  static unsigned constexpr MAGIC = 508723037 /*0x1e527f5d*/;
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
public:
  static unsigned constexpr MAGIC = 852389659 /*0x32ce6f1b*/;
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
  MomCEmitter cemit {itm};
  try
    {
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
                        "mom_emit_c_code %s failed with MOM runtime exception %s in %s",
                        mom_item_cstring(itm), e.what(), mom_item_cstring(cemit.current_function()));
      return false;
    }
  catch (const std::exception& e)
    {
      MOM_WARNPRINTF("mom_emit_c_code %s failed with exception %s in %s",
                     mom_item_cstring(itm), e.what(), mom_item_cstring(cemit.current_function()));
      return false;
    }
  catch (...)
    {
      MOM_WARNPRINTF("mom_emit_c_code %s failed in %s",
                     mom_item_cstring(itm), mom_item_cstring(cemit.current_function()));
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
  MomJavascriptEmitter jsemit {itm};
  try
    {
      jsemit.scan_top_module();
      jsemit.flush_todo_list(__LINE__);
#warning mom_emit_javascript_code incomplete
      MOM_FATAPRINTF("unimplemented mom_emit_javascript_code %s",
                     mom_item_cstring(itm));
    }
  catch (const MomRuntimeErrorAt& e)
    {
      MOM_WARNPRINTF_AT(e.file(), e.lineno(),
                        "mom_emit_javascript_code %s failed with MOM runtime exception %s in %s",
                        mom_item_cstring(itm), e.what(), mom_item_cstring(jsemit.current_function()));
      return false;
    }
  catch (const std::exception& e)
    {
      MOM_WARNPRINTF("mom_emit_javascript_code %s failed with exception %s in %s",
                     mom_item_cstring(itm), e.what(), mom_item_cstring(jsemit.current_function()));
      return false;
    }
  catch (...)
    {
      MOM_WARNPRINTF("mom_emit_javascript_code %s failed in %s",
                     mom_item_cstring(itm), mom_item_cstring(jsemit.current_function()));
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
                     _ce_blockitems {},
                     _ce_instritems {},
                     _ce_todoque {},
                     _ce_globalvarmap {},
                     _ce_localvarmap {},
_ce_curfunctionitm {nullptr}
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
        MOM_DEBUGPRINTF(gencod, "before scanning module element %s #%d from top module %s",
                        mom_item_cstring(curitm), ix, mom_item_cstring(_ce_topitm));
        em->scan_module_element(curitm);
        MOM_DEBUGPRINTF(gencod, "after scanning module element %s #%d from top module %s",
                        mom_item_cstring(curitm), ix, mom_item_cstring(_ce_topitm));
      });
      flush_todo_list(__LINE__);
      _ce_curfunctionitm = nullptr;
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
    case CASE_DESCR_MOM (global):
    case CASE_DESCR_MOM (thread_local):
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
  _ce_curfunctionitm = fuitm;
  bind_global(MOM_PREDEFITM(func),MOM_PREDEFITM(func),fuitm);
  bind_global(fuitm,MOM_PREDEFITM(func),fuitm);
  auto sigitm = mom_dyncast_item(mom_unsync_item_get_phys_attr (fuitm, MOM_PREDEFITM(signature)));
  auto bdyitm = mom_dyncast_item(mom_unsync_item_get_phys_attr (fuitm, MOM_PREDEFITM(body)));
  MOM_DEBUGPRINTF(gencod, "scan_func_element fuitm=%s sigitm=%s bdyitm=%s",
                  mom_item_cstring(fuitm), mom_item_cstring(sigitm), mom_item_cstring(bdyitm));
  if (sigitm == nullptr)
    throw MOM_RUNTIME_PRINTF("missing signature in func %s", mom_item_cstring(fuitm));
  lock_item(sigitm);
  {
    auto sigr=scan_signature(sigitm,fuitm);
    auto formaltup = sigr.sig_formals;
    if (mom_boxtuple_length(formaltup)==0
        || mom_boxtuple_nth(formaltup, 0) != MOM_PREDEFITM(this_closure))
      throw MOM_RUNTIME_PRINTF("func %s with signature %s of formals %s not starting with `this_closure`",
                               mom_item_cstring(fuitm),
                               mom_item_cstring(sigitm),
                               mom_value_cstring(formaltup));
    if (sigr.sig_result != MOM_PREDEFITM(value))
      throw  MOM_RUNTIME_PRINTF("func %s with signature %s of non-value result type %s",
                                mom_item_cstring(fuitm),
                                mom_item_cstring(sigitm),
                                mom_value_cstring(sigr.sig_result));
  }
  if (bdyitm == nullptr)
    throw MOM_RUNTIME_PRINTF("missing body in func %s", mom_item_cstring(fuitm));
  scan_block(bdyitm,fuitm);
  _ce_curfunctionitm = nullptr;
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



MomEmitter::sigdef_st
MomEmitter::scan_signature(struct mom_item_st*sigitm, struct mom_item_st*initm, bool nobind)
{
  MOM_DEBUGPRINTF(gencod, "scan_signature start sigitm=%s initm=%s nobind %s",
                  mom_item_cstring(sigitm), mom_item_cstring(initm), nobind?"true":"false");
  assert (is_locked_item(sigitm));
  struct mom_item_st*desitm = mom_unsync_item_descr(sigitm);
  MOM_DEBUGPRINTF(gencod, "scan_signature desitm=%s", mom_item_cstring(desitm));
  if (desitm != MOM_PREDEFITM(signature))
    throw MOM_RUNTIME_PRINTF("in %s signature %s of bad descr %s",
                             mom_item_cstring(initm), mom_item_cstring(sigitm), mom_item_cstring(desitm));
  if (!nobind) bind_local(MOM_PREDEFITM(signature),MOM_PREDEFITM(signature),sigitm);
  auto formtup =
    mom_dyncast_tuple(mom_unsync_item_get_phys_attr(sigitm, MOM_PREDEFITM(formals)));
  MOM_DEBUGPRINTF(gencod, "scan_signature sigitm=%s formtup=%s",
                  mom_item_cstring(sigitm), mom_value_cstring(formtup));
  if (formtup == nullptr)
    throw MOM_RUNTIME_PRINTF("missing formals in signature %s", mom_item_cstring(sigitm));
  if (!nobind) bind_local(MOM_PREDEFITM(formals),MOM_PREDEFITM(formals),formtup);
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
      if (!nobind) bind_local(curformitm, MOM_PREDEFITM(formal), sigitm, typfitm, ix);
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
  MOM_DEBUGPRINTF(gencod, "scan_signature sigitm=%s done formals=%s result=%s",
                  mom_item_cstring(sigitm), mom_value_cstring(formtup), mom_value_cstring(resv));
  return {formtup,resv};
} // end MomEmitter::scan_signature


void
MomEmitter::scan_block(struct mom_item_st*blkitm, struct mom_item_st*initm)
{
  MOM_DEBUGPRINTF(gencod, "scan_block start blkitm=%s initm=%s",
                  mom_item_cstring(blkitm), mom_item_cstring(initm));
  assert (is_locked_item(blkitm));
  if (_ce_blockitems.size() >= NB_MAX_BLOCKS)
    throw  MOM_RUNTIME_PRINTF("in %s block %s overflowing %d",
                              mom_item_cstring(initm),
                              mom_item_cstring(blkitm),
                              (int) (_ce_blockitems.size()));
  struct mom_item_st*desitm = mom_unsync_item_descr(blkitm);
  MOM_DEBUGPRINTF(gencod, "scan_block  blkitm=%s desitm=%s",  mom_item_cstring(blkitm), mom_item_cstring(desitm));
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
  _ce_blockitems.insert(blkitm);
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
        {
          auto whiltypitm = scan_expr(whilexpv,blkitm,0);
          if (whiltypitm == nullptr || whiltypitm == MOM_PREDEFITM(unit))
            throw MOM_RUNTIME_PRINTF("in %s loop %s has bad while type %s",
                                     mom_item_cstring(initm),
                                     mom_item_cstring(blkitm),
                                     mom_item_cstring(whiltypitm));
        }
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
  unbind(blkitm);
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
  if (_ce_instritems.size() >= NB_MAX_INSTRS)
    throw  MOM_RUNTIME_PRINTF("instr %s #%d of %s overflowing %d",
                              mom_item_cstring(insitm), rk, mom_item_cstring(blkitm),
                              (int)(_ce_instritems.size()));
  {
    auto insbind = get_binding(insitm);
    if (insbind)
      throw MOM_RUNTIME_PRINTF("instr %s #%d of %s already bound with role %s",
                               mom_item_cstring(insitm), rk, mom_item_cstring(blkitm),
                               mom_item_cstring(insbind->vd_rolitm));
    bind_local(insitm, MOM_PREDEFITM(instruction), blkitm, desitm, rk);
  }
#define NBMODOPER_MOM 97
#define CASE_OPER_MOM(Nam) momhashpredef_##Nam % NBMODOPER_MOM:		\
	  if (desitm == MOM_PREDEFITM(Nam)) goto foundcase_##Nam;	\
	  goto defaultcasedesc; foundcase_##Nam
  switch (desitm->hva_hash % NBMODOPER_MOM)
    {
    case CASE_OPER_MOM(assign):
    {
      auto tovaritm = mom_dyncast_item(mom_unsync_item_get_phys_attr(insitm, MOM_PREDEFITM(to)));
      auto fromexp = mom_unsync_item_get_phys_attr(insitm, MOM_PREDEFITM(from));
      if (insitm->itm_pcomp && mom_vectvaldata_count(insitm->itm_pcomp)>0)
        throw  MOM_RUNTIME_PRINTF("assign %s #%d in block %s with unexpected %d components",
                                  mom_item_cstring(insitm), rk,
                                  mom_item_cstring(blkitm),
                                  mom_vectvaldata_count(insitm->itm_pcomp));
      if (!tovaritm)
        throw  MOM_RUNTIME_PRINTF("assign %s #%d in block %s of %s without `to`",
                                  mom_item_cstring(insitm), rk,
                                  mom_item_cstring(blkitm), mom_item_cstring(blkitm));
      auto totypitm = scan_var(tovaritm,insitm);
      auto fromtypitm = fromexp?scan_expr(fromexp,insitm,0,totypitm):totypitm;
      if (!fromtypitm)
        throw MOM_RUNTIME_PRINTF("assign %s #%d in block %s with untypable `from` %s",
                                 mom_item_cstring(insitm), rk,
                                 mom_item_cstring(blkitm), mom_value_cstring(fromexp));
      if (totypitm != fromtypitm)
        throw MOM_RUNTIME_PRINTF("assign %s #%d in block %s with incompatible types from type %s to type %s",
                                 mom_item_cstring(insitm), rk,
                                 mom_item_cstring(blkitm), mom_item_cstring(fromtypitm), mom_item_cstring(totypitm));
    }
    break;
    case CASE_OPER_MOM(break):
    {
      auto outblkitm= mom_dyncast_item(mom_unsync_item_get_phys_attr(insitm, MOM_PREDEFITM(block)));
      if (!outblkitm)
        throw MOM_RUNTIME_PRINTF("break %s #%d in block %s without `block`",
                                 mom_item_cstring(insitm), rk,
                                 mom_item_cstring(blkitm));
      if (insitm->itm_pcomp && mom_vectvaldata_count(insitm->itm_pcomp)>0)
        throw  MOM_RUNTIME_PRINTF("break %s #%d in block %s with unexpected %d components",
                                  mom_item_cstring(insitm), rk,
                                  mom_item_cstring(blkitm),
                                  mom_vectvaldata_count(insitm->itm_pcomp));
      auto blkbind = get_local_binding(outblkitm);
      if (!blkbind)
        throw MOM_RUNTIME_PRINTF("break %s #%d in block %s from non-nested out `block`:%s",
                                 mom_item_cstring(insitm), rk,
                                 mom_item_cstring(blkitm),
                                 mom_item_cstring(outblkitm));
      if (blkbind->vd_rolitm != MOM_PREDEFITM(loop) && blkbind->vd_rolitm != MOM_PREDEFITM(sequence))
        throw MOM_RUNTIME_PRINTF("break %s #%d in block %s from `block`:%s with strange role %s",
                                 mom_item_cstring(insitm), rk,
                                 mom_item_cstring(blkitm),
                                 mom_item_cstring(outblkitm),
                                 mom_item_cstring(blkbind->vd_rolitm));
    }
    break;
    case CASE_OPER_MOM(continue):
    {
      auto loopitm= mom_dyncast_item(mom_unsync_item_get_phys_attr(insitm, MOM_PREDEFITM(loop)));
      if (!loopitm)
        throw MOM_RUNTIME_PRINTF("continue %s #%d in block %s without `loop`",
                                 mom_item_cstring(insitm), rk,
                                 mom_item_cstring(blkitm));
      if (insitm->itm_pcomp && mom_vectvaldata_count(insitm->itm_pcomp)>0)
        throw  MOM_RUNTIME_PRINTF("continue %s #%d in block %s with unexpected %d components",
                                  mom_item_cstring(insitm), rk,
                                  mom_item_cstring(blkitm),
                                  mom_vectvaldata_count(insitm->itm_pcomp));
      auto loopbind = get_local_binding(loopitm);
      if (!loopbind)
        throw MOM_RUNTIME_PRINTF("continue %s #%d in block %s from non-nested out `loop`:%s",
                                 mom_item_cstring(insitm), rk,
                                 mom_item_cstring(blkitm),
                                 mom_item_cstring(loopitm));
      if (loopbind->vd_rolitm != MOM_PREDEFITM(loop))
        throw MOM_RUNTIME_PRINTF("continue %s #%d in block %s from `loop`:%s with strange role %s",
                                 mom_item_cstring(insitm), rk,
                                 mom_item_cstring(blkitm),
                                 mom_item_cstring(loopitm),
                                 mom_item_cstring(loopbind->vd_rolitm));
    }
    break;
    case CASE_OPER_MOM(loop):
    case CASE_OPER_MOM(sequence):
    {
      MOM_DEBUGPRINTF(gencod, "scan_instr nested block insitm=%s rk#%d blkitm=%s",
                      mom_item_cstring(insitm), rk, mom_item_cstring(blkitm));
      todo([=](MomEmitter*em)
      {
        MOM_DEBUGPRINTF(gencod, "before scanning nested block insitm=%s rk#%d blkitm=%s",
                        mom_item_cstring(insitm), rk, mom_item_cstring(blkitm));
        em->scan_block(insitm, blkitm);
        MOM_DEBUGPRINTF(gencod, "after scanning nested block insitm=%s rk#%d blkitm=%s",
                        mom_item_cstring(insitm), rk, mom_item_cstring(blkitm));
      });
    }
    break;
    case CASE_OPER_MOM(cond):
    {
      auto condtup= mom_dyncast_tuple(mom_unsync_item_get_phys_attr(insitm, MOM_PREDEFITM(loop)));
      if (!condtup)
        throw MOM_RUNTIME_PRINTF("cond %s #%d in block %s without `cond`",
                                 mom_item_cstring(insitm), rk,
                                 mom_item_cstring(blkitm));
      unsigned nbcond = mom_raw_size(condtup);
      for (unsigned ix=0; ix<nbcond; ix++)
        {
          auto testitm = condtup->seqitem[ix];
          if (!testitm || mom_itype(testitm) != MOMITY_ITEM)
            throw
            MOM_RUNTIME_PRINTF("cond %s #%d in block %s missing test#%d",
                               mom_item_cstring(insitm), rk,
                               mom_item_cstring(blkitm),
                               ix);
          lock_item(testitm);
          {
            struct mom_item_st*destestitm = mom_unsync_item_descr(testitm);
            if (destestitm != MOM_PREDEFITM(test))
              throw
              MOM_RUNTIME_PRINTF("cond %s #%d in block %s with test#%d %s having bad descr %s",
                                 mom_item_cstring(insitm), rk,
                                 mom_item_cstring(blkitm),
                                 ix,
                                 mom_item_cstring(testitm),
                                 mom_item_cstring(destestitm));
          }
          auto testexp = mom_unsync_item_get_phys_attr(testitm, MOM_PREDEFITM(test));
          if ((testexp == nullptr
               || reinterpret_cast<const mom_item_st*>(testexp)==MOM_PREDEFITM(unit)))
            {
              if (ix+1<nbcond)
                MOM_WARNPRINTF("cond %s #%d in block %s with non-last trivially false test#%d %s",
                               mom_item_cstring(insitm), rk,
                               mom_item_cstring(blkitm),
                               ix,
                               mom_item_cstring(testitm));
            }
          else
            {
              auto testypitm = scan_expr(testexp,insitm,1);
              if (!testypitm)
                throw
                MOM_RUNTIME_PRINTF("cond %s #%d in block %s with untypable test#%d %s",
                                   mom_item_cstring(insitm), rk,
                                   mom_item_cstring(blkitm),
                                   ix,
                                   mom_item_cstring(testitm));
            }
          auto thenitm =
            mom_dyncast_item(mom_unsync_item_get_phys_attr(testitm, MOM_PREDEFITM(test)));
          if (thenitm == nullptr)
            throw
            MOM_RUNTIME_PRINTF("cond %s #%d in block %s with test#%d %s without `then`",
                               mom_item_cstring(insitm), rk,
                               mom_item_cstring(blkitm),
                               ix,
                               mom_item_cstring(testitm));
          lock_item(thenitm);
          todo([=](MomEmitter*em)
          {
            MOM_DEBUGPRINTF(gencod, "before scanning thenitm=%s from insitm=%s rk#%d blkitm=%s",
                            mom_item_cstring(thenitm),
                            mom_item_cstring(insitm), rk, mom_item_cstring(blkitm));
            em->scan_block(thenitm, blkitm);
            MOM_DEBUGPRINTF(gencod, "after scanning thenitm=%s insitm=%s rk#%d blkitm=%s",
                            mom_item_cstring(thenitm),
                            mom_item_cstring(insitm), rk, mom_item_cstring(blkitm));
          });
        }
    }
    break;
    default:
defaultcasedesc:
      {
        MOM_DEBUGPRINTF(gencod, "scan_instr special before insitm=%s desitm=%s rk#%d blkitm %s",
                        mom_item_cstring(insitm), mom_item_cstring(desitm), rk, mom_item_cstring(blkitm));
        scan_special_instr(insitm, desitm, rk, blkitm);
        MOM_DEBUGPRINTF(gencod, "scan_instr special after insitm=%s desitm=%s rk#%d blkitm %s",
                        mom_item_cstring(insitm), mom_item_cstring(desitm), rk, mom_item_cstring(blkitm));
      }
      break;
#undef CASE_OPER_MOM
#undef NBMODOPER_MOM
    }
} // end of MomEmitter::scan_instr


struct mom_item_st*
MomEmitter::scan_expr(const void*expv, struct mom_item_st*insitm, int depth, struct mom_item_st*typitm)
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
      return MOM_PREDEFITM(unit);
    case MOMITY_INT:
      if (typitm && typitm != MOM_PREDEFITM(int) && typitm != MOM_PREDEFITM(value))
        throw MOM_RUNTIME_PRINTF("int.expr %s in instr %s with type mismatch for %s",
                                 mom_value_cstring(expv), mom_item_cstring(insitm), mom_item_cstring(typitm));
      return typitm?typitm:MOM_PREDEFITM(int);
    case MOMITY_BOXSTRING:
      if (typitm && typitm != MOM_PREDEFITM(string) && typitm != MOM_PREDEFITM(value))
        throw MOM_RUNTIME_PRINTF("string.expr %s in instr %s with type mismatch for %s",
                                 mom_value_cstring(expv), mom_item_cstring(insitm), mom_item_cstring(typitm));
      return typitm?typitm:MOM_PREDEFITM(string);
    case MOMITY_BOXDOUBLE:
      if (typitm && typitm != MOM_PREDEFITM(double) && typitm != MOM_PREDEFITM(value))
        throw MOM_RUNTIME_PRINTF("double.expr %s in instr %s with type mismatch for %s",
                                 mom_value_cstring(expv), mom_item_cstring(insitm), mom_item_cstring(typitm));
      return typitm?typitm:MOM_PREDEFITM(double);
    case MOMITY_ITEM:
    {
      auto itmexp = (struct mom_item_st*)expv;
      lock_item(itmexp);
      return scan_item_expr(itmexp,insitm,depth,typitm);
      ;
    }
    case MOMITY_NODE:
    {
      auto nodexp = (const struct mom_boxnode_st*)expv;
      return scan_node_expr(nodexp,insitm,depth,typitm);
    }
    default:
      throw MOM_RUNTIME_PRINTF("unexpected expr %s in instr %s with type %s",
                               mom_value_cstring(expv), mom_item_cstring(insitm), mom_item_cstring(typitm));
    }
} // end of MomEmitter::scan_expr



struct mom_item_st*
MomEmitter::scan_node_expr(const struct mom_boxnode_st*expnod, struct mom_item_st*insitm,
                           int depth, struct mom_item_st*typitm)
{
  MOM_DEBUGPRINTF(gencod, "scan_node_expr start expnod=%s insitm=%s depth#%d typitm=%s",
                  mom_value_cstring(expnod), mom_item_cstring(insitm), depth, mom_item_cstring(typitm));
  assert (expnod != nullptr && expnod->va_itype==MOMITY_NODE);
  auto connitm = expnod->nod_connitm;
  assert (connitm != nullptr && connitm->va_itype==MOMITY_ITEM);
  unsigned nodarity = mom_size(expnod);
  lock_item(connitm);
#define NBEXPCONN_MOM 131
#define CASE_EXPCONN_MOM(Nam) momhashpredef_##Nam % NBEXPCONN_MOM:	\
	  if (connitm == MOM_PREDEFITM(Nam)) goto foundcaseconn_##Nam;	\
	  goto defaultcaseconn; foundcaseconn_##Nam
  switch (connitm->hva_hash % NBEXPCONN_MOM)
    {
    case CASE_EXPCONN_MOM(verbatim):
    {
      if (nodarity != 1)
        throw MOM_RUNTIME_PRINTF("verbatim expr %s of bad arity in instr %s with type %s",
                                 mom_value_cstring(expnod), mom_item_cstring(insitm),
                                 mom_item_cstring(typitm));
      auto quotval = expnod->nod_sons[0];
      if (typitm == nullptr)
        typitm = MOM_PREDEFITM(value);
      else if (typitm == MOM_PREDEFITM(item))
        {
          if (mom_itype(quotval) != MOMITY_ITEM)
            throw MOM_RUNTIME_PRINTF("verbatim expr %s is not an item in instr %s with type %s",
                                     mom_value_cstring(expnod), mom_item_cstring(insitm),
                                     mom_item_cstring(typitm));
          _ce_localvalueset.insert(quotval);
        }
      else if (typitm == MOM_PREDEFITM(value))
        _ce_localvalueset.insert(quotval);
      else
        throw MOM_RUNTIME_PRINTF("verbatim expr %s is of type %s in instr %s with type %s",
                                 mom_value_cstring(expnod), mom_item_cstring(typitm),
                                 mom_item_cstring(insitm),
                                 mom_item_cstring(typitm));
    }
    break;
    case CASE_EXPCONN_MOM(and):
    case CASE_EXPCONN_MOM(or):
    {
      if (nodarity == 0)
        {
          if (!typitm)
            throw MOM_RUNTIME_PRINTF("typeless empty %s expr in instr %s",
                                     mom_value_cstring(expnod), mom_item_cstring(insitm));
          return typitm;
        }
      for (unsigned ix=0; ix<nodarity; ix++)
        {
          auto subexpv = expnod->nod_sons[ix];
          auto newtypitm = scan_expr(subexpv, insitm, depth+1, typitm);
          if (!newtypitm)
            throw MOM_RUNTIME_PRINTF("son#%d %s of %s is typeless in instr %s",
                                     ix, mom_value_cstring(subexpv),
                                     mom_value_cstring(expnod), mom_item_cstring(insitm));
          else if (typitm==nullptr)
            typitm = newtypitm;
          else if (newtypitm != typitm)
            throw MOM_RUNTIME_PRINTF("son#%d %s of %s is badly typed %s expecting %s in instr %s",
                                     ix, mom_value_cstring(subexpv),
                                     mom_value_cstring(expnod),
                                     mom_item_cstring(newtypitm), mom_item_cstring(typitm),
                                     mom_item_cstring(insitm));
        }
      return typitm;
    }
    break;
    case CASE_EXPCONN_MOM(sequence):
    {
      if (nodarity == 0)
        {
          if (!typitm)
            throw MOM_RUNTIME_PRINTF("typeless empty %s expr in instr %s",
                                     mom_value_cstring(expnod), mom_item_cstring(insitm));
        }
      else
        {
          for (int ix=0; ix<(int)nodarity-1; ix++)
            {
              auto subexpv = expnod->nod_sons[ix];
              (void) scan_expr(subexpv, insitm, depth+1, nullptr);
            }
          auto lastsubexpv = expnod->nod_sons[nodarity-1];
          auto newtypitm = scan_expr(lastsubexpv, insitm, depth+1, typitm);
          if (!typitm) return newtypitm;
          else if (!newtypitm)
            throw MOM_RUNTIME_PRINTF("typeless last %s sub-expr of %s in instr %s",
                                     mom_value_cstring(lastsubexpv),
                                     mom_value_cstring(expnod), mom_item_cstring(insitm));
          else if (typitm != newtypitm)
            throw MOM_RUNTIME_PRINTF("last son %s of %s is badly typed %s expecting %s in instr %s",
                                     mom_value_cstring(lastsubexpv),
                                     mom_value_cstring(expnod),
                                     mom_item_cstring(newtypitm), mom_item_cstring(typitm),
                                     mom_item_cstring(insitm));

        }
      return typitm;
    }
    break;
    case CASE_EXPCONN_MOM(plus):
    case CASE_EXPCONN_MOM(mult):
    {
      if (nodarity == 0)
        throw MOM_RUNTIME_PRINTF("empty %s expr in instr %s",
                                 mom_value_cstring(expnod), mom_item_cstring(insitm));
      auto firstypitm = scan_expr(expnod->nod_sons[0], insitm, depth+1, typitm);
      if (firstypitm != MOM_PREDEFITM(int) && firstypitm != MOM_PREDEFITM(double))
        throw  MOM_RUNTIME_PRINTF("non-numerical type %s of expr %s in instr %s",
                                  mom_item_cstring(firstypitm), mom_value_cstring(expnod),
                                  mom_item_cstring(insitm));
      for (unsigned ix=1; ix<nodarity; ix++)
        {
          auto curson = expnod->nod_sons[ix];
          auto curtypitm = scan_expr(curson, insitm, depth+1, firstypitm);
          if (curtypitm != firstypitm)
            throw MOM_RUNTIME_PRINTF("numerical type mismatch (want %s) for son#%d %s of expr %s in instr %s",
                                     mom_item_cstring(firstypitm),
                                     ix, mom_value_cstring(curson),
                                     mom_value_cstring(expnod),
                                     mom_item_cstring(insitm));
        }
      return firstypitm;
    }
    break;
    case CASE_EXPCONN_MOM(sub):
    case CASE_EXPCONN_MOM(div):
    {
      if (nodarity != 2)
        throw MOM_RUNTIME_PRINTF("bad arity %s expr in instr %s",
                                 mom_value_cstring(expnod), mom_item_cstring(insitm));
      auto firstypitm = scan_expr(expnod->nod_sons[0], insitm, depth+1, typitm);
      if (firstypitm != MOM_PREDEFITM(int) && firstypitm != MOM_PREDEFITM(double))
        throw  MOM_RUNTIME_PRINTF("non-numerical type %s of expr %s in instr %s",
                                  mom_item_cstring(firstypitm), mom_value_cstring(expnod),
                                  mom_item_cstring(insitm));
      auto rightexp = expnod->nod_sons[1];
      auto rightypitm =  scan_expr(rightexp, insitm, depth+1, firstypitm);
      if (rightypitm != firstypitm)
        throw MOM_RUNTIME_PRINTF("numerical type mismatch (want %s) for right son %s of expr %s in instr %s",
                                 mom_item_cstring(firstypitm),
                                 mom_value_cstring(rightexp),
                                 mom_value_cstring(expnod),
                                 mom_item_cstring(insitm));
      return firstypitm;
    }
    break;
    case CASE_EXPCONN_MOM(mod):
    {
      if (nodarity != 2)
        throw MOM_RUNTIME_PRINTF("bad arity %s expr in instr %s",
                                 mom_value_cstring(expnod), mom_item_cstring(insitm));
      auto firstypitm = scan_expr(expnod->nod_sons[0], insitm, depth+1, typitm);
      if (firstypitm != MOM_PREDEFITM(int))
        throw  MOM_RUNTIME_PRINTF("non-numerical type %s of expr %s in instr %s",
                                  mom_item_cstring(firstypitm), mom_value_cstring(expnod),
                                  mom_item_cstring(insitm));
      auto rightexp = expnod->nod_sons[1];
      auto rightypitm =  scan_expr(rightexp, insitm, depth+1, firstypitm);
      if (rightypitm != firstypitm)
        throw MOM_RUNTIME_PRINTF("numerical type mismatch (want %s) for right son %s of expr %s in instr %s",
                                 mom_item_cstring(firstypitm),
                                 mom_value_cstring(rightexp),
                                 mom_value_cstring(expnod),
                                 mom_item_cstring(insitm));
      return firstypitm;
    }
    break;
    case CASE_EXPCONN_MOM(node):
      if (nodarity==0)
        throw MOM_RUNTIME_PRINTF("`node` expr %s in %s should have at least one argument",
                                 mom_value_cstring(expnod),
                                 mom_item_cstring(insitm));
      // failthru
    case CASE_EXPCONN_MOM(set):
    case CASE_EXPCONN_MOM(tuple):
    {
      for (unsigned ix=0; ix<nodarity; ix++)
        {
          auto subexpv = expnod->nod_sons[ix];
          auto subtypitm = scan_expr(subexpv, insitm, depth+1);
          if (subtypitm!=MOM_PREDEFITM(value) && subtypitm!=MOM_PREDEFITM(item))
            throw MOM_RUNTIME_PRINTF("type mismatch for son#%d of %s variadic expr %s in instr %s",
                                     ix, mom_item_cstring(connitm),
                                     mom_value_cstring(expnod),
                                     mom_item_cstring(insitm));
        }
      return MOM_PREDEFITM(value);
    }
    break;
    default:
defaultcaseconn:
      {
        lock_item(connitm);
        auto desconnitm = mom_unsync_item_descr(connitm);
        if (desconnitm == nullptr)
          throw MOM_RUNTIME_PRINTF("connective %s without `descr` in expr %s in instr %s",
                                   mom_item_cstring(connitm),
                                   mom_value_cstring(expnod),
                                   mom_item_cstring(insitm));
        lock_item(desconnitm);
        return scan_node_descr_conn_expr(expnod, desconnitm, insitm, depth, typitm);
      }
    } // end switch connitm
#undef NBEXPCONN_MOM
#undef CASE_EXPCONN_MOM
  MOM_FATAPRINTF("impossible scan_node_expr expnod=%s insitm=%s depth#%d typitm=%s",
                 mom_value_cstring(expnod), mom_item_cstring(insitm), depth, mom_item_cstring(typitm));
} // end of MomEmitter::scan_node_expr


struct mom_item_st*
MomEmitter::scan_node_descr_conn_expr(const struct mom_boxnode_st*expnod,
                                      struct mom_item_st*desconnitm,
                                      struct mom_item_st*insitm,
                                      int depth, struct mom_item_st*typitm)
{
  MOM_DEBUGPRINTF(gencod, "scan_node_descr_conn_expr start expnod=%s desconnitm=%s"
                  " insitm=%s depth#%d typitm=%s",
                  mom_value_cstring(expnod), mom_value_cstring(desconnitm),
                  mom_item_cstring(insitm), depth, mom_item_cstring(typitm));
  auto connitm = expnod->nod_connitm;
  assert (connitm != nullptr && connitm->va_itype==MOMITY_ITEM);
  unsigned nodarity = mom_size(expnod);
  assert (is_locked_item(connitm));
  assert (is_locked_item(desconnitm));
#define NBDESCONN_MOM 79
#define CASE_DESCONN_MOM(Nam) momhashpredef_##Nam % NBDESCONN_MOM:	\
	  if (connitm == MOM_PREDEFITM(Nam)) goto foundesconn_##Nam;	\
	  goto defaultdesconn; foundesconn_##Nam
  switch (desconnitm->hva_hash % NBDESCONN_MOM)
    {
    case CASE_DESCONN_MOM(signature):
    {
      // an unknown closure application
      auto sigr=scan_nonbinding_signature(connitm,insitm);
      auto sformaltup = sigr.sig_formals;
      auto sresultv = sigr.sig_result;
      unsigned lnformals = mom_boxtuple_length(sformaltup);
      if (lnformals<1 || mom_boxtuple_nth(sformaltup, 0) != MOM_PREDEFITM(this_closure))
        throw MOM_RUNTIME_PRINTF("bad formals %s of applied signature %s expnod %s instr %s",
                                 mom_value_cstring(sformaltup),
                                 mom_item_cstring(connitm),
                                 mom_value_cstring(expnod),
                                 mom_item_cstring(insitm));
      if (typitm == nullptr)
        typitm = MOM_PREDEFITM(value);
      else if (typitm != MOM_PREDEFITM(value))
        throw MOM_RUNTIME_PRINTF("application of signature %s in expnod %s instr %s"
                                 " incompatible with non-value type %s",
                                 mom_item_cstring(connitm),
                                 mom_value_cstring(expnod),
                                 mom_item_cstring(insitm),
                                 mom_item_cstring(typitm));
      if (lnformals != nodarity)
        throw MOM_RUNTIME_PRINTF("application of signature %s in expnod %s instr %s"
                                 " has mismatched arity (%d formals, arity %d)",
                                 mom_item_cstring(connitm),
                                 mom_value_cstring(expnod),
                                 mom_item_cstring(insitm),
                                 lnformals, nodarity);
      if (sresultv != MOM_PREDEFITM(value))
        throw MOM_RUNTIME_PRINTF("application of signature %s in expnod %s instr %s"
                                 " with non-value result type %s",
                                 mom_item_cstring(connitm),
                                 mom_value_cstring(expnod),
                                 mom_item_cstring(insitm),
                                 mom_value_cstring(sresultv));
      {
        auto funtypitm = scan_expr(expnod->nod_sons[0], insitm, depth+1, MOM_PREDEFITM(value));
        if (funtypitm != MOM_PREDEFITM(value))
          throw MOM_RUNTIME_PRINTF("application of signature %s in expnod %s instr %s"
                                   " has badly typed %s function argument",
                                   mom_item_cstring(connitm),
                                   mom_value_cstring(expnod),
                                   mom_item_cstring(insitm),
                                   mom_item_cstring(funtypitm));
      }
      for (unsigned ix=1; ix<nodarity; ix++)
        {
          auto curtypitm =  mom_boxtuple_nth(sformaltup, ix);
          auto argtypitm = scan_expr(expnod->nod_sons[ix], insitm, depth+1, curtypitm);
          if (curtypitm != argtypitm)
            throw MOM_RUNTIME_PRINTF("application of signature %s in expnod %s instr %s"
                                     " type mismatch for arg#%d, expecting %s",
                                     mom_item_cstring(connitm),
                                     mom_value_cstring(expnod),
                                     mom_item_cstring(insitm),
                                     ix, mom_item_cstring(curtypitm));
        }
      return MOM_PREDEFITM(value);
    }
    break;
    case CASE_DESCONN_MOM(routine):
    case CASE_DESCONN_MOM(primitive):
    {
      // a known routine application
      auto routsigitm =
        mom_dyncast_item(mom_unsync_item_get_phys_attr
                         (connitm,
                          MOM_PREDEFITM(signature)));
      if (routsigitm==nullptr)
        throw MOM_RUNTIME_PRINTF("applied %s %s in expnod %s instr %s without signature",
                                 mom_item_cstring(desconnitm),
                                 mom_item_cstring(connitm),
                                 mom_value_cstring(expnod),
                                 mom_item_cstring(insitm));
      lock_item(routsigitm);
      const struct mom_boxtuple_st* formaltup=nullptr;
      struct mom_item_st*restypitm=nullptr;
      {
        auto sigr=scan_nonbinding_signature(routsigitm,insitm);
        formaltup=sigr.sig_formals;
        restypitm=mom_dyncast_item(sigr.sig_result);
      }
      if (formaltup==nullptr || restypitm==nullptr)
        throw MOM_RUNTIME_PRINTF("applied %s %s in expnod %s instr %s with bad signature %s",
                                 mom_item_cstring(desconnitm),
                                 mom_item_cstring(connitm),
                                 mom_value_cstring(expnod),
                                 mom_item_cstring(insitm),
                                 mom_item_cstring(routsigitm));
      unsigned nbformals = mom_boxtuple_length(formaltup);
      if (nbformals != nodarity)
        throw MOM_RUNTIME_PRINTF("applied %s %s in expnod %s instr %s"
                                 " with wrong %d number of arguments (%d expected from signature %s)",
                                 mom_item_cstring(desconnitm),
                                 mom_item_cstring(connitm),
                                 mom_value_cstring(expnod),
                                 mom_item_cstring(insitm),
                                 nodarity, nbformals,
                                 mom_item_cstring(routsigitm));
      for (unsigned ix=0; ix<nbformals; ix++)
        {
          auto sonexpv = expnod->nod_sons[ix];
          auto curformalitm = mom_boxtuple_nth(formaltup, ix);
          assert (is_locked_item(curformalitm));
          auto curtypitm = mom_dyncast_item( mom_unsync_item_get_phys_attr (curformalitm, MOM_PREDEFITM(type)));
          if (curtypitm == nullptr)
            throw  MOM_RUNTIME_PRINTF("applied %s %s in expnod %s instr %s"
                                      " with signature %s of untyped formal#%d %s",
                                      mom_item_cstring(desconnitm),
                                      mom_item_cstring(connitm),
                                      mom_value_cstring(expnod),
                                      mom_item_cstring(insitm),
                                      mom_item_cstring(routsigitm),
                                      ix, mom_item_cstring(curformalitm));
          lock_item(curtypitm);
          auto exptypitm = scan_expr(sonexpv,insitm,depth+1,curtypitm);
          if (exptypitm != curtypitm)
            throw  MOM_RUNTIME_PRINTF("applied %s %s in expnod %s instr %s"
                                      " with signature %s type mismatch for #%d (formal %s)",
                                      mom_item_cstring(desconnitm),
                                      mom_item_cstring(connitm),
                                      mom_value_cstring(expnod),
                                      mom_item_cstring(insitm),
                                      mom_item_cstring(routsigitm),
                                      ix,  mom_item_cstring(curformalitm));
        };
      if (typitm != nullptr && typitm != restypitm)
        throw MOM_RUNTIME_PRINTF("applied %s %s in expnod %s instr %s"
                                 " with signature %s result type mismatch (expecting %s got %s)",
                                 mom_item_cstring(desconnitm),
                                 mom_item_cstring(connitm),
                                 mom_value_cstring(expnod),
                                 mom_item_cstring(insitm),
                                 mom_item_cstring(routsigitm),
                                 mom_item_cstring(typitm), mom_item_cstring(restypitm));
      return restypitm;
    }
    break;
defaultdesconn:
    break;
    }
#undef CASE_DESCONN_MOM
#undef NBDESCONN_MOM

#warning MomEmitter::scan_node_descr_conn_expr unimplemented
  MOM_FATAPRINTF("unimplemented scan_node_desc_conn_expr expnod=%s desconnitm=%s insitm=%s depth#%d typitm=%s",
                 mom_value_cstring(expnod), mom_value_cstring(desconnitm),
                 mom_item_cstring(insitm), depth, mom_item_cstring(typitm));
} // end of MomEmitter::scan_node_descr_conn_expr



struct mom_item_st*
MomEmitter::scan_item_expr(struct mom_item_st*expitm, struct mom_item_st*insitm, int depth, struct mom_item_st*typitm)
{
  MOM_DEBUGPRINTF(gencod, "scan_item_expr start expitm=%s insitm=%s depth#%d typitm=%s",
                  mom_item_cstring(expitm), mom_item_cstring(insitm), depth, mom_item_cstring(typitm));
  assert (is_locked_item(expitm));
  assert (is_locked_item(insitm));
  auto desitm =  mom_unsync_item_descr(expitm);
  if (desitm == MOM_PREDEFITM(variable) || desitm == MOM_PREDEFITM(global)
      || desitm == MOM_PREDEFITM(thread_local) || desitm == MOM_PREDEFITM(formal))
    return scan_var(expitm,insitm,typitm);
  else if (desitm == MOM_PREDEFITM(closed))
    return scan_closed(expitm,insitm);
  auto typexpitm =
    mom_dyncast_item(mom_unsync_item_get_phys_attr(expitm,MOM_PREDEFITM(type)));
  MOM_DEBUGPRINTF(gencod, "scan_item_expr expitm=%s typexpitm=%s desitm=%s",
                  mom_item_cstring(expitm), mom_item_cstring(typexpitm), mom_item_cstring(desitm));
#warning MomEmitter::scan_item_expr unimplemented
  MOM_FATAPRINTF("unimplemented scan_item_expr expitm=%s insitm=%s depth#%d typitm=%s",
                 mom_item_cstring(expitm), mom_item_cstring(insitm), depth, mom_item_cstring(typitm));
} // end of MomEmitter::scan_item_expr



struct mom_item_st*
MomEmitter::scan_var(struct mom_item_st*varitm, struct mom_item_st*insitm, struct mom_item_st*typitm)
{
  MOM_DEBUGPRINTF(gencod, "scan_var start varitm=%s insitm=%s typitm=%s",
                  mom_item_cstring(varitm), mom_item_cstring(insitm), mom_item_cstring(typitm));
  assert (is_locked_item(varitm));
  auto desvaritm =  mom_unsync_item_descr(varitm);
  if (!desvaritm)
    throw MOM_RUNTIME_PRINTF("variable %s in instruction %s without descr",
                             mom_item_cstring(varitm), mom_item_cstring(insitm));
  struct mom_item_st*typvaritm =
  mom_dyncast_item(mom_unsync_item_get_phys_attr(varitm,MOM_PREDEFITM(type)));
  if (!typvaritm)
    throw  MOM_RUNTIME_PRINTF("variable %s in instruction %s without `type`",
                              mom_item_cstring(varitm), mom_item_cstring(insitm));
  if (!typitm)
    typitm = typvaritm;
  else if (typitm != typvaritm)
    throw  MOM_RUNTIME_PRINTF("variable %s in instruction %s has `type`:%s incompatible with %s",
                              mom_item_cstring(varitm), mom_item_cstring(insitm),
                              mom_item_cstring(typvaritm), mom_item_cstring(typitm));
  MOM_DEBUGPRINTF(gencod, "scan_var end varitm=%s type %s desvaritm=%s",
                  mom_item_cstring(varitm), mom_item_cstring(typitm), mom_item_cstring(desvaritm));
#define NBVARDESC_MOM 43
#define CASE_VARDESCR_MOM(Nam) momhashpredef_##Nam % NBVARDESC_MOM:	\
	  if (desvaritm == MOM_PREDEFITM(Nam)) goto foundcase_##Nam;	\
	  goto defaultvardesc; foundcase_##Nam
  switch (desvaritm->hva_hash % NBVARDESC_MOM)
    {
    case CASE_VARDESCR_MOM(variable):
    {
      auto locvarbind = get_local_binding(varitm);
      if (!locvarbind)
        throw MOM_RUNTIME_PRINTF("variable %s in instruction %s is not locally bound",
                                 mom_item_cstring(varitm), mom_item_cstring(insitm));
      assert (!is_globally_bound(varitm));
      if (locvarbind->vd_rolitm != MOM_PREDEFITM(variable) && locvarbind->vd_rolitm != MOM_PREDEFITM(formal))
        throw  MOM_RUNTIME_PRINTF("variable %s in instruction %s is strangely %s locally bound",
                                  mom_item_cstring(varitm), mom_item_cstring(insitm), mom_item_cstring(locvarbind->vd_rolitm));
      return typitm;
    }
    break;
    case CASE_VARDESCR_MOM(global):
    {
      assert (!is_locally_bound(varitm));
      auto globvarbind = get_global_binding(varitm);
      if (!globvarbind)
        {
          bind_global(varitm, MOM_PREDEFITM(global), typitm);
        }
      else if (globvarbind->vd_rolitm !=  MOM_PREDEFITM(global))
        throw  MOM_RUNTIME_PRINTF("global %s in instruction %s is strangely %s locally bound",
                                  mom_item_cstring(varitm), mom_item_cstring(insitm), mom_item_cstring(globvarbind->vd_rolitm));
    }
    break;
    case CASE_VARDESCR_MOM(thread_local):
    {
      assert (!is_locally_bound(varitm));
      auto globvarbind = get_global_binding(varitm);
      if (!globvarbind)
        {
          if (magic() == MomJavascriptEmitter::MAGIC)
            throw MOM_RUNTIME_PRINTF("JavaScript forbids thread_local %s in instruction %s",
                                     mom_item_cstring(varitm), mom_item_cstring(insitm));
          bind_global(varitm, MOM_PREDEFITM(thread_local), typitm);
        }
      else if (globvarbind->vd_rolitm !=  MOM_PREDEFITM(thread_local))
        throw  MOM_RUNTIME_PRINTF("thread_local %s in instruction %s is strangely %s locally bound",
                                  mom_item_cstring(varitm), mom_item_cstring(insitm), mom_item_cstring(globvarbind->vd_rolitm));
    }
    break;
    case CASE_VARDESCR_MOM(formal):
    {
      assert (!is_globally_bound(varitm));
      auto locvarbind = get_local_binding(varitm);
      if (!locvarbind || locvarbind->vd_rolitm != MOM_PREDEFITM(formal))
        throw  MOM_RUNTIME_PRINTF("formal %s in instruction %s is strangely %s locally bound",
                                  mom_item_cstring(varitm), mom_item_cstring(insitm), mom_item_cstring(locvarbind->vd_rolitm));
    }
    break;
    default:
defaultvardesc:
      throw  MOM_RUNTIME_PRINTF("variable %s in instruction %s has unexpected descr %s",
                                mom_item_cstring(varitm), mom_item_cstring(insitm),
                                mom_item_cstring(desvaritm));
    }
#undef NBVARDESC_MOM
#undef CASE_VARDESCR_MOM
  return typitm;
} // end of MomEmitter::scan_var


struct mom_item_st*
MomEmitter::scan_closed(struct mom_item_st*cloitm, struct mom_item_st*insitm)
{
  MOM_DEBUGPRINTF(gencod, "scan_closed start cloitm=%s insitm=%s",
                  mom_item_cstring(cloitm), mom_item_cstring(insitm));
  assert (is_locked_item(cloitm));
  assert (mom_unsync_item_descr(cloitm)==MOM_PREDEFITM(closed));
  _ce_localcloseditems.insert(cloitm);
  return MOM_PREDEFITM(value);
} // end MomEmitter::scan_closed


void
MomEmitter::scan_routine_element(struct mom_item_st*rtitm)
{
  MOM_DEBUGPRINTF(gencod, "scan_routine_element start rtitm=%s", mom_item_cstring(rtitm));
  _ce_curfunctionitm = rtitm;
#warning unimplemented MomEmitter::scan_routine_element
  MOM_FATAPRINTF("unimplemented scan_routine_element %s", mom_item_cstring(rtitm));
  _ce_curfunctionitm = nullptr;
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

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
    long vd_rank;
  };
  typedef std::map<const struct mom_item_st*,
    vardef_st,
    MomItemLess,
    traceable_allocator<struct mom_item_st*>>
                                           traced_varmap_t;
  class EnvElem
{
  public:
    traced_varmap_t _ee_map;
    void* _ee_orig;
    long _ee_level;
    EnvElem(void*orig=nullptr, intptr_t level=0) : _ee_map {}, _ee_orig(orig), _ee_level(level) {};
    EnvElem(const EnvElem&) = default;
    EnvElem(EnvElem&&) = default;
    ~EnvElem() = default;
    bool is_bound(struct mom_item_st*itm) const
    {
      return mom_itype(itm) == MOMITY_ITEM && _ee_map.find(itm) != _ee_map.end();
    };
    void bind(struct mom_item_st*itm, struct mom_item_st*rolitm, const void*what=nullptr, intptr_t rank=0)
    {
      assert (mom_itype (rolitm) == MOMITY_ITEM);
      if (mom_itype(itm) == MOMITY_ITEM)
        {
          if (rolitm == MOM_PREDEFITM(data))
            MOM_DEBUGPRINTF(gencod,"bind %s data role rank %ld level#%ld", mom_item_cstring(itm),
                            (long)rank, _ee_level);
          else
            MOM_DEBUGPRINTF(gencod,"bind %s role %s what %s rank %ld level#%ld",
                            mom_item_cstring(itm), mom_item_cstring(rolitm),
                            mom_value_cstring((const mom_hashedvalue_st*)what), (long)rank, _ee_level);
          _ee_map.emplace(itm,vardef_st {rolitm,what,rank});
        }
    }
    void bind(struct mom_item_st*itm, const vardef_st&def)
    {
      if (mom_itype(itm) == MOMITY_ITEM)
        {
          auto rolitm = def.vd_rolitm;
          assert (mom_itype (rolitm) == MOMITY_ITEM);
          auto what = def.vd_what;
          auto rank = def.vd_rank;
          if (rolitm == MOM_PREDEFITM(data))
            MOM_DEBUGPRINTF(gencod,"bind %s data role rank %ld level#%ld", mom_item_cstring(itm),
                            (long)rank, _ee_level);
          else
            MOM_DEBUGPRINTF(gencod,"bind %s role %s what %s rank %ld level#%ld",
                            mom_item_cstring(itm), mom_item_cstring(rolitm),
                            mom_value_cstring((struct mom_hashedvalue_st*)what), (long)rank, _ee_level);
          _ee_map.emplace(itm,def);
        }
    }
    void unbind(struct mom_item_st*itm)
    {
      if (mom_itype(itm) == MOMITY_ITEM)
        {
          MOM_DEBUGPRINTF(gencod, "unbind %s level#%ld", mom_item_cstring(itm), _ee_level);
          _ee_map.erase(itm);
        }
    }
    const vardef_st* get_binding(struct mom_item_st*itm) const
    {
      if (mom_itype(itm) == MOMITY_ITEM)
        {
          auto it =  _ee_map.find(itm);
          if (it == _ee_map.end())
            return nullptr;
          else return &it->second;
        }
      else return nullptr;
    }
    void*orig(void) const
    {
      return _ee_orig;
    };
    intptr_t level(void) const
    {
      return _ee_level;
    };
  }; // end class EnvElem
private:
  const unsigned _ce_magic;
  struct mom_item_st* _ce_topitm;
  std::vector<struct mom_item_st*,traceable_allocator<struct mom_item_st*>> _ce_veclockeditems;
  traced_set_items_t _ce_setlockeditems;
  traced_set_items_t _ce_sigitems;
  std::deque<todofun_t,traceable_allocator<todofun_t>> _ce_todoque;
  std::vector<EnvElem,traceable_allocator<EnvElem>> _ce_envstack;
protected:
  MomEmitter(unsigned magic, struct mom_item_st*itm);
  MomEmitter(const MomEmitter&) = delete;
  virtual ~MomEmitter();
  virtual void scan_data_element(struct mom_item_st*itm);
  virtual void scan_func_element(struct mom_item_st*itm);
  virtual void scan_routine_element(struct mom_item_st*itm);
  void scan_signature(struct mom_item_st*sigitm, struct mom_item_st*initm);
  void scan_block(struct mom_item_st*blockitm, struct mom_item_st*initm);
public:
  void push_fresh_varenv (void*envorig=nullptr)
  {
    auto sz = _ce_envstack.size();
    _ce_envstack.emplace_back(EnvElem {envorig,(intptr_t)sz});
  }
  void pop_varenv(int lin=0)
  {
    auto sz = _ce_envstack.size();
    if (sz == 0)
      {
        throw MomRuntimeErrorAt(__FILE__,lin?lin:__LINE__,"empty varenv stack cannot be popped");
      };
    _ce_envstack.pop_back();
  }
  /// for automatic push/pop of a fresh varenv, declare in a block :
  ///    LocalVars lv{this};
  /// or
  ///    LocalVars lv{this,orig};
  class LocalVars
  {
    MomEmitter* _locem;
  public:
    LocalVars(MomEmitter*me,void*orig=nullptr) : _locem(me)
    {
      assert(me!=nullptr);
      me->push_fresh_varenv(orig);
    };
    ~LocalVars()
    {
      assert (_locem != nullptr);
      _locem->pop_varenv();
      _locem=nullptr;
    }
  private:
    LocalVars(const LocalVars&) = delete;
    LocalVars(LocalVars&&) = delete;
  };
  EnvElem& top_varenv(int lin=0)
  {
    auto sz = _ce_envstack.size();
    if (sz == 0)
      {
        throw MomRuntimeErrorAt(__FILE__,lin?lin:__LINE__,"empty varenv stack has no top");
      };
    return _ce_envstack[sz-1];
  } // end top_varenv
  void bind_top_var(struct mom_item_st*itm, struct mom_item_st*rolitm, int lin=0, const void*what=nullptr, intptr_t rank=0)
  {
    if (mom_itype(rolitm) != MOMITY_ITEM) /// should never happen
      MOM_FATAPRINTF("invalid rolitm %s for binding itm %s from lin#%d",
                     mom_value_cstring((struct mom_hashedvalue_st*)rolitm), mom_item_cstring(itm), lin);
    auto sz = _ce_envstack.size();
    if (sz == 0)
      {
        throw MomRuntimeErrorAt(__FILE__,lin?lin:__LINE__,
                                mom_gc_printf("empty varenv stack cannot bind item %s", mom_item_cstring(itm)));
      }
    if (mom_itype(itm) != MOMITY_ITEM)
      throw MomRuntimeErrorAt(__FILE__,lin?lin:__LINE__,
                              "varenv stack cannot bind non-item");
    top_varenv(lin).bind(itm,rolitm,what,rank);
  } // end bind_top_var
  void bind_top_var(struct mom_item_st*itm, struct mom_item_st*rolitm, const void*what=nullptr, intptr_t rank=0)
  {
    bind_top_var(itm,rolitm,0,what,rank);  // end bind_top_var noline
  }
  bool bound_var(struct mom_item_st*itm) const
  {
    if (mom_itype(itm) != MOMITY_ITEM) return false;
    auto sz = _ce_envstack.size();
    if (sz==0) return false;
    for (long ix= (long)(sz-1); ix>=0; ix--)
      if (_ce_envstack[ix].is_bound(itm)) return true;
    return false;
  }
  vardef_st*get_binding(struct mom_item_st*itm) const
  {
    if (mom_itype(itm) != MOMITY_ITEM) return nullptr;
    auto sz = _ce_envstack.size();
    if (sz==0) return nullptr;
    for (long ix= (long)(sz-1); ix>=0; ix--)
      {
        auto p = _ce_envstack[ix].get_binding(itm);
        if (p != nullptr) return const_cast<vardef_st*>(p);
      }
    return nullptr;
  }
  std::pair<vardef_st*,long> get_indexed_binding(struct mom_item_st*itm) const
  {
    if (mom_itype(itm) != MOMITY_ITEM) return {nullptr,0L};
    auto sz = _ce_envstack.size();
    if (sz==0) return {nullptr,0L};
    for (long ix= (long)(sz-1); ix>=0; ix--)
      {
        vardef_st* p = const_cast<vardef_st*>(_ce_envstack[ix].get_binding(itm));
        if (p != nullptr)
          {
            return {p, ix};
          }
      }
    return {nullptr,0};
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
_ce_sigitems {}
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
  LocalVars mv {this,_ce_topitm};
  bind_top_var(MOM_PREDEFITM(module),MOM_PREDEFITM(module),_ce_topitm);
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
  LocalVars lv {this,fuitm};
  bind_top_var(MOM_PREDEFITM(func),MOM_PREDEFITM(func),fuitm);
  bind_top_var(fuitm,MOM_PREDEFITM(func),fuitm);
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


void
MomEmitter::scan_signature(struct mom_item_st*sigitm, struct mom_item_st*initm)
{
  MOM_DEBUGPRINTF(gencod, "scan_signature start sigitm=%s initm=%s",
                  mom_item_cstring(sigitm), mom_item_cstring(initm));
  struct mom_item_st*desitm = mom_unsync_item_descr(sigitm);
  MOM_DEBUGPRINTF(gencod, "scan_signature desitm=%s", mom_item_cstring(desitm));
  if (desitm != MOM_PREDEFITM(signature))
    throw MOM_RUNTIME_PRINTF("in %s signature %s of bad descr %s",
                             mom_item_cstring(initm), mom_item_cstring(sigitm), mom_item_cstring(desitm));
#warning MomEmitter::scan_signature unimplemented
  MOM_FATAPRINTF("scan_signature unimplemented sigitm=%s initm=%s",
                 mom_item_cstring(sigitm), mom_item_cstring(initm));
} // end MomEmitter::scan_signature

void
MomEmitter::scan_block(struct mom_item_st*blkitm, struct mom_item_st*initm)
{
  MOM_DEBUGPRINTF(gencod, "scan_block start blkitm=%s initm=%s",
                  mom_item_cstring(blkitm), mom_item_cstring(initm));
  struct mom_item_st*desitm = mom_unsync_item_descr(blkitm);
  MOM_DEBUGPRINTF(gencod, "scan_block desitm=%s", mom_item_cstring(desitm));
  if (desitm != MOM_PREDEFITM(block))
    throw MOM_RUNTIME_PRINTF("in %s block %s of bad descr %s",
                             mom_item_cstring(initm),
                             mom_item_cstring(blkitm), mom_item_cstring(desitm));
#warning MomEmitter::scan_block unimplemented
  MOM_FATAPRINTF("scan_block unimplemented blkitm=%s initm=%s",
                 mom_item_cstring(blkitm), mom_item_cstring(initm));
} // end MomEmitter::scan_block

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

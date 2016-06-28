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
#include <memory>
#include <gc/gc_allocator.h>

class MomEmitter
{
public:
  typedef std::function<void(MomEmitter*)> todofun_t;
  typedef std::vector<const void*,
          traceable_allocator<const void*> >
          traced_vector_values_t;
  typedef std::set<const struct mom_item_st*,
    MomItemLess,
    traceable_allocator<struct mom_item_st*>>
                                           traced_set_items_t;
  typedef std::map<const struct mom_item_st*,struct mom_item_st*,
    MomItemLess,
    traceable_allocator<std::pair<struct mom_item_st*,struct mom_item_st*>>>
    traced_map_item2item_t;
  typedef std::map<const struct mom_item_st*,momvalue_t,
    MomItemLess,
    traceable_allocator<std::pair<struct mom_item_st*,momvalue_t>>>
    traced_map_item2value_t;
  typedef std::set<const void*,
          MomValueLess,
          traceable_allocator<void*>>
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
  typedef std::unordered_map
  <const struct mom_boxnode_st*,
    struct mom_item_st*,
    MomValueHash,
    MomValueEqual,
    traceable_allocator<std::pair<const struct mom_boxnode_st*,struct mom_item_st*> > >
    traced_node2itemhashmap_t;
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
  std::map<std::string,momvalue_t,
      std::less<std::string>,
      traceable_allocator<std::pair<std::string,momvalue_t>>> _ce_literalstringmap;
  std::deque<todofun_t,traceable_allocator<todofun_t>> _ce_todoque;
  traced_varmap_t _ce_globalvarmap;
  traced_varmap_t _ce_localvarmap;
  traced_set_values_t _ce_localvalueset;
  traced_set_items_t _ce_localcloseditems;
  traced_node2itemhashmap_t _ce_localnodetypecache;
  struct mom_item_st*_ce_curfunctionitm;
protected:
  class CaseScannerData
{
  protected:
    MomEmitter*cas_emitter;
    struct mom_item_st*cas_swtypitm;
    struct mom_item_st*cas_insitm;
    struct mom_item_st*cas_blkitm;
    unsigned cas_rank;
    traced_set_items_t cas_runset;
  public:
    virtual const char*name() const=0;
    MomEmitter*emitter() const
    {
      return cas_emitter;
    };
    struct mom_item_st*swtypitm() const
    {
      return cas_swtypitm;
    };
    struct mom_item_st*insitm() const
    {
      return cas_insitm;
    };
    struct mom_item_st*blkitm() const
    {
      return cas_blkitm;
    };
    unsigned rank() const
    {
      return cas_rank;
    };
    bool has_runitm(struct mom_item_st*itm) const
    {
      return itm!=nullptr && cas_runset.find(itm) != cas_runset.end();
    };
    void add_runitm(struct mom_item_st*itm)
    {
      assert (mom_itype(itm) == MOMITY_ITEM);
      cas_runset.insert(itm);
    };
    CaseScannerData(MomEmitter*em, struct mom_item_st*swtypitm, struct mom_item_st*insitm, struct mom_item_st*blkitm, unsigned rank)
      : cas_emitter(em), cas_swtypitm(swtypitm), cas_insitm(insitm), cas_blkitm(blkitm), cas_rank(rank), cas_runset() {};
    virtual ~CaseScannerData()
    {
      cas_emitter=nullptr;
      cas_swtypitm=nullptr;
      cas_insitm=nullptr;
      cas_blkitm=nullptr;
      cas_rank=0;
      cas_runset.clear();
    };
  };				// end class CaseScannerData
  class IntCaseScannerData  final : public CaseScannerData
  {
    std::map<long,struct mom_item_st*,std::less<long>,traceable_allocator<long>> cas_num2casemap;
  public:
    void process_intcase(const void*expv, struct mom_item_st*casitm, struct mom_item_st*runitm);
    const char*name() const
    {
      return "IntCaseScannerData";
    };
    IntCaseScannerData(MomEmitter*em, struct mom_item_st*swtypitm, struct mom_item_st*insitm, struct mom_item_st*blkitm, unsigned rank)
      : CaseScannerData(em,swtypitm,insitm,blkitm,rank), cas_num2casemap() {};
    virtual ~IntCaseScannerData()
    {
      cas_num2casemap.clear();
    };
  };				// end class IntCaseScannerData
  class StringCaseScannerData final : public CaseScannerData
  {
    std::map<std::string,struct mom_item_st*,std::less<std::string>,traceable_allocator<std::string>> cas_string2casemap;
  public:
    const char*name() const
    {
      return "StringCaseScannerData";
    };
    void process_stringcase(const void*expv, struct mom_item_st*casitm, struct mom_item_st*runitm);
    StringCaseScannerData(MomEmitter*em, struct mom_item_st*swtypitm, struct mom_item_st*insitm, struct mom_item_st*blkitm, unsigned rank)
      : CaseScannerData(em,swtypitm,insitm,blkitm,rank), cas_string2casemap() {};
    virtual ~StringCaseScannerData()
    {
      cas_string2casemap.clear();
    };
  };				// end class StringCaseScannerData
  class ItemCaseScannerData final : public CaseScannerData
  {
    traced_map_item2item_t cas_item2casemap;
  public:
    const char*name() const
    {
      return "ItemCaseScannerData";
    };
    void process_itemcase(const void*expv, struct mom_item_st*casitm, struct mom_item_st*runitm);
    ItemCaseScannerData(MomEmitter*em, struct mom_item_st*swtypitm, struct mom_item_st*insitm, struct mom_item_st*blkitm, unsigned rank)
      : CaseScannerData(em,swtypitm,insitm,blkitm,rank), cas_item2casemap() {};
    virtual ~ItemCaseScannerData()
    {
      cas_item2casemap.clear();
    };
  };
  MomEmitter(unsigned magic, struct mom_item_st*itm);
  MomEmitter(const MomEmitter&) = delete;
  virtual ~MomEmitter();
public:
  momvalue_t literal_string(const std::string&s)
  {
    auto it = _ce_literalstringmap.find(s);
    if (it != _ce_literalstringmap.end()) return it->second;
    auto strv = mom_boxstring_make(s.c_str());
    _ce_literalstringmap[s] = strv;
    return strv;
  }
  virtual void scan_data_element(struct mom_item_st*itm);
  virtual void scan_func_element(struct mom_item_st*itm);
  virtual void scan_routine_element(struct mom_item_st*itm);
  virtual const struct mom_boxnode_st* transform_data_element(struct mom_item_st*itm) =0;
  virtual const struct mom_boxnode_st* transform_body_element(struct mom_item_st*bdyitm, struct mom_item_st*routitm) =0;
  virtual const struct mom_boxnode_st* transform_func_element(struct mom_item_st*itm) =0;
  virtual const struct mom_boxnode_st* transform_routine_element(struct mom_item_st*itm) =0;
  virtual CaseScannerData* make_case_scanner_data(struct mom_item_st*swtypitm, struct mom_item_st*insitm, unsigned rk, struct mom_item_st*blkitm);
  virtual std::function<void(struct mom_item_st*, unsigned, CaseScannerData*)> case_scanner(struct mom_item_st*swtypitm, struct mom_item_st*insitm, unsigned rk, struct mom_item_st*blkitm) =0;
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
  const struct mom_boxnode_st*transform_top_module(void);
  void scan_module_element(struct mom_item_st*itm);
  const struct mom_boxnode_st*transform_module_element(struct mom_item_st*itm);
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
  void write_tree(FILE*out, unsigned depth, long &lastnl, const void*val, const void*forv=nullptr);
  void write_node(FILE*out, unsigned depth, long &lastnl, const struct mom_boxnode_st*nod,
                  const void*forv=nullptr);
  void write_balanced_node(FILE*out, unsigned depth,
                           const char*prefix, const char*suffix,
                           long &lastnl, const struct mom_boxnode_st*nod,
                           const void*forv=nullptr);
  static const unsigned constexpr MAXLINEWIDTH=64;
  static const unsigned constexpr MAXLINEINDENT=12;
  void write_indented_newline(FILE*out, unsigned depth, long &lastnl);
  void write_nl_or_space(FILE*out, unsigned depth, long &lastnl);
  unsigned magic() const
  {
    return _ce_magic;
  };
};				// end of MomEmitter

/***** acceptable nodes for write_node
   ^string(<string>) -> the doublequoted utf8cencoded <string>
   ^verbatim(<string>) -> the utf8cencoded <string>
   ^comma(...) -> comma separated arguments
   ^semicolon(...) -> semicolon separated arguments
   ^parenthesis(...) -> contents in parenthesis
   ^comment(...) -> the scalar elements as comment

 *****/

////////////////
class MomCEmitter final :public MomEmitter
{
  traced_vector_values_t _cec_globdecltree;
  traced_vector_values_t _cec_globdefintree;
  traced_set_items_t _cec_declareditems;
  friend bool mom_emit_c_code(struct mom_item_st*itm);

public:
  static const unsigned constexpr MAGIC = 508723037 /*0x1e527f5d*/;
  static   constexpr const char* CTYPE_PREFIX = "momty_";
  static   constexpr const char* CLOCAL_PREFIX = "momloc_";
  static   constexpr const char* CFORMAL_PREFIX = "momarg_";
  static   constexpr const char* CSIGNTYPE_PREFIX = "momsigty_";
  static   constexpr const char* CPREDEFITEM_MACRO = "MOM_PREDEFITM";
  static   constexpr const char* CINT_TYPE = "momint_t";
  static   constexpr const char* CVALUE_TYPE = "momvalue_t";
  static   constexpr const char* CDOUBLE_TYPE = "double";
  static   constexpr const char* CVOID_TYPE = "void";
  MomCEmitter(struct mom_item_st*itm)
    : MomEmitter(MAGIC, itm), _cec_globdecltree(), _cec_declareditems()  {};
  MomCEmitter(const MomCEmitter&) = delete;
  virtual ~MomCEmitter();
  void add_global_decl(const struct mom_boxnode_st*nod)
  {
    if (mom_itype(nod) == MOMITY_NODE) _cec_globdecltree.push_back(nod);
    else throw MOM_RUNTIME_PRINTF("bad global declaration %s", mom_value_cstring(nod));
  };
  void add_definition(const struct mom_boxnode_st*nod)
  {
    if (mom_itype(nod) == MOMITY_NODE) _cec_globdefintree.push_back(nod);
    else throw MOM_RUNTIME_PRINTF("bad definition %s", mom_value_cstring(nod));
  };
  momvalue_t declare_type (struct mom_item_st*typitm);
  const struct mom_boxnode_st* declare_funheader_for (struct mom_item_st*sigitm, struct mom_item_st*fitm);
  const struct mom_boxnode_st* declare_signature_type (struct mom_item_st*sigitm);
  virtual const struct mom_boxnode_st* transform_data_element(struct mom_item_st*itm);
  virtual const struct mom_boxnode_st* transform_func_element(struct mom_item_st*itm);
  virtual const struct mom_boxnode_st* transform_body_element(struct mom_item_st*bdyitm, struct mom_item_st*routitm);
  momvalue_t transform_block(struct mom_item_st*blkitm, struct mom_item_st*initm);
  momvalue_t transform_instruction(struct mom_item_st*insitm, struct mom_item_st*insideitm);
  momvalue_t transform_runinstr(struct mom_item_st*insitm, struct mom_item_st*runitm, struct mom_item_st*insideitm);
  momvalue_t transform_node_expr(const struct mom_boxnode_st*expnod, struct mom_item_st*insitm);
  momvalue_t transform_expr(momvalue_t expv, struct mom_item_st*insitm);
  momvalue_t transform_var(struct mom_item_st*varitm, struct mom_item_st*insitm, const vardef_st*varbind=nullptr);
  virtual const struct mom_boxnode_st* transform_routine_element(struct mom_item_st*elitm);
  CaseScannerData* make_case_scanner_data(struct mom_item_st*swtypitm, struct mom_item_st*insitm, unsigned rk, struct mom_item_st*blkitm);
  virtual std::function<void(struct mom_item_st*,unsigned,CaseScannerData*)> case_scanner(struct mom_item_st*swtypitm, struct mom_item_st*insitm, unsigned rk, struct mom_item_st*blkitm);
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
  static   constexpr const char* JSFUNC_PREFIX = "momjs_";
  MomJavascriptEmitter(struct mom_item_st*itm) : MomEmitter(MAGIC, itm) {};
  MomJavascriptEmitter(const MomJavascriptEmitter&) = delete;
  virtual ~MomJavascriptEmitter();
  virtual void scan_routine_element(struct mom_item_st*elitm)
  {
    throw MOM_RUNTIME_PRINTF("routine element %s unsupported for JavaScript", mom_item_cstring(elitm));
  };
  virtual const struct mom_boxnode_st* transform_data_element(struct mom_item_st*itm);
  virtual const struct mom_boxnode_st* transform_func_element(struct mom_item_st*itm);
  virtual const struct mom_boxnode_st* transform_body_element(struct mom_item_st*bdyitm, struct mom_item_st*routitm);
  momvalue_t transform_instruction(struct mom_item_st*insitm, struct mom_item_st*fromitm);
  const struct mom_boxnode_st* declare_funheader_for (struct mom_item_st*sigitm, struct mom_item_st*fitm);
  momvalue_t transform_block(struct mom_item_st*blkitm, struct mom_item_st*initm);
  momvalue_t transform_runinstr(struct mom_item_st*insitm, struct mom_item_st*runitm, struct mom_item_st*insideitm);
  virtual MomEmitter::CaseScannerData*
  make_case_scanner_data(struct mom_item_st*swtypitm, struct mom_item_st*insitm, unsigned rk, struct mom_item_st*blkitm);
  virtual std::function<void(struct mom_item_st*,unsigned,CaseScannerData*)> case_scanner(struct mom_item_st*swtypitm, struct mom_item_st*insitm, unsigned rk, struct mom_item_st*blkitm);
  virtual const struct mom_boxnode_st* transform_routine_element(struct mom_item_st*elitm)
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
      auto nod = cemit.transform_top_module();
      cemit.flush_todo_list(__LINE__);
      cemit.add_definition(nod);
      long r = 0;
      do
        {
          r= ((momrand_genrand_int32 () % 65536) << 32) |  momrand_genrand_int32();
        }
      while (MOM_UNLIKELY(r<100));
      cemit.flush_todo_list(__LINE__);
      MOM_DEBUGPRINTF(gencod, "mom_emit_c_code itm=%s nod=%s\n", mom_item_cstring(itm), mom_value_cstring(nod));
      const char*tmpath = mom_gc_printf(MOM_MODULEDIR "/" MOM_CPREFIX_PATH "%s.c+%lxp%d.tmp",
                                        mom_item_cstring(itm), r, (int)getpid());
      const char*gpath = mom_gc_printf(MOM_MODULEDIR "/" MOM_CPREFIX_PATH "%s.c", mom_item_cstring(itm));
      FILE* fout = fopen(tmpath, "w");
      if (!fout)
        MOM_FATAPRINTF("failed to fopen %s", tmpath);
      MOM_DEBUGPRINTF(gencod, "mom_emit_c_code tmpath=%s gpath=%s", tmpath, gpath);
      errno = 0;
      fprintf(fout, "// EmitC-generated file %s.c ** DO NOT EDIT\n\n",
              basename(gpath));
      mom_output_gplv3_notice (fout, "//", "",  basename(gpath));
      fprintf(fout, "\n\n#include \"meltmoni.h\"\n\n");
      int nbdecl = (int)cemit._cec_globdecltree.size();
      if (nbdecl>0)
        {
          fprintf(fout, "\n/// %d declarations:\n", nbdecl);
          for (int i=0; i<nbdecl; i++)
            {
              fputc('\n', fout);
              long nl = ftell(fout);
              cemit.write_tree(fout,0, nl, cemit._cec_globdecltree[i], itm);
              fputs("\n\n", fout);
            }
        }
      else fputs("// no declarations\n\n", fout);
      fflush(fout);
      int nbdef = (int)cemit._cec_globdefintree.size();
      if (nbdef>0)
        {
          fprintf(fout, "\n/// %d definitions:\n", nbdef);
          for (int i=0; i<nbdef; i++)
            {
              fputc('\n', fout);
              long nl = ftell(fout);
              cemit.write_tree(fout,0, nl, cemit._cec_globdefintree[i], itm);
              fputs("\n\n", fout);
            }
        }
      else fputs("// no definitions\n\n", fout);
      fprintf(fout,"/// end of generated file %s\n", basename(gpath));
      if (fclose(fout))
        MOM_FATAPRINTF("failed to fclose %s", tmpath);
      FILE*fold = fopen(gpath, "r");
      fout = fopen(tmpath, "r");
      if (!fout)
        MOM_FATAPRINTF("failed to re-fopen %s", tmpath);
      if (fold == nullptr)
        {
          fclose(fout);
          if (rename (tmpath, gpath))
            MOM_FATAPRINTF("failed to rename %s to %s", tmpath, gpath);
        }
      else
        {
          struct ::stat stold = {};
          struct ::stat stnew = {};
          if (fstat(fileno(fold),&stold))
            MOM_FATAPRINTF("failed to fstat#%d for %s", fileno(fold), gpath);
          if (fstat(fileno(fout), &stnew))
            MOM_FATAPRINTF("failed to fstat#%d for %s", fileno(fout), tmpath);
          bool same = stold.st_size == stnew.st_size;
          while (same)
            {
              int ob = fgetc(fold);
              int nb = fgetc(fout);
              if (ob != nb) same=false;
              if (ob == EOF) break;
            };
          fclose(fout);
          fclose(fold);
          if (same)
            {
              MOM_INFORMPRINTF("C code unchanged for module %s in %s", mom_item_cstring(itm), gpath);
              remove (tmpath);
            }
          else
            {
              const char*bakpath = mom_gc_printf(MOM_MODULEDIR "/" MOM_CPREFIX_PATH "%s.c~",
                                                 mom_item_cstring(itm));
              if (rename(gpath, bakpath))
                MOM_WARNPRINTF("failed to rename for backup %s -> %s (%m)", gpath, bakpath);
              else
                MOM_INFORMPRINTF("backed up %s -> %s", gpath, bakpath);
              if (rename(tmpath, gpath))
                MOM_FATAPRINTF("failed to rename %s to %s", tmpath, gpath);
              MOM_INFORMPRINTF("C code generated for module %s in %s of %ld bytes",
                               mom_item_cstring(itm), gpath, (long)stnew.st_size);
            }
        }
      return true;
    }
  catch (const MomRuntimeErrorAt& e)
    {
      MOM_WARNPRINTF_AT(e.file(), e.lineno(),
                        "mom_emit_c_code %s failed with MOM runtime exception %s in %s",
                        mom_item_cstring(itm), mom_gc_strdup(e.what()), mom_item_cstring(cemit.current_function()));
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
      auto nod = jsemit.transform_top_module();
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
                     _ce_localvalueset {},
                     _ce_localcloseditems {},
                     _ce_localnodetypecache {},
_ce_curfunctionitm {nullptr}
{
  if (!itm || itm==MOM_EMPTY_SLOT || itm->va_itype != MOMITY_ITEM)
    throw MOM_RUNTIME_ERROR("non item");
  lock_item(itm);
} // end MomEmitter::MomEmitter



const struct mom_boxnode_st*
MomEmitter::transform_top_module(void)
{
  auto descitm = mom_unsync_item_descr(_ce_topitm);
  traced_vector_values_t vecval;
  MOM_DEBUGPRINTF(gencod, "transform_top_module %s topitm=%s descitm=%s",
                  kindname(),
                  mom_item_cstring(_ce_topitm),
                  mom_item_cstring(descitm));
  if (descitm != MOM_PREDEFITM(module))
    throw MOM_RUNTIME_PRINTF("item %s has non-module descr: %s",
                             mom_item_cstring(_ce_topitm), mom_item_cstring(descitm));
  auto modulev = mom_unsync_item_get_phys_attr (_ce_topitm,  MOM_PREDEFITM(module));
  MOM_DEBUGPRINTF(gencod, "transform_top_module %s modulev %s",
                  kindname(),
                  mom_value_cstring(modulev));
  auto modulseq = mom_dyncast_seqitems (modulev);
  if (!modulseq)
    throw MOM_RUNTIME_PRINTF("item %s with bad module:%s",
                             mom_item_cstring(_ce_topitm), mom_value_cstring(modulev));
  bind_global(MOM_PREDEFITM(module),MOM_PREDEFITM(module),_ce_topitm);
  unsigned nbmodelem = mom_seqitems_length(modulseq);
  vecval.reserve(nbmodelem);
  auto itemsarr = mom_seqitems_arr(modulseq);
  for (unsigned ix=0; ix<nbmodelem; ix++)
    {
      auto curitm = itemsarr[ix];
      if (curitm==nullptr) continue;
      assert (mom_itype(curitm)==MOMITY_ITEM);
      lock_item(curitm);
      todo([=](MomEmitter*em)
      {
        MOM_DEBUGPRINTF(gencod, "transform_top_module before scanning module element %s #%d from top module %s",
                        mom_item_cstring(curitm), ix, mom_item_cstring(_ce_topitm));
        em->scan_module_element(curitm);
        MOM_DEBUGPRINTF(gencod, "transform_top_module after scanning module element %s #%d from top module %s",
                        mom_item_cstring(curitm), ix, mom_item_cstring(_ce_topitm));
      });
      flush_todo_list(__LINE__);
      todo([=, &vecval](MomEmitter*em)
      {
        MOM_DEBUGPRINTF(gencod, "transform_top_module before transforming"
                        " module element %s #%d from top module %s",
                        mom_item_cstring(curitm), ix, mom_item_cstring(_ce_topitm));
        auto nodelem = em->transform_module_element(curitm);
        MOM_DEBUGPRINTF(gencod, "transform_top_module after transforming"
                        " module element %s #%d from top module %s;\n"
                        " got %s",
                        mom_item_cstring(curitm), ix, mom_item_cstring(_ce_topitm),
                        mom_value_cstring(nodelem));
        if (nodelem==nullptr)
          throw MOM_RUNTIME_PRINTF("failed to transform element %s", mom_item_cstring(curitm));
        assert (mom_itype(nodelem) == MOMITY_NODE);
        const struct mom_item_st*nodconnitm = nodelem->nod_connitm;
        assert (mom_itype(nodconnitm) == MOMITY_ITEM);
        if (nodconnitm == MOM_PREDEFITM(sequence))
          {
            unsigned ln = mom_size(nodelem);
            vecval.reserve(ln+3);
            for (unsigned ix=0; ix<ln; ix++)
              vecval.push_back(nodelem->nod_sons[ix]);
          }
        else
          {
            vecval.push_back(nodelem);
          }
      });
      flush_todo_list(__LINE__);
      _ce_localvarmap.clear();
      _ce_localvarmap.clear();
      _ce_localcloseditems.clear();
      _ce_localnodetypecache.clear();
      _ce_curfunctionitm = nullptr;
    }
  auto modnod = mom_boxnode_make(MOM_PREDEFITM(module),vecval.size(),
                                 vecval.data());
  vecval.clear();
  MOM_DEBUGPRINTF(gencod, "transform_top_module from top module :=\n %s\n... gives modnod\n.. %s",
                  mom_item_content_cstring(_ce_topitm), mom_value_cstring(modnod));
  return modnod;
} // end of MomEmitter::transform_top_module


void
MomEmitter::scan_module_element(struct mom_item_st*elitm)
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
      goto datacase;
    case CASE_DESCR_MOM (thread_local):
      goto datacase;
    case CASE_DESCR_MOM (data):
datacase:
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


const struct mom_boxnode_st*
MomEmitter::transform_module_element(struct mom_item_st*elitm)
{
  const struct mom_boxnode_st*resnod = nullptr;
  struct mom_item_st*descitm = mom_unsync_item_descr(elitm);
  MOM_DEBUGPRINTF(gencod, "transform_module_element %s topitm=%s\nelitm:=%s\n descitm=%s",
                  kindname(),
                  mom_item_cstring(_ce_topitm),
                  mom_item_content_cstring(elitm), mom_item_cstring(descitm));
  if (!descitm)
    throw MOM_RUNTIME_PRINTF("module element %s without descr", mom_item_cstring(elitm));
#define NBMODELEMDESC_MOM 31
#define CASE_DESCR_MOM(Nam) momhashpredef_##Nam % NBMODELEMDESC_MOM:	\
	  if (descitm == MOM_PREDEFITM(Nam)) goto foundcase_##Nam;	\
	  goto defaultcasedesc; foundcase_##Nam
  switch (descitm->hva_hash % NBMODELEMDESC_MOM)
    {
    case CASE_DESCR_MOM (global):
      goto datacase;
    case CASE_DESCR_MOM (thread_local):
      goto datacase;
    case CASE_DESCR_MOM (data):
datacase:
      resnod = transform_data_element(elitm);
      MOM_DEBUGPRINTF(gencod, "transform_module_element data elitm=%s resnod=%s",
                      mom_item_cstring(elitm), mom_value_cstring(resnod));
      break;
    case CASE_DESCR_MOM (func):
      resnod = transform_func_element(elitm);
      MOM_DEBUGPRINTF(gencod, "transform_module_element func elitm=%s resnod=%s",
                      mom_item_cstring(elitm), mom_value_cstring(resnod));
      break;
    case CASE_DESCR_MOM (routine):
      resnod = transform_routine_element(elitm);
      MOM_DEBUGPRINTF(gencod, "transform_module_element routine elitm=%s resnod=%s",
                      mom_item_cstring(elitm), mom_value_cstring(resnod));
      break;
defaultcasedesc:
    default:
      throw MOM_RUNTIME_PRINTF("module element %s strange desc %s",
                               mom_item_cstring(elitm), mom_item_cstring(descitm));
    };
#undef NBMODELEMDESC_MOM
#undef CASE_DESCR_MOM
  MOM_DEBUGPRINTF(gencod, "transform_module_element topitm=%s\nelitm:=%s\n.. resnod=%s",
                  mom_item_cstring(_ce_topitm),
                  mom_item_content_cstring(elitm),
                  mom_value_cstring(resnod));
  return resnod;
} // end of MomEmitter::transform_module_element



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
  lock_item(sigitm);
  {
    auto sigr=scan_signature(sigitm,fuitm);
    auto formaltup = sigr.sig_formals;
    MOM_DEBUGPRINTF(gencod, "scan_func_element fuitm=%s formaltup=%s",
                    mom_item_cstring(fuitm), mom_value_cstring(formaltup));
    if (mom_boxtuple_length(formaltup)==0
        || mom_boxtuple_nth(formaltup, 0) != MOM_PREDEFITM(this_closure))
      {
        MOM_DEBUGPRINTF(gencod, "scan_func_element: bad func %s with signature %s of formals %s not starting with `this_closure`",
                        mom_item_cstring(fuitm),
                        mom_item_cstring(sigitm),
                        mom_value_cstring(formaltup));
        throw MOM_RUNTIME_PRINTF("func %s with signature %s of formals %s not starting with `this_closure`",
                                 mom_item_cstring(fuitm),
                                 mom_item_cstring(sigitm),
                                 mom_value_cstring(formaltup));
      }
  }
  if (bdyitm == nullptr)
    throw MOM_RUNTIME_PRINTF("missing or bad body in func %s", mom_item_cstring(fuitm));
  lock_item(bdyitm);
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
  auto formv = mom_unsync_item_get_phys_attr(sigitm, MOM_PREDEFITM(formals));
  auto formtup =
    mom_dyncast_tuple(formv);
  MOM_DEBUGPRINTF(gencod, "scan_signature sigitm=%s formtup=%s formv=%s",
                  mom_item_cstring(sigitm), mom_value_cstring(formtup), mom_value_cstring(formv));
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
      if (!nobind)
        {
          auto curformbind = get_binding(curformitm);
          if (curformbind)
            throw MOM_RUNTIME_PRINTF("already bound formal#%d %s to role %s in signature %s",
                                     ix, mom_item_cstring(curformitm),
                                     mom_item_cstring(curformbind->vd_rolitm), mom_item_cstring(sigitm));
        }
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
  _ce_sigitems.insert(sigitm);
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
  MOM_DEBUGPRINTF(gencod, "scan_block end blkitm=%s initm=%s\n",
                  mom_item_cstring(blkitm), mom_item_cstring(initm));
} // end MomEmitter::scan_block


void MomEmitter::scan_instr(struct mom_item_st*insitm, int rk, struct mom_item_st*blkitm)
{
  MOM_DEBUGPRINTF(gencod, "scan_instr start insitm=%s rk#%d blkitm=%s",
                  mom_item_cstring(insitm), rk, mom_item_cstring(blkitm));
  assert (is_locked_item(insitm));
  {
    auto insbind = get_local_binding(insitm);
    if (insbind != nullptr)
      throw MOM_RUNTIME_PRINTF("instruction %s rank #%d in block %s already"
                               " locally bound to role %s what %s",
                               mom_item_cstring(insitm), rk, mom_item_cstring(blkitm),
                               mom_item_cstring(insbind->vd_rolitm),
                               mom_value_cstring(insbind->vd_what));
  }
  struct mom_item_st*desitm = mom_unsync_item_descr(insitm);
  MOM_DEBUGPRINTF(gencod, "scan_instr insitm=%s desitm=%s rk#%d blkitm=%s",
                  mom_item_cstring(insitm),  mom_item_cstring(desitm),
                  rk, mom_item_cstring(blkitm));
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
      bind_local(insitm,MOM_PREDEFITM(assign),
                 mom_boxnode_make_va(MOM_PREDEFITM(assign),3,
                                     tovaritm, fromexp,  totypitm),
                 blkitm, rk);
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
      bind_local(insitm,MOM_PREDEFITM(break),
                 outblkitm, blkitm, rk);
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
      bind_local(insitm,MOM_PREDEFITM(continue),
                 loopitm, blkitm, rk);
    }
    break;
    case CASE_OPER_MOM(loop):
      goto sequencecase;
    case CASE_OPER_MOM(sequence):
sequencecase:
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
        bind_local(insitm,desitm,
                   insitm, blkitm, rk);
      }
      break;
    case CASE_OPER_MOM(cond):
    {
      auto condtup= mom_dyncast_tuple(mom_unsync_item_get_phys_attr(insitm, MOM_PREDEFITM(cond)));
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
      bind_local(insitm,MOM_PREDEFITM(cond),
                 condtup, blkitm, rk);
    }
    break;
    case CASE_OPER_MOM(call):
    {
      auto callsigitm =
        mom_dyncast_item(mom_unsync_item_get_phys_attr(insitm, MOM_PREDEFITM(call)));
      if (!callsigitm)
        throw MOM_RUNTIME_PRINTF("missing call signature in call instr %s rk#%d in block %s",
                                 mom_item_cstring(insitm), rk, mom_item_cstring(blkitm));
      auto resultv = mom_unsync_item_get_phys_attr(insitm, MOM_PREDEFITM(result));
      lock_item (callsigitm);
      auto sigr = scan_nonbinding_signature(callsigitm, insitm);
      auto sformaltup = sigr.sig_formals;
      auto sresultv = sigr.sig_result;
      unsigned sigarity = mom_boxtuple_length(sformaltup);
      auto comps = insitm->itm_pcomp;
      unsigned nbcomp = mom_vectvaldata_count(comps);
      if (nbcomp != sigarity)
        throw MOM_RUNTIME_PRINTF("call instr %s rk#%d in block %s with wrong arity,"
                                 " got %d expecting %d for signature %s",
                                 mom_item_cstring(insitm), rk, mom_item_cstring(blkitm),
                                 nbcomp, sigarity, mom_item_cstring(callsigitm));
      auto funv = mom_unsync_item_get_phys_attr(insitm, MOM_PREDEFITM(func));
      if (funv == nullptr)
        throw MOM_RUNTIME_PRINTF("call instr %s rk#%d in block %s without func",
                                 mom_item_cstring(insitm), rk, mom_item_cstring(blkitm));
      if (scan_expr(funv, insitm, 1, MOM_PREDEFITM(value)) != MOM_PREDEFITM(value))
        throw MOM_RUNTIME_PRINTF("call instr %s rk#%d in block %s with bad non-value func %s",
                                 mom_item_cstring(insitm), rk, mom_item_cstring(blkitm),
                                 mom_value_cstring(funv));
      for (unsigned ix=0; ix<nbcomp; ix++)
        {
          auto curformitm = mom_boxtuple_nth(sformaltup, ix);
          assert (curformitm != nullptr);
          assert (is_locked_item(curformitm));
          auto curtypitm =
            mom_dyncast_item(mom_unsync_item_get_phys_attr(curformitm, MOM_PREDEFITM(type)));
          assert (curtypitm != nullptr);
          assert (is_locked_item(curtypitm));
          auto curargv = mom_vectvaldata_nth(comps,ix);
          if (scan_expr(curargv, insitm, 1, curtypitm) != curtypitm)
            throw MOM_RUNTIME_PRINTF("call instr %s rk#%d in block %s "
                                     "with mistyped arg#%d %s (expecting %s)",
                                     mom_item_cstring(insitm), rk, mom_item_cstring(blkitm),
                                     rk, mom_value_cstring(curargv),
                                     mom_item_cstring(curtypitm));
        };
      auto resity = mom_itype(resultv);
      if (resity == MOMITY_ITEM)
        {
          auto formresitm = mom_dyncast_item(sresultv);
          if (formresitm == nullptr)
            throw  MOM_RUNTIME_PRINTF("call instr %s rk#%d in block %s "
                                      "with signature %s of non-item result %s",
                                      mom_item_cstring(insitm), rk, mom_item_cstring(blkitm),
                                      mom_item_cstring(callsigitm), mom_value_cstring(resultv));
          assert (is_locked_item(formresitm));
          auto formrestypitm =
            mom_dyncast_item(mom_unsync_item_get_phys_attr(formresitm, MOM_PREDEFITM(type)));
          assert (formrestypitm != nullptr);
          assert (is_locked_item(formrestypitm));
          auto resvaritm = (struct mom_item_st*)resultv;
          lock_item(resvaritm);
          if (scan_var(resvaritm, insitm, formrestypitm) != formrestypitm)
            throw  MOM_RUNTIME_PRINTF("call instr %s rk#%d in block %s "
                                      "with mistyped result %s, expecting %s",
                                      mom_item_cstring(insitm), rk, mom_item_cstring(blkitm),
                                      mom_item_cstring(resvaritm), mom_item_cstring(formrestypitm));

        }
      else if (resity == MOMITY_TUPLE)
        {
          const struct mom_boxtuple_st * formrestup = mom_dyncast_tuple(sresultv);
          const struct mom_boxtuple_st*resultup = (const struct mom_boxtuple_st*)resultv;
          unsigned nbformresults = mom_size(formrestup);
          unsigned nbresults = mom_size(resultup);
          if (nbformresults != nbresults)
            throw  MOM_RUNTIME_PRINTF("call instr %s rk#%d in block %s "
                                      "with %d results, expecting %d from signature %s",
                                      mom_item_cstring(insitm), rk, mom_item_cstring(blkitm),
                                      nbresults, nbformresults, mom_item_cstring(callsigitm));
          for (unsigned resix=0; resix < nbresults; resix++)
            {
              struct mom_item_st* curformresitm = ::mom_boxtuple_nth(formrestup, resix);
              struct mom_item_st* curesvaritm = ::mom_boxtuple_nth(resultup, resix);
              lock_item(curesvaritm);
              assert (is_locked_item(curformresitm));
              auto curformrestypitm =
                mom_dyncast_item(mom_unsync_item_get_phys_attr(curformresitm, MOM_PREDEFITM(type)));
              assert (curformrestypitm != nullptr);
              assert (is_locked_item(curformrestypitm));
              if (scan_var (curesvaritm, insitm, curformrestypitm) != curformrestypitm)
                throw MOM_RUNTIME_PRINTF("call instr %s rk#%d in block %s "
                                         "with mistyped result#%d %s, expecting type %s from signature %s",
                                         mom_item_cstring(insitm), rk, mom_item_cstring(blkitm),
                                         resix, mom_item_cstring(curesvaritm),
                                         mom_item_cstring(curformrestypitm), mom_item_cstring(callsigitm));
            }
        }
      else if (resity != MOMITY_NONE)
        throw  MOM_RUNTIME_PRINTF("call instr %s rk#%d in block %s "
                                  "with bad result %s",
                                  mom_item_cstring(insitm), rk, mom_item_cstring(blkitm),
                                  mom_value_cstring(resultv));
      bind_local(insitm,MOM_PREDEFITM(call),callsigitm, blkitm, rk);
    }
    break;
    case CASE_OPER_MOM(run):
    {
      auto primitm =
        mom_dyncast_item(mom_unsync_item_get_phys_attr(insitm, MOM_PREDEFITM(run)));
      if (!primitm)
        throw MOM_RUNTIME_PRINTF("missing primitive to `run` in instr %s rk#%d in block %s",
                                 mom_item_cstring(insitm), rk, mom_item_cstring(blkitm));
      auto resultv = mom_unsync_item_get_phys_attr(insitm, MOM_PREDEFITM(result));
      lock_item(primitm);
      auto primdescitm =  mom_unsync_item_descr(primitm);
      if (primdescitm != MOM_PREDEFITM(primitive))
        throw MOM_RUNTIME_PRINTF("bad primitive %s to `run` in instr %s rk#%d in block %s",
                                 mom_item_cstring(primitm), mom_item_cstring(insitm), rk, mom_item_cstring(blkitm));
      auto primsigitm =
        mom_dyncast_item(mom_unsync_item_get_phys_attr(primitm, MOM_PREDEFITM(signature)));
      if (!primsigitm)
        throw  MOM_RUNTIME_PRINTF("primitive %s without signature to `run` in instr %s rk#%d in block %s",
                                  mom_item_cstring(primitm), mom_item_cstring(insitm), rk, mom_item_cstring(blkitm));
      lock_item(primsigitm);
      auto sigr = scan_nonbinding_signature(primsigitm, insitm);
      auto sformaltup = sigr.sig_formals;
      auto sresultv = sigr.sig_result;
      unsigned sigarity = mom_boxtuple_length(sformaltup);
      auto comps = insitm->itm_pcomp;
      unsigned nbcomp = mom_vectvaldata_count(comps);
      if (nbcomp != sigarity)
        {
          MOM_DEBUGPRINTF(gencod, "badarity runinstr %s rk#%d",
                          mom_item_cstring(insitm), rk);
          MOM_DEBUGPRINTF(gencod, "badarity blkitm %s nbcomp %d sigarity %d",
                          mom_item_cstring(blkitm), nbcomp, sigarity);
          MOM_DEBUGPRINTF(gencod, "badarity primsigitm %s", mom_item_cstring(primsigitm));
          throw MOM_RUNTIME_PRINTF("run instr %s rk#%d in block %s with wrong arity,"
                                   " got %d expecting %d for signature %s",
                                   mom_item_cstring(insitm), rk, mom_item_cstring(blkitm),
                                   nbcomp, sigarity, mom_item_cstring(primsigitm));
        }
      for (unsigned ix=0; ix<nbcomp; ix++)
        {
          auto curformitm = mom_boxtuple_nth(sformaltup, ix);
          assert (curformitm != nullptr);
          assert (is_locked_item(curformitm));
          auto curtypitm =
            mom_dyncast_item(mom_unsync_item_get_phys_attr(curformitm, MOM_PREDEFITM(type)));
          assert (curtypitm != nullptr);
          assert (is_locked_item(curtypitm));
          auto curargv = mom_vectvaldata_nth(comps,ix);
          if (scan_expr(curargv, insitm, 1, curtypitm) != curtypitm)
            throw MOM_RUNTIME_PRINTF("run instr %s rk#%d in block %s "
                                     "with mistyped arg#%d %s (expecting %s)",
                                     mom_item_cstring(insitm), rk, mom_item_cstring(blkitm),
                                     rk, mom_value_cstring(curargv),
                                     mom_item_cstring(curtypitm));
        };
      auto resity = mom_itype(resultv);
      if (resity == MOMITY_ITEM)
        {
          auto formresitm = mom_dyncast_item(sresultv);
          if (formresitm == nullptr)
            throw  MOM_RUNTIME_PRINTF("run instr %s rk#%d in block %s "
                                      "with signature %s of non-item result %s",
                                      mom_item_cstring(insitm), rk, mom_item_cstring(blkitm),
                                      mom_item_cstring(primsigitm), mom_value_cstring(resultv));
          assert (is_locked_item(formresitm));
          auto formrestypitm =
            mom_dyncast_item(mom_unsync_item_get_phys_attr(formresitm, MOM_PREDEFITM(type)));
          assert (formrestypitm != nullptr);
          assert (is_locked_item(formrestypitm));
          auto resvaritm = (struct mom_item_st*)resultv;
          lock_item(resvaritm);
          if (scan_var(resvaritm, insitm, formrestypitm) != formrestypitm)
            throw  MOM_RUNTIME_PRINTF("run instr %s rk#%d in block %s "
                                      "with mistyped result %s, expecting %s",
                                      mom_item_cstring(insitm), rk, mom_item_cstring(blkitm),
                                      mom_item_cstring(resvaritm), mom_item_cstring(formrestypitm));

        }
      else if (resity == MOMITY_TUPLE)
        {
          const struct mom_boxtuple_st * formrestup = mom_dyncast_tuple(sresultv);
          const struct mom_boxtuple_st*resultup = (const struct mom_boxtuple_st*)resultv;
          unsigned nbformresults = mom_size(formrestup);
          unsigned nbresults = mom_size(resultup);
          if (nbformresults != nbresults)
            throw  MOM_RUNTIME_PRINTF("run instr %s rk#%d in block %s "
                                      "with %d results, expecting %d from signature %s",
                                      mom_item_cstring(insitm), rk, mom_item_cstring(blkitm),
                                      nbresults, nbformresults, mom_item_cstring(primsigitm));
          for (unsigned resix=0; resix < nbresults; resix++)
            {
              struct mom_item_st* curformresitm = ::mom_boxtuple_nth(formrestup, resix);
              struct mom_item_st* curesvaritm = ::mom_boxtuple_nth(resultup, resix);
              lock_item(curesvaritm);
              assert (is_locked_item(curformresitm));
              auto curformrestypitm =
                mom_dyncast_item(mom_unsync_item_get_phys_attr(curformresitm, MOM_PREDEFITM(type)));
              assert (curformrestypitm != nullptr);
              assert (is_locked_item(curformrestypitm));
              if (scan_var (curesvaritm, insitm, curformrestypitm) != curformrestypitm)
                throw MOM_RUNTIME_PRINTF("run instr %s rk#%d in block %s "
                                         "with mistyped result#%d %s, expecting type %s from signature %s",
                                         mom_item_cstring(insitm), rk, mom_item_cstring(blkitm),
                                         resix, mom_item_cstring(curesvaritm),
                                         mom_item_cstring(curformrestypitm), mom_item_cstring(primsigitm));
            }
        }
      else if (resity != MOMITY_NONE)
        throw MOM_RUNTIME_PRINTF("run instr %s rk#%d in block %s "
                                 "with bad result %s",
                                 mom_item_cstring(insitm), rk, mom_item_cstring(blkitm),
                                 mom_value_cstring(resultv));
      bind_local(insitm,MOM_PREDEFITM(run),primitm, blkitm, rk);
    }
    break;
    case CASE_OPER_MOM(switch):
    {
      auto swtypitm =
        mom_dyncast_item(mom_unsync_item_get_phys_attr(insitm, MOM_PREDEFITM(switch)));
      auto casehdr = case_scanner(swtypitm,insitm,rk,blkitm);
      auto argv = mom_unsync_item_get_phys_attr(insitm, MOM_PREDEFITM(arg));
      if (argv==nullptr)
        throw  MOM_RUNTIME_PRINTF("switch instr %s rk#%d in block %s "
                                  "with missing arg",
                                  mom_item_cstring(insitm), rk, mom_item_cstring(blkitm));
      auto caseseq = mom_dyncast_seqitems(mom_unsync_item_get_phys_attr(insitm, MOM_PREDEFITM(case)));
      if (caseseq==nullptr)
        throw  MOM_RUNTIME_PRINTF("switch instr %s rk#%d in block %s "
                                  "with missing case",
                                  mom_item_cstring(insitm), rk, mom_item_cstring(blkitm));
      auto otherwv = mom_unsync_item_get_phys_attr(insitm, MOM_PREDEFITM(otherwise));
      auto argtypitm = scan_expr(argv,insitm,1);
      if (!argtypitm)
        throw  MOM_RUNTIME_PRINTF("switch instr %s rk#%d in block %s "
                                  "with untypeable arg",
                                  mom_item_cstring(insitm), rk, mom_item_cstring(blkitm));
      if (argtypitm != swtypitm && argtypitm != MOM_PREDEFITM(value))
        throw  MOM_RUNTIME_PRINTF("switch instr %s rk#%d in block %s "
                                  "with arg of incompatible type %s for %s",
                                  mom_item_cstring(insitm), rk, mom_item_cstring(blkitm),
                                  mom_item_cstring(argtypitm), mom_item_cstring(swtypitm));
      std::unique_ptr<CaseScannerData> casdata {make_case_scanner_data(swtypitm,insitm,rk,blkitm)};
      assert (casdata != nullptr);
      auto casescan = case_scanner(swtypitm,insitm,rk,blkitm);
      unsigned nbcases = mom_seqitems_length(caseseq);
      for (unsigned casix=0; casix<nbcases; casix++)
        {
          auto curcasitm = mom_seqitems_nth(caseseq, casix);
          if (!curcasitm)
            throw  MOM_RUNTIME_PRINTF("switch instr %s rk#%d in block %s "
                                      "without case#%d",
                                      mom_item_cstring(insitm), rk, mom_item_cstring(blkitm),
                                      casix);
          lock_item(curcasitm);
          if (mom_unsync_item_descr(curcasitm) != MOM_PREDEFITM(case))
            throw MOM_RUNTIME_PRINTF("switch instr %s rk#%d in block %s "
                                     "with bad case#%d : %s",
                                     mom_item_cstring(insitm), rk, mom_item_cstring(blkitm),
                                     casix, mom_item_cstring(curcasitm));
          casescan(curcasitm,casix,casdata.get());
        }
      if (otherwv)
        {
          auto otherwitm = mom_dyncast_item(otherwv);
          if (!otherwitm)
            throw MOM_RUNTIME_PRINTF("switch instr %s rk#%d in block %s "
                                     "with bad otherwise %s",
                                     mom_item_cstring(insitm), rk, mom_item_cstring(blkitm),
                                     mom_value_cstring(otherwv));
          lock_item(otherwitm);
          scan_instr(otherwitm,-1,insitm);
        }
      bind_local(insitm,MOM_PREDEFITM(switch),
                 mom_boxnode_make_va(MOM_PREDEFITM(switch),3,
                                     argv, caseseq, otherwv),
                 blkitm, rk);
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
  MOM_DEBUGPRINTF(gencod, "scan_instr end insitm=%s rk#%d blkitm=%s\n",
                  mom_item_cstring(insitm), rk, mom_item_cstring(blkitm));
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
    }
    case MOMITY_NODE:
    {
      auto nodexp = (const struct mom_boxnode_st*)expv;
      auto it =_ce_localnodetypecache.find(nodexp);
      if (it == _ce_localnodetypecache.end())
        {
          auto ntypitm = scan_node_expr(nodexp,insitm,depth,typitm);
          if (typitm && ntypitm != typitm)
            throw MOM_RUNTIME_PRINTF("node expr %s in instr %s has incompatible type, got %s but expecting %s",
                                     mom_value_cstring(nodexp), mom_item_cstring(insitm),
                                     mom_item_cstring(ntypitm), mom_item_cstring(typitm));
          if (ntypitm == nullptr)
            throw MOM_RUNTIME_PRINTF("node expr %s in instr %s is untypable",
                                     mom_value_cstring(nodexp), mom_item_cstring(insitm));
          _ce_localnodetypecache[nodexp] = ntypitm;
          return ntypitm;
        }
      else
        {
          auto nodtypitm = it->second;
          assert (nodtypitm != nullptr && nodtypitm->va_itype == MOMITY_ITEM);
          if (typitm && nodtypitm != typitm)
            throw MOM_RUNTIME_PRINTF("node expr %s in instr %s has incompatible type, got %s but expecting %s",
                                     mom_value_cstring(nodexp), mom_item_cstring(insitm),
                                     mom_item_cstring(nodtypitm), mom_item_cstring(typitm));
          return nodtypitm;
        }
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
      goto orcase;
    case CASE_EXPCONN_MOM(or):
orcase:
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
      goto multcase;
    case CASE_EXPCONN_MOM(mult):
multcase:
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
      goto divcase;
    case CASE_EXPCONN_MOM(div):
divcase:
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
      goto tuplecase;
    case CASE_EXPCONN_MOM(set):
      goto tuplecase;
    case CASE_EXPCONN_MOM(tuple):
tuplecase:
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
      goto primitivecase;
    case CASE_DESCONN_MOM(primitive):
primitivecase:
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
  throw  MOM_RUNTIME_PRINTF("expnod %s has unexpected connective %s of descr %s in instr %s depth %d",
                            mom_value_cstring(expnod), mom_item_cstring(connitm),
                            mom_item_cstring(desconnitm), mom_value_cstring(insitm), depth);
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
  if (typexpitm)
    {
      if (typitm && typexpitm != typitm)
        throw  MOM_RUNTIME_PRINTF("item %s in instr %s has type %s but expecting %s",
                                  mom_item_cstring(expitm),
                                  mom_item_cstring(insitm),
                                  mom_item_cstring(typexpitm),
                                  mom_item_cstring(typitm));
      return typexpitm;
    }
  MOM_DEBUGPRINTF(gencod, "scan_item_expr expitm=%s typitm=%s typexpitm=%s",
                  mom_item_cstring(expitm), mom_item_cstring(typitm),
                  mom_item_cstring(typexpitm));
  _ce_localvalueset.insert(expitm);
  bind_local(expitm, MOM_PREDEFITM(item), expitm);
  if (typitm == MOM_PREDEFITM(item))
    return typitm;
  else if (typitm == nullptr || typitm == MOM_PREDEFITM(value))
    return MOM_PREDEFITM(value);
  else
    throw  MOM_RUNTIME_PRINTF("item %s in instr %s, expecting type %s",
                              mom_item_cstring(expitm),
                              mom_item_cstring(insitm),
                              mom_item_cstring(typitm));
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
                                  mom_item_cstring(varitm), mom_item_cstring(insitm),
                                  mom_item_cstring(globvarbind->vd_rolitm));
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
  _ce_topitm = nullptr;
  _ce_sigitems.clear();
  _ce_typitems.clear();
  _ce_blockitems.clear();
  _ce_literalstringmap.clear();
  _ce_todoque.clear();
  _ce_globalvarmap.clear();
  _ce_localvarmap.clear();
  _ce_localvalueset.clear();
  _ce_localcloseditems.clear();
  _ce_localnodetypecache.clear();
  _ce_curfunctionitm = nullptr;
} // end MomEmitter::~MomEmitter



void
MomEmitter::write_tree(FILE*out, unsigned depth, long &lastnl, const void*val, const void*forv)
{
  assert (out != nullptr);
  MOM_DEBUGPRINTF(gencod, "write_tree start depth=%d lastnl=%ld val=%s forv=%s",
                  depth, lastnl, mom_value_cstring(val), mom_value_cstring(forv));
  unsigned ty = mom_itype(val);
  switch (ty)
    {
    case MOMITY_NONE:
      break;
    case MOMITY_ITEM:
      fputs(mom_item_cstring((const mom_item_st*)val), out);
      break;
    case MOMITY_SET:
      throw MOM_RUNTIME_PRINTF("cannot write_tree set:%s depth=%d forv=%s",
                               mom_value_cstring(val), depth, mom_value_cstring(forv));
    case MOMITY_TUPLE:
    {
      auto tup = (const mom_boxtuple_st*)val;
      unsigned siz = mom_raw_size(tup);
      for (unsigned ix=0; ix<siz; ix++)
        {
          auto compitm=tup->seqitem[ix];
          if (compitm==nullptr) continue;
          assert (mom_itype(compitm)==MOMITY_ITEM);
          fputs(mom_item_cstring(compitm), out);
        }
    }
    break;
    case MOMITY_INT:
    {
      long long l = mom_int_val_def (val, 0);
      fprintf(out, "%lld", l);
    }
    break;
    case MOMITY_BOXDOUBLE:
    {
      char buf[48];
      double x= ((const struct mom_boxdouble_st *)val)->boxd_dbl;
      memset(buf, 0, sizeof(buf));
      fputs(mom_double_to_cstr (x,buf,sizeof(buf)), out);
    }
    break;
    case MOMITY_BOXSTRING:
    {
      fputs(((const struct mom_boxstring_st*)val)->cstr, out);
    }
    break;
    case MOMITY_NODE:
    {
      write_node(out,depth,lastnl,(const struct mom_boxnode_st*)val, forv);
    }
    break;
    default:
      throw MOM_RUNTIME_PRINTF("unexpected write_tree val:%s depth=%d forv=%s",
                               mom_value_cstring(val), depth, mom_value_cstring(forv));
    }
} // end of MomEmitter::write_tree


void
MomEmitter::write_indented_newline(FILE*out, unsigned depth, long &lastnl)
{
  fputc('\n', out);
  lastnl = ftell(out);
  if (MOM_UNLIKELY(lastnl <= 0))
    MOM_FATAPRINTF("failed to ftell (out@%p, fileno#%d)", out, fileno(out));
  for (int ix=(int)depth % MAXLINEINDENT; ix>=0; ix--)
    fputc(' ', out);
} // end of MomEmitter::write_indented_newline


void
MomEmitter::write_nl_or_space(FILE*out, unsigned depth, long &lastnl)
{
  assert (out != nullptr);
  long curoff = ftell(out);
  if (MOM_UNLIKELY(curoff < 0))
    MOM_FATAPRINTF("failed to ftell (out@%p, fileno#%d)", out, fileno(out));
  if (curoff >= lastnl + MAXLINEWIDTH)
    write_indented_newline(out,depth,lastnl);
  else fputc(' ', out);
} // end of MomEmitter::write_nl_or_space

void
MomEmitter::write_balanced_node(FILE*out, unsigned depth,
                                const char*prefix, const char*suffix,
                                long &lastnl, const  struct mom_boxnode_st*nod,
                                const void*forv)
{
  assert (mom_itype(nod) == MOMITY_NODE);
  fputs(prefix, out);
  write_nl_or_space(out, depth, lastnl);
  unsigned arity = mom_raw_size(nod);
  for (int ix=0; ix<(int)arity; ix++)
    {
      write_tree(out,depth+1,lastnl,nod->nod_sons[ix],forv);
    }
  fputc(' ', out);
  fputs(suffix, out);
} // end of MomEmitter::write_balanced_node


void
MomEmitter::write_node(FILE*out, unsigned depth, long &lastnl, const  struct mom_boxnode_st*nod,
                       const void*forv)
{
  assert (out != nullptr);
  assert (mom_itype(nod) == MOMITY_NODE);
  MOM_DEBUGPRINTF(gencod, "write_node start depth=%d lastnl=%ld nod=%s forv=%s",
                  depth, lastnl, mom_value_cstring(nod), mom_value_cstring(forv));
  unsigned arity = mom_raw_size(nod);
  const struct mom_item_st*connitm = nod->nod_connitm;
  assert (mom_itype(connitm) == MOMITY_ITEM);
#define NBNODECONN_MOM 97
#define CASE_NODECONN_MOM(Nam) momhashpredef_##Nam % NBNODECONN_MOM:	\
	  if (connitm == MOM_PREDEFITM(Nam)) goto foundcase_##Nam;	\
	  goto defaultcaseconn; foundcase_##Nam
  switch (connitm->hva_hash % NBNODECONN_MOM)
    {
    case CASE_NODECONN_MOM(string):
      goto verbatimcase;
    case CASE_NODECONN_MOM(verbatim):
verbatimcase:
      {
        bool verb = (connitm == MOM_PREDEFITM(verbatim));
        if (arity != 1) goto badarity;
        auto sonv = nod->nod_sons[0];
        if (mom_itype(sonv) != MOMITY_BOXSTRING)
          throw  MOM_RUNTIME_PRINTF("expecting string son write_node %s depth=%d forv=%s",
                                    mom_value_cstring(nod), depth, mom_value_cstring(forv));
        auto str = ((const struct mom_boxstring_st*)sonv)->cstr;
        auto ln = mom_raw_size(sonv);
        if (!verb) fputs(" \"", out);
        mom_output_utf8_encoded (out, str, ln);
        if (!verb) fputs("\"", out);
      }
      break;
    case CASE_NODECONN_MOM(comment):
    {
      fputs("/**", out);
      for (int i=0; i<arity; i++)
        {
          auto sonv = nod->nod_sons[i];
          if (mom_itype(sonv) == MOMITY_BOXSTRING)
            {
              auto strson = (const mom_boxstring_st*)sonv;
              if (strstr(strson->cstr, "*/") || strchr(strson->cstr, '\n'))
                throw MOM_RUNTIME_PRINTF("bad string son in comment: %s",
                                         mom_value_cstring(nod));
              else
                fputs(strson->cstr, out);
            }
          else
            write_tree(out, depth+1, lastnl, sonv, forv);
          write_nl_or_space(out,depth,lastnl);
        }
      fputs("**/", out);
      write_nl_or_space(out,depth,lastnl);
    }
    break;
    case CASE_NODECONN_MOM(comma):
      goto semicoloncase;
    case CASE_NODECONN_MOM(semicolon):
semicoloncase:
      {
        bool semic = (connitm == MOM_PREDEFITM(semicolon));
        int cnt=0;
        for (int ix=0; ix<(int)arity; ix++)
          {
            if (nod->nod_sons[ix] == nullptr)
              continue;
            if (cnt>0)
              {
                fputc(semic?';':',', out);
                write_nl_or_space(out,depth,lastnl);
              }
            write_tree(out, depth+1, lastnl, nod->nod_sons[ix], forv);
            cnt++;
          }
      }
      break;
    case CASE_NODECONN_MOM(sequence):
    {
      for (int ix=0; ix<(int)arity; ix++)
        {
          write_tree(out, depth+1, lastnl, nod->nod_sons[ix], forv);
        }
    }
    break;
    case CASE_NODECONN_MOM(parenthesis):
      write_balanced_node(out,depth,"(",")",lastnl,nod,forv);
      break;
    case CASE_NODECONN_MOM(brace):
      write_balanced_node(out,depth,"{","}",lastnl,nod,forv);
      break;
    case CASE_NODECONN_MOM(bracket):
      write_balanced_node(out,depth,"[","]",lastnl,nod,forv);
      break;
    case CASE_NODECONN_MOM(module):
    {
      for (int ix=0; ix<(int)arity; ix++)
        {
          fputc('\n', out);
          lastnl = ftell(out);
          write_tree(out, depth+1, lastnl, nod->nod_sons[ix], forv);
          fputc('\n', out);
        }
    }
    break;
    default:
defaultcaseconn:
      throw  MOM_RUNTIME_PRINTF("unexpected write_node nod:%s depth=%d forv=%s",
                                mom_value_cstring(nod), depth, mom_value_cstring(forv));
badarity:
      throw  MOM_RUNTIME_PRINTF("bad arity  write_node nod:%s depth=%d forv=%s",
                                mom_value_cstring(nod), depth, mom_value_cstring(forv));

    }
#undef CASE_NODECONN_MOM
#undef NBNODECONN_MOM
} // end MomEmitter::write_node
////////////////////////////////////////////////////////////////
MomCEmitter::~MomCEmitter()
{
  MOM_DEBUGPRINTF(gencod, "end %s for this@%p", kindname(), this);
} // end MomCEmitter::~MomCEmitter

const struct mom_boxnode_st*
MomCEmitter::transform_data_element(struct mom_item_st*itm)
{
#warning unimplemented MomCEmitter::transform_data_element
  MOM_FATAPRINTF("unimplemented MomCEmitter::transform_data_element itm=%s", mom_item_cstring(itm));
} // end MomCEmitter::transform_data_element

const struct mom_boxnode_st*
MomCEmitter::declare_funheader_for (struct mom_item_st*sigitm, struct mom_item_st*fitm)
{
  assert (is_locked_item(sigitm));
  assert (is_locked_item(fitm));
  MOM_DEBUGPRINTF(gencod, "c-emitter declare_funheader_for start sigitm=%s fitm=%s",
                  mom_item_cstring(sigitm), mom_item_cstring(fitm));
  auto formtup = mom_dyncast_tuple(mom_unsync_item_get_phys_attr (sigitm, MOM_PREDEFITM(formals)));
  assert (formtup != nullptr);
  auto restyv = mom_unsync_item_get_phys_attr (sigitm, MOM_PREDEFITM(result));
  MOM_DEBUGPRINTF(gencod, "c-declare_funheader_for sigitm=%s formtup=%s restyv=%s",
                  mom_item_cstring(sigitm), mom_value_cstring(formtup), mom_value_cstring(restyv));
  int nbform = mom_raw_size(formtup);
  momvalue_t smallformdeclarr[8] = {nullptr};
  momvalue_t *formdeclarr =
    (nbform<(int)(sizeof(smallformdeclarr)/sizeof(smallformdeclarr[0])))
    ? smallformdeclarr
    : (momvalue_t*)mom_gc_alloc(nbform*sizeof(momvalue_t));
  for (int ix=0; ix<nbform; ix++)
    {
      momvalue_t curformdeclv = nullptr;
      struct mom_item_st*curformitm = formtup->seqitem[ix];
      MOM_DEBUGPRINTF(gencod, "c-declare_funheader_for sigitm=%s  ix#%d curformitm=%s",
                      mom_item_cstring(sigitm), ix, mom_item_cstring(curformitm));
      assert (is_locked_item(curformitm));
      struct mom_item_st*formtypitm =
      mom_dyncast_item(mom_unsync_item_get_phys_attr (curformitm, MOM_PREDEFITM(type)));
      MOM_DEBUGPRINTF(gencod, "c-declare_funheader_for curformitm=%s formtypitm=%s",
                      mom_item_cstring(curformitm),
                      mom_item_cstring(formtypitm));
      assert (is_locked_item(formtypitm));
      auto formtypnod = declare_type(formtypitm);
      MOM_DEBUGPRINTF(gencod, "c-declare_funheader_for formtypitm=%s formtypnod=%s",
                      mom_item_cstring(formtypitm), mom_value_cstring(formtypnod));
      curformdeclv = mom_boxnode_make_va(MOM_PREDEFITM(sequence),4,
                                         formtypnod, literal_string(" "),
                                         literal_string(CFORMAL_PREFIX),
                                         curformitm);
      MOM_DEBUGPRINTF(gencod, "c-declare_funheader_for ix#%d curformitm=%s curformdeclv=%s",
                      ix,
                      mom_item_cstring(curformitm), mom_value_cstring(curformdeclv));
      formdeclarr[ix] = curformdeclv;
    }
  for (int j=0; j<nbform; j++)
    MOM_DEBUGPRINTF(gencod, "formdeclarr[%d] :@%p %s", j, formdeclarr[j], mom_value_cstring(formdeclarr[j]));
  auto formtreev =  mom_boxnode_make_va(MOM_PREDEFITM(parenthesis),1,
                                        mom_boxnode_make(MOM_PREDEFITM(comma),nbform,formdeclarr));
  if (formdeclarr != smallformdeclarr) GC_FREE(formdeclarr);
  formdeclarr = nullptr;
  MOM_DEBUGPRINTF(gencod, "c-declare_funheader_for sigitm=%s formtreev=%s restyv=%s",
                  mom_item_cstring(sigitm), mom_value_cstring(formtreev), mom_value_cstring(restyv));
  momvalue_t restytree = nullptr;
  switch(mom_itype(restyv))
    {
    case MOMITY_ITEM:
      restytree = declare_type((struct mom_item_st*)restyv);
      break;
    default:
      throw MOM_RUNTIME_PRINTF("bad result type %s for signature %s function %s",
                               mom_value_cstring(restyv),
                               mom_item_cstring(sigitm),
                               mom_item_cstring(fitm));
    }
  MOM_DEBUGPRINTF(gencod, "c-declare_funheader_for sigitm=%s restyv=%s restytree=%s",
                  mom_item_cstring(sigitm), mom_value_cstring(restyv), mom_value_cstring(restytree));
  auto funheadv =  mom_boxnode_make_sentinel(MOM_PREDEFITM(sequence),
                   restytree,
                   literal_string(" "),
                   literal_string(MOM_FUNC_PREFIX),
                   fitm,
                   formtreev );
  MOM_DEBUGPRINTF(gencod, "c-declare_funheader_for sigitm=%s fitm=%s result funheadv=%s",
                  mom_item_cstring(sigitm),
                  mom_item_cstring(fitm),
                  mom_value_cstring(funheadv));
  return funheadv;
} // end MomCEmitter::declare_funheader_for


const struct mom_boxnode_st*
MomCEmitter::declare_signature_type (struct mom_item_st*sigitm)
{
  assert (is_locked_item(sigitm));
  MOM_DEBUGPRINTF(gencod, "c-declare_signature_type start sigitm=%s", mom_item_cstring(sigitm));
  auto formtup = mom_dyncast_tuple(mom_unsync_item_get_phys_attr (sigitm, MOM_PREDEFITM(formals)));
  assert (formtup != nullptr);
  auto restyitm = mom_dyncast_item(mom_unsync_item_get_phys_attr (sigitm, MOM_PREDEFITM(result)));
  MOM_DEBUGPRINTF(gencod, "c-declare_signature_type sigitm=%s formtup=%s restyitm=%s",
                  mom_item_cstring(sigitm), mom_value_cstring(formtup), mom_item_cstring(restyitm));
  int nbform = mom_raw_size(formtup);
  momvalue_t smallformtyparr[8] = {nullptr};
  momvalue_t *formtyparr =
    (nbform<(int)(sizeof(smallformtyparr)/sizeof(smallformtyparr[0])))
    ? smallformtyparr
    : (momvalue_t*)mom_gc_alloc(nbform*sizeof(momvalue_t));
  for (int ix=0; ix<nbform; ix++)
    {
      struct mom_item_st*curformitm = formtup->seqitem[ix];
      MOM_DEBUGPRINTF(gencod, "c-declare_signature_type sigitm=%s  ix#%d curformitm=%s",
                      mom_item_cstring(sigitm), ix, mom_item_cstring(curformitm));
      assert (is_locked_item(curformitm));
      struct mom_item_st*formtypitm =
      mom_dyncast_item(mom_unsync_item_get_phys_attr (curformitm, MOM_PREDEFITM(type)));
      MOM_DEBUGPRINTF(gencod, "c-declare_signature_type curformitm=%s formtypitm=%s",
                      mom_item_cstring(curformitm),
                      mom_item_cstring(formtypitm));
      assert (is_locked_item(formtypitm));
      auto formtypnod = declare_type(formtypitm);
      MOM_DEBUGPRINTF(gencod, "c-declare_signature_type ix#%d formtypnod=%s", ix, mom_value_cstring(formtypnod));
      formtyparr[ix] = formtypnod;
    }
  auto formtytree =  mom_boxnode_make_va(MOM_PREDEFITM(parenthesis),1,
                                         mom_boxnode_make(MOM_PREDEFITM(comma),nbform,formtyparr));
  if (formtyparr != smallformtyparr) GC_FREE(formtyparr);
  formtyparr = nullptr;
  MOM_DEBUGPRINTF(gencod, "c-declare_signature_type sigitm=%s formtytree=%s restyitm=%s",
                  mom_item_cstring(sigitm), mom_value_cstring(formtytree), mom_item_cstring(restyitm));
  assert (is_locked_item(restyitm));
  auto restytree = declare_type(restyitm);
  MOM_DEBUGPRINTF(gencod, "c-declare_signature_type sigitm=%s restytree=%s",
                  mom_item_cstring(sigitm), mom_value_cstring(restytree));
  auto sigdeclv = //
    mom_boxnode_make_sentinel(MOM_PREDEFITM(sequence),
                              literal_string("typedef "),
                              restytree,
                              literal_string(" "),
                              literal_string(CSIGNTYPE_PREFIX),
                              sigitm,
                              literal_string(" "),
                              formtytree,
                              literal_string(";")
                             );
  MOM_DEBUGPRINTF(gencod, "c-declare_signature_type sigitm=%s result sigheadv=%s",
                  mom_item_cstring(sigitm),
                  mom_value_cstring(sigdeclv));
  return sigdeclv;
} // end of MomCEmitter::declare_signature_type


momvalue_t
MomCEmitter::declare_type (struct mom_item_st*typitm)
{
  MOM_DEBUGPRINTF(gencod, "declare_type start typitm=%s",
                  mom_item_cstring(typitm));
  assert (is_locked_item(typitm));
  auto cextypv = mom_unsync_item_get_phys_attr (typitm, MOM_PREDEFITM(c_code));
  MOM_DEBUGPRINTF(gencod, "declare_type typitm=%s cextypv=%s",
                  mom_item_cstring(typitm), mom_value_cstring(cextypv));
  if (cextypv != nullptr)
    return cextypv;
#define NBKNOWNTYPE_MOM 31
#define CASE_KNOWNTYPE_MOM(Nam) momhashpredef_##Nam % NBKNOWNTYPE_MOM:	\
	  if (typitm == MOM_PREDEFITM(Nam)) goto foundcase_##Nam;	\
	  goto defaultcasetype; foundcase_##Nam
  switch (typitm->hva_hash % NBKNOWNTYPE_MOM)
    {
    case CASE_KNOWNTYPE_MOM (int):
      return literal_string(CINT_TYPE);
    case CASE_KNOWNTYPE_MOM (unit):
      return literal_string(CVOID_TYPE);
    case CASE_KNOWNTYPE_MOM (value):
      return literal_string(CVALUE_TYPE);
    case CASE_KNOWNTYPE_MOM (double):
      return literal_string(CDOUBLE_TYPE);
defaultcasetype:
    default:
      break;
    }
#undef NBKNOWNTYPE_MOM
#undef CASE_KNOWNTYPE_MOM
#warning MomCEmitter::declare_type partially unimplemented
  // should handle records, structs, unions, ...
  MOM_FATAPRINTF( "c-emitter declare_type partially unimplemented typitm=%s",
                  mom_item_cstring(typitm));
} // end of MomCEmitter::declare_type


const struct mom_boxnode_st*
MomCEmitter::transform_func_element(struct mom_item_st*fuitm)
{
  assert (is_locked_item(fuitm));
  MOM_DEBUGPRINTF(gencod, "c-emitter transform func start fuitm:=\n%s",
                  mom_item_content_cstring(fuitm));
  auto sigitm = mom_dyncast_item(mom_unsync_item_get_phys_attr (fuitm, MOM_PREDEFITM(signature)));
  auto bdyitm = mom_dyncast_item(mom_unsync_item_get_phys_attr (fuitm, MOM_PREDEFITM(body)));
  MOM_DEBUGPRINTF(gencod, "c-emitter transform func fuitm %s sigitm %s bdyitm %s",
                  mom_item_cstring(fuitm), mom_item_cstring(sigitm), mom_item_cstring(bdyitm));
  assert (is_locked_item(sigitm));
  assert (is_locked_item(bdyitm));
  auto funhnod = declare_funheader_for(sigitm,fuitm);
  MOM_DEBUGPRINTF(gencod, "c-emitter transform func fuitm %s sigitm %s funhnod %s",
                  mom_item_cstring(fuitm), mom_item_cstring(sigitm), mom_value_cstring(funhnod));
  if (_cec_declareditems.find(sigitm) == _cec_declareditems.end())
    {
      _cec_declareditems.insert(sigitm);
      auto sigtypnod = declare_signature_type(sigitm);
      MOM_DEBUGPRINTF(gencod, "c-emitter transform func sigitm %s sigtypnod %s",
                      mom_item_cstring(sigitm), mom_value_cstring(sigtypnod));
      add_global_decl(sigtypnod);
    }
  auto fundecl =
    mom_boxnode_make_sentinel(MOM_PREDEFITM(sequence),
                              literal_string(CSIGNTYPE_PREFIX),
                              sigitm,
                              literal_string(" "),
                              literal_string(MOM_FUNC_PREFIX),
                              fuitm,
                              literal_string(";"));
  add_global_decl(fundecl);
  MOM_DEBUGPRINTF(gencod, "c-emitter transform func fuitm %s fundecl %s", mom_item_cstring(fuitm),
                  mom_value_cstring(fundecl));
  auto sigdef =
    mom_boxnode_make_sentinel(MOM_PREDEFITM(sequence),
                              literal_string("const char "),
                              literal_string(MOM_SIGNATURE_PREFIX),
                              fuitm,
                              literal_string("[]= \""),
                              sigitm,
                              literal_string("\";"));
  add_global_decl(sigdef);
  MOM_DEBUGPRINTF(gencod, "c-emitter transform func fuitm %s sigdef %s bdyitm %s", mom_item_cstring(fuitm),
                  mom_value_cstring(sigdef), mom_item_cstring(bdyitm));
  auto bdynod = transform_body_element(bdyitm,fuitm);
  MOM_DEBUGPRINTF(gencod, "c-emitter transform func fuitm %s bdyitm %s bdynod %s\n ... funhnod=%s",
                  mom_item_cstring(fuitm), mom_item_cstring(bdyitm),
                  mom_value_cstring(bdynod), mom_value_cstring(funhnod));
  auto fundef = mom_boxnode_make_va(MOM_PREDEFITM(sequence),3,
                                    funhnod,
                                    literal_string("\n"),
                                    bdynod);
  MOM_DEBUGPRINTF(gencod, "c-emitter transform func fuitm %s result fundef=%s",  mom_item_cstring(fuitm),
                  mom_value_cstring(fundef));
  return fundef;
} // end MomCEmitter::transform_func_element


const struct mom_boxnode_st*
MomCEmitter::transform_body_element(struct mom_item_st*bdyitm, struct mom_item_st*routitm)
{
  MOM_DEBUGPRINTF(gencod, "c-transform_body_element bdyitm=%s routitm=%s",
                  mom_item_cstring(bdyitm), mom_item_cstring(routitm));
  MOM_DEBUGPRINTF(gencod, "c-transform_body_element bdyitm:=\n %s",
                  mom_item_content_cstring(bdyitm));
  auto bdytree = transform_block(bdyitm, routitm);
  MOM_DEBUGPRINTF(gencod, "c-transform_body_element bdyitm=%s bdytree=%s",
                  mom_item_cstring(bdyitm), mom_value_cstring(bdytree));
  return mom_dyncast_node(bdytree);
} // end of MomCEmitter::transform_body_element


momvalue_t
MomCEmitter::transform_block(struct mom_item_st*blkitm, struct mom_item_st*initm)
{
  MOM_DEBUGPRINTF(gencod, "c-transform_block initm=%s blkitm:=\n%s",
                  mom_item_cstring(initm), mom_item_content_cstring(blkitm));
  assert (is_locked_item(blkitm));
  auto blkbind = get_local_binding(blkitm);
  assert (blkbind != nullptr);
  MOM_DEBUGPRINTF(gencod,
                  "c-transform_block blkitm=%s blkbind rol %s what %s detail %s rank %ld",
                  mom_item_cstring(blkitm), mom_item_cstring(blkbind->vd_rolitm),
                  mom_value_cstring(blkbind->vd_what), mom_value_cstring(blkbind->vd_detail), blkbind->vd_rank);
  struct mom_item_st*rolitm = blkbind->vd_rolitm;
  assert (mom_itype(rolitm) == MOMITY_ITEM);
  auto bodytup =
    mom_dyncast_tuple(mom_unsync_item_get_phys_attr (blkitm, MOM_PREDEFITM(body)));
  MOM_DEBUGPRINTF(gencod,
                  "c-transform_block blkitm=%s bodytup=%s",
                  mom_item_cstring(blkitm), mom_value_cstring(bodytup));
#define NBBLOCKROLE_MOM 19
#define CASE_BLOCKROLE_MOM(Nam) momhashpredef_##Nam % NBBLOCKROLE_MOM:	\
	  if (rolitm == MOM_PREDEFITM(Nam)) goto foundcase_##Nam;	\
	  goto defaultcasebrole; foundcase_##Nam
  switch (rolitm->hva_hash % NBBLOCKROLE_MOM)
    {
    case CASE_BLOCKROLE_MOM (sequence):
    {
      auto localseq = mom_dyncast_seqitems(mom_unsync_item_get_phys_attr (blkitm, MOM_PREDEFITM(locals)));
      unsigned nblocals = mom_seqitems_length(localseq);
      MOM_DEBUGPRINTF(gencod,
                      "c-transform_block blkitm=%s nblocals#%d localseq:%s",
                      mom_item_cstring(blkitm), nblocals, mom_value_cstring(localseq));
      momvalue_t localtree= nullptr;
      if (nblocals>0)
        {
          momvalue_t smallocarr[8]= {};
          momvalue_t* locarr = (nblocals<sizeof(smallocarr)/sizeof(momvalue_t)) ? smallocarr
                               : (momvalue_t*) mom_gc_alloc(nblocals*sizeof(momvalue_t));
          for (int lix=0; lix<(int)nblocals; lix++)
            {
              momvalue_t curloctree = nullptr;
              struct mom_item_st*locitm = localseq->seqitem[lix];
              assert (is_locked_item(locitm));
              MOM_DEBUGPRINTF(gencod,
                              "c-transform_block blkitm=%s lix#%d locitm:=\n%s",
                              mom_item_cstring(blkitm), lix, mom_item_content_cstring(locitm));
              MOM_FATAPRINTF("c-transform_block blkitm=%s unhandled local lix#%d locitm:=\n%s",
                             mom_item_cstring(blkitm), lix, mom_item_content_cstring(locitm));
#warning MomCEmitter::transform_block unhandled local
            }
        }
      MOM_DEBUGPRINTF(gencod,
                      "c-transform_block blkitm=%s sequence of body %s localtree=%s",
                      mom_item_content_cstring(blkitm), mom_value_cstring(bodytup),
                      mom_value_cstring(localtree));
      int bodylen = mom_raw_size(bodytup);
      momvalue_t smalbodyarr[8]= {};
      momvalue_t* bodyarr = (bodylen<(int)sizeof(smalbodyarr)/sizeof(momvalue_t)) ? smalbodyarr
                            : (momvalue_t*) mom_gc_alloc(bodylen*sizeof(momvalue_t));
      for (int bix=0; bix<bodylen; bix++)
        {
          struct mom_item_st*insitm = bodytup->seqitem[bix];
          MOM_DEBUGPRINTF(gencod,
                          "c-transform_block blkitm=%s bix#%d insitm:=\n%s",
                          mom_item_cstring(blkitm), bix, mom_item_content_cstring(insitm));
          assert (is_locked_item(insitm));
          auto instree = transform_instruction(insitm, blkitm);
          MOM_DEBUGPRINTF(gencod,
                          "c-transform_block insitm=%s instree=%s",
                          mom_item_cstring(insitm), mom_value_cstring(instree));
          assert (instree != nullptr);
          bodyarr[bix] = instree;
        }
      auto bodytree = mom_boxnode_make(MOM_PREDEFITM(semicolon),bodylen,bodyarr);
      auto bracetree =
        mom_boxnode_make_va(MOM_PREDEFITM(brace),
                            4,
                            mom_boxnode_make_va(MOM_PREDEFITM(comment),3,
                                literal_string("block"),
                                literal_string(" "),
                                blkitm),
                            bodytree, literal_string(";"),
                            mom_boxnode_make_va(MOM_PREDEFITM(comment),3,
                                literal_string("endblock"),
                                literal_string(" "),
                                blkitm)
                           );
      MOM_DEBUGPRINTF(gencod,
                      "c-transform_block blkitm=%s gives bracetree=%s",
                      mom_item_cstring(blkitm), mom_value_cstring(bracetree));
      return bracetree;

    }
    break;
    default:
defaultcasebrole: // should never happen
      MOM_FATAPRINTF("unexpected role %s in blkitm %s",
                     mom_item_cstring(rolitm), mom_item_cstring(blkitm));
      break;
    }
#undef NBBLOCKROLE_MOM
#undef CASE_BLOCKROLE_MOM
#warning unimplemented MomCEmitter::transform_body_element
  MOM_FATAPRINTF("unimplemented MomCEmitter::transform_block blkitm=%s initm=%s",
                 mom_item_cstring(blkitm), mom_item_cstring(initm));
} // end of MomCEmitter::transform_block



momvalue_t
MomCEmitter::transform_node_expr(const struct mom_boxnode_st* expnod, struct mom_item_st*initm)
{
  MOM_DEBUGPRINTF(gencod, "c-transform_node_expr expnod=%s initm=%s",
                  mom_value_cstring(expnod), mom_item_cstring(initm));
#warning unimplemented MomCEmitter::transform_node_expr
  MOM_FATAPRINTF("unimplemented MomCEmitter::transform_node_expr expnod=%s initm=%s",
                 mom_value_cstring(expnod), mom_item_cstring(initm));
} // end MomCEmitter::transform_node_expr


momvalue_t
MomCEmitter::transform_expr(momvalue_t expv, struct mom_item_st*initm)
{
  MOM_DEBUGPRINTF(gencod, "c-transform_expr expv=%s initm=%s",
                  mom_value_cstring(expv), mom_item_cstring(initm));
  unsigned expty = mom_itype(expv);
  switch (expty)
    {
    case MOMITY_INT:
    case MOMITY_BOXDOUBLE:
      return expv;
    case MOMITY_BOXSTRING:
      return mom_boxnode_make_va(MOM_PREDEFITM(string),1,expv);
    case MOMITY_ITEM:
    {
      auto expitm = (struct mom_item_st*)expv;
      auto expbind = get_binding(expitm);
      auto rolitm = expbind?expbind->vd_rolitm:nullptr;
      MOM_DEBUGPRINTF(gencod, "c-transform_expr expitm=%s bind rol %s what %s",
                      mom_item_cstring(expitm), mom_item_cstring(rolitm),
                      expbind?mom_value_cstring(expbind->vd_what):"°");
#define NBROLE_MOM 31
#define CASE_ROLE_MOM(Nam) momhashpredef_##Nam % NBROLE_MOM:	\
 if (rolitm == MOM_PREDEFITM(Nam)) goto foundrolcase_##Nam;	\
 goto defaultrole; foundrolcase_##Nam
      switch (rolitm?rolitm->hva_hash % NBROLE_MOM : 0)
        {
        case CASE_ROLE_MOM(formal):
          return transform_var(expitm,initm,expbind);
        case CASE_ROLE_MOM(locals):
          return transform_var(expitm,initm,expbind);
        case CASE_ROLE_MOM(item):
        {
          MOM_DEBUGPRINTF(gencod,
                          "c-transform_expr expitm=%s constant",
                          mom_item_cstring(expitm));
          if (mom_item_space(expitm) == MOMSPA_PREDEF)
            {
              auto prnod =
                mom_boxnode_make_va(MOM_PREDEFITM(sequence),4,
                                    literal_string(CPREDEFITEM_MACRO),
                                    literal_string("("),
                                    expitm,
                                    literal_string(")"));
              MOM_DEBUGPRINTF(gencod,
                              "c-transform_expr expitm=%s gives prnod=%s",
                              mom_item_cstring(expitm), mom_value_cstring(prnod));
              return prnod;
            }
          else
            {
#warning transform_expr does not handle yet non-predefined items
              MOM_FATAPRINTF("transform_expr non-predefined expitm=%s initm=%s",
                             mom_item_cstring(expitm), mom_item_cstring(initm));
            }
        }
        break;
defaultrole:
        default:
          MOM_FATAPRINTF("transform_expr bad expitm=%s rolitm=%s initm=%s",
                         mom_item_cstring(expitm), mom_item_cstring(rolitm), mom_item_cstring(initm));
        }
#undef NBROLE_MOM
#undef CASE_ROLE_MOM
    }
    break;
    case MOMITY_NODE:
      return transform_node_expr((struct mom_boxnode_st*)expv, initm);
    default:
      MOM_FATAPRINTF("transform_expr bad expv=%s initm=%s",
                     mom_value_cstring(expv), mom_item_cstring(initm));

    }
#warning unimplemented MomCEmitter::transform_expr
  MOM_FATAPRINTF("unimplemented MomCEmitter::transform_expr expv=%s initm=%s",
                 mom_value_cstring(expv), mom_item_cstring(initm));
} // end of MomCEmitter::transform_expr


momvalue_t
MomCEmitter::transform_var(struct mom_item_st*varitm, struct mom_item_st*insitm, const vardef_st*varbind)
{
  if (!varbind)
    varbind = get_binding(varitm);
  if (MOM_UNLIKELY(!varbind)) // should never happen
    MOM_FATAPRINTF("c-transform_var varitm=%s in insitm %s lacking binding",
                   mom_item_cstring(varitm), mom_item_cstring(insitm));
  auto rolitm = varbind->vd_rolitm;
  MOM_DEBUGPRINTF(gencod, "c-transform_var varitm:=\n%s ... insitm=%s, rolitm=%s what=%s",
                  mom_item_content_cstring(varitm), mom_item_cstring(insitm),
                  mom_item_cstring(rolitm), mom_value_cstring(varbind->vd_what));
  momvalue_t vartree = nullptr;
#define NBROLE_MOM 31
#define CASE_ROLE_MOM(Nam) momhashpredef_##Nam % NBROLE_MOM:	\
 if (rolitm == MOM_PREDEFITM(Nam)) goto foundrolcase_##Nam;	\
 goto defaultrole; foundrolcase_##Nam
  switch (rolitm->hva_hash % NBROLE_MOM)
    {
    case CASE_ROLE_MOM(formal):
      vartree =
        mom_boxnode_make_va(MOM_PREDEFITM(sequence),2,
                            literal_string(CFORMAL_PREFIX),
                            varitm);
      break;
defaultrole:
    default:
      break;
    }
#undef NBROLE_MOM
#undef CASE_ROLE_MOM
  MOM_DEBUGPRINTF(gencod, "c-transform_var varitm=%s insitm=%s rolitm=%s vartree=%s",
                  mom_item_content_cstring(varitm), mom_item_cstring(insitm),
                  mom_item_cstring(rolitm), mom_value_cstring(vartree));
  if (vartree) return vartree;
#warning unimplemented MomCEmitter::transform_var
  MOM_FATAPRINTF("unimplemented MomCEmitter::transform_var varitm=%s insitm=%s rolitm=%s",
                 mom_item_cstring(varitm), mom_item_cstring(insitm), mom_item_cstring(rolitm));
} // end of MomCEmitter::transform_var

momvalue_t
MomCEmitter::transform_instruction(struct mom_item_st*insitm, struct mom_item_st*fromitm)
{
  MOM_DEBUGPRINTF(gencod, "c-transform_instruction fromitm=%s insitm:=\n%s",
                  mom_item_cstring(fromitm), mom_item_content_cstring(insitm));
  assert (is_locked_item(insitm));
  auto insbind = get_local_binding(insitm);
  assert (insbind != nullptr);
  MOM_DEBUGPRINTF(gencod,
                  "c-transform_instruction insitm=%s insbind rol %s what %s detail %s rank %ld",
                  mom_item_cstring(insitm), mom_item_cstring(insbind->vd_rolitm),
                  mom_value_cstring(insbind->vd_what), mom_value_cstring(insbind->vd_detail), insbind->vd_rank);
  struct mom_item_st*rolitm = insbind->vd_rolitm;
  assert (mom_itype(rolitm) == MOMITY_ITEM);
#define NBINSTROLE_MOM 73
#define CASE_INSTROLE_MOM(Nam) momhashpredef_##Nam % NBINSTROLE_MOM:	\
	  if (rolitm == MOM_PREDEFITM(Nam)) goto foundcase_##Nam;	\
	  goto defaultcaseirole; foundcase_##Nam
  switch (rolitm->hva_hash % NBINSTROLE_MOM)
    {
    case CASE_INSTROLE_MOM(run):
    {
      auto runitm = mom_dyncast_item(insbind->vd_what);
      assert (insbind != nullptr);
      MOM_DEBUGPRINTF(gencod,
                      "c-transform_instruction insitm=%s runitm:=\n%s",
                      mom_item_cstring(insitm), mom_item_content_cstring(runitm));
      assert (is_locked_item(runitm));
      return transform_runinstr(insitm, runitm, fromitm);
    }
    break;
    default:
defaultcaseirole:
      MOM_FATAPRINTF("unexpected role %s in insitm %s",
                     mom_item_cstring(rolitm), mom_item_cstring(insitm));
      break;
    }
#undef NBINSTROLE_MOM
#undef CASE_INSTROLE_MOM
#warning unimplemented MomCEmitter::transform_instruction
  MOM_FATAPRINTF("unimplemented c-transform_instruction insitm=%s", mom_item_cstring(insitm));
} // end of MomCEmitter::transform_instruction

momvalue_t
MomCEmitter::transform_runinstr(struct mom_item_st*insitm, struct mom_item_st*runitm, struct mom_item_st*fromitm)
{
  MOM_DEBUGPRINTF(gencod, "c-transform_runinstr fromitm=%s runitm:=\n%s insitm:=\n%s",
                  mom_item_cstring(fromitm), mom_item_content_cstring(runitm), mom_item_content_cstring(insitm));
  assert (is_locked_item(insitm));
  assert (is_locked_item(runitm));
  auto desrunitm = mom_unsync_item_descr(runitm);
  if (desrunitm == MOM_PREDEFITM(primitive))
    {
      auto sigitm = mom_dyncast_item(mom_unsync_item_get_phys_attr (runitm, MOM_PREDEFITM(signature)));
      MOM_DEBUGPRINTF(gencod, "c-transform_runinstr runitm=%s sigitm:=\n%s",
                      mom_item_cstring(runitm), mom_item_content_cstring(sigitm));
      assert (is_locked_item(sigitm));
      auto formaltup = mom_dyncast_tuple(mom_unsync_item_get_phys_attr (sigitm, MOM_PREDEFITM(formals)));
      assert (formaltup != nullptr);
      unsigned nbformals = mom_size(formaltup);
      traced_map_item2value_t argmap;
      auto inscomp = insitm->itm_pcomp;
      momvalue_t treev = nullptr;
      auto expnod = mom_dyncast_node(mom_unsync_item_get_phys_attr (runitm, MOM_PREDEFITM(c_expansion)));
      MOM_DEBUGPRINTF(gencod, "c-transform_runinstr formaltup=%s expnod=%s", mom_value_cstring(formaltup),
                      mom_value_cstring(expnod));
      if (!expnod || expnod->nod_connitm != MOM_PREDEFITM(code_chunk))
        MOM_FATAPRINTF("c-transform_runinstr insitm=%s runitm=%s bad expnod=%s",
                       mom_item_cstring(insitm), mom_item_cstring(runitm),
                       mom_value_cstring(expnod));
      for (int aix=0; aix<(int)nbformals; aix++)
        {
          auto curfitm = formaltup->seqitem[aix];
          auto curarg = mom_vectvaldata_nth(inscomp, aix);
          MOM_DEBUGPRINTF(gencod, "c-transform_runinstr aix#%d curfitm=%s curarg=%s", aix,
                          mom_item_cstring(curfitm), mom_value_cstring(curarg));
          assert (mom_itype(curfitm)==MOMITY_ITEM);
          auto argtree =  transform_expr(curarg, insitm);
          MOM_DEBUGPRINTF(gencod, "c-transform_runinstr insitm=%s aix#%d curarg=%s argtree=%s",
                          mom_item_cstring(fromitm), aix, mom_value_cstring(curarg), mom_value_cstring(argtree));
          argmap[curfitm] = argtree;
        }
      int exparity = mom_raw_size(expnod);
      momvalue_t smalltreearr[8]= {};
      momvalue_t* treearr = (exparity<sizeof(smalltreearr)/sizeof(momvalue_t)) ? smalltreearr
                            : (momvalue_t*) mom_gc_alloc(exparity*sizeof(momvalue_t));
      for (int ix=0; ix<exparity; ix++)
        {
          momvalue_t curtreev = nullptr;
          momvalue_t curson = expnod->nod_sons[ix];
          auto cursonitm = mom_dyncast_item(curson);
          if (cursonitm != nullptr)
            {
              auto it = argmap.find(cursonitm);
              if (it != argmap.end())
                curtreev = it->second;
              else
                curtreev = curson;
            }
          else
            curtreev = curson;
          treearr[ix] = curtreev;
        }
      treev = mom_boxnode_make(MOM_PREDEFITM(sequence),exparity,treearr);
      MOM_DEBUGPRINTF(gencod, "c-transform_runinstr insitm=%s treev=%s",
                      mom_item_cstring(insitm), mom_value_cstring(treev));
      return treev;
    }
#warning unimplemented MomCEmitter::transform_runinstr
  MOM_FATAPRINTF("unimplemented c-transform_runinstr insitm=%s", mom_item_cstring(insitm));
} // end of MomCEmitter::transform_runinstr

const struct mom_boxnode_st*
MomCEmitter::transform_routine_element(struct mom_item_st*itm)
{
#warning unimplemented MomCEmitter::transform_routine_element
  MOM_FATAPRINTF("unimplemented MomCEmitter::transform_routine_element itm=%s", mom_item_cstring(itm));
} // end MomCEmitter::transform_routine_element


MomEmitter::CaseScannerData*
MomEmitter::make_case_scanner_data(struct mom_item_st*swtypitm, struct mom_item_st*insitm, unsigned rk, struct mom_item_st*blkitm)
{
  assert (is_locked_item(swtypitm));
#define NBSWTYPE_MOM 43
#define CASE_SWTYPE_MOM(Nam) momhashpredef_##Nam % NBSWTYPE_MOM:	\
 if (swtypitm == MOM_PREDEFITM(Nam)) goto foundcaseswtyp_##Nam;	\
 goto defaultcaseswtyp; foundcaseswtyp_##Nam
  switch (swtypitm->hva_hash % NBSWTYPE_MOM)
    {
    case CASE_SWTYPE_MOM(int):
      return new IntCaseScannerData(this,swtypitm,insitm,blkitm,rk);
    case CASE_SWTYPE_MOM(string):
      return new StringCaseScannerData(this,swtypitm,insitm,blkitm,rk);
defaultcaseswtyp:
    default:
      throw  MOM_RUNTIME_PRINTF("switch instr %s rk#%d in block %s "
                                "with bad switch type %s",
                                mom_item_cstring(insitm), rk, mom_item_cstring(blkitm),
                                mom_item_cstring(swtypitm));
    }
#undef NBSWTYPE_MOM
#undef CASE_SWTYPE_MOM
} // end MomEmitter::make_case_scanner_data


MomEmitter::CaseScannerData*
MomCEmitter::make_case_scanner_data(struct mom_item_st*swtypitm, struct mom_item_st*insitm, unsigned rk, struct mom_item_st*blkitm)
{
  assert (is_locked_item(swtypitm));
  if (swtypitm == MOM_PREDEFITM(item))
    return new ItemCaseScannerData(this,swtypitm,insitm,blkitm,rk);
  else
    return MomEmitter::make_case_scanner_data(swtypitm,insitm,rk,blkitm);
} // end MomEmitter::make_case_scanner_data


std::function<void(struct mom_item_st*,unsigned,MomEmitter::CaseScannerData*)>
MomCEmitter::case_scanner(struct mom_item_st*swtypitm, struct mom_item_st*insitm, unsigned rk, struct mom_item_st*blkitm)
{
  assert (is_locked_item(swtypitm));
#define NBSWTYPE_MOM 43
#define CASE_SWTYPE_MOM(Nam) momhashpredef_##Nam % NBSWTYPE_MOM:	\
 if (swtypitm == MOM_PREDEFITM(Nam)) goto foundcaseswtyp_##Nam;	\
 goto defaultcaseswtyp; foundcaseswtyp_##Nam
  switch (swtypitm->hva_hash % NBSWTYPE_MOM)
    {
    case CASE_SWTYPE_MOM(int):
      return [=](struct mom_item_st*casitm,unsigned casix,MomEmitter::CaseScannerData*casdata)
      {
        assert (is_locked_item(casitm));
        auto runitm =
          mom_dyncast_item(mom_unsync_item_get_phys_attr(casitm,MOM_PREDEFITM(run)));
        auto casev =
          mom_unsync_item_get_phys_attr(casitm,MOM_PREDEFITM(case));
        MOM_DEBUGPRINTF(gencod, "C-case_scanner intcase %s casix %d casdata@%p (%s) insitm=%s runitm=%s casev=%s",
                        mom_item_cstring(casitm), casix, (void*)casdata, casdata->name(),
                        mom_item_cstring(insitm), mom_item_cstring(runitm), mom_value_cstring(casev));
        if (runitm==nullptr)
          throw  MOM_RUNTIME_PRINTF("intcase#%d %s  without `run` "
                                    "in switch instr %s #%d in block %s",
                                    casix, mom_item_cstring(casitm),
                                    mom_item_cstring(insitm),
                                    rk, mom_item_cstring(blkitm));
        if (casev==nullptr)
          throw  MOM_RUNTIME_PRINTF("intcase#%d %s  without `case` "
                                    "in switch instr %s #%d in block %s",
                                    casix, mom_item_cstring(casitm),
                                    mom_item_cstring(insitm),
                                    rk, mom_item_cstring(blkitm));
        auto intcasdata = dynamic_cast<IntCaseScannerData*>(casdata);
        assert (intcasdata != nullptr);
        if (intcasdata->has_runitm(runitm))
          throw  MOM_RUNTIME_PRINTF("intcase#%d %s with reused run %s "
                                    "in switch instr %s #%d in block %s",
                                    casix, mom_item_cstring(casitm),
                                    mom_item_cstring(runitm),
                                    mom_item_cstring(insitm),
                                    rk, mom_item_cstring(blkitm));
        intcasdata->process_intcase(casev,casitm,runitm);
        todo([=](MomEmitter*em)
        {
          MOM_DEBUGPRINTF(gencod,
                          "C-case_scanner before intcase#%d %s run %s "
                          "in switch instr %s #%d in block %s",
                          casix, mom_item_cstring(casitm),
                          mom_item_cstring(runitm),
                          mom_item_cstring(insitm),
                          rk, mom_item_cstring(blkitm));
          em->scan_instr(runitm,casix,insitm);
          MOM_DEBUGPRINTF(gencod,
                          "C-case_scanner after intcase#%d %s run %s "
                          "in switch instr %s #%d in block %s",
                          casix, mom_item_cstring(casitm),
                          mom_item_cstring(runitm),
                          mom_item_cstring(insitm),
                          rk, mom_item_cstring(blkitm));
        });
        intcasdata->add_runitm(runitm);
      };
    /////
    case CASE_SWTYPE_MOM(string):
      return [=](struct mom_item_st*casitm,unsigned casix,MomEmitter::CaseScannerData*casdata)
      {
        assert (is_locked_item(casitm));
        auto runitm =
          mom_dyncast_item(mom_unsync_item_get_phys_attr(casitm,MOM_PREDEFITM(run)));
        auto casev =
          mom_unsync_item_get_phys_attr(casitm,MOM_PREDEFITM(case));
        MOM_DEBUGPRINTF(gencod, "C-case_scanner stringcase %s casix %d casdata@%p (%s) insitm=%s runitm=%s casev=%s",
                        mom_item_cstring(casitm), casix, (void*)casdata, casdata->name(),
                        mom_item_cstring(insitm), mom_item_cstring(runitm), mom_value_cstring(casev));
        if (runitm==nullptr)
          throw  MOM_RUNTIME_PRINTF("stringcase#%d %s  without `run` "
                                    "in switch instr %s #%d in block %s",
                                    casix, mom_item_cstring(casitm),
                                    mom_item_cstring(insitm),
                                    rk, mom_item_cstring(blkitm));
        if (casev==nullptr)
          throw  MOM_RUNTIME_PRINTF("stringcase#%d %s  without `case` "
                                    "in switch instr %s #%d in block %s",
                                    casix, mom_item_cstring(casitm),
                                    mom_item_cstring(insitm),
                                    rk, mom_item_cstring(blkitm));
        auto strcasdata = dynamic_cast<StringCaseScannerData*>(casdata);
        assert (strcasdata != nullptr);
        if (strcasdata->has_runitm(runitm))
          throw  MOM_RUNTIME_PRINTF("stringcase#%d %s with reused run %s "
                                    "in switch instr %s #%d in block %s",
                                    casix, mom_item_cstring(casitm),
                                    mom_item_cstring(runitm),
                                    mom_item_cstring(insitm),
                                    rk, mom_item_cstring(blkitm));
        strcasdata->process_stringcase(casev,casitm,runitm);
        todo([=](MomEmitter*em)
        {
          MOM_DEBUGPRINTF(gencod,
                          "C-case_scanner before stringcase#%d %s run %s "
                          "in switch instr %s #%d in block %s",
                          casix, mom_item_cstring(casitm),
                          mom_item_cstring(runitm),
                          mom_item_cstring(insitm),
                          rk, mom_item_cstring(blkitm));
          em->scan_instr(runitm,casix,insitm);
          MOM_DEBUGPRINTF(gencod,
                          "C-case_scanner after stringcase#%d %s run %s "
                          "in switch instr %s #%d in block %s",
                          casix, mom_item_cstring(casitm),
                          mom_item_cstring(runitm),
                          mom_item_cstring(insitm),
                          rk, mom_item_cstring(blkitm));
        });
        strcasdata->add_runitm(runitm);
      };
    /////
    case CASE_SWTYPE_MOM(item):
      return [=](struct mom_item_st*casitm,unsigned casix,MomEmitter::CaseScannerData*casdata)
      {
        assert (is_locked_item(casitm));
        auto runitm =
          mom_dyncast_item(mom_unsync_item_get_phys_attr(casitm,MOM_PREDEFITM(run)));
        auto casev =
          mom_unsync_item_get_phys_attr(casitm,MOM_PREDEFITM(case));
        MOM_DEBUGPRINTF(gencod, "C-case_scanner stringcase %s casix %d casdata@%p (%s) insitm=%s runitm=%s casev=%s",
                        mom_item_cstring(casitm), casix, (void*)casdata, casdata->name(),
                        mom_item_cstring(insitm), mom_item_cstring(runitm), mom_value_cstring(casev));
        if (runitm==nullptr)
          throw  MOM_RUNTIME_PRINTF("stringcase#%d %s  without `run` "
                                    "in switch instr %s #%d in block %s",
                                    casix, mom_item_cstring(casitm),
                                    mom_item_cstring(insitm),
                                    rk, mom_item_cstring(blkitm));
        if (casev==nullptr)
          throw  MOM_RUNTIME_PRINTF("stringcase#%d %s  without `case` "
                                    "in switch instr %s #%d in block %s",
                                    casix, mom_item_cstring(casitm),
                                    mom_item_cstring(insitm),
                                    rk, mom_item_cstring(blkitm));
        auto itemcasdata = dynamic_cast<ItemCaseScannerData*>(casdata);
        assert (itemcasdata != nullptr);
        if (itemcasdata->has_runitm(runitm))
          throw  MOM_RUNTIME_PRINTF("stringcase#%d %s with reused run %s "
                                    "in switch instr %s #%d in block %s",
                                    casix, mom_item_cstring(casitm),
                                    mom_item_cstring(runitm),
                                    mom_item_cstring(insitm),
                                    rk, mom_item_cstring(blkitm));
        itemcasdata->process_itemcase(casev,casitm,runitm);
        todo([=](MomEmitter*em)
        {
          MOM_DEBUGPRINTF(gencod,
                          "C-case_scanner before stringcase#%d %s run %s "
                          "in switch instr %s #%d in block %s",
                          casix, mom_item_cstring(casitm),
                          mom_item_cstring(runitm),
                          mom_item_cstring(insitm),
                          rk, mom_item_cstring(blkitm));
          em->scan_instr(runitm,casix,insitm);
          MOM_DEBUGPRINTF(gencod,
                          "C-case_scanner after stringcase#%d %s run %s "
                          "in switch instr %s #%d in block %s",
                          casix, mom_item_cstring(casitm),
                          mom_item_cstring(runitm),
                          mom_item_cstring(insitm),
                          rk, mom_item_cstring(blkitm));
        });
        itemcasdata->add_runitm(runitm);
      };
defaultcaseswtyp:
    default:
      throw  MOM_RUNTIME_PRINTF("switch instr %s rk#%d in block %s "
                                "with bad switch type %s",
                                mom_item_cstring(insitm), rk, mom_item_cstring(blkitm),
                                mom_item_cstring(swtypitm));
    }
#undef NBSWTYPE_MOM
#undef CASE_SWTYPE_MOM
} // end of MomCEmitter::case_scanner


void
MomEmitter::IntCaseScannerData::process_intcase(const void*expv,
    struct mom_item_st*casitm,
    struct mom_item_st*runitm)
{
  const long constexpr maxrangewidth = 65536;
  const long constexpr maxsize = 16*maxrangewidth;
  unsigned expty = mom_itype(expv);
  assert (cas_emitter->is_locked_item(casitm));
  assert (cas_emitter->is_locked_item(runitm));
  if (cas_num2casemap.size() > maxsize)
    throw  MOM_RUNTIME_PRINTF("too many %ld int cases in case item %s"
                              " run item %s insitm %s #%d blkitm %s",
                              (long)cas_num2casemap.size(), mom_item_cstring(casitm),
                              mom_item_cstring(runitm),
                              mom_item_cstring(cas_insitm),
                              cas_rank, mom_item_cstring(cas_blkitm));

  if (expty == MOMITY_INT)
    {
      auto iv = mom_int_val_def(expv,0);
      if (cas_num2casemap.find(iv) != cas_num2casemap.end())
        throw  MOM_RUNTIME_PRINTF("duplicate int %ld case in case item %s"
                                  " run item %s insitm %s #%d blkitm %s",
                                  (long)iv, mom_item_cstring(casitm),
                                  mom_item_cstring(runitm),
                                  mom_item_cstring(cas_insitm),
                                  cas_rank, mom_item_cstring(cas_blkitm));
      cas_num2casemap[iv] = casitm;
      return;
    }
  else if (expty != MOMITY_NODE)
    throw  MOM_RUNTIME_PRINTF("non-node int case %s in case item %s"
                              " run item %s insitm %s #%d blkitm %s",
                              mom_value_cstring(expv), mom_item_cstring(casitm),
                              mom_item_cstring(runitm),
                              mom_item_cstring(cas_insitm),
                              cas_rank, mom_item_cstring(cas_blkitm));
  auto nodexp = (const struct mom_boxnode_st*)expv;
  auto connitm = nodexp->nod_connitm;
  unsigned arity = mom_raw_size(nodexp);
  assert (mom_itype(connitm) == MOMITY_ITEM);
  if (connitm==MOM_PREDEFITM(or))
    {
      for (unsigned ix=0; ix<arity; ix++)
        process_intcase(nodexp->nod_sons[ix],casitm,runitm);
    }
  else if (connitm==MOM_PREDEFITM(range))
    {
      const void*leftv=nullptr;
      const void* rightv=nullptr;
      long leftnum=0, rightnum=0;
      if (arity != 2
          || (leftv = nodexp->nod_sons[0]) == nullptr
          || mom_itype(leftv) != MOMITY_INT
          || (rightv = nodexp->nod_sons[1]) == nullptr
          || mom_itype(rightv) != MOMITY_INT
          || ((leftnum = mom_int_val_def(leftv,0))
              > (rightnum = mom_int_val_def(rightv,0))))
        throw  MOM_RUNTIME_PRINTF("bad range int case %s in case item %s"
                                  " run item %s insitm %s #%d blkitm %s",
                                  mom_value_cstring(expv), mom_item_cstring(casitm),
                                  mom_item_cstring(runitm),
                                  mom_item_cstring(cas_insitm),
                                  cas_rank, mom_item_cstring(cas_blkitm));
      if (leftnum + maxrangewidth < rightnum)
        throw  MOM_RUNTIME_PRINTF("too wide range int case %s in case item %s"
                                  " run item %s insitm %s #%d blkitm %s",
                                  mom_value_cstring(expv), mom_item_cstring(casitm),
                                  mom_item_cstring(runitm),
                                  mom_item_cstring(cas_insitm),
                                  cas_rank, mom_item_cstring(cas_blkitm));
      for (long ix=leftnum; ix<=rightnum; ix++)
        {
          if (cas_num2casemap.find(ix) != cas_num2casemap.end())
            throw  MOM_RUNTIME_PRINTF("duplicate int %ld case in case item %s"
                                      " run item %s insitm %s #%d blkitm %s",
                                      (long)ix, mom_item_cstring(casitm),
                                      mom_item_cstring(runitm),
                                      mom_item_cstring(cas_insitm),
                                      cas_rank, mom_item_cstring(cas_blkitm));
          cas_num2casemap[ix] = casitm;
        }
    }
  else
    throw MOM_RUNTIME_PRINTF("bad node int case %s in case item %s"
                             " run item %s insitm %s #%d blkitm %s",
                             mom_value_cstring(expv), mom_item_cstring(casitm),
                             mom_item_cstring(runitm),
                             mom_item_cstring(cas_insitm),
                             cas_rank, mom_item_cstring(cas_blkitm));
} // end MomEmitter::IntCaseScannerData::process_intcase



void
MomEmitter::StringCaseScannerData::process_stringcase(const void*expv,
    struct mom_item_st*casitm,
    struct mom_item_st*runitm)
{
  const long constexpr maxsize = 65536;
  unsigned expty = mom_itype(expv);
  assert (cas_emitter->is_locked_item(casitm));
  assert (cas_emitter->is_locked_item(runitm));
  if (cas_string2casemap.size() > maxsize)
    throw  MOM_RUNTIME_PRINTF("too many %ld string cases in case item %s"
                              " run item %s insitm %s #%d blkitm %s",
                              (long)cas_string2casemap.size(), mom_item_cstring(casitm),
                              mom_item_cstring(runitm),
                              mom_item_cstring(cas_insitm),
                              cas_rank, mom_item_cstring(cas_blkitm));

  if (expty == MOMITY_BOXSTRING)
    {
      std::string sv {mom_boxstring_cstr(expv)};
      if (cas_string2casemap.find(sv) != cas_string2casemap.end())
        throw  MOM_RUNTIME_PRINTF("duplicate string '%s' case in case item %s"
                                  " run item %s insitm %s #%d blkitm %s",
                                  sv.c_str(), mom_item_cstring(casitm),
                                  mom_item_cstring(runitm),
                                  mom_item_cstring(cas_insitm),
                                  cas_rank, mom_item_cstring(cas_blkitm));
      cas_string2casemap[sv] = casitm;
      return;
    }
  else if (expty != MOMITY_NODE)
    throw  MOM_RUNTIME_PRINTF("non-node string case %s in case item %s"
                              " run item %s insitm %s #%d blkitm %s",
                              mom_value_cstring(expv), mom_item_cstring(casitm),
                              mom_item_cstring(runitm),
                              mom_item_cstring(cas_insitm),
                              cas_rank, mom_item_cstring(cas_blkitm));
  auto nodexp = (const struct mom_boxnode_st*)expv;
  auto connitm = nodexp->nod_connitm;
  unsigned arity = mom_raw_size(nodexp);
  assert (mom_itype(connitm) == MOMITY_ITEM);
  if (connitm==MOM_PREDEFITM(or))
    {
      for (unsigned ix=0; ix<arity; ix++)
        process_stringcase(nodexp->nod_sons[ix],casitm,runitm);
    }
  else
    throw MOM_RUNTIME_PRINTF("bad node string case %s in case item %s"
                             " run item %s insitm %s #%d blkitm %s",
                             mom_value_cstring(expv), mom_item_cstring(casitm),
                             mom_item_cstring(runitm),
                             mom_item_cstring(cas_insitm),
                             cas_rank, mom_item_cstring(cas_blkitm));
} // end MomEmitter::StringCaseScannerData::process_stringcase



void
MomEmitter::ItemCaseScannerData::process_itemcase(const void*expv,
    struct mom_item_st*casitm,
    struct mom_item_st*runitm)
{
  const long constexpr maxsize = 65536;
  unsigned expty = mom_itype(expv);
  assert (cas_emitter->is_locked_item(casitm));
  assert (cas_emitter->is_locked_item(runitm));
  if (cas_item2casemap.size() > maxsize)
    throw  MOM_RUNTIME_PRINTF("too many %ld item cases in case item %s"
                              " run item %s insitm %s #%d blkitm %s",
                              (long)cas_item2casemap.size(), mom_item_cstring(casitm),
                              mom_item_cstring(runitm),
                              mom_item_cstring(cas_insitm),
                              cas_rank, mom_item_cstring(cas_blkitm));

  if (expty == MOMITY_ITEM)
    {
      auto itm = (struct mom_item_st*)expv;
      if (cas_item2casemap.find(itm) != cas_item2casemap.end())
        throw  MOM_RUNTIME_PRINTF("duplicate item %s case in case item %s"
                                  " run item %s insitm %s #%d blkitm %s",
                                  mom_item_cstring(itm), mom_item_cstring(casitm),
                                  mom_item_cstring(runitm),
                                  mom_item_cstring(cas_insitm),
                                  cas_rank, mom_item_cstring(cas_blkitm));
      cas_item2casemap[itm] = casitm;
      return;
    }
  else if (expty == MOMITY_SET)
    {
      unsigned card = mom_size(expv);
      auto set = (const struct mom_boxset_st*)expv;
      for (unsigned ix=0; ix<card; ix++)
        {
          auto itm = set->seqitem[ix];
          assert (mom_itype(itm) == MOMITY_ITEM);
          if (cas_item2casemap.find(itm) != cas_item2casemap.end())
            throw  MOM_RUNTIME_PRINTF("duplicate item %s case in case item %s"
                                      " run item %s insitm %s #%d blkitm %s",
                                      mom_item_cstring(itm), mom_item_cstring(casitm),
                                      mom_item_cstring(runitm),
                                      mom_item_cstring(cas_insitm),
                                      cas_rank, mom_item_cstring(cas_blkitm));
          cas_item2casemap[itm] = casitm;
        }
    }
  else if (expty != MOMITY_NODE)
    throw  MOM_RUNTIME_PRINTF("non-node item case %s in case item %s"
                              " run item %s insitm %s #%d blkitm %s",
                              mom_value_cstring(expv), mom_item_cstring(casitm),
                              mom_item_cstring(runitm),
                              mom_item_cstring(cas_insitm),
                              cas_rank, mom_item_cstring(cas_blkitm));
  auto nodexp = (const struct mom_boxnode_st*)expv;
  auto connitm = nodexp->nod_connitm;
  unsigned arity = mom_raw_size(nodexp);
  assert (mom_itype(connitm) == MOMITY_ITEM);
  if (connitm==MOM_PREDEFITM(or))
    {
      for (unsigned ix=0; ix<arity; ix++)
        process_itemcase(nodexp->nod_sons[ix],casitm,runitm);
    }
  else
    throw MOM_RUNTIME_PRINTF("bad node item case %s in case item %s"
                             " run item %s insitm %s #%d blkitm %s",
                             mom_value_cstring(expv), mom_item_cstring(casitm),
                             mom_item_cstring(runitm),
                             mom_item_cstring(cas_insitm),
                             cas_rank, mom_item_cstring(cas_blkitm));
} // end MomEmitter::ItemCaseScannerData::process_itemcase

const struct mom_boxnode_st*
MomJavascriptEmitter::transform_data_element(struct mom_item_st*itm)
{
#warning unimplemented MomJavascriptEmitter::transform_data_element
  MOM_FATAPRINTF("unimplemented MomJavascriptEmitter::transform_data_element itm=%s", mom_item_cstring(itm));
} // end MomJavascriptEmitter::transform_data_element



const struct mom_boxnode_st*
MomJavascriptEmitter::declare_funheader_for (struct mom_item_st*sigitm, struct mom_item_st*fitm)
{
  assert (is_locked_item(sigitm));
  assert (is_locked_item(fitm));
  MOM_DEBUGPRINTF(gencod, "JS-emitter declare_funheader_for start sigitm=%s fitm=%s",
                  mom_item_cstring(sigitm), mom_item_cstring(fitm));
  auto formtup = mom_dyncast_tuple(mom_unsync_item_get_phys_attr (sigitm, MOM_PREDEFITM(formals)));
  assert (formtup != nullptr);
  auto restyv = mom_unsync_item_get_phys_attr (sigitm, MOM_PREDEFITM(result));
  MOM_DEBUGPRINTF(gencod, "JS-declare_funheader_for sigitm=%s formtup=%s restyv=%s",
                  mom_item_cstring(sigitm), mom_value_cstring(formtup), mom_value_cstring(restyv));
  int nbform = mom_raw_size(formtup);
  momvalue_t smallformdeclarr[8] = {nullptr};
  momvalue_t *formdeclarr =
    (nbform<(int)(sizeof(smallformdeclarr)/sizeof(smallformdeclarr[0])))
    ? smallformdeclarr
    : (momvalue_t*)mom_gc_alloc(nbform*sizeof(momvalue_t));
  for (int ix=0; ix<nbform; ix++)
    {
      momvalue_t curformdeclv = nullptr;
      struct mom_item_st*curformitm = formtup->seqitem[ix];
      MOM_DEBUGPRINTF(gencod, "JS-declare_funheader_for sigitm=%s  ix#%d curformitm=%s",
                      mom_item_cstring(sigitm), ix, mom_item_cstring(curformitm));
      assert (is_locked_item(curformitm));
      formdeclarr[ix] = curformitm;
    }
  auto formtreev =  mom_boxnode_make_va(MOM_PREDEFITM(parenthesis),1,
                                        mom_boxnode_make(MOM_PREDEFITM(comma),nbform,formdeclarr));
  if (formdeclarr != smallformdeclarr) GC_FREE(formdeclarr);
  formdeclarr = nullptr;
  MOM_DEBUGPRINTF(gencod, "JS-declare_funheader_for sigitm=%s formtreev=%s",
                  mom_item_cstring(sigitm), mom_value_cstring(formtreev));
  auto funheadv =  mom_boxnode_make_sentinel(MOM_PREDEFITM(sequence),
                   literal_string("function"),
                   literal_string(" "),
                   literal_string(JSFUNC_PREFIX),
                   fitm,
                   formtreev);
  MOM_DEBUGPRINTF(gencod, "c-declare_funheader_for sigitm=%s fitm=%s result funheadv=%s",
                  mom_item_cstring(sigitm),
                  mom_item_cstring(fitm),
                  mom_value_cstring(funheadv));
  return funheadv;
} // end of MomJavaScriptEmitter::declare_funheader_for

const struct mom_boxnode_st*
MomJavascriptEmitter::transform_func_element(struct mom_item_st*fuitm)
{
  assert (is_locked_item(fuitm));
  MOM_DEBUGPRINTF(gencod, "JS-emitter transform func start fuitm:=\n%s",
                  mom_item_content_cstring(fuitm));
  auto sigitm = mom_dyncast_item(mom_unsync_item_get_phys_attr (fuitm, MOM_PREDEFITM(signature)));
  auto bdyitm = mom_dyncast_item(mom_unsync_item_get_phys_attr (fuitm, MOM_PREDEFITM(body)));
  MOM_DEBUGPRINTF(gencod, "JS-emitter transform func fuitm %s sigitm %s bdyitm %s",
                  mom_item_cstring(fuitm), mom_item_cstring(sigitm), mom_item_cstring(bdyitm));
  assert (is_locked_item(sigitm));
  assert (is_locked_item(bdyitm));
  auto funhnod = declare_funheader_for(sigitm,fuitm);
  MOM_DEBUGPRINTF(gencod, "JS-emitter transform func fuitm %s sigitm %s funhnod %s",
                  mom_item_cstring(fuitm), mom_item_cstring(sigitm), mom_value_cstring(funhnod));
  auto bdynod = transform_body_element(bdyitm,fuitm);
  MOM_DEBUGPRINTF(gencod, "JS-emitter transform func fuitm %s bdyitm %s bdynod %s\n ... funhnod=%s",
                  mom_item_cstring(fuitm), mom_item_cstring(bdyitm),
                  mom_value_cstring(bdynod), mom_value_cstring(funhnod));
#warning unimplemented MomJavascriptEmitter::transform_func_element
  MOM_FATAPRINTF("unimplemented MomJavascriptEmitter::transform_func_element fuitm=%s", mom_item_cstring(fuitm));
} // end MomJavascriptEmitter::transform_func_element


const struct mom_boxnode_st*
MomJavascriptEmitter::transform_body_element(struct mom_item_st*bdyitm, struct mom_item_st*routitm)
{
  MOM_DEBUGPRINTF(gencod, "js-transform_body_element bdyitm=%s routitm=%s",
                  mom_item_cstring(bdyitm), mom_item_cstring(routitm));
  MOM_DEBUGPRINTF(gencod, "js-transform_body_element bdyitm:=\n %s",
                  mom_item_content_cstring(bdyitm));
  auto bdytree = transform_block(bdyitm, routitm);
  MOM_DEBUGPRINTF(gencod, "js-transform_body_element bdyitm=%s bdytree=%s",
                  mom_item_cstring(bdyitm), mom_value_cstring(bdytree));
  return mom_dyncast_node(bdytree);
} // end of MomJavascriptEmitter::transform_body_element

momvalue_t
MomJavascriptEmitter::transform_block(struct mom_item_st*blkitm, struct mom_item_st*initm)
{
  MOM_DEBUGPRINTF(gencod, "js-transform_block initm=%s blkitm:=\n%s",
                  mom_item_cstring(initm), mom_item_content_cstring(blkitm));
  assert (is_locked_item(blkitm));
  auto blkbind = get_local_binding(blkitm);
  assert (blkbind != nullptr);
  MOM_DEBUGPRINTF(gencod,
                  "js-transform_block blkitm=%s blkbind rol %s what %s detail %s rank %ld",
                  mom_item_cstring(blkitm), mom_item_cstring(blkbind->vd_rolitm),
                  mom_value_cstring(blkbind->vd_what), mom_value_cstring(blkbind->vd_detail), blkbind->vd_rank);
  struct mom_item_st*rolitm = blkbind->vd_rolitm;
  assert (mom_itype(rolitm) == MOMITY_ITEM);
  auto bodytup =
    mom_dyncast_tuple(mom_unsync_item_get_phys_attr (blkitm, MOM_PREDEFITM(body)));
  MOM_DEBUGPRINTF(gencod,
                  "c-transform_block blkitm=%s bodytup=%s",
                  mom_item_cstring(blkitm), mom_value_cstring(bodytup));
#define NBBLOCKROLE_MOM 19
#define CASE_BLOCKROLE_MOM(Nam) momhashpredef_##Nam % NBBLOCKROLE_MOM:	\
	  if (rolitm == MOM_PREDEFITM(Nam)) goto foundcase_##Nam;	\
	  goto defaultcasebrole; foundcase_##Nam
  switch (rolitm->hva_hash % NBBLOCKROLE_MOM)
    {
    case CASE_BLOCKROLE_MOM (sequence):
    {
      auto localseq = mom_dyncast_seqitems(mom_unsync_item_get_phys_attr (blkitm, MOM_PREDEFITM(locals)));
      unsigned nblocals = mom_seqitems_length(localseq);
      MOM_DEBUGPRINTF(gencod,
                      "js-transform_block blkitm=%s nblocals#%d localseq:%s",
                      mom_item_cstring(blkitm), nblocals, mom_value_cstring(localseq));
      momvalue_t localtree= nullptr;
      if (nblocals>0)
        {
          momvalue_t smallocarr[8]= {};
          momvalue_t* locarr = (nblocals<sizeof(smallocarr)/sizeof(momvalue_t)) ? smallocarr
                               : (momvalue_t*) mom_gc_alloc(nblocals*sizeof(momvalue_t));
          for (int lix=0; lix<(int)nblocals; lix++)
            {
              momvalue_t curloctree = nullptr;
              struct mom_item_st*locitm = localseq->seqitem[lix];
              assert (is_locked_item(locitm));
              MOM_DEBUGPRINTF(gencod,
                              "js-transform_block blkitm=%s lix#%d locitm:=\n%s",
                              mom_item_cstring(blkitm), lix, mom_item_content_cstring(locitm));
              MOM_FATAPRINTF("js-transform_block blkitm=%s unhandled local lix#%d locitm:=\n%s",
                             mom_item_cstring(blkitm), lix, mom_item_content_cstring(locitm));
#warning MomCEmitter::transform_block unhandled local
            }
        }
      MOM_DEBUGPRINTF(gencod,
                      "js-transform_block blkitm=%s sequence of body %s localtree=%s",
                      mom_item_content_cstring(blkitm), mom_value_cstring(bodytup),
                      mom_value_cstring(localtree));
      int bodylen = mom_raw_size(bodytup);
      momvalue_t smalbodyarr[8]= {};
      momvalue_t* bodyarr = (bodylen<(int)sizeof(smalbodyarr)/sizeof(momvalue_t)) ? smalbodyarr
                            : (momvalue_t*) mom_gc_alloc(bodylen*sizeof(momvalue_t));
      for (int bix=0; bix<bodylen; bix++)
        {
          struct mom_item_st*insitm = bodytup->seqitem[bix];
          MOM_DEBUGPRINTF(gencod,
                          "js-transform_block blkitm=%s bix#%d insitm:=\n%s",
                          mom_item_cstring(blkitm), bix, mom_item_content_cstring(insitm));
          assert (is_locked_item(insitm));
          auto instree = transform_instruction(insitm, blkitm);
          MOM_DEBUGPRINTF(gencod,
                          "js-transform_block insitm=%s instree=%s",
                          mom_item_cstring(insitm), mom_value_cstring(instree));
          assert (instree != nullptr);
          bodyarr[bix] = instree;
        }
      auto bodytree = mom_boxnode_make(MOM_PREDEFITM(semicolon),bodylen,bodyarr);
      auto bracetree =
        mom_boxnode_make_va(MOM_PREDEFITM(brace),
                            4,
                            mom_boxnode_make_va(MOM_PREDEFITM(comment),3,
                                literal_string("block"),
                                literal_string(" "),
                                blkitm),
                            bodytree, literal_string(";"),
                            mom_boxnode_make_va(MOM_PREDEFITM(comment),3,
                                literal_string("endblock"),
                                literal_string(" "),
                                blkitm)
                           );
      MOM_DEBUGPRINTF(gencod,
                      "js-transform_block blkitm=%s gives bracetree=%s",
                      mom_item_cstring(blkitm), mom_value_cstring(bracetree));
      return bracetree;

    }
    break;
    default:
defaultcasebrole: // should never happen
      MOM_FATAPRINTF("unexpected role %s in blkitm %s",
                     mom_item_cstring(rolitm), mom_item_cstring(blkitm));
      break;
    }
#undef NBBLOCKROLE_MOM
#undef CASE_BLOCKROLE_MOM
#warning unimplemented MomJavascriptEmitter::transform_block
  MOM_FATAPRINTF("unimplemented MomJavascriptEmitter::transform_block blkitm=%s initm=%s",
                 mom_item_cstring(blkitm), mom_item_cstring(initm));
} // end of MomJavascriptEmitter::transform_block



momvalue_t
MomJavascriptEmitter::transform_instruction(struct mom_item_st*insitm, struct mom_item_st*fromitm)
{
  MOM_DEBUGPRINTF(gencod, "js-transform_instruction fromitm=%s insitm:=\n%s",
                  mom_item_cstring(fromitm), mom_item_content_cstring(insitm));
  assert (is_locked_item(insitm));
  auto insbind = get_local_binding(insitm);
  assert (insbind != nullptr);
  MOM_DEBUGPRINTF(gencod,
                  "js-transform_instruction insitm=%s insbind rol %s what %s detail %s rank %ld",
                  mom_item_cstring(insitm), mom_item_cstring(insbind->vd_rolitm),
                  mom_value_cstring(insbind->vd_what), mom_value_cstring(insbind->vd_detail), insbind->vd_rank);
  struct mom_item_st*rolitm = insbind->vd_rolitm;
  assert (mom_itype(rolitm) == MOMITY_ITEM);
#define NBINSTROLE_MOM 73
#define CASE_INSTROLE_MOM(Nam) momhashpredef_##Nam % NBINSTROLE_MOM:	\
	  if (rolitm == MOM_PREDEFITM(Nam)) goto foundcase_##Nam;	\
	  goto defaultcaseirole; foundcase_##Nam
  switch (rolitm->hva_hash % NBINSTROLE_MOM)
    {
    case CASE_INSTROLE_MOM(run):
    {
      auto runitm = mom_dyncast_item(insbind->vd_what);
      assert (is_locked_item(runitm));
      return transform_runinstr(insitm, runitm, fromitm);
    }

    break;
    default:
defaultcaseirole:
      MOM_FATAPRINTF("unexpected role %s in insitm %s",
                     mom_item_cstring(rolitm), mom_item_cstring(insitm));
      break;
    }
#undef NBINSTROLE_MOM
#undef CASE_INSTROLE_MOM
#warning unimplemented MomJavascriptEmitter::transform_instruction
  MOM_FATAPRINTF("unimplemented js-transform_instruction insitm=%s", mom_item_cstring(insitm));
} // end of MomJavascriptEmitter::transform_instruction

momvalue_t
MomJavascriptEmitter::transform_runinstr(struct mom_item_st*insitm, struct mom_item_st*runitm, struct mom_item_st*fromitm)
{
  MOM_DEBUGPRINTF(gencod, "js-transform_runinstr fromitm=%s runitm:=\n%s\n ..insitm:=\n%s",
                  mom_item_cstring(fromitm), mom_item_content_cstring(runitm), mom_item_content_cstring(insitm));
#warning unimplemented MomJavascriptEmitter::transform_runinstr
  MOM_FATAPRINTF("unimplemented js-transform_runinstr insitm=%s",
                 mom_item_cstring(insitm));
}






MomEmitter::CaseScannerData*
MomJavascriptEmitter::make_case_scanner_data(struct mom_item_st*swtypitm, struct mom_item_st*insitm, unsigned rk, struct mom_item_st*blkitm)
{
  if (swtypitm == MOM_PREDEFITM(item))
    throw   MOM_RUNTIME_PRINTF("JavaScript switch instr %s rk#%d in block %s "
                               "with bad switch type item",
                               mom_item_cstring(insitm), rk, mom_item_cstring(blkitm));
  return MomEmitter::make_case_scanner_data(swtypitm,insitm,rk,blkitm);
} // end MomJavascriptEmitter::make_case_scanner_data



std::function<void(struct mom_item_st*,unsigned,MomEmitter::CaseScannerData*)>
MomJavascriptEmitter::case_scanner(struct mom_item_st*swtypitm, struct mom_item_st*insitm, unsigned rk, struct mom_item_st*blkitm)
{
  assert (is_locked_item(swtypitm));
#define NBSWTYPE_MOM 43
#define CASE_SWTYPE_MOM(Nam) momhashpredef_##Nam % NBSWTYPE_MOM:	\
 if (swtypitm == MOM_PREDEFITM(Nam)) goto foundcaseswtyp_##Nam;	\
 goto defaultcaseswtyp; foundcaseswtyp_##Nam
  switch (swtypitm->hva_hash % NBSWTYPE_MOM)
    {
    case CASE_SWTYPE_MOM(int):
    case CASE_SWTYPE_MOM(string):
    case CASE_SWTYPE_MOM(item):
defaultcaseswtyp:
#warning incomplete MomJavascriptEmitter::case_scanner
    default:
      throw  MOM_RUNTIME_PRINTF("switch instr %s rk#%d in block %s "
                                "with bad switch type %s",
                                mom_item_cstring(insitm), rk, mom_item_cstring(blkitm),
                                mom_item_cstring(swtypitm));
    }
#undef NBSWTYPE_MOM
#undef CASE_SWTYPE_MOM
} // end of MomJavascriptEmitter::case_scanner

MomJavascriptEmitter::~MomJavascriptEmitter()
{
  MOM_DEBUGPRINTF(gencod, "end %s for this@%p", kindname(), this);
} // end MomJavascriptEmitter::~MomJavascriptEmitter

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
  typedef std::vector<momvalue_t,
		      traceable_allocator<momvalue_t> >
  traced_vector_values_t;
  typedef std::vector<struct mom_item_st*,
		      traceable_allocator<struct mom_item_st*> >
  traced_vector_items_t;
  typedef std::set<const struct mom_item_st*,
		   MomItemLess,
		   traceable_allocator<struct mom_item_st*>>
  traced_set_items_t;
  typedef std::map<const struct mom_item_st*,struct mom_item_st*,
		   MomItemLess,
		   traceable_allocator<std::pair<struct mom_item_st*,struct mom_item_st*>>>
  traced_map_item2item_t;
  typedef std::map<const struct mom_item_st*,long,
		   MomItemLess,
		   traceable_allocator<std::pair<struct mom_item_st*,long>>>
  traced_map_item2long_t;
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
  bool _ce_failing;
  struct mom_item_st* _ce_topitm;
  std::vector<struct mom_item_st*,traceable_allocator<struct mom_item_st*>> _ce_veclockeditems;
  traced_set_items_t _ce_setlockeditems;
protected:
  traced_set_items_t _ce_sigitems;
  traced_set_items_t _ce_typitems;
  traced_set_items_t _ce_blockitems;
  traced_set_items_t _ce_instritems;
  traced_set_items_t _ce_constitems;
  traced_map_item2long_t _ce_breakcountmap;
  traced_map_item2long_t _ce_continuecountmap;
  std::map<std::string,momvalue_t,
	   std::less<std::string>,
	   traceable_allocator<std::pair<std::string,momvalue_t>>> _ce_literalstringmap;
  std::deque<todofun_t,traceable_allocator<todofun_t>> _ce_todoque;
  /* postpone something, but do it between module elements, e.g. because it might make some bindings, etc... */
  std::deque<todofun_t,traceable_allocator<todofun_t>> _ce_todoafterelementque;
  /* perhaps we want to modify some items after a successful
     compilation, e.g. put ranks of fields inside them, etc.. */
  std::deque<todofun_t,traceable_allocator<todofun_t>> _ce_doatendque;
  traced_varmap_t _ce_globalvarmap;
  traced_varmap_t _ce_localvarmap;
  traced_set_values_t _ce_localvalueset;
  traced_set_items_t _ce_localcloseditems;
  traced_node2itemhashmap_t _ce_localnodetypecache;
  struct mom_item_st*_ce_curfunctionitm;
  struct sigdef_st _ce_cursigdef;
  struct mom_item_st* _ce_emitteritm;
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
  };        // end class CaseScannerData
  class IntCaseScannerData  final : public CaseScannerData
  {
    std::map<long,struct mom_item_st*,std::less<long>,traceable_allocator<long>> cas_num2casemap;
  public:
    void process_intcase(const void*expv, struct mom_item_st*casitm, struct mom_item_st*runitm);
    momvalue_t c_transform_intcase(class MomCEmitter*cem, const void*expv, struct mom_item_st*casitm, struct mom_item_st*runitm);
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
  };        // end class IntCaseScannerData
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
  };        // end class StringCaseScannerData
  class ItemCaseScannerData final : public CaseScannerData
  {
    traced_map_item2item_t cas_item2casemap;
  public:
    const char*name() const
    {
      return "ItemCaseScannerData";
    };
    void process_itemcase(const void*expv, struct mom_item_st*casitm, struct mom_item_st*runitm);
    void each_item_case(std::function<void(struct mom_item_st*, struct mom_item_st*)> fun,
                        const char*nam="?", int lineno=0)
    {
      for (auto it : cas_item2casemap)
        {
          struct mom_item_st*keyitm = const_cast<struct mom_item_st*>(it.first);
          struct mom_item_st*casitm = it.second;
          MOM_DEBUGPRINTF(gencod, "each_item_case keyitm=%s casitm=%s nam=%s lin#%d",
                          mom_item_cstring(keyitm),
                          mom_item_cstring(casitm),
                          nam,
                          lineno);
          fun(keyitm,casitm);
        }
    }
    size_t nb_item_cases (void) const
    {
      return cas_item2casemap.size();
    };
    struct mom_item_st*case_for_item(struct mom_item_st*ditm) const
    {
      struct mom_item_st*casitm = nullptr;
      if (mom_itype(ditm)==MOMITY_ITEM)
        {
          auto it = cas_item2casemap.find(ditm);
          if (it != cas_item2casemap.end())
            casitm = it->second;
        };
      return casitm;
    };
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
  bool is_type_binding(const struct vardef_st*vd)
  {
    return vd !=nullptr
      && (vd->vd_rolitm == MOM_PREDEFITM(type)
	  || vd->vd_rolitm == MOM_PREDEFITM(struct)
	  || vd->vd_rolitm == MOM_PREDEFITM(union)
	  || vd->vd_rolitm == MOM_PREDEFITM(enum));
  };
  void set_emitter_item(struct mom_item_st*emititm)
  {
    if (!emititm || emititm==MOM_EMPTY_SLOT)
      {
        _ce_emitteritm = nullptr;
        return;
      }
    assert (mom_itype(emititm)==MOMITY_ITEM);
    _ce_emitteritm = emititm;
  };
  struct mom_item_st*emitter_item(void) const
  {
    return _ce_emitteritm;
  };
public:
  void failing(void)
  {
    _ce_failing = true;
  };
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
  virtual void scan_routine_element(struct mom_item_st*itm, bool ignorebody=false);
  virtual const struct mom_boxnode_st* transform_data_element(struct mom_item_st*itm) =0;
  virtual const struct mom_boxnode_st* transform_body_element(struct mom_item_st*bdyitm, struct mom_item_st*routitm) =0;
  virtual const struct mom_boxnode_st* transform_func_element(struct mom_item_st*itm) =0;
  virtual const struct mom_boxnode_st* transform_routine_element(struct mom_item_st*itm) =0;
  virtual const struct mom_boxnode_st* transform_other_element(struct mom_item_st*itm, struct mom_item_st*descitm) =0;
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
  void scan_type_item(struct mom_item_st*typitm);
  void scan_type_expr(momvalue_t tyval, struct mom_item_st*initm=nullptr);
  struct mom_item_st*top_item(void) const
  {
    return _ce_topitm;
  };
  struct mom_item_st*current_function(void) const
  {
    return _ce_curfunctionitm;
  };
  // scanning of expressions & variables return their type item
  // when lvar is true, we require a location
  struct mom_item_st* scan_expr(const void*expv, struct mom_item_st*insitm, int depth, struct mom_item_st*typitm=nullptr, bool lvar=false);
  struct mom_item_st* scan_node_expr(const struct mom_boxnode_st*expnod, struct mom_item_st*insitm, int depth, struct mom_item_st*typitm=nullptr, bool lvar=false);
  struct mom_item_st* scan_node_descr_conn_expr(const struct mom_boxnode_st*expnod,
						struct mom_item_st*desconnitm,
						struct mom_item_st*insitm,
						int depth, struct mom_item_st*typitm=nullptr,
						bool lvar=false);
  struct mom_item_st* scan_item_expr(struct mom_item_st*expitm, struct mom_item_st*insitm, int depth, struct mom_item_st*typitm=nullptr, bool lvar=false);
  struct mom_item_st* scan_var(struct mom_item_st*varitm, struct mom_item_st*insitm, struct mom_item_st*typitm=nullptr);
  struct mom_item_st* scan_constant_item(struct mom_item_st*constitm, struct mom_item_st*insitm, struct mom_item_st*typitm=nullptr);
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
  void bind_global_at(const struct mom_item_st*itm, const vardef_st& vd, int lin)
  {
    if (mom_itype(itm) != MOMITY_ITEM)
      throw MOM_RUNTIME_PRINTF("global binding non-item lin#%d", lin);
    if (vd.vd_rolitm == MOM_PREDEFITM(data))
      MOM_DEBUGPRINTF(gencod, "global binding %s to data @%p rank#%ld lin#%d",
                      mom_item_cstring(itm), vd.vd_what, vd.vd_rank, lin);

    else
      MOM_DEBUGPRINTF(gencod, "global binding %s to role %s what %s detail %s rank#%ld lin#%d",
                      mom_item_cstring(itm), mom_item_cstring(vd.vd_rolitm),
                      mom_value_cstring(vd.vd_what),
                      mom_value_cstring(vd.vd_detail),
                      vd.vd_rank, lin);
    _ce_globalvarmap[itm] = vd;
  }
  void bind_global_at(const struct mom_item_st*itm, struct mom_item_st*rolitm, const void*what, int lin,
                      const void*detail=nullptr, long rank=0)
  {
    bind_global_at(itm,vardef_st {rolitm,what,detail,rank},lin);
  }
  void bind_local_at(const struct mom_item_st*itm, const vardef_st& vd,int lin)
  {
    if (mom_itype(itm) != MOMITY_ITEM)
      throw MOM_RUNTIME_PRINTF("local binding non-item lin#%d", lin);
    if (vd.vd_rolitm == MOM_PREDEFITM(data))
      MOM_DEBUGPRINTF(gencod, "local binding %s to data @%p rank#%ld lin#%d",
                      mom_item_cstring(itm), vd.vd_what, vd.vd_rank, lin);

    else
      MOM_DEBUGPRINTF(gencod, "local binding %s to role %s what %s detail %s rank#%ld lin#%d",
                      mom_item_cstring(itm), mom_item_cstring(vd.vd_rolitm),
                      mom_value_cstring(vd.vd_what),
                      mom_value_cstring(vd.vd_detail),
                      vd.vd_rank, lin);
    _ce_localvarmap[itm] = vd;
  }
  void bind_local_at(const struct mom_item_st*itm, struct mom_item_st*rolitm, const void*what, int lin, const void*detail=nullptr, long rank=0)
  {
    bind_local_at(itm,vardef_st {rolitm,what,detail,rank}, lin);
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
  virtual void after_preparation_transform(void) {};
  void scan_module_element(struct mom_item_st*itm);
  const struct mom_boxnode_st*transform_module_element(struct mom_item_st*itm);
  void todo(const todofun_t& tf)
  {
    _ce_todoque.push_back(tf);
  };
  void todo_after_element(const todofun_t& tf)
  {
    _ce_todoafterelementque.push_back(tf);
  };
  void do_at_end(const todofun_t& tf)
  {
    _ce_doatendque.push_back(tf);
  }
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
  void flush_todo_after_element_list(int lin=0)
  {
    long count = 0;
    if (lin)
      MOM_DEBUGPRINTF_AT(__FILE__,lin,gencod,"flush_todo_after_element_list %s with %ld todos",
                         kindname(), nb_todos());
    else
      MOM_DEBUGPRINTF(gencod,"flush_todo_after_element_list %s with %ld todos",
                      kindname(), nb_todos());
    while (!_ce_todoafterelementque.empty())
      {
        auto tf = _ce_todoafterelementque.front();
        _ce_todoafterelementque.pop_front();
        tf(this);
        count++;
      }
    _ce_todoafterelementque.shrink_to_fit();
    if (lin)
      MOM_DEBUGPRINTF_AT(__FILE__,lin,gencod,"flush_todo_after_element_list %s done %ld todos",
                         kindname(), count);
    else
      MOM_DEBUGPRINTF(gencod,"flush_todo_after_list %s done %ld todos",
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
};        // end of MomEmitter

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
  friend bool mom_emit_c_code(struct mom_item_st*itm);
  friend bool mom_emit_header_code(struct mom_item_st*itm);
  traced_vector_values_t _cec_globdecltree;
  traced_vector_values_t _cec_globdefintree;
  traced_set_items_t _cec_declareditems;
  bool _cec_includeheader;
public:
  static const unsigned constexpr MAGIC = 508723037 /*0x1e527f5d*/;
  static constexpr const char* CTYPE_PREFIX = "momty_";
  static constexpr const char* CONSTMAC_PREFIX = "MOMK_";
  static constexpr const char* CSTRUCT_PREFIX = "momstruct_";
  static constexpr const char* CUNION_PREFIX = "momunion_";
  static constexpr const char* CENUM_PREFIX = "momenum_";
  static constexpr const char* CENUVAL_PREFIX = "momenuva_";
  static constexpr const char* CENUVALEXT_PREFIX = "momenuvx_";
  static constexpr const char* CENUFROM_INFIX = "__momenfr__";
  static constexpr const char* CLOCAL_PREFIX = "momloc_";
  static constexpr const char* CFORMAL_PREFIX = "momarg_";
  static constexpr const char* CFIELD_PREFIX = "momfi_";
  static constexpr const char* CDATA_PREFIX = "momda_";
  static constexpr const char* CSIGNTYPE_PREFIX = "momsigty_";
  static constexpr const char* CPREDEFITEM_MACRO = "MOM_PREDEFITM";
  static constexpr const char* CCONSTITEM_PREFIX = "momcstitem_";
  static constexpr const char* CBREAKLAB_PREFIX = "mombreaklab_";
  static constexpr const char* CCONTINUELAB_PREFIX = "momcontilab_";
  static constexpr const char* COTHERWISELAB_PREFIX = "momotherwiselab_";
  static constexpr const char* CCASELAB_PREFIX = "momcaselab_";
  static constexpr const char* CENDCASELAB_PREFIX = "momendcaselab_";
  static constexpr const char* CITEMSW_PREFIX = "momitemsw_";
  static constexpr const char* CINT_TYPE = "momint_t";
  static constexpr const char* CVALUE_TYPE = "momvalue_t";
  static constexpr const char* CDOUBLE_TYPE = "double";
  static constexpr const char* CVOID_TYPE = "void";
  MomCEmitter(struct mom_item_st*itm, bool includeh=true)
    : MomEmitter(MAGIC, itm), _cec_globdecltree(), _cec_declareditems(), _cec_includeheader(includeh)
  {
    auto emititm = mom_clone_item(itm);
    lock_item(emititm);
    mom_unsync_item_put_phys_attr(emititm, MOM_PREDEFITM(descr), MOM_PREDEFITM(c_expansion));
    mom_unsync_item_put_phys_attr(emititm, MOM_PREDEFITM(from), itm);
    set_emitter_item(emititm);
    MOM_DEBUGPRINTF(gencod, "start c-emitter itm=%s emitteritm:=%s",
                    mom_item_cstring(itm), mom_item_content_cstring(emititm));
  };
  MomCEmitter(const MomCEmitter&) = delete;
  virtual ~MomCEmitter();
  void declare_item(struct mom_item_st*declitm);
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
  momvalue_t declare_type (struct mom_item_st*typitm, bool*scalarp=nullptr);
  // declare a field, and bind it if fromitm is non-null
  momvalue_t declare_field (struct mom_item_st*flditm, struct mom_item_st*fromitm, int rank);
  momvalue_t declare_field_unbound(struct mom_item_st*flditm)
  {
    return declare_field(flditm,nullptr,0);
  };
  // declare a struct member, either a field or a union of fields
  momvalue_t declare_struct_member(struct mom_item_st*memitm, struct mom_item_st*fromitm, int rank);
  momvalue_t declare_struct_member_unbound(struct mom_item_st*memitm)
  {
    return declare_struct_member(memitm,nullptr,0);
  };
  // declare an enumerator and bind it if fromitm is non-null; return a tree ending with a comma
  momvalue_t declare_enumerator(struct mom_item_st*enuritm,  struct mom_item_st*fromitm, int rank, int&preval,
                                struct mom_item_st* initm=nullptr);
  const struct mom_boxnode_st* declare_funheader_for (struct mom_item_st*sigitm, struct mom_item_st*fitm);
  const struct mom_boxnode_st* declare_signature_type (struct mom_item_st*sigitm);
  virtual void after_preparation_transform(void);
  virtual const struct mom_boxnode_st* transform_data_element(struct mom_item_st*itm);
  virtual const struct mom_boxnode_st* transform_func_element(struct mom_item_st*itm);
  virtual const struct mom_boxnode_st* transform_body_element(struct mom_item_st*bdyitm, struct mom_item_st*routitm);
  virtual const struct mom_boxnode_st* transform_other_element(struct mom_item_st*itm, struct mom_item_st*descitm);
  momvalue_t transform_block(struct mom_item_st*blkitm, struct mom_item_st*initm);
  momvalue_t transform_instruction(struct mom_item_st*insitm, struct mom_item_st*insideitm);
  momvalue_t transform_runinstr(struct mom_item_st*insitm, struct mom_item_st*runitm, struct mom_item_st*insideitm);
  momvalue_t transform_switchinstr(struct mom_item_st*insitm, momvalue_t whatv, struct mom_item_st*fromitm);
  momvalue_t transform_node_expr(const struct mom_boxnode_st*expnod, struct mom_item_st*insitm);
  momvalue_t transform_node_primitive_expr(const struct mom_boxnode_st*expnod, struct mom_item_st*insitm);
  momvalue_t transform_node_inline_expr(const struct mom_boxnode_st*expnod, struct mom_item_st*insitm);
  momvalue_t transform_node_cast_expr(const struct mom_boxnode_st*expnod, struct mom_item_st*insitm);
  momvalue_t transform_node_field_expr(const struct mom_boxnode_st*expnod, struct mom_item_st*insitm);
  momvalue_t transform_expr(momvalue_t expv, struct mom_item_st*insitm, struct mom_item_st*typitm=nullptr);
  momvalue_t transform_type_for(momvalue_t typexpv, momvalue_t vartree, bool*scalarp= nullptr);
  momvalue_t transform_constant_item(struct mom_item_st*cstitm, struct mom_item_st*insitm);
  momvalue_t transform_var(struct mom_item_st*varitm, struct mom_item_st*insitm, const vardef_st*varbind=nullptr);
  virtual const struct mom_boxnode_st* transform_routine_element(struct mom_item_st*elitm);
  const struct mom_boxnode_st* declare_header_for (struct mom_item_st*sigitm, struct mom_item_st*ilttm, bool inlined);
  const struct mom_boxnode_st* declare_inlineheader_for (struct mom_item_st*sigitm, struct mom_item_st*ilitm)
  {
    return declare_header_for(sigitm,ilitm,true);
  };
  const struct mom_boxnode_st* declare_routheader_for (struct mom_item_st*sigitm, struct mom_item_st*rtitm)
  {
    return declare_header_for(sigitm,rtitm,false);
  };
  const struct mom_boxnode_st* transform_inline_element(struct mom_item_st*elitm);
  CaseScannerData* make_case_scanner_data(struct mom_item_st*swtypitm, struct mom_item_st*insitm, unsigned rk, struct mom_item_st*blkitm);
  virtual std::function<void(struct mom_item_st*,unsigned,CaseScannerData*)> case_scanner(struct mom_item_st*swtypitm, struct mom_item_st*insitm, unsigned rk, struct mom_item_st*blkitm);
  virtual const char*kindname() const
  {
    return "C-emitter";
  };
};        // end class MomCEmitter


////////////////
class MomJavascriptEmitter final : public MomEmitter
{
  friend bool mom_emit_javascript_code(struct mom_item_st*itm, FILE*out);
public:
  static unsigned constexpr MAGIC = 852389659 /*0x32ce6f1b*/;
  static   constexpr const char* JSFUNC_PREFIX = "momjs_";
  static   constexpr const char* JSFORMAL_PREFIX = "momjarg_";
  static   constexpr const char* JSLOCAL_PREFIX = "momjloc_";
  static   constexpr const char* JSLABEL_PREFIX = "momjlab_";
  MomJavascriptEmitter(struct mom_item_st*itm) : MomEmitter(MAGIC, itm) {};
  MomJavascriptEmitter(const MomJavascriptEmitter&) = delete;
  virtual ~MomJavascriptEmitter();
  virtual void scan_routine_element(struct mom_item_st*elitm, bool ignorebody)
  {
    throw MOM_RUNTIME_PRINTF("routine element %s unsupported for JavaScript", mom_item_cstring(elitm));
  };
  virtual const struct mom_boxnode_st* transform_data_element(struct mom_item_st*itm);
  virtual const struct mom_boxnode_st* transform_func_element(struct mom_item_st*itm);
  virtual const struct mom_boxnode_st* transform_body_element(struct mom_item_st*bdyitm, struct mom_item_st*routitm);
  virtual const struct mom_boxnode_st* transform_other_element(struct mom_item_st*itm, struct mom_item_st*descitm)
  {
    MOM_DEBUGPRINTF(gencod, "js-transform_other_element itm=%s descitm=%s",
                    mom_item_cstring(itm),
                    mom_item_cstring(descitm));
    return nullptr;
  }
  momvalue_t transform_instruction(struct mom_item_st*insitm, struct mom_item_st*fromitm);
  const struct mom_boxnode_st* declare_funheader_for (struct mom_item_st*sigitm, struct mom_item_st*fitm);
  momvalue_t transform_block(struct mom_item_st*blkitm, struct mom_item_st*initm);
  momvalue_t transform_runinstr(struct mom_item_st*insitm, struct mom_item_st*runitm, struct mom_item_st*insideitm);
  momvalue_t transform_node_expr(const struct mom_boxnode_st*expnod, struct mom_item_st*insitm);
  momvalue_t transform_expr(momvalue_t expv, struct mom_item_st*insitm);
  momvalue_t transform_var(struct mom_item_st*varitm, struct mom_item_st*insitm, const vardef_st*varbind=nullptr);
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
};/// end class MomJavascriptEmitter



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
      fprintf(fout, "\n///////@@@BODY %s\n", basename(gpath));
      if (nbdecl>0)
        {
          fprintf(fout, "\n\n/// %d declarations:\n", nbdecl);
          for (int i=0; i<nbdecl; i++)
            {
              fputc('\n', fout);
              long nl = ftell(fout);
              MOM_DEBUGPRINTF(gencod, "declaration #%d: %s",
                              i, mom_value_cstring(cemit._cec_globdecltree[i]));
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
              MOM_DEBUGPRINTF(gencod, "definition #%d: %s",
                              i, mom_value_cstring(cemit._cec_globdefintree[i]));
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
      cemit.failing();
      MOM_WARNPRINTF_AT(e.file(), e.lineno(),
                        "mom_emit_c_code %s failed with MOM runtime exception %s in function %s",
                        mom_item_cstring(itm), mom_gc_strdup(e.what()), mom_item_cstring(cemit.current_function()));
      return false;
    }
  catch (const std::exception& e)
    {
      cemit.failing();
      MOM_WARNPRINTF("mom_emit_c_code %s failed with exception %s in %s",
                     mom_item_cstring(itm), e.what(), mom_item_cstring(cemit.current_function()));
      return false;
    }
  catch (...)
    {
      cemit.failing();
      MOM_WARNPRINTF("mom_emit_c_code %s failed in %s",
                     mom_item_cstring(itm), mom_item_cstring(cemit.current_function()));
      return false;
    }
} // end of mom_emit_c_code



////////////////////////////////////////////////////////////////
bool mom_emit_header_code(struct mom_item_st*itm)
{
  if (!itm || itm==MOM_EMPTY_SLOT || itm->va_itype != MOMITY_ITEM)
    {
      MOM_WARNPRINTF("invalid item for mom_emit_header_code");
      return false;
    }
  MOM_DEBUGPRINTF(gencod, "mom_emit_header_code start itm=%s", mom_item_cstring(itm));
  errno = 0;
  MomCEmitter hemit {itm,false};
  try
    {
      auto nod = hemit.transform_top_module();
      hemit.flush_todo_list(__LINE__);
      hemit.add_definition(nod);
      long r = 0;
      do
        {
          r= ((momrand_genrand_int32 () % 65536) << 32) |  momrand_genrand_int32();
        }
      while (MOM_UNLIKELY(r<100));
      hemit.flush_todo_list(__LINE__);
      MOM_DEBUGPRINTF(gencod, "mom_emit_header_code itm=%s nod=%s\n", mom_item_cstring(itm), mom_value_cstring(nod));
      const char*tmpath = mom_gc_printf(MOM_MODULEDIR "/" MOM_CPREFIX_PATH "%s.h+%lxp%d.tmp",
                                        mom_item_cstring(itm), r, (int)getpid());
      const char*gpath = mom_gc_printf(MOM_MODULEDIR "/" MOM_CPREFIX_PATH "%s.h", mom_item_cstring(itm));
      FILE* fout = fopen(tmpath, "w");
      if (!fout)
        MOM_FATAPRINTF("failed to fopen %s", tmpath);
      MOM_DEBUGPRINTF(gencod, "mom_emit_header_code tmpath=%s gpath=%s", tmpath, gpath);
      errno = 0;
      fprintf(fout, "// EmitC-generated header file %s ** DO NOT EDIT\n\n",
              basename(gpath));
      mom_output_gplv3_notice (fout, "//", "",  basename(gpath));
      fprintf(fout, "\n\n#ifndef MONIMELT_INCLUDED_\n");
      fprintf(fout, "#error meltmoni.h should be included before %s\n",  basename(gpath));
      fprintf(fout, "#endif /*MONIMELT_INCLUDED_*/\n\n");
      int nbdecl = (int)hemit._cec_globdecltree.size();
      if (nbdecl>0)
        {
          fprintf(fout, "\n\n/// %d declarations:\n", nbdecl);
          for (int i=0; i<nbdecl; i++)
            {
              fputc('\n', fout);
              long nl = ftell(fout);
              MOM_DEBUGPRINTF(gencod, "declaration #%d: %s",
                              i, mom_value_cstring(hemit._cec_globdecltree[i]));
              hemit.write_tree(fout,0, nl, hemit._cec_globdecltree[i], itm);
              fputs("\n\n", fout);
            }
        }
      else fputs("// no declarations\n\n", fout);
      fflush(fout);
      int nbdef = (int)hemit._cec_globdefintree.size();
      if (nbdef>0)
        {
          fprintf(fout, "\n/// %d definitions:\n", nbdef);
          for (int i=0; i<nbdef; i++)
            {
              fputc('\n', fout);
              long nl = ftell(fout);
              MOM_DEBUGPRINTF(gencod, "definition #%d: %s",
                              i, mom_value_cstring(hemit._cec_globdefintree[i]));
              hemit.write_tree(fout,0, nl, hemit._cec_globdefintree[i], itm);
              fputs("\n\n", fout);
            }
        }
      else fputs("// no definitions\n\n", fout);
      fprintf(fout,"/// end of generated header file %s\n", basename(gpath));
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
              MOM_INFORMPRINTF("C code unchanged for header %s in %s", mom_item_cstring(itm), gpath);
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
              MOM_INFORMPRINTF("C code generated for header %s in %s of %ld bytes",
                               mom_item_cstring(itm), gpath, (long)stnew.st_size);
            }
        }
      return true;
    }
  catch (const MomRuntimeErrorAt& e)
    {
      hemit.failing();
      MOM_WARNPRINTF_AT(e.file(), e.lineno(),
                        "mom_emit_header_code %s failed with MOM runtime exception %s in %s",
                        mom_item_cstring(itm), mom_gc_strdup(e.what()), mom_item_cstring(hemit.current_function()));
      return false;
    }
  catch (const std::exception& e)
    {
      hemit.failing();
      MOM_WARNPRINTF("mom_emit_header_code %s failed with exception %s in %s",
                     mom_item_cstring(itm), e.what(), mom_item_cstring(hemit.current_function()));
      return false;
    }
  catch (...)
    {
      hemit.failing();
      MOM_WARNPRINTF("mom_emit_header_code %s failed in %s",
                     mom_item_cstring(itm), mom_item_cstring(hemit.current_function()));
      return false;
    }
} // end of mom_emit_header_code




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
      MOM_DEBUGPRINTF(gencod, "mom_emit_javascript_code itm=%s nod=%s",
                      mom_item_cstring(itm), mom_value_cstring(nod));
      fprintf(fil, "/// EmitC Javascript %s *** DO NOT EDIT ***\n", mom_item_cstring(itm));
      mom_output_gplv3_notice (fil, "///", "", mom_item_cstring(itm));
      fputs("\n\n", fil);
      long nl = ftell(fil);
      jsemit.write_tree(fil, 0, nl, nod, itm);
      fprintf(fil, "\n\n" "//// end generated Javascript %s\n", mom_item_cstring(itm));
      fflush(fil);
      return true;
    }
  catch (const MomRuntimeErrorAt& e)
    {
      jsemit.failing();
      MOM_WARNPRINTF_AT(e.file(), e.lineno(),
                        "mom_emit_javascript_code %s failed with MOM runtime exception %s in %s",
                        mom_item_cstring(itm), e.what(), mom_item_cstring(jsemit.current_function()));
      return false;
    }
  catch (const std::exception& e)
    {
      jsemit.failing();
      MOM_WARNPRINTF("mom_emit_javascript_code %s failed with exception %s in %s",
                     mom_item_cstring(itm), e.what(), mom_item_cstring(jsemit.current_function()));
      return false;
    }
  catch (...)
    {
      jsemit.failing();
      MOM_WARNPRINTF("mom_emit_javascript_code %s failed in %s",
                     mom_item_cstring(itm), mom_item_cstring(jsemit.current_function()));
      return false;
    }
} // end mom_emit_javascript_code

////////////////////////////////////////////////////////////////

MomEmitter::MomEmitter(unsigned magic, struct mom_item_st*itm)
  : _ce_magic(magic),
    _ce_failing(false),
    _ce_topitm(itm),
    _ce_veclockeditems {},
    _ce_setlockeditems {},
    _ce_sigitems {},
    _ce_typitems {},
    _ce_blockitems {},
    _ce_instritems {},
    _ce_constitems {},
    _ce_todoque {},
    _ce_doatendque {},
    _ce_globalvarmap {},
    _ce_localvarmap {},
    _ce_localvalueset {},
    _ce_localcloseditems {},
    _ce_localnodetypecache {},
    _ce_curfunctionitm {nullptr},
    _ce_cursigdef {nullptr,nullptr},
    _ce_emitteritm {nullptr}
{
  if (!itm || itm==MOM_EMPTY_SLOT || itm->va_itype != MOMITY_ITEM)
    throw MOM_RUNTIME_ERROR("non item");
  lock_item(itm);
} // end MomEmitter::MomEmitter



const struct mom_boxnode_st*
MomEmitter::transform_top_module(void)
{
  assert (is_locked_item(_ce_topitm));
  auto descitm = mom_unsync_item_descr(_ce_topitm);
  traced_vector_values_t vecval;
  MOM_DEBUGPRINTF(gencod, "transform_top_module %s topitm=%s descitm=%s",
                  kindname(),
                  mom_item_cstring(_ce_topitm),
                  mom_item_cstring(descitm));
  if (descitm != MOM_PREDEFITM(module))
    throw MOM_RUNTIME_PRINTF("item %s has non-module descr: %s",
                             mom_item_cstring(_ce_topitm), mom_item_cstring(descitm));
  auto prepv = mom_unsync_item_get_phys_attr (_ce_topitm,  MOM_PREDEFITM(preparation));
  MOM_DEBUGPRINTF(gencod, "transform_top_module %s prepv %s",
                  kindname(),
                  mom_value_cstring(prepv));
  if (prepv != nullptr)
    {
      auto prepnod = mom_dyncast_node(prepv);
      if (prepnod == nullptr)
        throw MOM_RUNTIME_PRINTF("module item %s has non-node preparation %s",
                                 mom_item_cstring(_ce_topitm), mom_value_cstring(prepv));
      auto prepconnitm = prepnod->nod_connitm;
      assert (mom_itype(prepconnitm) == MOMITY_ITEM);
      lock_item(prepconnitm);
      MOM_DEBUGPRINTF(gencod, "transform_top_module module %s prepconnitm=%s",
                      mom_item_cstring(_ce_topitm), mom_item_cstring(prepconnitm));
      if (prepconnitm->itm_paylkind != MOM_PREDEFITM(signature_func_item_to_void)
          || !prepconnitm->itm_payldata)
        throw MOM_RUNTIME_PRINTF("module item %s has preparation %s"
                                 " with bad connective of kind %s, expecting signature_func_item_to_void",
                                 mom_item_cstring(_ce_topitm), mom_value_cstring(prepv),
                                 mom_item_cstring(prepconnitm->itm_paylkind));
      auto funptr = (mom_func_item_to_void_sig_t *)prepconnitm->itm_payldata;
      MOM_DEBUGPRINTF(gencod, "transform_top_module module %s before calling preparation %s on emitter %s",
                      mom_item_cstring(_ce_topitm), mom_value_cstring(prepv),
                      mom_item_cstring(emitter_item()));
      (*funptr)(prepnod, emitter_item());
      MOM_DEBUGPRINTF(gencod, "transform_top_module module %s did preparation %s on emitter %s",
                      mom_item_cstring(_ce_topitm), mom_value_cstring(prepv),
                      mom_item_cstring(emitter_item()));
    }
  MOM_DEBUGPRINTF(gencod, "transform_top_module module %s before after_preparation_transform",
                  mom_item_cstring(_ce_topitm));
  after_preparation_transform();
  MOM_DEBUGPRINTF(gencod, "transform_top_module module %s after after_preparation_transform",
                  mom_item_cstring(_ce_topitm));
  auto modulev = mom_unsync_item_get_phys_attr (_ce_topitm,  MOM_PREDEFITM(module));
  MOM_DEBUGPRINTF(gencod, "transform_top_module %s modulev %s prepv %s",
                  kindname(),
                  mom_value_cstring(modulev),
                  mom_value_cstring(prepv));
  auto modulseq = mom_dyncast_seqitems (modulev);
  if (!modulseq)
    throw MOM_RUNTIME_PRINTF("item %s with bad module:%s",
                             mom_item_cstring(_ce_topitm), mom_value_cstring(modulev));
  bind_global_at(MOM_PREDEFITM(module),MOM_PREDEFITM(module),_ce_topitm, __LINE__);
  unsigned nbmodelem = mom_seqitems_length(modulseq);
  vecval.reserve(nbmodelem);
  auto itemsarr = mom_seqitems_arr(modulseq);
  for (unsigned ix=0; ix<nbmodelem; ix++)
    {
      auto curitm = itemsarr[ix];
      MOM_DEBUGPRINTF(gencod, "transform_top_module ix#%d startloop\n.. curitm:=%s\n",
                      ix, mom_item_content_cstring(curitm));
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
      if (MOM_IS_DEBUGGING(gencod))
        {
          MOM_DEBUGPRINTF(gencod, "after scanning %s #%d, %zd global bindings",
                          mom_item_cstring(curitm), ix, _ce_globalvarmap.size());
          for (auto it : _ce_localvarmap)
            {
              MOM_DEBUGPRINTF(gencod, "local %s bound role %s what %s",
                              mom_item_cstring(it.first), mom_item_cstring(it.second.vd_rolitm),  mom_value_cstring(it.second.vd_what));
            };
          MOM_DEBUGPRINTF(gencod, "after scanning %s #%d, %zd global bindings",
                          mom_item_cstring(curitm), ix, _ce_globalvarmap.size());
          for (auto it : _ce_globalvarmap)
            {
              MOM_DEBUGPRINTF(gencod, "global %s bound role %s what %s",
                              mom_item_cstring(it.first), mom_item_cstring(it.second.vd_rolitm),  mom_value_cstring(it.second.vd_what));
            }
        }
      MOM_DEBUGPRINTF(gencod, "transform_top_module scanned curitm=%s", mom_item_cstring(curitm));
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
	     vecval.push_back(mom_boxnode_make_va(MOM_PREDEFITM(out_newline),0));
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
      MOM_DEBUGPRINTF(gencod, "transform_top_module ix#%d curitm=%s beforeflushtodo",
                      ix, mom_item_cstring(curitm));
      flush_todo_list(__LINE__);
      _ce_localvarmap.clear();
      _ce_localcloseditems.clear();
      _ce_localnodetypecache.clear();
      _ce_curfunctionitm = nullptr;
      MOM_DEBUGPRINTF(gencod, "transform_top_module ix#%d finishingloop curitm=%s before flushtodoafter\n",
                      ix, mom_item_cstring(curitm));
      flush_todo_after_element_list(__LINE__);
      MOM_DEBUGPRINTF(gencod, "transform_top_module ix#%d finishedloop curitm=%s\n",
                      ix, mom_item_cstring(curitm));
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
  MOM_DEBUGPRINTF(gencod, "scan_module_element %s topitm=%s descitm=%s\n.. elitm:=%s",
                  kindname(),
                  mom_item_cstring(_ce_topitm),
                  mom_item_cstring(descitm), mom_item_content_cstring(elitm));
  if (!descitm)
    throw MOM_RUNTIME_PRINTF("module element %s without descr", mom_item_cstring(elitm));
#define NBMODELEMDESC_MOM 61
#define CASE_DESCR_MOM(Nam) momhashpredef_##Nam % NBMODELEMDESC_MOM:	\
  if (descitm == MOM_PREDEFITM(Nam)) goto foundcase_##Nam;		\
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
    case CASE_DESCR_MOM (inline):
      goto scanroutine;
    case CASE_DESCR_MOM (routine):
    scanroutine:
      todo([=](MomEmitter*thisemit)
	   {
	     thisemit->scan_routine_element(elitm);
	   });
    break;
    case CASE_DESCR_MOM (type):
      todo([=](MomEmitter*thisemit)
	   {
	     thisemit->scan_type_item(elitm);
	   });
      break;
    case CASE_DESCR_MOM (signature):
      todo([=](MomEmitter*thisemit)
	   {
	     thisemit->scan_signature(elitm,(_ce_topitm));
	   });
      break;
    case CASE_DESCR_MOM (constant):
      todo([=](MomEmitter*thisemit)
	   {
	     thisemit->scan_constant_item(elitm,nullptr);
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
  if (descitm == MOM_PREDEFITM(Nam)) goto foundcase_##Nam;		\
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
      MOM_DEBUGPRINTF(gencod, "transform_module_element other element elitm=%s descitm=%s",
                      mom_item_cstring(elitm), mom_item_cstring(descitm));
      resnod = transform_other_element(elitm, descitm);
      MOM_DEBUGPRINTF(gencod, "transform_module_element other elitm=%s descitm=%s got resnod=%s",
                      mom_item_cstring(elitm), mom_item_cstring(descitm), mom_value_cstring(resnod));
      if (resnod == nullptr)
        throw MOM_RUNTIME_PRINTF("module element %s strange desc %s",
                                 mom_item_cstring(elitm), mom_item_cstring(descitm));
      break;
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
  MOM_DEBUGPRINTF(gencod, "scan_data_element start daitm:=%s",
                  mom_item_content_cstring(daitm));
  assert (is_locked_item(daitm));
  auto dabind = get_binding(daitm);
  if (dabind)
    throw MOM_RUNTIME_PRINTF("data element %s already bound to role %s what %s",
                             mom_item_cstring(daitm),
                             mom_item_cstring(dabind->vd_rolitm),
                             mom_value_cstring(dabind->vd_what));
  auto typitm = mom_dyncast_item(mom_unsync_item_get_phys_attr(daitm,MOM_PREDEFITM(type)));
  MOM_DEBUGPRINTF(gencod, "scan_data_element daitm=%s typitm=%s",
                  mom_item_content_cstring(daitm), mom_item_cstring(typitm));
  if (!typitm)
    throw MOM_RUNTIME_PRINTF("data element %s without type", mom_item_cstring(daitm));
  lock_item(typitm);
  scan_type_item (typitm);
  bind_global_at(daitm,MOM_PREDEFITM(data),typitm, __LINE__);
  MOM_DEBUGPRINTF(gencod, "scan_data_element done daitm=%s",
                  mom_item_cstring(daitm));
} // end  MomEmitter::scan_data_element



void
MomEmitter::scan_func_element(struct mom_item_st*fuitm)
{
  MOM_DEBUGPRINTF(gencod, "scan_func_element start fuitm=%s", mom_item_cstring(fuitm));
  assert (is_locked_item(fuitm));
  bind_global_at(MOM_PREDEFITM(func),MOM_PREDEFITM(func),fuitm, __LINE__);
  bind_global_at(fuitm,MOM_PREDEFITM(func),fuitm, __LINE__);
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
    _ce_cursigdef = sigr;
    MOM_DEBUGPRINTF(gencod, "scan_func_element fuitm=%s formaltup=%s",
                    mom_item_cstring(fuitm), mom_value_cstring(formaltup));
  }
  if (bdyitm == nullptr)
    throw MOM_RUNTIME_PRINTF("missing or bad body in func %s", mom_item_cstring(fuitm));
  flush_todo_list(__LINE__);
  lock_item(bdyitm);
  scan_block(bdyitm,fuitm);
  _ce_curfunctionitm = nullptr;
  MOM_DEBUGPRINTF(gencod, "scan_func_element done fuitm=%s\n", mom_item_cstring(fuitm));
} // end  MomEmitter::scan_func_element

////////////////
void
MomEmitter::scan_type_expr(momvalue_t tyval, struct mom_item_st*initm)
{
  MOM_DEBUGPRINTF(gencod, "scan_type_expr start tyval=%s initm=%s",
                  mom_value_cstring(tyval), mom_item_cstring(initm));
  auto ity = mom_itype(tyval);
  if (ity == MOMITY_ITEM)
    {
      auto typitm = (struct mom_item_st*)tyval;
      lock_item(typitm);
      scan_type_item(typitm);
    }
  else if (ity == MOMITY_NODE)
    {
      auto tynod = (const struct mom_boxnode_st*)tyval;
      unsigned tysiz = mom_raw_size(tynod);
      auto tyconnitm = tynod->nod_connitm;
      if (tyconnitm == MOM_PREDEFITM(pointer) && tysiz == 1)
        {
          auto dereftypv = tynod->nod_sons[0];
          MOM_DEBUGPRINTF(gencod, "scan_type_expr tyval=%s dereftypv=%s",
                          mom_value_cstring(tyval), mom_value_cstring(dereftypv));
          scan_type_expr(dereftypv, initm);
        }
      else if (tyconnitm == MOM_PREDEFITM(signature) && tysiz == 1)
        {
          auto sigitm = mom_dyncast_item(tynod->nod_sons[0]);
          if (!sigitm)
            goto invalidtype;
          lock_item(sigitm);
          MOM_DEBUGPRINTF(gencod, "scan_type_expr tyval=%s sigitm=%s",
                          mom_value_cstring(tyval), mom_item_cstring(sigitm));
          scan_signature(sigitm,initm);
        }
      else goto invalidtype;
    }
  else
  invalidtype:
    throw MOM_RUNTIME_PRINTF("invalid type expression %s in %s",
                             mom_value_cstring(tyval), mom_item_cstring(initm));
  MOM_DEBUGPRINTF(gencod, "scan_type_expr end tyval=%s initm=%s",
                  mom_value_cstring(tyval), mom_item_cstring(initm));
} // end MomEmitter::scan_type_expr


////////////////
void
MomEmitter::scan_type_item(struct mom_item_st*typitm)
{
  MOM_DEBUGPRINTF(gencod, "scan_type_item start typitm=%s", mom_item_cstring(typitm));
  assert (is_locked_item(typitm));
  struct mom_item_st*desitm = mom_unsync_item_descr(typitm);
  if (desitm != MOM_PREDEFITM(type))
    throw MOM_RUNTIME_PRINTF("type %s has bad descr %s",
                             mom_item_cstring(typitm), mom_item_cstring(desitm));
  auto tytypitm = mom_dyncast_item(mom_unsync_item_get_phys_attr(typitm,
								 MOM_PREDEFITM(type)));
  MOM_DEBUGPRINTF(gencod, "scan_type_item typitm=%s tytypitm=%s",
                  mom_item_cstring(typitm), mom_item_cstring(tytypitm));
  if (_ce_typitems.find(typitm) == _ce_typitems.end())
    {
      _ce_typitems.insert(typitm);
      MOM_DEBUGPRINTF(gencod, "scan_type_item fresh typitm=%s", mom_item_cstring(typitm));
      if (tytypitm != nullptr)
        {
          auto cem = dynamic_cast<MomCEmitter*>(this);
          if (cem != nullptr)
            {
              auto bind = get_binding(typitm);
              MOM_DEBUGPRINTF(gencod, "scan_type_item should declare typitm=%s bind role %s what %s",
                              mom_item_cstring(typitm), mom_item_cstring(bind?bind->vd_rolitm:nullptr),
                              mom_value_cstring(bind?bind->vd_what:nullptr));
              if (bind == nullptr)
                {
                  MOM_DEBUGPRINTF(gencod, "scan_type_item declaring typitm=%s", mom_item_cstring(typitm));
                  auto dtytree = cem->declare_type(typitm);
                  MOM_DEBUGPRINTF(gencod, "scan_type_item typitm=%s declared, dtytree=%s",
                                  mom_item_cstring(typitm), mom_value_cstring(dtytree));
                }
            }
        };
    }
  MOM_DEBUGPRINTF(gencod, "scan_type_item done typitm=%s", mom_item_cstring(typitm));
} // end  MomEmitter::scan_type_item



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
  if (!nobind)
    bind_local_at(MOM_PREDEFITM(signature),MOM_PREDEFITM(signature),sigitm,__LINE__);
  auto formv = mom_unsync_item_get_phys_attr(sigitm, MOM_PREDEFITM(formals));
  auto formtup =
    mom_dyncast_tuple(formv);
  MOM_DEBUGPRINTF(gencod, "scan_signature sigitm=%s formtup=%s formv=%s",
                  mom_item_cstring(sigitm), mom_value_cstring(formtup), mom_value_cstring(formv));
  if (formtup == nullptr)
    throw MOM_RUNTIME_PRINTF("missing formals in signature %s", mom_item_cstring(sigitm));
  if (!nobind)
    bind_local_at(MOM_PREDEFITM(formals),MOM_PREDEFITM(formals),formtup,__LINE__);
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
      MOM_DEBUGPRINTF(gencod, "formal#%d in signature %s := %s",
                      ix, mom_item_cstring(sigitm),
                      mom_item_content_cstring(curformitm));
      if (mom_unsync_item_descr(curformitm) != MOM_PREDEFITM(formal))
        throw MOM_RUNTIME_PRINTF("bad formal#%d %s (of descr %s not `formal`) in signature %s",
                                 ix, mom_item_cstring(curformitm),
                                 mom_item_cstring(mom_unsync_item_descr(curformitm)),
                                 mom_item_cstring(sigitm));
      auto typfv =
        mom_unsync_item_get_phys_attr(curformitm,MOM_PREDEFITM(type));
      MOM_DEBUGPRINTF(gencod, "formal#%d %s has type %s",
                      ix, mom_item_cstring(curformitm), mom_value_cstring(typfv));
      if (!typfv)
        throw MOM_RUNTIME_PRINTF("untyped formal#%d %s in signature %s",
                                 ix, mom_item_cstring(curformitm),
                                 mom_item_cstring(sigitm));
      scan_type_expr(typfv, curformitm);
      if (!nobind)
        bind_local_at(curformitm, MOM_PREDEFITM(formal), sigitm, __LINE__, typfv, ix);
    }
  auto resv = mom_unsync_item_get_phys_attr(sigitm, MOM_PREDEFITM(result));
  MOM_DEBUGPRINTF(gencod, "scan_signature sigitm=%s resv=%s",
                  mom_item_cstring(sigitm), mom_value_cstring(resv));
  unsigned restyp = mom_itype(resv);
  if (restyp == MOMITY_ITEM)
    {
      auto restypitm = reinterpret_cast<struct mom_item_st*>((void*)resv);
      lock_item(restypitm);
      scan_type_item(restypitm);
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
          scan_type_item(curtyitm);
        }
    }
  else if (restyp != MOMITY_NONE)
    throw  MOM_RUNTIME_PRINTF("invalid result %s (should be item or tuple) for signature %s",
                              mom_value_cstring(resv), mom_item_cstring(sigitm));
  _ce_sigitems.insert(sigitm);
  MOM_DEBUGPRINTF(gencod, "scan_signature sigitm=%s done formals=%s result=%s",
                  mom_item_cstring(sigitm), mom_value_cstring(formtup), mom_value_cstring(resv));
  return {formtup,resv};
} // end MomEmitter::scan_signature


void
MomEmitter::scan_block(struct mom_item_st*blkitm, struct mom_item_st*initm)
{
  MOM_DEBUGPRINTF(gencod, "scan_block start blkitm:=%s\n.. initm=%s",
                  mom_item_content_cstring(blkitm), mom_item_cstring(initm));
  assert (is_locked_item(blkitm));
  if (_ce_blockitems.size() >= NB_MAX_BLOCKS)
    throw  MOM_RUNTIME_PRINTF("in %s block %s overflowing %d",
                              mom_item_cstring(initm),
                              mom_item_cstring(blkitm),
                              (int) (_ce_blockitems.size()));
  struct mom_item_st*desitm = mom_unsync_item_descr(blkitm);
  _ce_blockitems.insert(blkitm);
  MOM_DEBUGPRINTF(gencod, "scan_block  blkitm=%s desitm=%s",  mom_item_cstring(blkitm), mom_item_cstring(desitm));
  if (desitm == MOM_PREDEFITM(indirect))
    {
      auto indiritm = mom_dyncast_item(mom_unsync_item_get_phys_attr(blkitm, MOM_PREDEFITM(indirect)));
      MOM_DEBUGPRINTF(gencod, "scan_block indirect blkitm=%s indiritm=%s",
                      mom_item_cstring(blkitm), mom_item_cstring(indiritm));
      if (indiritm == nullptr)
        throw MOM_RUNTIME_PRINTF("indirect block %s in %s with bad `indirect`",
                                 mom_item_cstring(blkitm), mom_item_cstring(initm));
      lock_item(indiritm);
      scan_block(indiritm, initm);
      return;
    }
  auto blkbind = get_binding(blkitm);
  if (blkbind != nullptr)
    throw MOM_RUNTIME_PRINTF("in %s block %s already bound to role %s what %s",
                             mom_item_cstring(initm),
                             mom_item_cstring(blkitm),
                             mom_item_cstring(blkbind->vd_rolitm),
                             mom_value_cstring(blkbind->vd_what));
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
      bind_local_at(blkitm, MOM_PREDEFITM(sequence), initm, __LINE__);
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
          auto typlocv =
            mom_unsync_item_get_phys_attr(curlocitm,MOM_PREDEFITM(type));
          MOM_DEBUGPRINTF(gencod, "in block %s local %s has type %s",
                          mom_item_cstring(blkitm),
                          mom_item_cstring(curlocitm), mom_value_cstring(typlocv));
          if (!typlocv)
            throw  MOM_RUNTIME_PRINTF("in %s sequence %s with local#%d %s without type",
                                      mom_item_cstring(initm),
                                      mom_item_cstring(blkitm),
                                      locix, mom_item_cstring(curlocitm));
          scan_type_expr(typlocv, curlocitm);
          bind_local_at(curlocitm, MOM_PREDEFITM(variable), blkitm, __LINE__, typlocv, locix);
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
      bind_local_at(blkitm, MOM_PREDEFITM(loop), initm, __LINE__);
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





void
MomEmitter::scan_instr(struct mom_item_st*insitm, int rk, struct mom_item_st*blkitm)
{
  MOM_DEBUGPRINTF(gencod, "scan_instr start insitm:=\n%s\n.. rk#%d blkitm=%s",
                  mom_item_content_cstring(insitm), rk, mom_item_cstring(blkitm));
  assert (is_locked_item(insitm));
  auto insbind = get_binding(insitm);
  {
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
    if (insbind)
      throw MOM_RUNTIME_PRINTF("instr %s #%d of %s already bound with role %s",
                               mom_item_cstring(insitm), rk, mom_item_cstring(blkitm),
                               mom_item_cstring(insbind->vd_rolitm));
  }
#define NBMODOPER_MOM 97
#define CASE_OPER_MOM(Nam) momhashpredef_##Nam % NBMODOPER_MOM: \
  if (desitm == MOM_PREDEFITM(Nam)) goto foundcase_##Nam;	\
  goto defaultcasedesc; foundcase_##Nam
  switch (desitm->hva_hash % NBMODOPER_MOM)
    {
    case CASE_OPER_MOM(assign): //////////
      {
	auto destexp = mom_unsync_item_get_phys_attr(insitm, MOM_PREDEFITM(to));
	auto fromexp = mom_unsync_item_get_phys_attr(insitm, MOM_PREDEFITM(from));
	MOM_DEBUGPRINTF(gencod, "scan_instr assign %s destexp %s fromexp %s",
			mom_item_cstring(insitm),
			mom_value_cstring(destexp),
			mom_value_cstring(fromexp));
	if (insitm->itm_pcomp && mom_vectvaldata_count(insitm->itm_pcomp)>0)
	  throw  MOM_RUNTIME_PRINTF("assign %s #%d in block %s with unexpected %d components",
				    mom_item_cstring(insitm), rk,
				    mom_item_cstring(blkitm),
				    mom_vectvaldata_count(insitm->itm_pcomp));
	MOM_DEBUGPRINTF(gencod, "scan_instr assign %s destexp %s",
			mom_item_cstring(insitm),
			mom_value_cstring(destexp));
	if (!destexp)
	  throw  MOM_RUNTIME_PRINTF("assign %s #%d in block %s of %s without `to`",
				    mom_item_cstring(insitm), rk,
				    mom_item_cstring(blkitm), mom_item_cstring(blkitm));
	auto totypitm = scan_expr(destexp,insitm,0,nullptr,true);
	MOM_DEBUGPRINTF(gencod, "scan_instr assign %s totypitm %s fromexp %s",
			mom_item_cstring(insitm), mom_item_cstring(totypitm),
			mom_value_cstring(fromexp));
	auto fromtypitm = fromexp?scan_expr(fromexp,insitm,0,totypitm):totypitm;
	if (!fromtypitm)
	  throw MOM_RUNTIME_PRINTF("assign %s #%d in block %s with untypable `from` %s",
				   mom_item_cstring(insitm), rk,
				   mom_item_cstring(blkitm), mom_value_cstring(fromexp));
	if (totypitm != fromtypitm)
	  throw MOM_RUNTIME_PRINTF("assign %s #%d in block %s with incompatible types from type %s to type %s",
				   mom_item_cstring(insitm), rk,
				   mom_item_cstring(blkitm), mom_item_cstring(fromtypitm), mom_item_cstring(totypitm));
	bind_local_at(insitm,MOM_PREDEFITM(assign),
		      mom_boxnode_make_va(MOM_PREDEFITM(assign),3,
					  destexp, fromexp,  totypitm),
		      __LINE__,
		      blkitm, rk);
      } //// end assign
      break;
      /////
    case CASE_OPER_MOM(break): //////////
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
	bind_local_at(insitm,MOM_PREDEFITM(break),outblkitm,
		      __LINE__,
		      blkitm, rk);
	{
	  auto it = _ce_breakcountmap.find(outblkitm);
	  if (it != _ce_breakcountmap.end())
	    it->second++;
	  else
	    _ce_breakcountmap[outblkitm] = 1;
	}
      } //// end break
      break;
      ////
    case CASE_OPER_MOM(continue): ///////////////
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
	bind_local_at(insitm,MOM_PREDEFITM(continue),
		      loopitm, __LINE__,
		      blkitm, rk);
	{
	  auto it = _ce_continuecountmap.find(loopitm);
	  if (it != _ce_continuecountmap.end())
	    it->second++;
	  else
	    _ce_continuecountmap[loopitm] = 1;
	}
      } /// end continue
      break;
      ////
    case CASE_OPER_MOM(loop): /////////
      goto sequencecase;
    case CASE_OPER_MOM(sequence): ////////
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
      } /// end sequence (& loop)
    break;
    //////
    case CASE_OPER_MOM(cond): ////////////////////
      {
	auto condtup= mom_dyncast_tuple(mom_unsync_item_get_phys_attr(insitm, MOM_PREDEFITM(cond)));
	MOM_DEBUGPRINTF(gencod,
			"scan_instr cond %s #%d condtup=%s",
			mom_item_cstring(insitm), rk,
			mom_value_cstring(condtup));
	if (!condtup)
	  throw MOM_RUNTIME_PRINTF("cond %s #%d in block %s without `cond`",
				   mom_item_cstring(insitm), rk,
				   mom_item_cstring(blkitm));
	unsigned nbcond = mom_raw_size(condtup);
	bind_local_at(insitm, MOM_PREDEFITM(cond), condtup, __LINE__, blkitm, rk);
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
	    MOM_DEBUGPRINTF(gencod,
			    "scan_instr cond %s #%d in block %s; ix#%d testitm:=%s",
			    mom_item_cstring(insitm), rk,
			    mom_item_cstring(blkitm), ix,
			    mom_item_content_cstring(testitm));

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
		 || mom_dyncast_item(testexp)==MOM_PREDEFITM(unit)
		 || mom_dyncast_item(testexp)==MOM_PREDEFITM(truth)))
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
		auto testypitm = scan_expr(testexp,testitm,1);
		if (!testypitm)
		  throw
		    MOM_RUNTIME_PRINTF("cond %s #%d in block %s with untypable test#%d %s",
				       mom_item_cstring(insitm), rk,
				       mom_item_cstring(blkitm),
				       ix,
				       mom_item_cstring(testitm));
	      }
	    auto thenitm =
	      mom_dyncast_item(mom_unsync_item_get_phys_attr(testitm, MOM_PREDEFITM(then)));
	    MOM_DEBUGPRINTF(gencod, "scan_instr cond %s #%d thenitm %s", mom_item_cstring(insitm), rk,
			    mom_item_cstring(thenitm));
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
		   em->scan_instr(thenitm, 0, testitm);
		   MOM_DEBUGPRINTF(gencod, "after scanning thenitm=%s insitm=%s rk#%d blkitm=%s",
				   mom_item_cstring(thenitm),
				   mom_item_cstring(insitm), rk, mom_item_cstring(blkitm));
		 });
	  }
	MOM_DEBUGPRINTF(gencod, "scan_instr cond insitm=%s condtup=%s",
			mom_item_cstring(insitm), mom_value_cstring(condtup));
      } // end cond
      break;
      /////
    case CASE_OPER_MOM(call): /////////////
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
	bind_local_at(insitm,MOM_PREDEFITM(call),callsigitm, __LINE__, blkitm, rk);
      } // end call
      break;
      /////
    case CASE_OPER_MOM(run):  //////////////////
      {
	auto runitm =
	  mom_dyncast_item(mom_unsync_item_get_phys_attr(insitm, MOM_PREDEFITM(run)));
	if (!runitm)
	  throw MOM_RUNTIME_PRINTF("missing primitive to `run` in instr %s rk#%d in block %s",
				   mom_item_cstring(insitm), rk, mom_item_cstring(blkitm));
	auto resultv = mom_unsync_item_get_phys_attr(insitm, MOM_PREDEFITM(result));
	MOM_DEBUGPRINTF(gencod,"run instr %s runitm %s resultv %s",
			mom_item_cstring(insitm),
			mom_item_cstring(runitm),
			mom_value_cstring(resultv));
	lock_item(runitm);
	auto rundescitm =  mom_unsync_item_descr(runitm);
	if (rundescitm != MOM_PREDEFITM(primitive)
	    && rundescitm != MOM_PREDEFITM(routine))
	  throw MOM_RUNTIME_PRINTF("bad primitive %s (of wrong descr %s) to `run` in instr %s rk#%d in block %s",
				   mom_item_cstring(runitm),
				   mom_item_cstring(rundescitm),
				   mom_item_cstring(insitm), rk, mom_item_cstring(blkitm));
	auto runsigitm =
	  mom_dyncast_item(mom_unsync_item_get_phys_attr(runitm, MOM_PREDEFITM(signature)));
	if (!runsigitm)
	  throw  MOM_RUNTIME_PRINTF("primitive %s without signature to `run` in instr %s rk#%d in block %s",
				    mom_item_cstring(runitm), mom_item_cstring(insitm), rk, mom_item_cstring(blkitm));
	lock_item(runsigitm);
	auto sigr = scan_nonbinding_signature(runsigitm, insitm);
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
	    MOM_DEBUGPRINTF(gencod, "badarity runsigitm %s", mom_item_cstring(runsigitm));
	    throw MOM_RUNTIME_PRINTF("run instr %s rk#%d in block %s with wrong arity,"
				     " got %d expecting %d for signature %s",
				     mom_item_cstring(insitm), rk, mom_item_cstring(blkitm),
				     nbcomp, sigarity, mom_item_cstring(runsigitm));
	  }
	for (unsigned ix=0; ix<nbcomp; ix++)
	  {
	    auto curformitm = mom_boxtuple_nth(sformaltup, ix);
	    MOM_DEBUGPRINTF(gencod, "scan_instr run insitm=%s ix#%d curformitm=%s",
			    mom_item_cstring(insitm), ix,
			    mom_item_cstring(curformitm));
	    assert (curformitm != nullptr);
	    assert (is_locked_item(curformitm));
	    auto curtypv =
	      mom_unsync_item_get_phys_attr(curformitm, MOM_PREDEFITM(type));
	    assert (curtypv != nullptr);
	    auto curtypitm = mom_dyncast_item(curtypv);
	    assert (curtypitm==nullptr || is_locked_item(curtypitm));
	    auto curargv = mom_vectvaldata_nth(comps,ix);
	    if (scan_expr(curargv, insitm, 1, curtypitm) != curtypitm
		&& curtypitm != nullptr)
	      throw MOM_RUNTIME_PRINTF("run instr %s rk#%d in block %s "
				       "with mistyped arg#%d %s (expecting %s)",
				       mom_item_cstring(insitm), rk, mom_item_cstring(blkitm),
				       rk, mom_value_cstring(curargv),
				       mom_item_cstring(curtypitm));
	  };
	MOM_DEBUGPRINTF(gencod,
			"run instr %s resultv %s runsigitm %s sresultv %s",
			mom_item_cstring(insitm),
			mom_value_cstring(resultv),
			mom_item_cstring(runsigitm),
			mom_value_cstring(sresultv));
	auto resity = mom_itype(resultv);
	if (resity == MOMITY_ITEM)
	  {
	    auto formrtypitm = mom_dyncast_item(sresultv);
	    assert (is_locked_item(formrtypitm));
	    MOM_DEBUGPRINTF(gencod,
			    "run instr %s formrtypitm %s",
			    mom_value_cstring(insitm),
			    mom_item_cstring(formrtypitm));
	    auto resvaritm = (struct mom_item_st*)resultv;
	    lock_item(resvaritm);
	    if (scan_var(resvaritm, insitm, formrtypitm) != formrtypitm)
	      throw  MOM_RUNTIME_PRINTF("run instr %s rk#%d in block %s "
					"with mistyped result %s, expecting %s",
					mom_item_cstring(insitm), rk, mom_item_cstring(blkitm),
					mom_item_cstring(resvaritm), mom_item_cstring(formrtypitm));

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
					nbresults, nbformresults, mom_item_cstring(runsigitm));
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
					   mom_item_cstring(curformrestypitm), mom_item_cstring(runsigitm));
	      }
	  }
	else if (resity == MOMITY_NODE)
	  {
	    auto formrtypitm = mom_dyncast_item(sresultv);
	    if(formrtypitm == nullptr)
	      throw MOM_RUNTIME_PRINTF("run instr %s rk#%d in block %s with bad formal result %s for result expr %s",
				       mom_item_cstring(insitm), rk, mom_item_cstring(blkitm),
				       mom_value_cstring(sresultv),
				       mom_value_cstring(resultv));
	    MOM_DEBUGPRINTF(gencod, "scan run instr %s formtypitm %s resultv %s",
			    mom_item_cstring(insitm), mom_item_cstring(formrtypitm),
			    mom_value_cstring(resultv));
	    scan_node_expr (mom_dyncast_node(resultv), insitm, 0, formrtypitm, true);
	  }
	else if (resity != MOMITY_NONE)
	  throw MOM_RUNTIME_PRINTF("run instr %s rk#%d in block %s "
				   "with bad result %s",
				   mom_item_cstring(insitm), rk, mom_item_cstring(blkitm),
				   mom_value_cstring(resultv));
	bind_local_at(insitm,MOM_PREDEFITM(run),runitm, __LINE__, blkitm, rk);
      } // end run
      break;
      /////
    case CASE_OPER_MOM(switch): ////////////////
      {
	auto swtypitm =
	  mom_dyncast_item(mom_unsync_item_get_phys_attr(insitm, MOM_PREDEFITM(switch)));
	auto casehdr = case_scanner(swtypitm,insitm,rk,blkitm);
	auto argv = mom_unsync_item_get_phys_attr(insitm, MOM_PREDEFITM(arg));
	MOM_DEBUGPRINTF(gencod, "switch instr insitm=%s swtypitm=%s argv=%s",
			mom_item_cstring(insitm), mom_item_cstring(swtypitm), mom_value_cstring(argv));
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
	MOM_DEBUGPRINTF(gencod, "scaninstr switch swtypitm=%s argv=%s caseseq=%s insitm=%s",
			mom_value_cstring(swtypitm),
			mom_value_cstring(argv),
			mom_value_cstring(caseseq),
			mom_item_cstring(insitm));
	auto bindsw =
	  mom_boxnode_make_va(MOM_PREDEFITM(switch), 4,
			      (momvalue_t)swtypitm, (momvalue_t)argv,
			      (momvalue_t)caseseq, (momvalue_t)otherwv);
	MOM_DEBUGPRINTF(gencod, "scaninstr switch insitm=%s bindsw=%s", mom_item_cstring(insitm),
			mom_value_cstring(bindsw));
	bind_local_at(insitm,MOM_PREDEFITM(switch),bindsw, __LINE__,
		      blkitm, rk);
      } //// end switch
      break;
      /////
    case CASE_OPER_MOM(return): ////////////////
      {
	auto retexpv =
	  mom_unsync_item_get_phys_attr(insitm, MOM_PREDEFITM(return));
	MOM_DEBUGPRINTF(gencod, "scaninstr return insitm=%s retexpv=%s sigresult=%s",
			mom_item_cstring(insitm), mom_value_cstring(retexpv),
			mom_value_cstring(_ce_cursigdef.sig_result));
	auto curestypitm = mom_dyncast_item(_ce_cursigdef.sig_result);
	auto retypv = scan_expr(retexpv,insitm,0,curestypitm);
	MOM_DEBUGPRINTF(gencod, "scaninstr return insitm=%s retexpv=%s retypv=%s",
			mom_item_cstring(insitm), mom_value_cstring(retexpv),
			mom_value_cstring(retypv));
	bind_local_at(insitm,MOM_PREDEFITM(return),retypv,__LINE__,
		      blkitm, rk);
	MOM_DEBUGPRINTF(gencod, "scaninstr return insitm=%s retypv=%s",
			mom_item_cstring(insitm), mom_value_cstring(retypv));
#warning scan_instr return incomplete
      } //end return
      break;
    case CASE_OPER_MOM(indirect): //////////
      {
	auto indiritm =
	  mom_dyncast_item(mom_unsync_item_get_phys_attr(insitm, MOM_PREDEFITM(indirect)));
	MOM_DEBUGPRINTF(gencod, "scaninstr indirect insitm=%s  indiritm=%s",
			mom_item_cstring(insitm), mom_item_cstring(indiritm));
	if (indiritm==nullptr)
	  throw MOM_RUNTIME_PRINTF("indirect instruction %s rk#%d block %s with missing indirect item",
				   mom_item_cstring(insitm), rk, mom_item_cstring(blkitm));
	lock_item(indiritm);
	scan_instr(indiritm, rk, blkitm);
      }
      break;
      ////
    default:
    defaultcasedesc: ////
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
MomEmitter::scan_expr(const void*expv, struct mom_item_st*insitm, int depth, struct mom_item_st*typitm, bool lvar)
{
  MOM_DEBUGPRINTF(gencod, "scan_expr start expv=%s insitm=%s depth#%d typitm=%s %s",
                  mom_value_cstring(expv), mom_item_cstring(insitm), depth, mom_item_cstring(typitm),
                  lvar?"lvar":"expr");
  if (depth >= MAX_DEPTH_EXPR)
    throw  MOM_RUNTIME_PRINTF("expr %s in instr %s is too deep (%d)",
                              mom_value_cstring(expv), mom_item_cstring(insitm), depth);
  if (typitm)
    {
      lock_item(typitm);
      scan_type_item(typitm);
    }
  unsigned typexp = mom_itype(expv);
  switch (typexp)
    {
    case MOMITY_NONE:
      if (lvar) goto nonlvar;
      return typitm?typitm:MOM_PREDEFITM(unit);
    case MOMITY_INT:
      if (lvar) goto nonlvar;
      if (typitm && typitm != MOM_PREDEFITM(int) && typitm != MOM_PREDEFITM(value))
        throw MOM_RUNTIME_PRINTF("int.expr %s in instr %s with type mismatch for %s",
                                 mom_value_cstring(expv), mom_item_cstring(insitm), mom_item_cstring(typitm));
      return typitm?typitm:MOM_PREDEFITM(int);
    case MOMITY_BOXSTRING:
      if (lvar) goto nonlvar;
      if (typitm && typitm != MOM_PREDEFITM(string) && typitm != MOM_PREDEFITM(value))
        throw MOM_RUNTIME_PRINTF("string.expr %s in instr %s with type mismatch for %s",
                                 mom_value_cstring(expv), mom_item_cstring(insitm), mom_item_cstring(typitm));
      return typitm?typitm:MOM_PREDEFITM(string);
    case MOMITY_BOXDOUBLE:
      if (lvar) goto nonlvar;
      if (typitm && typitm != MOM_PREDEFITM(double) && typitm != MOM_PREDEFITM(value))
        throw MOM_RUNTIME_PRINTF("double.expr %s in instr %s with type mismatch for %s",
                                 mom_value_cstring(expv), mom_item_cstring(insitm), mom_item_cstring(typitm));
      return typitm?typitm:MOM_PREDEFITM(double);
    case MOMITY_ITEM:
      {
	auto itmexp = (struct mom_item_st*)expv;
	lock_item(itmexp);
	return scan_item_expr(itmexp,insitm,depth,typitm,lvar);
      }
    case MOMITY_NODE:
      {
	auto nodexp = (const struct mom_boxnode_st*)expv;
	auto it =_ce_localnodetypecache.find(nodexp);
	if (it == _ce_localnodetypecache.end())
	  {
	    auto ntypitm = scan_node_expr(nodexp,insitm,depth,typitm,lvar);
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
 nonlvar:
  throw MOM_RUNTIME_PRINTF("expr %s in instr %s with type %s should be a left-var",
                           mom_value_cstring(expv),
                           mom_item_cstring(insitm),
                           mom_item_cstring(typitm));
} // end of MomEmitter::scan_expr



struct mom_item_st*
MomEmitter::scan_node_expr(const struct mom_boxnode_st*expnod, struct mom_item_st*insitm,
                           int depth, struct mom_item_st*typitm, bool lvar)
{
  MOM_DEBUGPRINTF(gencod, "scan_node_expr start expnod=%s insitm=%s depth#%d typitm=%s %s",
                  mom_value_cstring(expnod), mom_item_cstring(insitm), depth, mom_item_cstring(typitm),
                  lvar?"lvar":"expr");
  assert (expnod != nullptr && expnod->va_itype==MOMITY_NODE);
  auto connitm = expnod->nod_connitm;
  assert (connitm != nullptr && connitm->va_itype==MOMITY_ITEM);
  unsigned nodarity = mom_size(expnod);
  lock_item(connitm);
#define NBEXPCONN_MOM 131
#define CASE_EXPCONN_MOM(Nam) momhashpredef_##Nam % NBEXPCONN_MOM:	\
  if (connitm == MOM_PREDEFITM(Nam)) goto foundcaseconn_##Nam;		\
  goto defaultcaseconn; foundcaseconn_##Nam
  switch (connitm->hva_hash % NBEXPCONN_MOM)
    {
    case CASE_EXPCONN_MOM(verbatim):
      {
	if (lvar) goto nonlvar;
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
    case CASE_EXPCONN_MOM(sizeof):
      {
	const char *badmsg = "?";
	if (lvar) goto nonlvar;
	if (nodarity != 1)
	  throw MOM_RUNTIME_PRINTF("sizeof expr %s of bad arity in instr %s with type %s",
				   mom_value_cstring(expnod), mom_item_cstring(insitm),
				   mom_item_cstring(typitm));
	auto sztypitm = mom_dyncast_item(expnod->nod_sons[0]);
	vardef_st* sztybind = nullptr;
	MOM_DEBUGPRINTF(gencod, "sizeof expr %s with sztypitm %s",
			mom_value_cstring(expnod), mom_item_cstring(sztypitm));
	if (sztypitm == nullptr)
	  {
	    badmsg = "non-item arg";
	    goto badsizeof;
	  }
	lock_item(sztypitm);
	sztybind = get_binding(sztypitm);
	if (!sztybind)
	  {
	    MOM_DEBUGPRINTF(gencod, "sizeof expr %s with fresh sztypitm %s",
			    mom_value_cstring(expnod),
			    mom_item_cstring(sztypitm));
	    scan_type_item(sztypitm);
	    sztybind = get_binding(sztypitm);
	  }
	assert (sztybind != nullptr);
	MOM_DEBUGPRINTF(gencod, "sizeof expr %s sztypitm %s sztybind role %s what %s",
			mom_value_cstring(expnod),
			mom_item_cstring(sztypitm),
			mom_item_cstring(sztybind->vd_rolitm),
			mom_value_cstring(sztybind->vd_what));
	if (!is_type_binding(sztybind))
	  {
	    badmsg = "non-type arg";
	    goto badsizeof;
	  }
	MOM_DEBUGPRINTF(gencod, "sizeof expr %s typitm %s",
			mom_value_cstring(expnod),
			mom_item_cstring(typitm));
	if (typitm && typitm != MOM_PREDEFITM(int))
	  {
	    badmsg = "ill typed";
	    goto badsizeof;
	  }
	MOM_DEBUGPRINTF(gencod, "sizeof expr %s gives int", mom_value_cstring(expnod));
	return MOM_PREDEFITM(int);
      badsizeof:
	throw MOM_RUNTIME_PRINTF("bad sizeof expr %s in instr %s with type %s (%s)",
				 mom_value_cstring(expnod), mom_item_cstring(insitm),
				 mom_item_cstring(typitm), badmsg);

      }
      break;
    case CASE_EXPCONN_MOM(and):
      goto orcase;
    case CASE_EXPCONN_MOM(or):
    orcase:
      {
        if (lvar) goto nonlvar;
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
	if (lvar) goto nonlvar;
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
      ////
    case CASE_EXPCONN_MOM(get):
      {
	if (nodarity != 2)
	  throw MOM_RUNTIME_PRINTF("get expr %s in instr %s should be binary",
				   mom_value_cstring(expnod),
				   mom_item_cstring(insitm));
	auto ptrexpv = expnod->nod_sons[0];
	auto flditm = mom_dyncast_item(expnod->nod_sons[1]);
	MOM_DEBUGPRINTF(gencod, "scan get expr %s in instr %s ptrexpv=%s flditm=%s",
			mom_value_cstring(expnod), mom_item_cstring(insitm),
			mom_value_cstring(ptrexpv),
			mom_item_cstring(flditm));
	auto ptrtypitm = scan_expr(ptrexpv, insitm, depth+1);
	if (ptrtypitm != MOM_PREDEFITM(value) && ptrtypitm != MOM_PREDEFITM(item))
	  throw MOM_RUNTIME_PRINTF("in get expr %s in instr %s left son is not value or item but %s",
				   mom_value_cstring(expnod),
				   mom_item_cstring(insitm),
				   mom_item_cstring(ptrtypitm));
	if (flditm == nullptr)
	  throw MOM_RUNTIME_PRINTF("in get expr %s in instr %s bad right son",
				   mom_value_cstring(expnod),
				   mom_item_cstring(insitm));
	lock_item(flditm);
	auto fldbind = get_binding(flditm);
	if (!fldbind)
	  throw  MOM_RUNTIME_PRINTF("in get expr %s in instr %s unknown field %s",
				    mom_value_cstring(expnod),
				    mom_item_cstring(insitm),
				    mom_item_cstring(flditm));
	MOM_DEBUGPRINTF(gencod, "scan get expr %s in instr %s field %s role %s what %s detail %s",
			mom_value_cstring(expnod), mom_item_cstring(insitm),
			mom_item_cstring(flditm), mom_item_cstring(fldbind->vd_rolitm),
			mom_value_cstring(fldbind->vd_what),
			mom_value_cstring(fldbind->vd_detail));
	if (fldbind->vd_rolitm != MOM_PREDEFITM(field))
	  throw  MOM_RUNTIME_PRINTF("in get expr %s in instr %s bad field %s with role %s",
				    mom_value_cstring(expnod),
				    mom_item_cstring(insitm),
				    mom_item_cstring(flditm),
				    mom_item_cstring(fldbind->vd_rolitm));
	auto contypitm = mom_dyncast_item(fldbind->vd_what);
	assert (is_locked_item(contypitm));
	auto fldtypitm = mom_dyncast_item(fldbind->vd_detail);
	assert (is_locked_item(fldtypitm));
	MOM_DEBUGPRINTF(gencod, "scan get expr %s in instr %s gives fldtypitm %s",
			mom_value_cstring(expnod),
			mom_item_cstring(insitm),
			mom_item_cstring(fldtypitm));
	return fldtypitm;
      }
      break;
      ///
    case CASE_EXPCONN_MOM(at):
      {
	if (nodarity != 3)
	  throw MOM_RUNTIME_PRINTF("at expr %s in instr %s should be ternary",
				   mom_value_cstring(expnod),
				   mom_item_cstring(insitm));
	auto ptrexpv = expnod->nod_sons[0];
	auto flditm = mom_dyncast_item(expnod->nod_sons[1]);
	auto indexpv = expnod->nod_sons[2];
	MOM_DEBUGPRINTF(gencod, "scan at expr %s in instr %s ptrexpv=%s flditm=%s indexpv=%s",
			mom_value_cstring(expnod), mom_item_cstring(insitm),
			mom_value_cstring(ptrexpv),
			mom_item_cstring(flditm),
			mom_value_cstring(indexpv));
	auto ptrtypitm = scan_expr(ptrexpv, insitm, depth+1);
	if (ptrtypitm != MOM_PREDEFITM(value) && ptrtypitm != MOM_PREDEFITM(item))
	  throw MOM_RUNTIME_PRINTF("in at expr %s in instr %s first son is not value or item but %s",
				   mom_value_cstring(expnod),
				   mom_item_cstring(insitm),
				   mom_item_cstring(ptrtypitm));
	if (flditm == nullptr)
	  throw MOM_RUNTIME_PRINTF("in at expr %s in instr %s bad second son",
				   mom_value_cstring(expnod),
				   mom_item_cstring(insitm));
	lock_item(flditm);
	auto fldbind = get_binding(flditm);
	if (!fldbind)
	  throw MOM_RUNTIME_PRINTF("in at expr %s in instr %s unknown field %s",
				   mom_value_cstring(expnod),
				   mom_item_cstring(insitm),
				   mom_item_cstring(flditm));
	MOM_DEBUGPRINTF(gencod, "scan at expr %s in instr %s field %s role %s what %s",
			mom_value_cstring(expnod), mom_item_cstring(insitm),
			mom_item_cstring(flditm), mom_item_cstring(fldbind->vd_rolitm),
			mom_value_cstring(fldbind->vd_what));
	if (fldbind->vd_rolitm != MOM_PREDEFITM(field))
	  throw  MOM_RUNTIME_PRINTF("in at expr %s in instr %s bad field %s with role %s",
				    mom_value_cstring(expnod),
				    mom_item_cstring(insitm),
				    mom_item_cstring(flditm),
				    mom_item_cstring(fldbind->vd_rolitm));
	auto contypitm = mom_dyncast_item(fldbind->vd_what);
	auto fldtypv = (momvalue_t) (fldbind->vd_detail);
	MOM_DEBUGPRINTF(gencod, "scan at expr %s in instr %s has contypitm %s fldtypv %s",
			mom_value_cstring(expnod),
			mom_item_cstring(insitm),
			mom_item_cstring(contypitm),
			mom_value_cstring(fldtypv));
	auto fldtypnod = mom_dyncast_node(fldtypv);
	if (fldtypnod == nullptr
	    || (fldtypnod->nod_connitm != MOM_PREDEFITM(array)
		&& fldtypnod->nod_connitm != MOM_PREDEFITM(flexible_array)))
	  throw MOM_RUNTIME_PRINTF("at expr %s in instr %s with field %s non-array but %s",
				   mom_value_cstring(expnod),
				   mom_item_cstring(insitm),
				   mom_item_cstring(flditm),
				   mom_value_cstring(fldtypnod));
	auto elemtypitm = mom_dyncast_item(fldtypnod->nod_sons[0]);
	MOM_DEBUGPRINTF(gencod, "scan at expr %s in instr %s elemtypitm %s",
			mom_value_cstring(expnod), mom_item_cstring(insitm),
			mom_item_cstring(elemtypitm));
	if (!is_locked_item(elemtypitm))
	  throw MOM_RUNTIME_PRINTF("at expr %s in instr %s field %s has bad element type %s",
				   mom_value_cstring(expnod), mom_item_cstring(insitm),
				   mom_item_cstring(flditm),
				   mom_value_cstring(fldtypnod->nod_sons[0]));
	auto ixtypitm = scan_expr(indexpv, insitm, depth+1, MOM_PREDEFITM(int));
	if (ixtypitm != MOM_PREDEFITM(int))
	  throw MOM_RUNTIME_PRINTF("at expr %s in instr %s has non-int %s index third son",
				   mom_value_cstring(expnod),
				   mom_item_cstring(insitm),
				   mom_item_cstring(ixtypitm));
	MOM_DEBUGPRINTF(gencod, "scan at expr %s in instr %s gives elemtypitm %s",
			mom_value_cstring(expnod), mom_item_cstring(insitm),
			mom_item_cstring(elemtypitm));
	return elemtypitm;
      }
      break;
      ////
    case CASE_EXPCONN_MOM(plus):
      goto multcase;
    case CASE_EXPCONN_MOM(mult):
    multcase:
      {
        if (lvar) goto nonlvar;
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
        if (lvar) goto nonlvar;
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
	if (lvar) goto nonlvar;
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
        if (lvar) goto nonlvar;
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
    case CASE_EXPCONN_MOM(flatten_set):
      goto flatupcase;
    case CASE_EXPCONN_MOM(flatten_tuple):
    flatupcase:
      {
        if (lvar) goto nonlvar;
        bool isflatset = connitm==MOM_PREDEFITM(flatten_set);
        if (nodarity==0)
          throw MOM_RUNTIME_PRINTF("%s expr %s in %s should have at least one argument",
                                   isflatset?"flatten_set":"flatten_tuple",
                                   mom_value_cstring(expnod),
                                   mom_item_cstring(insitm));
        if (scan_expr(expnod->nod_sons[0], insitm, depth+1, MOM_PREDEFITM(item)) != MOM_PREDEFITM(item))
          throw MOM_RUNTIME_PRINTF("%s expr %s in %s starts with non-item",
                                   isflatset?"flatten_set":"flatten_tuple",
                                   mom_value_cstring(expnod),
                                   mom_item_cstring(insitm));
        for (unsigned ix=1; ix<nodarity; ix++)
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
        return scan_node_descr_conn_expr(expnod, desconnitm, insitm, depth, typitm, lvar);
      }
    } // end switch connitm
#undef NBEXPCONN_MOM
#undef CASE_EXPCONN_MOM
  MOM_FATAPRINTF("impossible scan_node_expr expnod=%s insitm=%s depth#%d typitm=%s",
                 mom_value_cstring(expnod), mom_item_cstring(insitm), depth, mom_item_cstring(typitm));
 nonlvar:
  throw MOM_RUNTIME_PRINTF("expr %s in instr %s depth %d is not an lvar for type %s",
                           mom_value_cstring(expnod), mom_item_cstring(insitm),
                           depth,
                           mom_item_cstring(typitm));
} // end of MomEmitter::scan_node_expr




struct mom_item_st*
MomEmitter::scan_node_descr_conn_expr(const struct mom_boxnode_st*expnod,
                                      struct mom_item_st*desconnitm,
                                      struct mom_item_st*insitm,
                                      int depth, struct mom_item_st*typitm,
                                      bool lvar)
{
  MOM_DEBUGPRINTF(gencod, "scan_node_descr_conn_expr start expnod=%s desconnitm=%s"
                  " insitm=%s depth#%d typitm=%s %s",
                  mom_value_cstring(expnod), mom_value_cstring(desconnitm),
                  mom_item_cstring(insitm), depth, mom_item_cstring(typitm),
                  lvar?"lvar":"expr");
  auto connitm = expnod->nod_connitm;
  assert (connitm != nullptr && connitm->va_itype==MOMITY_ITEM);
  unsigned nodarity = mom_size(expnod);
  assert (is_locked_item(connitm));
  assert (is_locked_item(desconnitm));
  MOM_DEBUGPRINTF(gencod, "scan_node_descr_conn_expr expnod=%s desconnitm=%s nodarity=%d",
                  mom_value_cstring(expnod),
                  mom_item_cstring(desconnitm), nodarity);
#define NBDESCONN_MOM 101
#define CASE_DESCONN_MOM(Nam) momhashpredef_##Nam % NBDESCONN_MOM:	\
  if (desconnitm == MOM_PREDEFITM(Nam)) goto foundesconn_##Nam;		\
  goto defaultdesconn; foundesconn_##Nam
  switch (desconnitm->hva_hash % NBDESCONN_MOM)
    {
    case CASE_DESCONN_MOM(signature):
      {
	// an unknown closure application
	if (lvar) goto nonlvar;
	auto sigr=scan_nonbinding_signature(connitm,insitm);
	auto sformaltup = sigr.sig_formals;
	auto sresultv = sigr.sig_result;
	MOM_DEBUGPRINTF(gencod,
			"scan_node_descr_conn_expr expnod=%s signature formtup=%s resultv=%s",
			mom_value_cstring(expnod),
			mom_value_cstring(sformaltup),
			mom_value_cstring(sresultv));
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
      /////
    case CASE_DESCONN_MOM(type):
      MOM_DEBUGPRINTF(gencod,
                      "scan_node_descr_conn_expr expnod=%s type cast %s",
                      mom_value_cstring(expnod), mom_item_cstring(connitm));
      if (lvar) goto nonlvar;
      if (nodarity != 1)
        throw MOM_RUNTIME_PRINTF("cast node %s should have one son, but has arity %d", mom_value_cstring(expnod), nodarity);
      return connitm;
      break;
      /////
    case CASE_DESCONN_MOM(routine):
      MOM_DEBUGPRINTF(gencod,
                      "scan_node_descr_conn_expr expnod=%s routine",
                      mom_value_cstring(expnod));
      goto primitivecase;
      ////
    case CASE_DESCONN_MOM(inline):
      MOM_DEBUGPRINTF(gencod,
                      "scan_node_descr_conn_expr expnod=%s inline",
                      mom_value_cstring(expnod));
      goto primitivecase;
      ////
    case CASE_DESCONN_MOM(primitive):
      MOM_DEBUGPRINTF(gencod,
                      "scan_node_descr_conn_expr expnod=%s primitive",
                      mom_value_cstring(expnod));
    primitivecase:
      {
        if (lvar) goto nonlvar;
        // a known routine or primitive application
        auto routsigitm =
          mom_dyncast_item(mom_unsync_item_get_phys_attr
                           (connitm,
                            MOM_PREDEFITM(signature)));
        MOM_DEBUGPRINTF(gencod,
                        "scan_node_descr_conn_expr expnod=%s desconnitm=%s routsigitm=%s",
                        mom_value_cstring(expnod),
                        mom_item_cstring(desconnitm),
                        mom_item_cstring(routsigitm));
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
            auto curtypitm = mom_dyncast_item(mom_unsync_item_get_phys_attr (curformalitm, MOM_PREDEFITM(type)));
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
        MOM_DEBUGPRINTF(gencod, "scan_node_descr_conn_expr expnod=%s gives restypitm=%s",
                        mom_value_cstring(expnod),
                        mom_item_cstring(restypitm));
        return restypitm;
      }
      break;
      ///
    case CASE_DESCONN_MOM(field):
      {
	auto fldbind = get_binding(connitm);
	auto typval = mom_unsync_item_get_phys_attr(connitm, MOM_PREDEFITM(type));
	if (!fldbind)
	  throw MOM_RUNTIME_PRINTF("undeclared field %s in expnod %s instr %s",
				   mom_item_cstring(connitm),
				   mom_value_cstring(expnod),
				   mom_item_cstring(insitm));
	MOM_DEBUGPRINTF(gencod,
			"scan_node_descr_conn_expr expnod=%s field bind role %s what %s typval=%s typitm=%s",
			mom_value_cstring(expnod), mom_item_cstring(fldbind->vd_rolitm),
			mom_value_cstring(fldbind->vd_what), mom_value_cstring(typval), mom_item_cstring(typitm));
	if (mom_itype(typval)==MOMITY_ITEM)
	  {
	    auto typvitm = (mom_item_st*)typval;
	    if (typvitm == typitm || !typitm)
	      {
		MOM_DEBUGPRINTF(gencod,
				"scan_node_descr_conn_expr expnod=%s gives typvitm=%s",
				mom_value_cstring(expnod), mom_item_cstring(typvitm));
		return typvitm;
	      }
	  }
#warning C-scan_node_descr_conn_expr unimplemented field
	MOM_FATAPRINTF("scan_node_descr_conn_expr unimplemented field expnod=%s", mom_value_cstring(expnod));
      }
      break;
      ////
    defaultdesconn:
      MOM_DEBUGPRINTF(gencod, "scan_node_descr_conn_expr expnod=%s default desconnitm=%s",
		      mom_value_cstring(expnod), mom_item_cstring(desconnitm));
      break;
    }
#undef CASE_DESCONN_MOM
#undef NBDESCONN_MOM
  throw  MOM_RUNTIME_PRINTF("expnod %s has unexpected connective %s of descr %s in instr %s depth %d",
                            mom_value_cstring(expnod), mom_item_cstring(connitm),
                            mom_item_cstring(desconnitm), mom_value_cstring(insitm), depth);
 nonlvar:
  throw  MOM_RUNTIME_PRINTF("expnod %s has unexpected connective %s of descr %s in instr %s depth %d is not lvar",
                            mom_value_cstring(expnod), mom_item_cstring(connitm),
                            mom_item_cstring(desconnitm), mom_value_cstring(insitm), depth);
} // end of MomEmitter::scan_node_descr_conn_expr



struct mom_item_st*
MomEmitter::scan_item_expr(struct mom_item_st*expitm, struct mom_item_st*insitm, int depth, struct mom_item_st*typitm, bool lvar)
{
  MOM_DEBUGPRINTF(gencod, "scan_item_expr start expitm=%s insitm=%s depth#%d typitm=%s %s",
                  mom_item_cstring(expitm), mom_item_cstring(insitm), depth, mom_item_cstring(typitm),
                  lvar?"lvar":"expr");
  MOM_DEBUGPRINTF(gencod, "scan_item_expr expitm:=%s",
                  mom_item_content_cstring(expitm));
  assert (is_locked_item(expitm));
  assert (is_locked_item(insitm));
  auto desitm =  mom_unsync_item_descr(expitm);
  auto bind = get_binding(expitm);
  vardef_st* oldbind = nullptr;
  struct mom_item_st* typexpitm = nullptr;
  MOM_DEBUGPRINTF(gencod, "scan_item_expr expitm %s desitm %s bind role %s what %s",
                  mom_item_cstring(expitm),
                  mom_item_cstring(desitm),
                  mom_item_cstring(bind?bind->vd_rolitm:NULL),
                  mom_value_cstring(bind?bind->vd_what:NULL));
  if (desitm == MOM_PREDEFITM(variable) || desitm == MOM_PREDEFITM(global)
      || desitm == MOM_PREDEFITM(thread_local) || desitm == MOM_PREDEFITM(formal))
    return scan_var(expitm,insitm,typitm);
  else if (desitm == MOM_PREDEFITM(closed))
    {
      if (lvar) goto nonlvar;
      if (typitm && typitm != MOM_PREDEFITM(value))
        throw MOM_RUNTIME_PRINTF("closed item %s in instr %s but expecting non-value type %s",
                                 mom_item_cstring(expitm),
                                 mom_item_cstring(insitm),
                                 mom_item_cstring(typitm));
      return scan_closed(expitm,insitm);
    }
  else if (desitm == MOM_PREDEFITM(constant))
    {
      if (lvar) goto nonlvar;
      return scan_constant_item(expitm,insitm,typitm);
    }
  else if (desitm == MOM_PREDEFITM(indirect))
    {
      auto indexpv = mom_unsync_item_get_phys_attr(expitm,MOM_PREDEFITM(indirect));
      MOM_DEBUGPRINTF(gencod, "scan_item_expr indirect expitm=%s indexpv=%s",
                      mom_item_cstring(expitm), mom_value_cstring(indexpv));
      if (indexpv == nullptr)
        throw MOM_RUNTIME_PRINTF("indirect item %s in instr %s without `indirect`", mom_item_cstring(expitm), mom_item_cstring(insitm));
      auto intypitm = scan_expr(indexpv, insitm, depth+1, typitm, lvar);
      MOM_DEBUGPRINTF(gencod, "scan_item_expr indirect expitm=%s gives intypitm=%s",
                      mom_item_cstring(expitm), mom_item_cstring(intypitm));
      return intypitm;
    }

  typexpitm =
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
  oldbind = get_binding(expitm);
  MOM_DEBUGPRINTF(gencod, "scan_item_expr expitm=%s typitm=%s typexpitm=%s oldbind role %s what %s",
                  mom_item_cstring(expitm), mom_item_cstring(typitm),
                  mom_item_cstring(typexpitm),
                  mom_item_cstring(oldbind?oldbind->vd_rolitm:nullptr),
                  mom_value_cstring(oldbind?oldbind->vd_what:nullptr));
  _ce_localvalueset.insert(expitm);
  if (oldbind==nullptr)
    {
      MOM_DEBUGPRINTF(gencod, "scan_item_expr expitm=%s localitem", mom_item_cstring(expitm));
      bind_local_at(expitm, MOM_PREDEFITM(item), expitm,__LINE__);
    }
  else
    {
      auto rolitm = oldbind->vd_rolitm;
      auto whatv = oldbind->vd_what;
      MOM_DEBUGPRINTF(gencod, "scan_item_expr expitm=%s rolitm=%s whatv=%s typitm=%s",
                      mom_item_cstring(expitm), mom_item_cstring(rolitm),
                      mom_value_cstring(whatv), mom_item_cstring(typitm));
#define NBKNOWNROLE_MOM 31
#define CASE_KNOWNROLE_MOM(Nam) momhashpredef_##Nam % NBKNOWNROLE_MOM:  \
      if (rolitm == MOM_PREDEFITM(Nam)) goto foundcase_##Nam;		\
      goto defaultcaserole; foundcase_##Nam
      switch (rolitm->hva_hash % NBKNOWNROLE_MOM)
        {
        case CASE_KNOWNROLE_MOM (enumerator):
	  {
	    assert (mom_itype(whatv) == MOMITY_NODE && mom_size(whatv)>=3);
	    auto roltypitm = mom_dyncast_item(mom_boxnode_nth(whatv, 0));
	    assert (mom_itype(roltypitm)==MOMITY_ITEM);
	    if (!typitm || roltypitm == typitm)
	      {
		MOM_DEBUGPRINTF(gencod, "scan_item_expr expitm=%s gives enum roltypitm=%s",
				mom_item_cstring(expitm), mom_item_cstring(roltypitm));
		return roltypitm;
	      }
	    else
	      throw MOM_RUNTIME_PRINTF("expitm %s is enumerator of type %s, but expecting %s",
				       mom_item_cstring(expitm),
				       mom_item_cstring(roltypitm),
				       mom_item_cstring(typitm));
	    // @@@ we should handle extended enum-s...
	    goto defaultcaserole;
	  }
	  break;
        default:
	defaultcaserole:
#warning incomplete scan_item_expr with oldbind
          MOM_FATAPRINTF("unimplemented scan_item_expr expitm=%s with oldbind rolitm=%s whatv=%s",
                         mom_item_cstring(expitm), mom_item_cstring(rolitm),
                         mom_value_cstring(whatv));
        }
#undef NBKNOWNROLE_MOM
#undef CASE_KNOWNROLE_MOM
    }
  if (typitm == MOM_PREDEFITM(item))
    return typitm;
  else if (typitm == nullptr || typitm == MOM_PREDEFITM(value))
    return MOM_PREDEFITM(value);
  else
    throw  MOM_RUNTIME_PRINTF("item %s in instr %s, expecting type %s",
                              mom_item_cstring(expitm),
                              mom_item_cstring(insitm),
                              mom_item_cstring(typitm));
 nonlvar:
  if (lvar)
    throw MOM_RUNTIME_PRINTF("item %s in instr %s (type %s) is not lvar",
                             mom_item_cstring(expitm),
                             mom_item_cstring(insitm),
                             mom_item_cstring(typitm));
  MOM_FATAPRINTF("unexpected item %s in instr %s (type %s)",
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
  auto typvarv =
    mom_unsync_item_get_phys_attr(varitm,MOM_PREDEFITM(type));
  if (!typvarv)
    throw  MOM_RUNTIME_PRINTF("variable %s in instruction %s without `type`",
                              mom_item_cstring(varitm), mom_item_cstring(insitm));
  MOM_DEBUGPRINTF(gencod, "scan_var varitm=%s insitm=%s typitm=%s typvarv=%s",
                  mom_item_cstring(varitm), mom_item_cstring(insitm), mom_item_cstring(typitm), mom_value_cstring(typvarv));
  scan_type_expr(typvarv, varitm);
  auto typvaritm = mom_dyncast_item(typvarv);
  if (!typitm)
    typitm = typvaritm;
  else if (typitm != typvaritm)
    {
      MOM_DEBUGPRINTF(gencod, "scan_var illtyped varitm:=%s\n .. insitm:=%s\n .. typitm:=%s\n.. typvaritm=%s",
                      mom_item_content_cstring(varitm), mom_item_content_cstring(insitm), mom_item_content_cstring(typitm),
                      mom_item_content_cstring(typvaritm));
      throw  MOM_RUNTIME_PRINTF("variable %s in instruction %s has `type`:%s incompatible with %s",
                                mom_item_cstring(varitm), mom_item_cstring(insitm),
                                mom_item_cstring(typvaritm), mom_item_cstring(typitm));
    }
  MOM_DEBUGPRINTF(gencod, "scan_var end varitm=%s type %s desvaritm=%s",
                  mom_item_cstring(varitm), mom_item_cstring(typitm), mom_item_cstring(desvaritm));
#define NBVARDESC_MOM 43
#define CASE_VARDESCR_MOM(Nam) momhashpredef_##Nam % NBVARDESC_MOM:	\
  if (desvaritm == MOM_PREDEFITM(Nam)) goto foundcase_##Nam;		\
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
	    bind_global_at(varitm, MOM_PREDEFITM(global), typitm, __LINE__);
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
	    bind_global_at(varitm, MOM_PREDEFITM(thread_local), typitm, __LINE__);
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
MomEmitter::scan_constant_item(struct mom_item_st*cstitm, struct mom_item_st*insitm, struct mom_item_st*typitm)
{
  MOM_DEBUGPRINTF(gencod, "scan_constant_item start cstitm:=%s\n.. insitm=%s typitm=%s",
                  mom_item_content_cstring(cstitm),
                  mom_item_cstring(insitm), mom_item_cstring(typitm));
  assert (is_locked_item(cstitm));
  assert(mom_unsync_item_descr(cstitm)==MOM_PREDEFITM(constant));
  if (typitm == MOM_PREDEFITM(bool) && cstitm==MOM_PREDEFITM(truth))
    {
      MOM_DEBUGPRINTF(gencod, "scan_constant_item special truth bool");
      return MOM_PREDEFITM(bool);
    }
  auto cstypv = mom_unsync_item_get_phys_attr(cstitm, MOM_PREDEFITM(type));
  auto cstypitm = mom_dyncast_item(cstypv);
  if (cstypitm == nullptr)
    cstypitm = MOM_PREDEFITM(value);
  if (typitm != nullptr && typitm != cstypitm)
    throw MOM_RUNTIME_PRINTF("constant %s in instruction %s has type %s but expecting %s",
                             mom_item_cstring(cstitm), mom_item_cstring(insitm),
                             mom_item_cstring(cstypitm),
                             mom_item_cstring(typitm));
  MOM_DEBUGPRINTF(gencod, "scan_constant_item cstitm=%s insitm=%s cstypitm=%s",
                  mom_item_cstring(cstitm), mom_item_cstring(insitm), mom_item_cstring(cstypitm));
  bind_global_at(cstitm, MOM_PREDEFITM(constant), cstypitm, __LINE__);
  _ce_constitems.insert(cstitm);
  return cstypitm;
} // end MomEmitter::scan_constant_item


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
MomEmitter::scan_routine_element(struct mom_item_st*rtitm, bool ignorebody)
{
  MOM_DEBUGPRINTF(gencod, "scan_routine_element start %s rtitm:=%s",
                  ignorebody?"ignorebody":"scanbody",
                  mom_item_content_cstring(rtitm));
  _ce_curfunctionitm = rtitm;
  assert (is_locked_item(rtitm));
  auto descitm = mom_unsync_item_descr(rtitm);
  assert (descitm == MOM_PREDEFITM(routine) || descitm==MOM_PREDEFITM(inline));
  bind_global_at(descitm,descitm,rtitm,__LINE__);
  bind_global_at(rtitm,descitm,rtitm,__LINE__);
  auto sigitm = mom_dyncast_item(mom_unsync_item_get_phys_attr (rtitm, MOM_PREDEFITM(signature)));
  auto bdyitm = mom_dyncast_item(mom_unsync_item_get_phys_attr (rtitm, MOM_PREDEFITM(body)));
  MOM_DEBUGPRINTF(gencod, "scan_routine_element rtitm=%s sigitm=%s bdyitm=%s",
                  mom_item_cstring(rtitm), mom_item_cstring(sigitm), mom_item_cstring(bdyitm));
  if (sigitm == nullptr)
    throw MOM_RUNTIME_PRINTF("missing signature in routine %s", mom_item_cstring(rtitm));
  lock_item(sigitm);
  {
    auto sigr=scan_signature(sigitm,rtitm);
    _ce_cursigdef = sigr;
    auto formaltup = sigr.sig_formals;
    auto restypv = sigr.sig_result;
    MOM_DEBUGPRINTF(gencod, "scan_routine_element rtitm=%s formaltup=%s restypv=%s",
                    mom_item_cstring(rtitm), mom_value_cstring(formaltup), mom_value_cstring(restypv));
  }
  flush_todo_list(__LINE__);
  if (!ignorebody)
    {
      MOM_DEBUGPRINTF(gencod, "scan_routine_element rtitm=%s sigitm=%s doing bdyitm=%s",
                      mom_item_cstring(rtitm), mom_item_cstring(sigitm), mom_item_cstring(bdyitm));
      if (bdyitm == nullptr)
        throw MOM_RUNTIME_PRINTF("missing or bad body in routine %s", mom_item_cstring(rtitm));
      lock_item(bdyitm);
      scan_block(bdyitm,rtitm);
    };
  _ce_curfunctionitm = nullptr;
  MOM_DEBUGPRINTF(gencod, "scan_routine_element done rtitm=%s\n", mom_item_cstring(rtitm));
} // end  MomEmitter::scan_routine_element



////////////////
MomEmitter::~MomEmitter()
{
  if (!_ce_failing)
    for (auto ef : _ce_doatendque)
      {
        ef(this);
      }
  _ce_doatendque.clear();
  int nbit = _ce_veclockeditems.size();
  for (int ix=nbit-1; ix>=0; ix--)
    mom_item_unlock(_ce_veclockeditems[ix]);
  _ce_veclockeditems.clear();
  _ce_setlockeditems.clear();
  _ce_topitm = nullptr;
  _ce_emitteritm = nullptr;
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
  _ce_cursigdef = sigdef_st {nullptr,nullptr};
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
	fputs(((const struct mom_boxstring_st*)val)->boxs_cstr, out);
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
  if (connitm == MOM_PREDEFITM(Nam)) goto foundcase_##Nam;		\
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
        auto str = ((const struct mom_boxstring_st*)sonv)->boxs_cstr;
        auto ln = mom_raw_size(sonv);
        if (!verb) fputs(" \"", out);
        mom_output_utf8_encoded (out, str, ln);
        if (!verb) fputs("\"", out);
      }
    break;
    case CASE_NODECONN_MOM(comment):
      {
	fputs("/**", out);
	for (int i=0; i<(int)arity; i++)
	  {
	    auto sonv = nod->nod_sons[i];
	    if (mom_itype(sonv) == MOMITY_BOXSTRING)
	      {
		auto strson = (const mom_boxstring_st*)sonv;
		if (strstr(strson->boxs_cstr, "*/") || strchr(strson->boxs_cstr, '\n'))
		  throw MOM_RUNTIME_PRINTF("bad string son in comment: %s",
					   mom_value_cstring(nod));
		else
		  fputs(strson->boxs_cstr, out);
	      }
	    else
	      {
		write_tree(out, depth+1, lastnl, sonv, forv);
		write_nl_or_space(out,depth,lastnl);
	      }
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
    case CASE_NODECONN_MOM(out_newline):
      {
	write_indented_newline(out,depth,lastnl);
	for (int ix=0; ix<(int)arity; ix++)
	  {
	    write_tree(out, depth+1, lastnl, nod->nod_sons[ix], forv);
	  }
      }
      break;
    case CASE_NODECONN_MOM(output):
      {
	write_nl_or_space(out,depth,lastnl);
	for (int ix=0; ix<(int)arity; ix++)
	  {
	    write_tree(out, depth+1, lastnl, nod->nod_sons[ix], forv);
	    write_nl_or_space(out,depth,lastnl);
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
  _cec_globdecltree.clear();
  _cec_globdefintree.clear();
  _cec_declareditems.clear();
} // end MomCEmitter::~MomCEmitter



void
MomCEmitter::declare_item(struct mom_item_st*declitm)
{
  MOM_DEBUGPRINTF(gencod, "c-declare_item start declitm:=%s", mom_item_content_cstring(declitm));
  lock_item(declitm);
  auto descitm = mom_unsync_item_descr(declitm);
  if (!descitm)
    throw MOM_RUNTIME_PRINTF("declared item %s without descr", mom_item_cstring(declitm));
#define NBDECLD_MOM 91
#define CASE_DECLD_MOM(Nam) momhashpredef_##Nam % NBDECLD_MOM:  \
  if (descitm == MOM_PREDEFITM(Nam)) goto foundcase_##Nam;	\
  goto defaultcasedescd; foundcase_##Nam
  const char*varqual=nullptr;
  switch (descitm->hva_hash % NBDECLD_MOM)
    {
    case CASE_DECLD_MOM(type):
      {
	scan_type_item(declitm);
	MOM_DEBUGPRINTF(gencod, "c-declare_item type %s", mom_item_cstring(declitm));
	auto dtytree = mom_dyncast_node(declare_type(declitm));
	MOM_DEBUGPRINTF(gencod, "c-declare_item type %s dtytree=%s",
			mom_item_cstring(declitm), mom_value_cstring(dtytree));
	if (_cec_declareditems.find(declitm) == _cec_declareditems.end())
	  {
	    MOM_DEBUGPRINTF(gencod, "c-declare_item new type %s", mom_item_cstring(declitm));
	    _cec_declareditems.insert(declitm);
	    add_global_decl(dtytree);
	  }
      }
      break;
    case CASE_DECLD_MOM(signature):
      {
	scan_signature(declitm,nullptr);
	MOM_DEBUGPRINTF(gencod, "c-declare_item signature %s", mom_item_cstring(declitm));
	auto dsigtree= mom_dyncast_node(declare_signature_type(declitm));
	MOM_DEBUGPRINTF(gencod, "c-declare_item signature %s dsigtree=%s",
			mom_item_cstring(declitm), mom_value_cstring(dsigtree));
	if (_cec_declareditems.find(declitm) == _cec_declareditems.end())
	  {
	    _cec_declareditems.insert(declitm);
	    add_global_decl(dsigtree);
	  }
      }
      break;
    case CASE_DECLD_MOM(func):
      {
	scan_func_element(declitm);
	MOM_DEBUGPRINTF(gencod, "c-declare_item func %s", mom_item_cstring(declitm));
	auto sigitm = mom_dyncast_item(mom_unsync_item_get_phys_attr (declitm, MOM_PREDEFITM(signature)));
	if (_cec_declareditems.find(sigitm) == _cec_declareditems.end())
	  {
	    _cec_declareditems.insert(sigitm);
	    auto dsigtree= mom_dyncast_node(declare_signature_type(declitm));
	    MOM_DEBUGPRINTF(gencod, "c-declare_item func %s sigitm=%s dsigtree=%s",
			    mom_item_cstring(declitm), mom_item_cstring(sigitm), mom_value_cstring(dsigtree));
	    add_global_decl(dsigtree);
	  }
	if (_cec_declareditems.find(declitm) == _cec_declareditems.end())
	  {
	    _cec_declareditems.insert(declitm);
	    auto dftree =
	      mom_boxnode_make_sentinel(MOM_PREDEFITM(sequence),
					literal_string("extern"),
					literal_string(" "),
					literal_string(CSIGNTYPE_PREFIX),
					sigitm,
					literal_string(" "),
					literal_string(MOM_FUNC_PREFIX),
					declitm,
					literal_string(";"));
	    MOM_DEBUGPRINTF(gencod, "c-declare_item func %s dftree %s", mom_item_cstring(declitm),
			    mom_value_cstring(dftree));
	    add_global_decl(dftree);
	  }
      }
      break;
    case CASE_DECLD_MOM(inline):
      {
	MOM_DEBUGPRINTF(gencod, "c-declare_item inline %s", mom_item_cstring(declitm));
	auto sigitm = mom_dyncast_item(mom_unsync_item_get_phys_attr (declitm, MOM_PREDEFITM(signature)));
	if (_cec_declareditems.find(sigitm) == _cec_declareditems.end())
	  {
	    _cec_declareditems.insert(sigitm);
	    auto dsigtree= mom_dyncast_node(declare_signature_type(sigitm));
	    MOM_DEBUGPRINTF(gencod, "c-declare_item inline %s sigitm=%s dsigtree=%s",
			    mom_item_cstring(declitm), mom_item_cstring(sigitm), mom_value_cstring(dsigtree));
	    add_global_decl(dsigtree);
	  }
	if (_cec_declareditems.find(declitm) == _cec_declareditems.end())
	  {
	    auto iltree = transform_inline_element(declitm);
	    MOM_DEBUGPRINTF(gencod, "c-declare_item inline %s iltree %s",
			    mom_item_cstring(declitm), mom_value_cstring(iltree));
	    add_global_decl(iltree);
	    _cec_declareditems.insert(declitm);
	  }
      }
      break;
    case CASE_DECLD_MOM(routine):
      {
	scan_routine_element(declitm, true);
	MOM_DEBUGPRINTF(gencod, "c-declare_item routine %s", mom_item_cstring(declitm));
	auto sigitm = mom_dyncast_item(mom_unsync_item_get_phys_attr (declitm, MOM_PREDEFITM(signature)));
	if (!sigitm)
	  throw MOM_RUNTIME_PRINTF("c-declare_item routine %s without signature",
				   mom_item_cstring(declitm));
	lock_item(sigitm);
	scan_signature(sigitm, declitm, true);
	if (_cec_declareditems.find(sigitm) == _cec_declareditems.end())
	  {
	    _cec_declareditems.insert(sigitm);
	    auto dsigtree= mom_dyncast_node(declare_signature_type(sigitm));
	    MOM_DEBUGPRINTF(gencod, "c-declare_item routine %s sigitm=%s dsigtree=%s",
			    mom_item_cstring(declitm), mom_item_cstring(sigitm), mom_value_cstring(dsigtree));
	    add_global_decl(dsigtree);
	  }
	if (_cec_declareditems.find(declitm) == _cec_declareditems.end())
	  {
	    auto dftree =
	      mom_boxnode_make_sentinel(MOM_PREDEFITM(sequence),
					literal_string("extern"),
					literal_string(" "),
					literal_string(CSIGNTYPE_PREFIX),
					sigitm,
					literal_string(" "),
					literal_string(MOM_FUNC_PREFIX),
					declitm,
					literal_string(";"));
	    MOM_DEBUGPRINTF(gencod, "c-declare_item func %s dftree %s", mom_item_cstring(declitm),
			    mom_value_cstring(dftree));
	    add_global_decl(dftree);
	    _cec_declareditems.insert(declitm);
	  }
      }
      break;

    case CASE_DECLD_MOM(global):
      varqual = "/*global*/";
      goto declare_data;
    case CASE_DECLD_MOM(thread_local):
      varqual = "thread_local";
      goto declare_data;
    declare_data:
      {
        MOM_DEBUGPRINTF(gencod, "c-declare_item data %s varqual:%s", mom_item_cstring(declitm), varqual);
        if (_cec_declareditems.find(declitm) == _cec_declareditems.end())
          {
            MOM_DEBUGPRINTF(gencod, "c-declare_item new data %s varqual:%s", mom_item_cstring(declitm), varqual);
            _cec_declareditems.insert(declitm);
            auto typval = mom_unsync_item_get_phys_attr(declitm, MOM_PREDEFITM(type));
            if (typval == nullptr)
              throw MOM_RUNTIME_PRINTF("declared %s data %s without type",
                                       varqual, mom_item_cstring(declitm));
            auto vartree = mom_boxnode_make_va(MOM_PREDEFITM(sequence), 2,
                                               literal_string(CDATA_PREFIX),
                                               declitm);
            bind_global_at(declitm,descitm,typval,__LINE__);
            auto dtree = transform_type_for(typval,vartree);
            MOM_DEBUGPRINTF(gencod, "c-declare_item data %s dtree=%s", mom_item_cstring(declitm), mom_value_cstring(dtree));
            auto decltree = mom_boxnode_make_va(MOM_PREDEFITM(sequence),6,
                                                literal_string("extern"),
                                                literal_string(" "),
                                                literal_string(varqual),
                                                literal_string(" "),
                                                dtree,
                                                literal_string(";"));
            MOM_DEBUGPRINTF(gencod, "c-declare_item declitm %s with decltree %s",
                            mom_item_cstring(declitm), mom_value_cstring(decltree));
            add_global_decl(decltree);
          }
      }
      break;
    defaultcasedescd:
    default:
      throw MOM_RUNTIME_PRINTF("declared item %s with strange descr %s",
                               mom_item_cstring(declitm), mom_item_cstring(descitm));
    }
#undef CASE_DECLD_MOM
#undef NBDECLD_MOM
  MOM_DEBUGPRINTF(gencod, "c-declare_item end declitm=%s\n", mom_item_cstring(declitm));
} // end MomCEmitter::declare_item


void
MomCEmitter::after_preparation_transform(void)
{
  MOM_DEBUGPRINTF(gencod, "c-after_preparation_transform start topitm %s %zd constitems",
                  mom_item_cstring(top_item()),
                  _ce_constitems.size());
  auto declv = mom_unsync_item_get_phys_attr(top_item(), MOM_PREDEFITM(declare));
  auto declseq = mom_dyncast_seqitems(declv);
  if (declv != nullptr && !declseq)
    throw MOM_RUNTIME_PRINTF("invalid `declare` %s in c-module %s",
                             mom_value_cstring(declv), mom_item_cstring(top_item()));
  if (declseq)
    {
      unsigned nbdecl = mom_size(declseq);
      for (unsigned dix=0; dix<nbdecl; dix++)
        {
          auto declitm = mom_dyncast_item(declseq->seqitem[dix]);
          MOM_DEBUGPRINTF(gencod, "c-after_preparation_transform dix#%d declitm:=%s",
                          dix, mom_item_content_cstring(declitm));
          declare_item(declitm);
	  _ce_localvarmap.clear();
        }
    }
  for (const mom_item_st* citm : _ce_constitems)
    {
      MOM_DEBUGPRINTF(gencod, "c-after_preparation_transform citm=%s", mom_item_cstring(citm));
      if (mom_item_space((mom_item_st*)citm) == MOMSPA_PREDEF)
        {
          auto ctree = mom_boxnode_make_va(MOM_PREDEFITM(comment), 2,
                                           literal_string("predefined "),
                                           citm);
          MOM_DEBUGPRINTF(gencod, "c-after_preparation_transform predef ctree=%s", mom_value_cstring(ctree));
          add_global_decl(ctree);
        }
      else
        {
          auto dtree = mom_boxnode_make_va(MOM_PREDEFITM(sequence), 4,
                                           literal_string("static momitem_st* "),
                                           literal_string(CCONSTITEM_PREFIX),
                                           citm,
                                           literal_string(";"));
          MOM_DEBUGPRINTF(gencod, "c-after_preparation_transform constitm dtree=%s", mom_value_cstring(dtree));
          add_global_decl(dtree);
        }
      _ce_localvarmap.clear();
    }
  MOM_DEBUGPRINTF(gencod, "c-after_preparation_transform topitm %s %zd typeitems",
                  mom_value_cstring(top_item()),
                  _ce_typitems.size());
  for (const mom_item_st* typitm : _ce_typitems)
    {
      MOM_DEBUGPRINTF(gencod, "c-after_preparation_transform typitm=%s",
                      mom_item_cstring(typitm));
    }
  MOM_DEBUGPRINTF(gencod, "c-after_preparation_transform end topitm %s\n",
                  mom_value_cstring(top_item()));
} // end of MomCEmitter::after_preparation_transform


const struct mom_boxnode_st*
MomCEmitter::transform_data_element(struct mom_item_st*itm)
{
  MOM_DEBUGPRINTF(gencod, "c-transform_data_element start itm:=%s",
                  mom_item_content_cstring(itm));
  assert (is_locked_item(itm));
  auto descitm = mom_unsync_item_descr(itm);
  auto dabind = get_binding(itm);
  assert (dabind != nullptr);
  MOM_DEBUGPRINTF(gencod, "c-transform_data_element itm=%s dabind role %s what %s descitm=%s",
                  mom_item_cstring(itm),
                  mom_item_cstring(dabind->vd_rolitm),
                  mom_value_cstring(dabind->vd_what),
                  mom_item_cstring(descitm));
  assert (dabind->vd_rolitm == MOM_PREDEFITM(data));
  auto typval = dabind->vd_what;
  auto datree = mom_boxnode_make_va(MOM_PREDEFITM(sequence), 2,
                                    literal_string (CDATA_PREFIX), itm);
  MOM_DEBUGPRINTF(gencod, "c-transform_data_element itm=%s datree=%s",
                  mom_item_cstring(itm), mom_value_cstring(datree));
  auto dcltree = transform_type_for(typval,datree);
  MOM_DEBUGPRINTF(gencod, "c-transform_data_element itm=%s has dcltree=%s",
                  mom_item_cstring(itm), mom_value_cstring(dcltree));
  if (descitm == MOM_PREDEFITM(global))
    {
      auto restree = mom_boxnode_make_va(MOM_PREDEFITM(sequence),4,
                                         literal_string("/*global*/"),
                                         literal_string(" "),
                                         dcltree,
                                         literal_string(";"));
      MOM_DEBUGPRINTF(gencod, "c-transform_data_element itm=%s gives restree=%s",
                      mom_item_cstring(itm), mom_value_cstring(restree));
      return restree;
    }
  else if (descitm== MOM_PREDEFITM(thread_local))
    {
      auto restree = mom_boxnode_make_va(MOM_PREDEFITM(sequence),4,
                                         literal_string("thread_local"),
                                         literal_string(" "),
                                         dcltree,
                                         literal_string(";"));
      MOM_DEBUGPRINTF(gencod, "c-transform_data_element itm=%s gives restree=%s",
                      mom_item_cstring(itm), mom_value_cstring(restree));
      return restree;
    }
  else
    MOM_FATAPRINTF("unimplemented c-transform_data_element itm=%s of descr %s",
                   mom_item_cstring(itm), mom_item_cstring(descitm));
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
  MOM_DEBUGPRINTF(gencod, "c-declare_signature_type start sigitm:=%s",
                  mom_item_content_cstring(sigitm));
  auto formtup = mom_dyncast_tuple(mom_unsync_item_get_phys_attr (sigitm, MOM_PREDEFITM(formals)));
  if (formtup == nullptr)
    throw MOM_RUNTIME_PRINTF("signature %s without formals", mom_item_cstring(sigitm));
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
  auto restytree = (restyitm==MOM_PREDEFITM(unit))?literal_string("/*unitres*/ void"):declare_type(restyitm);
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
MomCEmitter::declare_field (struct mom_item_st*flditm, struct mom_item_st*fromitm, int rank)
{
  MOM_DEBUGPRINTF(gencod, "c-declare_field start flditm:=%s\n.. fromitm=%s rank#%d",
                  mom_item_content_cstring(flditm),
                  mom_item_cstring(fromitm), rank);
  assert (is_locked_item(flditm));
  if (mom_unsync_item_descr(flditm) != MOM_PREDEFITM(field))
    throw MOM_RUNTIME_PRINTF("%s is not a field (from %s)",
                             mom_item_cstring(flditm), mom_item_cstring(fromitm));
  auto typval = mom_unsync_item_get_phys_attr(flditm, MOM_PREDEFITM(type));
  MOM_DEBUGPRINTF(gencod, "c-declare_field flditm=%s has typval=%s",
                  mom_item_cstring(flditm), mom_value_cstring(typval));
  if (typval == nullptr)
    throw  MOM_RUNTIME_PRINTF("field %s (from %s) without type",
                              mom_item_cstring(flditm), mom_item_cstring(fromitm));
  if (fromitm != nullptr)
    {
      auto fbnd = get_binding(flditm);
      if (fbnd)
        throw MOM_RUNTIME_PRINTF("field %s #%d from %s is already bound to role %s what %s",
                                 mom_item_cstring(flditm),
                                 rank, mom_item_cstring(fromitm),
                                 mom_item_cstring(fbnd->vd_rolitm),
                                 mom_value_cstring(fbnd->vd_what));
      bind_global_at(flditm, MOM_PREDEFITM(field), fromitm, __LINE__, typval, rank);
      do_at_end([=](MomEmitter*em)
		{
		  assert (em != nullptr);
		  MOM_DEBUGPRINTF(gencod, "atend flditm=%s at #%d",
				  mom_item_cstring(flditm), rank);
		  mom_unsync_item_put_phys_attr(flditm, MOM_PREDEFITM(at),
						mom_int_make(rank));
		});
    }
  auto fldtree = mom_boxnode_make_va(MOM_PREDEFITM(sequence), 2,
                                     literal_string (CFIELD_PREFIX), flditm);
  MOM_DEBUGPRINTF(gencod, "c-declare_field flditm=%s fldtree=%s",
                  mom_item_cstring(flditm), mom_value_cstring(fldtree));
  auto dcltree = transform_type_for(typval,fldtree);
  MOM_DEBUGPRINTF(gencod, "c-declare_field flditm=%s gives dcltree=%s",
                  mom_item_cstring(flditm), mom_value_cstring(dcltree));
  return dcltree;
} // end of MomCEmitter::declare_field



momvalue_t
MomCEmitter::declare_struct_member (struct mom_item_st*memitm, struct mom_item_st*fromitm, int rank)
{
  MOM_DEBUGPRINTF(gencod, "c-declare_struct_member start memitm:=%s\n.. fromitm=%s rank#%d",
                  mom_item_content_cstring(memitm),
                  mom_item_cstring(fromitm), rank);
  assert (is_locked_item(memitm));
  auto membind = get_binding(memitm);
  if (membind != nullptr && fromitm != nullptr)
    throw MOM_RUNTIME_PRINTF("struct member %s from %s rk#%d already bound to role %s what %s",
                             mom_item_cstring(memitm), mom_item_cstring(fromitm), rank,
                             mom_item_cstring(membind->vd_rolitm),
                             mom_value_cstring(membind->vd_what));

  auto descitm = mom_unsync_item_descr(memitm);
  if (descitm == MOM_PREDEFITM(field))
    {
      auto fldtree = declare_field(memitm,fromitm,rank);
      MOM_DEBUGPRINTF(gencod, "c-declare_struct_member field memitm=%s gives %s",
                      mom_item_cstring(memitm), mom_value_cstring(fldtree));
      return fldtree;
    }
  else if (descitm == MOM_PREDEFITM(union))
    {
      auto ufldseq = mom_dyncast_seqitems(mom_unsync_item_get_phys_attr(memitm, MOM_PREDEFITM(union)));
      auto ufldlen = mom_size(ufldseq);
      if (ufldlen == 0)
        throw MOM_RUNTIME_PRINTF("bad or empty union in struct member %s from %s rk#%d",
                                 mom_item_cstring(memitm), mom_item_cstring(fromitm), rank);
      if (fromitm != nullptr)
        {
          bind_global_at(memitm, MOM_PREDEFITM(type),
                         mom_boxnode_make_va(MOM_PREDEFITM(union), 2,
                                             fromitm, mom_int_make(rank)),
                         __LINE__);
          do_at_end([=](MomEmitter*em)
		    {
		      assert (em != nullptr);
		      MOM_DEBUGPRINTF(gencod, "atend union %s at rank %d",
				      mom_item_cstring(memitm), rank);
		      mom_unsync_item_put_phys_attr(memitm, MOM_PREDEFITM(at),
						    mom_int_make(rank));
		    });
        }
      traced_vector_values_t vecomptree;
      vecomptree.reserve(ufldlen);
      for (int uix=0; uix<(int)ufldlen; uix++)
        {
          auto curfitm = ufldseq->seqitem[uix];
          MOM_DEBUGPRINTF(gencod, "c-declare_struct_member union memitm=%s uix#%d\n.. curfitm:=%s",
                          mom_item_cstring(memitm), uix, mom_item_content_cstring(curfitm));
          if (curfitm == nullptr)
            throw
	      MOM_RUNTIME_PRINTF("member %s from %s rk#%d is union with null #%d component",
				 mom_item_cstring(memitm), mom_item_cstring(fromitm), rank, uix);
          lock_item(curfitm);
          MOM_DEBUGPRINTF(gencod, "c-declare_struct_member union memitm=%s uix#%d curfitm=%s",
                          mom_item_cstring(memitm), uix, mom_item_cstring(curfitm));
          auto curftree = declare_field(curfitm, fromitm, rank);
          MOM_DEBUGPRINTF(gencod, "c-declare_struct_member curfitm=%s uix#%d curftree=%s",
                          mom_item_cstring(curfitm), uix, mom_value_cstring(curftree));
          vecomptree.push_back(curftree);
          MOM_DEBUGPRINTF(gencod, "c-declare_struct_member union memitm=%s uix#%d done curfitm=%s",
                          mom_item_cstring(memitm), uix, mom_item_cstring(curfitm));
        }
      vecomptree.push_back(literal_string(";"));
      auto bractree = //
        mom_boxnode_make_va (MOM_PREDEFITM(brace), 1,
                             mom_boxnode_make(MOM_PREDEFITM(semicolon),vecomptree.size(),vecomptree.data()));
      auto unitree =
        mom_boxnode_make_sentinel(MOM_PREDEFITM(sequence),
                                  literal_string("union"),
                                  literal_string(" /*"),
                                  memitm,
                                  literal_string("*/ "),
                                  bractree);
      MOM_DEBUGPRINTF(gencod, "c-declare_struct_member memitm=%s from %s rank %d gives unitree=%s",
                      mom_item_cstring(memitm), mom_item_cstring(fromitm), rank,
                      mom_value_cstring(unitree));
      return unitree;
    }
  else
    throw MOM_RUNTIME_PRINTF("member %s from %s rk#%d has bad descr %s",
                             mom_item_cstring(memitm), mom_item_cstring(fromitm), rank,
                             mom_item_cstring(descitm));
} // end of MomCEmitter::declare_struct_member



////////////////
momvalue_t
MomCEmitter::declare_enumerator(struct mom_item_st*enuritm,  struct mom_item_st*fromitm,
                                int rank, int& preval, struct mom_item_st* initm)
{
  MOM_DEBUGPRINTF(gencod, "c-declare_enumerator start enuritm=%s fromitm=%s rank#%d old preval=%d initm=%s",
                  mom_item_cstring(enuritm), mom_item_cstring(fromitm),
                  rank, preval, mom_value_cstring(initm));
  assert (is_locked_item(enuritm));
  if (mom_unsync_item_descr(enuritm) != MOM_PREDEFITM(enumerator))
    throw MOM_RUNTIME_PRINTF("in enum %s enumerator #%d %s has not `descr`: `enumerator`",
                             mom_item_cstring(fromitm), rank, mom_item_cstring(enuritm));
  if (fromitm != nullptr)
    {
      auto curenuroldbind = get_binding(enuritm);
      if (curenuroldbind != nullptr)
        throw MOM_RUNTIME_PRINTF("in enum %s enumerator #%d %s already bound to role %s what %s",
                                 mom_item_cstring(fromitm), rank,
                                 mom_item_cstring(enuritm),
                                 mom_item_cstring(curenuroldbind->vd_rolitm),
                                 mom_value_cstring(curenuroldbind->vd_what));
    }
  int curival = preval+1;
  {
    auto curenumeratorv = mom_unsync_item_get_phys_attr(enuritm, MOM_PREDEFITM(enumerator));
    if (curenumeratorv != nullptr)
      {
        if (mom_itype(curenumeratorv) != MOMITY_INT)
          throw  MOM_RUNTIME_PRINTF("in enum %s enumerator #%d %s has bad `enumerator` value %s - should be int",
                                    mom_item_cstring(fromitm), rank,
                                    mom_item_cstring(enuritm),
                                    mom_value_cstring(curenumeratorv));
        curival = mom_int_val_def (curenumeratorv,curival);
      };
  }
  if (fromitm != nullptr)
    {
      bind_global_at(enuritm, MOM_PREDEFITM(enumerator),
                     mom_boxnode_make_va(MOM_PREDEFITM(enumerator),3,
                                         fromitm, mom_int_make(rank), mom_int_make(curival)),
                     __LINE__);
      do_at_end([=](MomEmitter*em)
		{
		  assert (em != nullptr);
		  MOM_DEBUGPRINTF(gencod, "c-declare_enumerator atend enumerator %s at rank %d",
				  mom_item_cstring(enuritm), rank);
		  mom_unsync_item_put_phys_attr(enuritm, MOM_PREDEFITM(at),
						mom_int_make(rank));
		  mom_unsync_item_put_phys_attr(enuritm, MOM_PREDEFITM(value),
						mom_int_make(curival));
		});
    }
  momvalue_t enutree = nullptr;
  if (initm)
    enutree = mom_boxnode_make_va(MOM_PREDEFITM(sequence), 7,
                                  literal_string(CENUVALEXT_PREFIX),
                                  enuritm,
                                  literal_string(CENUFROM_INFIX),
                                  initm,
                                  literal_string(" = "),
                                  mom_int_make(curival),
                                  literal_string(","));
  else
    enutree = mom_boxnode_make_va(MOM_PREDEFITM(sequence), 5,
                                  literal_string(CENUVAL_PREFIX),
                                  enuritm,
                                  literal_string(" = "),
                                  mom_int_make(curival),
                                  literal_string(","));
  MOM_DEBUGPRINTF(gencod, "c-declare_enumerator enuritm=%s gives enutree=%s preval=%d",
                  mom_item_cstring(enuritm), mom_value_cstring(enutree), preval);
  preval = curival;
  return enutree;
} // end of MomCEmitter::declare_enumerator




////////////////
momvalue_t
MomCEmitter::declare_type (struct mom_item_st*typitm, bool*scalarp)
{
  MOM_DEBUGPRINTF(gencod, "c-declare_type start typitm=%s",
                  mom_item_cstring(typitm));
  assert (is_locked_item(typitm));
  auto cextypv = mom_unsync_item_get_phys_attr (typitm, MOM_PREDEFITM(c_code));
  MOM_DEBUGPRINTF(gencod, "c-declare_type typitm=%s cextypv=%s",
                  mom_item_cstring(typitm), mom_value_cstring(cextypv));
  if (cextypv != nullptr)
    {
      if (scalarp)
        *scalarp = true;
      return cextypv;
    }
  auto tytree = mom_boxnode_make_va(MOM_PREDEFITM(sequence),2,
                                    literal_string(CTYPE_PREFIX),
                                    typitm);
  auto tybind = get_binding(typitm);
  MOM_DEBUGPRINTF(gencod, "c-declare_type typitm=%s has tytree %s tybind role %s what %s",
                  mom_item_cstring(typitm), mom_value_cstring(tytree),
                  mom_item_cstring(tybind?tybind->vd_rolitm:nullptr),
                  mom_value_cstring(tybind?tybind->vd_what:nullptr));
  if (tybind != nullptr)
    {
      MOM_DEBUGPRINTF(gencod, "c-declare_type typitm %s bound to role %s what %s",
                      mom_item_cstring(typitm),
                      mom_item_cstring(tybind->vd_rolitm),
                      mom_value_cstring(tybind->vd_what));
      if (tybind && !is_type_binding(tybind))
        throw MOM_RUNTIME_PRINTF("type %s already bound to role %s",
                                 mom_item_cstring(typitm), mom_item_cstring(tybind->vd_rolitm));
      MOM_DEBUGPRINTF(gencod,
                      "c-declare_type known typitm=%s gives tytree=%s",
                      mom_item_cstring(typitm), mom_value_cstring(tytree));
      return tytree;
    }
  else
    {
      MOM_DEBUGPRINTF(gencod,
                      "c-declare_type fresh typitm=%s",
                      mom_item_cstring(typitm));
      /// temporary binding, will be rebound later
      bind_global_at(typitm, MOM_PREDEFITM(type), nullptr,__LINE__);
    }

#define NBKNOWNTYPE_MOM 31
#define CASE_KNOWNTYPE_MOM(Nam) momhashpredef_##Nam % NBKNOWNTYPE_MOM:  \
  if (typitm == MOM_PREDEFITM(Nam)) goto foundcase_##Nam;		\
  goto defaultcasetype; foundcase_##Nam
  switch (typitm->hva_hash % NBKNOWNTYPE_MOM)
    {
    case CASE_KNOWNTYPE_MOM (int):
      if (scalarp)
        *scalarp = true;
      return literal_string(CINT_TYPE);
    case CASE_KNOWNTYPE_MOM (unit):
      if (scalarp)
        *scalarp = true;
      return literal_string(CVOID_TYPE);
    case CASE_KNOWNTYPE_MOM (value):
      if (scalarp)
        *scalarp = true;
      return literal_string(CVALUE_TYPE);
    case CASE_KNOWNTYPE_MOM (double):
      if (scalarp)
        *scalarp = true;
      return literal_string(CDOUBLE_TYPE);
    defaultcasetype:
    default:
      break;
    }
#undef NBKNOWNTYPE_MOM
#undef CASE_KNOWNTYPE_MOM
  if (_cec_declareditems.find(typitm) != _cec_declareditems.end())
    {
      MOM_DEBUGPRINTF(gencod, "c-declare_type typitm=%s known so gives tytree %s",
                      mom_item_cstring(typitm), mom_value_cstring(tytree));
      return tytree;
    }
  auto tytypv =  (mom_unsync_item_get_phys_attr (typitm, MOM_PREDEFITM(type)));
  auto tytypitm = mom_dyncast_item(tytypv);
  MOM_DEBUGPRINTF(gencod, "c-declare_type typitm=%s has tytypv=%s",
                  mom_item_cstring(typitm), mom_value_cstring(tytypv));
  auto extendv = mom_unsync_item_get_phys_attr(typitm, MOM_PREDEFITM(extend));
  struct mom_item_st*extenditm = mom_dyncast_item(extendv);
  if (tytypitm == MOM_PREDEFITM(struct))
    {
      const struct mom_boxtuple_st*fieldtup = nullptr;
      auto myfieldtup =
        mom_dyncast_tuple(mom_unsync_item_get_phys_attr (typitm, MOM_PREDEFITM(struct)));
      auto mynbfields = mom_size(myfieldtup);
      MOM_DEBUGPRINTF(gencod, "c-declare_type struct typitm=%s myfieldtup=%s",
                      mom_item_cstring(typitm), mom_value_cstring(myfieldtup));
      if (extenditm == nullptr && (myfieldtup==nullptr || mynbfields == 0))
        throw MOM_RUNTIME_PRINTF("struct type item %s missing or empty `struct` : field-tuple",
                                 mom_item_cstring(typitm));
      auto strudecltree = //
        mom_boxnode_make_sentinel(MOM_PREDEFITM(sequence),
                                  literal_string("typedef "),
                                  literal_string("struct "),
                                  literal_string(CSTRUCT_PREFIX),
                                  typitm,
                                  literal_string(" "),
                                  literal_string(CTYPE_PREFIX),
                                  typitm,
                                  literal_string(";"));
      MOM_DEBUGPRINTF(gencod, "c-declare_type struct typitm=%s strudecltree=%s",
                      mom_item_cstring(typitm),
                      mom_value_cstring(strudecltree));
      add_global_decl(strudecltree);
      _cec_declareditems.insert(typitm);
      const struct mom_boxtuple_st*extfieldtup = nullptr;
      unsigned extfieldlen = 0;
      traced_vector_values_t vectree;
      traced_vector_items_t vecfields;
      if (extenditm != nullptr)
        {
          MOM_DEBUGPRINTF(gencod, "c-declare_type struct typitm=%s extenditm=%s",
                          mom_item_cstring(typitm), mom_item_cstring(extenditm));
          lock_item(extenditm);
          if (!is_bound(extenditm))
            {
              MOM_DEBUGPRINTF(gencod, "c-declare_type struct typitm=%s unbound extenditm=%s",
                              mom_item_cstring(typitm), mom_item_cstring(extenditm));
              auto extree = declare_type(extenditm);
              MOM_DEBUGPRINTF(gencod, "c-declare_type struct typitm=%s with extenditm=%s extree=%s",
                              mom_item_cstring(typitm), mom_item_cstring(extenditm),
                              mom_value_cstring(extree));
            }
          auto extendbind = get_global_binding(extenditm);
          if (extendbind == nullptr)
            throw MOM_RUNTIME_PRINTF(" `extend` %s in struct type %s is unbound",
                                     mom_item_cstring(extenditm), mom_item_cstring(typitm));
          MOM_DEBUGPRINTF(gencod, "c-declare_type struct typitm=%s extenditm=%s extendbind role %s what %s",
                          mom_item_cstring(typitm), mom_item_cstring(extenditm),
                          mom_item_cstring(extendbind->vd_rolitm),
                          mom_value_cstring(extendbind->vd_what));
          if (extendbind->vd_rolitm != MOM_PREDEFITM(struct))
            throw  MOM_RUNTIME_PRINTF(" `extend` %s in struct type %s is wrongly bound to role %s what %s",
                                      mom_item_cstring(extenditm), mom_item_cstring(typitm),
                                      mom_item_cstring(extendbind->vd_rolitm),
                                      mom_value_cstring(extendbind->vd_what));
          extfieldtup = mom_dyncast_tuple(extendbind->vd_what);
          MOM_DEBUGPRINTF(gencod, "c-declare_type struct typitm=%s extenditm=%s extfieldtup=%s",
                          mom_item_cstring(typitm), mom_item_cstring(extenditm), mom_value_cstring(extfieldtup));
          assert (extfieldtup != nullptr);
          extfieldlen = mom_raw_size(extfieldtup);
          vectree.reserve(extfieldlen+mynbfields+3);
          vecfields.reserve(extfieldlen+mynbfields+1);
          auto introtree =
            mom_boxnode_make_va(MOM_PREDEFITM(sequence), 3,
                                literal_string("/*extending struct "),
                                extenditm,
                                literal_string("*/"));
          MOM_DEBUGPRINTF(gencod, "c-declare_type struct extended typitm=%s introtree=%s\n.. extenditm=%s extfieldtup=%s",
                          mom_item_cstring(typitm), mom_value_cstring(introtree),
                          mom_item_cstring(extenditm), mom_value_cstring(extfieldtup));
          vectree.push_back(introtree);
          for (int efix=0; efix<(int)extfieldlen; efix++)
            {
              auto curextfitm = extfieldtup->seqitem[efix];
              assert (is_locked_item(curextfitm));
              vecfields.push_back(curextfitm);
              MOM_DEBUGPRINTF(gencod, "c-declare_type struct extended typitm=%s extenditm=%s efix#%d curextfitm=%s",
                              mom_item_cstring(typitm),   mom_item_cstring(extenditm), efix, mom_item_cstring(curextfitm));
              auto curextftree = declare_struct_member_unbound (curextfitm);
              MOM_DEBUGPRINTF(gencod, "c-declare_type struct extended typitm=%s extenditm=%s efix#%d curextfitm=%s curextftree=%s",
                              mom_item_cstring(typitm),   mom_item_cstring(extenditm), efix,
                              mom_item_cstring(curextfitm), mom_value_cstring(curextftree));
              assert (curextftree != nullptr);
              vectree.push_back(curextftree);
            };
          auto aftertree =
            mom_boxnode_make_va(MOM_PREDEFITM(sequence), 5,
                                literal_string("/*extended struct "),
                                extenditm,
                                literal_string(" in "),
                                typitm,
                                literal_string("*/"));
          MOM_DEBUGPRINTF(gencod, "c-declare_type typitm=%s aftertree=%s",
                          mom_item_cstring(typitm), mom_value_cstring(aftertree));
          vectree.push_back(aftertree);
        }
      else   /* no extension in struct */
        {
          vecfields.reserve(mynbfields+1);
          MOM_DEBUGPRINTF(gencod, "c-declare_type struct typitm=%s myfieldtup=%s",
                          mom_item_cstring(typitm), mom_value_cstring(myfieldtup));
        }
      MOM_DEBUGPRINTF(gencod, "c-declare_type struct typitm=%s mynbfields#%d",
                      mom_item_cstring(typitm), mynbfields);
      for (int fix=0; fix<(int)mynbfields; fix++)
        {
          auto curflditm = myfieldtup->seqitem[fix];
          MOM_DEBUGPRINTF(gencod, "c-declare_type typitm=%s fix#%d curflditm=%s",
                          mom_item_cstring(typitm), fix, mom_item_cstring(curflditm));
          if (curflditm==nullptr)
            throw MOM_RUNTIME_PRINTF("in struct type %s field #%d is null",
                                     mom_item_cstring(typitm), fix);
          lock_item(curflditm);
          auto curftree = declare_struct_member(curflditm, typitm, fix+extfieldlen);
          MOM_DEBUGPRINTF(gencod, "c-declare_type typitm=%s fix#%d curflditm=%s curftree=%s",
                          mom_item_cstring(typitm), fix,
                          mom_item_cstring(curflditm), mom_value_cstring(curftree));
          vectree.push_back(curftree);
          vecfields.push_back(curflditm);
        }
      auto allfldtup = mom_boxtuple_make_arr(vecfields.size(), vecfields.data());
      MOM_DEBUGPRINTF(gencod, "c-declare_type typitm=%s binding allfldtup=%s",
                      mom_item_cstring(typitm), mom_value_cstring(allfldtup));
      bind_global_at(typitm, MOM_PREDEFITM(struct), allfldtup,__LINE__);
      auto endtree =
        mom_boxnode_make_va(MOM_PREDEFITM(sequence), 3,
                            literal_string("/*ending struct "),
                            typitm,
                            literal_string("*/"));
      MOM_DEBUGPRINTF(gencod, "c-declare_type typitm=%s endtree=%s",
                      mom_item_cstring(typitm), mom_value_cstring(endtree));
      vectree.push_back(endtree);
      auto bractree =
        mom_boxnode_make_va(MOM_PREDEFITM(brace), 1,
                            mom_boxnode_make(MOM_PREDEFITM(semicolon), vectree.size(), vectree.data()));
      vectree.clear();
      auto strudeftree = //
        mom_boxnode_make_sentinel(MOM_PREDEFITM(sequence),
                                  literal_string("struct "),
                                  literal_string(CSTRUCT_PREFIX),
                                  typitm,
                                  literal_string(" "),
                                  bractree,
                                  literal_string(";"));
      MOM_DEBUGPRINTF(gencod, "c-declare_type typitm=%s strudeftree=%s",
                      mom_item_cstring(typitm), mom_value_cstring(strudeftree));
      add_global_decl(strudeftree);
    } // end handling struct
  ////////////////
  else if  (tytypitm == MOM_PREDEFITM(union))
    {
      const struct mom_seqitems_st*extfieldseq = nullptr;
      unsigned extfieldlen = 0;
      traced_vector_values_t vectree;
      const struct mom_seqitems_st*myfieldseq =
	mom_dyncast_seqitems(mom_unsync_item_get_phys_attr(typitm, MOM_PREDEFITM(union)));
      unsigned mynbfields = mom_size(myfieldseq);
      MOM_DEBUGPRINTF(gencod, "c-declare_type typitm=%s union of fields %s",
                      mom_item_cstring(typitm), mom_value_cstring(myfieldseq));
      if(extenditm == nullptr && mynbfields==0)
        throw MOM_RUNTIME_PRINTF("union type %s with missing or empty `union` sequence",
                                 mom_item_cstring(typitm));
      {
        if (extendv != nullptr)
          {
            traced_vector_items_t vecfields;
            if (extenditm == nullptr)
              throw MOM_RUNTIME_PRINTF("invalid `extend` %s in union type %s",
                                       mom_value_cstring(extendv), mom_item_cstring(typitm));
            lock_item(extenditm);
            if (!is_bound(extenditm))
              {
                MOM_DEBUGPRINTF(gencod, "c-declare_type union typitm=%s unbound extenditm=%s",
                                mom_item_cstring(typitm), mom_item_cstring(extenditm));
                auto extree = declare_type(extenditm);
                MOM_DEBUGPRINTF(gencod, "c-declare_type union typitm=%s with extenditm=%s extree=%s",
                                mom_item_cstring(typitm), mom_item_cstring(extenditm),
                                mom_value_cstring(extree));
              }
            auto extendbind = get_global_binding(extenditm);
            if (extendbind == nullptr)
              throw MOM_RUNTIME_PRINTF(" `extend` %s in union type %s is unbound",
                                       mom_item_cstring(extenditm), mom_item_cstring(typitm));
            MOM_DEBUGPRINTF(gencod, "c-declare_type typitm=%s extenditm=%s extendbind role %s what %s",
                            mom_item_cstring(typitm), mom_item_cstring(extenditm),
                            mom_item_cstring(extendbind->vd_rolitm),
                            mom_value_cstring(extendbind->vd_what));
            if (extendbind->vd_rolitm != MOM_PREDEFITM(union))
              throw  MOM_RUNTIME_PRINTF(" `extend` %s in union type %s is wrongly bound to role %s what %s",
                                        mom_item_cstring(extenditm), mom_item_cstring(typitm),
                                        mom_item_cstring(extendbind->vd_rolitm),
                                        mom_value_cstring(extendbind->vd_what));
            extfieldseq = mom_dyncast_seqitems(extendbind->vd_what);
            MOM_DEBUGPRINTF(gencod, "c-declare_type typitm=%s extended union extfieldseq=%s",
                            mom_item_cstring(typitm), mom_value_cstring(extfieldseq));
            assert (extfieldseq != nullptr);
            extfieldlen = mom_raw_size(extfieldseq);
            vecfields.reserve(extfieldlen+mynbfields+1);
            auto introtree =
              mom_boxnode_make_va(MOM_PREDEFITM(sequence), 3,
                                  literal_string("/*extending union "),
                                  extenditm,
                                  literal_string("*/"));
            vectree.push_back(introtree);
            MOM_DEBUGPRINTF(gencod, "c-declare_type typitm=%s introtree=%s",
                            mom_item_cstring(typitm),
                            mom_value_cstring(introtree));
            for (int efix=0; efix<(int)extfieldlen; efix++)
              {
                auto xflditm = extfieldseq->seqitem[efix];
                MOM_DEBUGPRINTF(gencod, "c-declare_type typitm=%s union xflditm=%s efix#%d",
                                mom_item_cstring(typitm), mom_item_cstring(xflditm), efix);
                assert (is_locked_item(xflditm));
                vecfields.push_back(xflditm);
                auto xfldtree = declare_field_unbound(xflditm);
                MOM_DEBUGPRINTF(gencod, "c-declare_type typitm=%s union xflditm=%s efix#%d xfldtree=%s",
                                mom_item_cstring(typitm), mom_item_cstring(xflditm),
                                efix, mom_value_cstring(xfldtree));
                assert (xfldtree != nullptr);
                vectree.push_back(xfldtree);
              }
            auto aftertree =
              mom_boxnode_make_va(MOM_PREDEFITM(sequence), 3,
                                  literal_string("/*extended with union "),
                                  extenditm,
                                  literal_string("*/"));
            MOM_DEBUGPRINTF(gencod, "c-declare_type typitm=%s aftertree=%s",
                            mom_item_cstring(typitm),
                            mom_value_cstring(aftertree));
            vectree.push_back(aftertree);
            for (int fix=0; fix<(int)mynbfields; fix++)
              {
                auto curflditm = myfieldseq->seqitem[fix];
                MOM_DEBUGPRINTF(gencod, "c-declare_type typitm=%s union fix#%d curflditm=%s",
                                mom_item_cstring(typitm), fix,
                                mom_item_cstring(curflditm));
                if (curflditm == nullptr)
                  throw MOM_RUNTIME_PRINTF("in union type %s field #%d is missing",
                                           mom_item_cstring(typitm), fix);
                lock_item(curflditm);
                vecfields.push_back(curflditm);
                auto curfldtree = declare_field(curflditm, typitm, fix+extfieldlen);
                MOM_DEBUGPRINTF(gencod, "c-declare_type typitm=%s extunion fix#%d curflditm=%s curfldtree=%s",
                                mom_item_cstring(typitm), fix,
                                mom_item_cstring(curflditm),
                                mom_value_cstring(curfldtree));
                vectree.push_back(curfldtree);
              }
            auto fullfieldtup = mom_boxtuple_make_arr (vecfields.size(), vecfields.data());
            MOM_DEBUGPRINTF(gencod, "c-declare_type typitm=%s union fullfieldtup=%s",
                            mom_item_cstring(typitm),
                            mom_value_cstring(fullfieldtup));
            bind_global_at(typitm, MOM_PREDEFITM(union), fullfieldtup,__LINE__);
          }  // end union with extension
        else   // union without extension
          {
            bind_global_at(typitm, MOM_PREDEFITM(union), myfieldseq,__LINE__);
            for (int fix=0; fix<(int)mynbfields; fix++)
              {
                auto curflditm = myfieldseq->seqitem[fix];
                MOM_DEBUGPRINTF(gencod, "c-declare_type typitm=%s union fix#%d curflditm=%s",
                                mom_item_cstring(typitm), fix,
                                mom_item_cstring(curflditm));
                if (curflditm == nullptr)
                  throw MOM_RUNTIME_PRINTF("in union type %s field #%d is missing",
                                           mom_item_cstring(typitm), fix);
                lock_item(curflditm);
                auto curfldtree = declare_field(curflditm, typitm, fix+extfieldlen);
                MOM_DEBUGPRINTF(gencod, "c-declare_type typitm=%s union fix#%d curflditm=%s curfldtree=%s",
                                mom_item_cstring(typitm), fix,
                                mom_item_cstring(curflditm),
                                mom_value_cstring(curfldtree));
                vectree.push_back(curfldtree);
              }
          }
      }
      auto endtree =
        mom_boxnode_make_va(MOM_PREDEFITM(sequence), 3,
                            literal_string("/*end union "),
                            extenditm,
                            literal_string("*/"));
      MOM_DEBUGPRINTF(gencod, "c-declare_type typitm=%s endtree=%s",
                      mom_item_cstring(typitm),
                      mom_value_cstring(endtree));
      vectree.push_back(endtree);
      auto bractree =
        mom_boxnode_make_va(MOM_PREDEFITM(brace), 1,
                            mom_boxnode_make(MOM_PREDEFITM(semicolon), vectree.size(), vectree.data()));
      vectree.clear();
      auto unideftree = //
        mom_boxnode_make_sentinel(MOM_PREDEFITM(sequence),
                                  literal_string("union "),
                                  literal_string(CUNION_PREFIX),
                                  typitm,
                                  literal_string(" "),
                                  bractree,
                                  literal_string(";"));
      MOM_DEBUGPRINTF(gencod, "c-declare_type typitm=%s unideftree=%s",
                      mom_item_cstring(typitm), mom_value_cstring(unideftree));
      add_global_decl(unideftree);
    } /// end union
  ////////////////
  else if  (tytypitm == MOM_PREDEFITM(enum))
    {
      const struct mom_boxtuple_st*extenumtup = nullptr;
      unsigned extenumlen = 0;
      traced_vector_values_t vectree;
      auto myenutup =
        mom_dyncast_tuple(mom_unsync_item_get_phys_attr (typitm, MOM_PREDEFITM(enum)));
      if (myenutup==nullptr)
        throw MOM_RUNTIME_PRINTF("enum type item %s missing `enum` : enuvalseq",
                                 mom_item_cstring(typitm));
      unsigned mynbenur = mom_size(myenutup);
      MOM_DEBUGPRINTF(gencod, "c-declare_type typitm=%s enum of enumerators %s",
                      mom_item_cstring(typitm), mom_value_cstring(myenutup));
      if(extenditm == nullptr && mynbenur==0)
        throw MOM_RUNTIME_PRINTF("enum type %s with missing or empty `enum` sequence",
                                 mom_item_cstring(typitm));
      if (extenditm)
        {
          MOM_DEBUGPRINTF(gencod, "c-declare_type %s enum extenditm %s",
                          mom_item_cstring(typitm), mom_item_cstring(extenditm));
          lock_item(extenditm);
          if (!is_bound(extenditm))
            {
              MOM_DEBUGPRINTF(gencod, "c-declare_type enum typitm=%s unbound extenditm=%s",
                              mom_item_cstring(typitm), mom_item_cstring(extenditm));
              auto extree = declare_type(extenditm);
              MOM_DEBUGPRINTF(gencod, "c-declare_type enum typitm=%s with extenditm=%s extree=%s",
                              mom_item_cstring(typitm), mom_item_cstring(extenditm),
                              mom_value_cstring(extree));
            }
        }
      int preval = -1;
      if (extendv != nullptr)
        {
          traced_vector_items_t vecenurs;
          if (extenditm == nullptr)
            throw MOM_RUNTIME_PRINTF("invalid `extend` %s in enum type %s",
                                     mom_value_cstring(extendv), mom_item_cstring(typitm));
          auto extendbind = get_global_binding(extenditm);
          if (extendbind == nullptr)
            throw MOM_RUNTIME_PRINTF(" `extend` %s in enum type %s is unbound",
                                     mom_item_cstring(extenditm), mom_item_cstring(typitm));
          MOM_DEBUGPRINTF(gencod, "c-declare_type typitm=%s extenditm=%s extendbind role %s what %s",
                          mom_item_cstring(typitm), mom_item_cstring(extenditm),
                          mom_item_cstring(extendbind->vd_rolitm),
                          mom_value_cstring(extendbind->vd_what));
          if (extendbind->vd_rolitm != MOM_PREDEFITM(enum))
            throw  MOM_RUNTIME_PRINTF(" `extend` %s in enum type %s is wrongly bound to role %s what %s",
                                      mom_item_cstring(extenditm), mom_item_cstring(typitm),
                                      mom_item_cstring(extendbind->vd_rolitm),
                                      mom_value_cstring(extendbind->vd_what));
          extenumtup = mom_dyncast_tuple(extendbind->vd_what);
          MOM_DEBUGPRINTF(gencod, "c-declare_type typitm=%s extended enum extenumtup=%s",
                          mom_item_cstring(typitm), mom_value_cstring(extenumtup));
          assert (extenumtup != nullptr);
          extenumlen = mom_raw_size(extenumtup);
          vecenurs.reserve(2*extenumlen+2*mynbenur+1);
          auto introtree =
            mom_boxnode_make_va(MOM_PREDEFITM(sequence), 3,
                                literal_string("/*extending enum "),
                                extenditm,
                                literal_string("*/"));
          vectree.push_back(introtree);
          MOM_DEBUGPRINTF(gencod, "c-declare_type typitm %s introtree=%s",
                          mom_item_cstring(typitm), mom_value_cstring(introtree));
          for (int eix=0; eix<(int)extenumlen; eix++)
            {
              auto curxenuritm = extenumtup->seqitem[eix];
              MOM_DEBUGPRINTF(gencod, "c-declare_type typitm %s eix#%d curxenuritm=%s",
                              mom_item_cstring(typitm), eix, mom_item_cstring(curxenuritm));
              assert (is_locked_item(curxenuritm));
              vecenurs.push_back(curxenuritm);
              auto curxenurbind = get_binding(curxenuritm);
              assert (curxenurbind != nullptr && curxenurbind->vd_rolitm == MOM_PREDEFITM(enumerator));
              auto curxenurtree = declare_enumerator(curxenuritm, nullptr, eix, preval, extenditm);
              MOM_DEBUGPRINTF(gencod, "c-declare_type typitm %s eix#%d curxenuritm=%s curxenurtree=%s",
                              mom_item_cstring(typitm), eix, mom_item_cstring(curxenuritm),
                              mom_value_cstring(curxenurtree));
              vectree.push_back(mom_boxnode_make_va(MOM_PREDEFITM(out_newline),0));
              vectree.push_back(curxenurtree);
            }
          auto aftertree =
            mom_boxnode_make_va(MOM_PREDEFITM(sequence), 3,
                                literal_string("/*extended enum "),
                                extenditm,
                                literal_string("*/"));
          vectree.push_back(aftertree);
          MOM_DEBUGPRINTF(gencod, "c-declare_type typitm %s aftertree=%s",
                          mom_item_cstring(typitm), mom_value_cstring(aftertree));
          for (int nix=0; nix<(int)mynbenur; nix++)
            {
              auto curenuritm = myenutup->seqitem[nix];
              MOM_DEBUGPRINTF(gencod, "c-declare_type extenum typitm %s nix#%d curenuritm=%s",
                              mom_item_cstring(typitm), nix, mom_item_cstring(curenuritm));
              if (curenuritm==nullptr)
                throw MOM_RUNTIME_PRINTF("enum %s missing enumerator #%d", mom_item_cstring(typitm), nix);
              lock_item(curenuritm);
              auto enurtree = declare_enumerator(curenuritm, typitm, nix, preval);
              vecenurs.push_back(curenuritm);
              MOM_DEBUGPRINTF(gencod, "c-declare_type enum typitm %s nix#%d curenuritm=%s enurtree=%s",
                              mom_item_cstring(typitm), nix, mom_item_cstring(curenuritm),
                              mom_value_cstring(enurtree));
              vectree.push_back(mom_boxnode_make_va(MOM_PREDEFITM(out_newline),0));
              vectree.push_back(enurtree);
            }
          auto xenurtup = mom_boxtuple_make_arr(vecenurs.size(), vecenurs.data());
          MOM_DEBUGPRINTF(gencod, "c-declare_type extendedenum typitm %s xenurtup=%s",
                          mom_item_cstring(typitm), mom_value_cstring(xenurtup));
          bind_global_at(typitm, MOM_PREDEFITM(enum), xenurtup, __LINE__);
        }
      else   // enum without extension
        {
          vectree.reserve(mynbenur+2);
          auto prologtree =
            mom_boxnode_make_va(MOM_PREDEFITM(sequence), 5,
                                literal_string("/*pristine enum "),
                                typitm,
                                literal_string(" of "),
                                mom_int_make(mynbenur),
                                literal_string("*/"));
          vectree.push_back(prologtree);
          for (int nix=0; nix<(int)mynbenur; nix++)
            {
              auto curenuritm = myenutup->seqitem[nix];
              MOM_DEBUGPRINTF(gencod, "c-declare_type enum typitm %s nix#%d curenuritm=%s",
                              mom_item_cstring(typitm), nix, mom_item_cstring(curenuritm));
              if (curenuritm==nullptr)
                throw MOM_RUNTIME_PRINTF("enum %s missing enumerator #%d", mom_item_cstring(typitm), nix);
              lock_item(curenuritm);
              vectree.push_back(mom_boxnode_make_va(MOM_PREDEFITM(out_newline),0));
              auto enurtree = declare_enumerator(curenuritm, typitm, nix, preval);
              MOM_DEBUGPRINTF(gencod, "c-declare_type enum typitm %s nix#%d curenuritm=%s enurtree=%s",
                              mom_item_cstring(typitm), nix, mom_item_cstring(curenuritm),
                              mom_value_cstring(enurtree));
              vectree.push_back(enurtree);
            }
          MOM_DEBUGPRINTF(gencod, "c-declare_type enum typitm %s myenutup=%s",
                          mom_item_cstring(typitm), mom_value_cstring(myenutup));
          bind_global_at(typitm, MOM_PREDEFITM(enum), myenutup, __LINE__);
        }
      auto enumdecltree = //
        mom_boxnode_make_sentinel(MOM_PREDEFITM(sequence),
                                  literal_string("typedef "),
                                  literal_string("enum "),
                                  literal_string(CENUM_PREFIX),
                                  typitm,
                                  mom_boxnode_make_va(MOM_PREDEFITM(brace), 1,
						      mom_boxnode_make(MOM_PREDEFITM(output),
								       vectree.size(),
								       vectree.data())),
                                  literal_string(" "),
                                  literal_string(CTYPE_PREFIX),
                                  typitm,
                                  literal_string(";"));
      MOM_DEBUGPRINTF(gencod, "c-declare_type typitm=%s enumdecltree=%s",
                      mom_item_cstring(typitm),
                      mom_value_cstring(enumdecltree));
      add_global_decl(enumdecltree);
      _cec_declareditems.insert(typitm);
    }/// end enum
  else if (tytypv != nullptr)
    {
      MOM_DEBUGPRINTF(gencod, "c-declare_type typitm=%s with tytypv=%s",
                      mom_item_cstring(typitm),
                      mom_value_cstring(tytypv));
      scan_type_expr(tytypv, typitm);
      auto typtree = mom_boxnode_make_va(MOM_PREDEFITM(sequence), 2,
                                         literal_string(CTYPE_PREFIX),
                                         typitm);
      MOM_DEBUGPRINTF(gencod, "c-declare_type typitm=%s typtree=%s",
                      mom_item_cstring(typitm),
                      mom_value_cstring(typtree));
      auto decltree = transform_type_for(tytypv, typtree);
      MOM_DEBUGPRINTF(gencod, "c-declare_type typitm=%s decltree=%s",
                      mom_item_cstring(typitm),
                      mom_value_cstring(decltree));
      auto tydecltree = mom_boxnode_make_va(MOM_PREDEFITM(sequence), 3,
                                            literal_string("typedef "),
                                            decltree,
                                            literal_string(";"));
      MOM_DEBUGPRINTF(gencod, "c-declare_type typitm=%s tydecltree=%s",
                      mom_item_cstring(typitm),
                      mom_value_cstring(tydecltree));
      add_global_decl(tydecltree);
      _cec_declareditems.insert(typitm);
    }
  ////////////////
  else
    throw MOM_RUNTIME_PRINTF("invalid type item %s", mom_item_cstring(typitm));
  MOM_DEBUGPRINTF(gencod, "c-declare_type typitm=%s gives %s\n",
                  mom_item_cstring(typitm), mom_value_cstring(tytree));
  return tytree;
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



const struct mom_boxnode_st*
MomCEmitter::transform_other_element(struct mom_item_st*elitm, struct mom_item_st*descitm)
{
  const struct mom_boxnode_st*resnod= nullptr;
  MOM_DEBUGPRINTF(gencod, "c-transform_other_element elitm:=%s\n.. descitm=%s",
                  mom_item_content_cstring(elitm), mom_item_cstring(descitm));
  assert (is_locked_item(elitm));
  assert (mom_unsync_item_descr(elitm)==descitm);
  if (descitm == MOM_PREDEFITM(type))
    {
      resnod = mom_dyncast_node(declare_type(elitm));
      MOM_DEBUGPRINTF(gencod, "c-transform_other_element type elitm=%s resnod=%s",
                      mom_item_cstring(elitm), mom_value_cstring(resnod));
      assert (resnod != nullptr);
      auto tycomnod = mom_boxnode_make_va(MOM_PREDEFITM(comment), 2,
                                          literal_string("type "),
                                          elitm);
      MOM_DEBUGPRINTF(gencod, "c-transform_other_element type elitm=%s gives tycomnod=%s",
                      mom_item_cstring(elitm), mom_value_cstring(tycomnod));
      return tycomnod;
    }
  else if  (descitm == MOM_PREDEFITM(signature))
    {
      resnod = mom_dyncast_node(declare_signature_type(elitm));
      MOM_DEBUGPRINTF(gencod, "c-transform_other_element signature elitm=%s resnod=%s",
                      mom_item_cstring(elitm), mom_value_cstring(resnod));
      auto sigcomnod = mom_boxnode_make_va(MOM_PREDEFITM(comment), 2,
                                           literal_string("signature "),
                                           elitm);
      MOM_DEBUGPRINTF(gencod, "c-transform_other_element signature elitm=%s gives sigcomnod=%s",
                      mom_item_cstring(elitm), mom_value_cstring(sigcomnod));
      add_global_decl(resnod);
      _cec_declareditems.insert(elitm);
      return sigcomnod;
    }
  else if (descitm == MOM_PREDEFITM(inline))
    {
      MOM_DEBUGPRINTF(gencod, "c-transform_other_element inline elitm=%s",
                      mom_item_cstring(elitm));
      resnod = transform_inline_element(elitm);
      MOM_DEBUGPRINTF(gencod, "c-transform_other_element inline elitm=%s resnod=%s",
                      mom_item_cstring(elitm), mom_value_cstring(resnod));
      return resnod;
    }
  else if (descitm == MOM_PREDEFITM(constant))
    {
      MOM_DEBUGPRINTF(gencod, "c-transform_other_element constant elitm:=%s",
                      mom_item_content_cstring(elitm));
      auto valexpv = mom_unsync_item_get_phys_attr(elitm, MOM_PREDEFITM(value));
      auto cxhkv = mom_unsync_item_get_phys_attr(elitm, MOM_PREDEFITM(c_expansion));
      auto typexpv = mom_unsync_item_get_phys_attr(elitm, MOM_PREDEFITM(type));
      unsigned chkty = mom_itype(cxhkv);
      if (chkty == MOMITY_INT
          || chkty == MOMITY_BOXDOUBLE || chkty == MOMITY_BOXSTRING)
        {
          MOM_DEBUGPRINTF(gencod, "c-transform_other_element constantchunk elitm=%s", mom_item_cstring(elitm));
          auto kdecltree
            = mom_boxnode_make_sentinel(MOM_PREDEFITM(sequence),
                                        literal_string("#define "),
                                        literal_string(CONSTMAC_PREFIX),
                                        elitm,
                                        literal_string(" "),
                                        cxhkv);
          add_global_decl(kdecltree);
          _cec_declareditems.insert(elitm);
          auto kcomnod = mom_boxnode_make_va(MOM_PREDEFITM(comment), 2,
                                             literal_string("constmacro "),
                                             elitm);
          MOM_DEBUGPRINTF(gencod, "c-transform_other_element constantchunk elitm=%s kdecltree=%s gives kcomnod=%s",
                          mom_item_cstring(elitm), mom_value_cstring(kdecltree), mom_value_cstring(kcomnod));
          return kcomnod;
        }
      else if (mom_itype(valexpv)==MOMITY_INT && typexpv == (momvalue_t)MOM_PREDEFITM(int))
        {
          MOM_DEBUGPRINTF(gencod, "c-transform_other_element constantint elitm=%s", mom_item_cstring(elitm));
          auto kdecltree
            = mom_boxnode_make_sentinel(MOM_PREDEFITM(sequence),
                                        literal_string("#define "),
                                        literal_string(CONSTMAC_PREFIX),
                                        elitm,
                                        literal_string(" /*constint*/ "),
                                        valexpv);
          add_global_decl(kdecltree);
          _cec_declareditems.insert(elitm);
          auto kcomnod = mom_boxnode_make_va(MOM_PREDEFITM(comment), 2,
                                             literal_string("constint "),
                                             elitm);
          return kcomnod;
        }
      else if (mom_itype(valexpv)==MOMITY_BOXDOUBLE && typexpv == (momvalue_t)MOM_PREDEFITM(double))
        {
          MOM_DEBUGPRINTF(gencod, "c-transform_other_element constantdbl elitm=%s", mom_item_cstring(elitm));
          auto kdecltree
            = mom_boxnode_make_sentinel(MOM_PREDEFITM(sequence),
                                        literal_string("#define "),
                                        literal_string(CONSTMAC_PREFIX),
                                        elitm,
                                        literal_string(" /*constdbl*/ "),
                                        valexpv);
          add_global_decl(kdecltree);
          _cec_declareditems.insert(elitm);
          auto kcomnod = mom_boxnode_make_va(MOM_PREDEFITM(comment), 2,
                                             literal_string("constdbl "),
                                             elitm);
          return kcomnod;
        }
      else
        throw MOM_RUNTIME_PRINTF("c-transform_other_element failed to handle constant %s", mom_item_cstring(elitm));

    }
  MOM_WARNPRINTF("c-transform_other_element elitm=%s failing",
                 mom_item_cstring(elitm));
  return nullptr;
} // end MomCEmitter::transform_other_element

momvalue_t
MomCEmitter::transform_block(struct mom_item_st*blkitm, struct mom_item_st*initm)
{
  MOM_DEBUGPRINTF(gencod, "c-transform_block initm=%s blkitm:=\n%s",
                  mom_item_cstring(initm), mom_item_content_cstring(blkitm));
  assert (is_locked_item(blkitm));
  auto desitm = mom_unsync_item_descr(blkitm);
  if (desitm == MOM_PREDEFITM(indirect))
    {
      auto indiritm = mom_dyncast_item(mom_unsync_item_get_phys_attr(blkitm, MOM_PREDEFITM(indirect)));
      MOM_DEBUGPRINTF(gencod, "c-transform_block indirect blkitm=%s indiritm=%s",
                      mom_item_cstring(blkitm), mom_item_cstring(indiritm));
      assert (is_locked_item(indiritm));
      auto indtree = transform_block(indiritm, initm);
      MOM_DEBUGPRINTF(gencod, "c-transform_block indirect blkitm=%s indiritm=%s indtree=%s",
                      mom_item_cstring(blkitm), mom_item_cstring(indiritm), mom_value_cstring(indtree));
      auto restree = mom_boxnode_make_va(MOM_PREDEFITM(sequence), 4,
                                         literal_string("/*indirblock "),
                                         blkitm,
                                         literal_string(":*/"),
                                         indtree);
      MOM_DEBUGPRINTF(gencod, "c-transform_block indirect blkitm=%s gives restree=%s",
                      mom_item_cstring(blkitm), mom_value_cstring(restree));
      return restree;
    }
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
#define NBBLOCKROLE_MOM 31
#define CASE_BLOCKROLE_MOM(Nam) momhashpredef_##Nam % NBBLOCKROLE_MOM:  \
  if (rolitm == MOM_PREDEFITM(Nam)) goto foundcase_##Nam;		\
  goto defaultcasebrole; foundcase_##Nam
  switch (rolitm->hva_hash % NBBLOCKROLE_MOM)
    {
    case CASE_BLOCKROLE_MOM (sequence):
      {
	if (_ce_continuecountmap[blkitm]>0)
	  throw MOM_RUNTIME_PRINTF("sequence block %s is continue-d %ld times",
				   mom_item_cstring(blkitm), (_ce_continuecountmap[blkitm]));
	if (_ce_breakcountmap[blkitm]>0)
	  throw MOM_RUNTIME_PRINTF("sequence block %s is break-d %ld times",
				   mom_item_cstring(blkitm), (_ce_breakcountmap[blkitm]));
	auto localseq = mom_dyncast_seqitems(mom_unsync_item_get_phys_attr (blkitm, MOM_PREDEFITM(locals)));
	unsigned nblocals = mom_seqitems_length(localseq);
	MOM_DEBUGPRINTF(gencod,
			"c-transform_block blkitm=%s nblocals#%d localseq:%s",
			mom_item_cstring(blkitm), nblocals, mom_value_cstring(localseq));
	momvalue_t localtree= nullptr;
	if (nblocals>0)
	  {
	    traced_vector_values_t vecloctree;
	    vecloctree.reserve(nblocals+4);
	    vecloctree.push_back(mom_boxnode_make_va(MOM_PREDEFITM(comment), 3,
						     mom_int_make(nblocals),
						     literal_string("locals in block:"),
						     blkitm));
	    vecloctree.push_back(mom_boxnode_make_va(MOM_PREDEFITM(out_newline), 0));
	    for (int lix=0; lix<(int)nblocals; lix++)
	      {
		momvalue_t curloctree = nullptr;
		struct mom_item_st*locitm = localseq->seqitem[lix];
		assert (is_locked_item(locitm));
		MOM_DEBUGPRINTF(gencod,
				"c-transform_block blkitm=%s lix#%d locitm:=\n%s",
				mom_item_cstring(blkitm), lix, mom_item_content_cstring(locitm));
		momvalue_t typeval = mom_unsync_item_get_phys_attr (locitm, MOM_PREDEFITM(type));
		auto typeitm = mom_dyncast_item(typeval);
		MOM_DEBUGPRINTF(gencod,
				"c-transform_block blkitm=%s lix#%d locitm=%s typeitm=%s",
				mom_item_cstring(blkitm), lix, mom_item_cstring(locitm), mom_item_cstring(typeitm));
		if (typeitm != nullptr)
		  {
		    assert (is_locked_item(typeitm));
		    MOM_DEBUGPRINTF(gencod,
				    "c-transform_block blkitm=%s lix#%d locitm=%s typeitm:=%s",
				    mom_item_cstring(blkitm), lix,
				    mom_item_cstring(locitm), mom_item_content_cstring(typeitm));
		    momvalue_t typecexp = nullptr;
		    if (mom_unsync_item_descr(typeitm) == MOM_PREDEFITM(type))
		      {

			typecexp = mom_unsync_item_get_phys_attr(typeitm, MOM_PREDEFITM(c_code));
			if (typecexp != nullptr)
			  {
			    curloctree = //
			      mom_boxnode_make_sentinel(MOM_PREDEFITM(sequence),
							typecexp,
							literal_string(" "),
							literal_string(CLOCAL_PREFIX),
							locitm,
							literal_string(" = ("),
							typecexp,
							literal_string("/*nothing*/)0;"));
			  }
			else
			  {
			    auto tytypitm = mom_dyncast_item(mom_unsync_item_get_phys_attr(typeitm, MOM_PREDEFITM(type)));
			    auto tybind = get_binding(typeitm);
			    MOM_DEBUGPRINTF(gencod,
					    "c-transform_block blkitm=%s lix#%d locitm=%s typeitm=%s tytypitm=%s tybind rol=%s what=%s",
					    mom_item_cstring(blkitm), lix,
					    mom_item_cstring(locitm), mom_item_cstring(typeitm), mom_item_cstring(tytypitm),
					    mom_item_cstring(tybind?tybind->vd_rolitm:NULL),
					    mom_value_cstring(tybind?tybind->vd_what:NULL));
			    if (tybind && is_type_binding(tybind))
			      {
				typecexp = mom_boxnode_make_va(MOM_PREDEFITM(sequence),2,
							       literal_string(CTYPE_PREFIX),
							       typeitm);
				curloctree = //
				  mom_boxnode_make_sentinel(MOM_PREDEFITM(sequence),
							    typecexp,
							    literal_string(" "),
							    literal_string(CLOCAL_PREFIX),
							    locitm,
							    literal_string(" = ("),
							    typecexp,
							    literal_string("/*nothing*/)0;"));
				MOM_DEBUGPRINTF(gencod,
						"c-transform_block locitm=%s typeitm=%s typecexp=%s curloctree=%s",
						mom_item_cstring(locitm),
						mom_item_cstring(typeitm),
						mom_value_cstring(typecexp),
						mom_value_cstring(curloctree));

			      }
			    else
#warning c-transform_block local with unhandled type
			      MOM_FATAPRINTF("c-transform_block blkitm=%s lix#%d locitm=%s unhandled typeitm=%s",
					     mom_item_cstring(blkitm), lix,
					     mom_item_cstring(locitm), mom_item_cstring(typeitm));
			  }
		      }
		    MOM_DEBUGPRINTF(gencod,
				    "c-transform_block lix#%d locitm=%s typeitm=%s curloctree=%s",
				    lix, mom_item_cstring(locitm),
				    mom_item_cstring(typeitm), mom_value_cstring(curloctree));
		  }
		else
		  {
		    bool scalar = true;
		    auto vartree = mom_boxnode_make_va(MOM_PREDEFITM(sequence),2,
						       literal_string(CLOCAL_PREFIX),
						       locitm);
		    MOM_DEBUGPRINTF(gencod,
				    "c-transform_block blkitm=%s lix#%d locitm=%s vartree=%s",
				    mom_item_cstring(blkitm), lix,
				    mom_item_cstring(locitm), mom_value_cstring(vartree));
		    auto declvtree = transform_type_for(typeval, vartree, &scalar);
		    MOM_DEBUGPRINTF(gencod,
				    "c-transform_block blkitm=%s lix#%d locitm=%s declvtree=%s scalar %s",
				    mom_item_cstring(blkitm), lix,
				    mom_item_cstring(locitm),
				    mom_value_cstring(declvtree),
				    scalar?"true":"false");
		    if (scalar)
		      curloctree = //
			mom_boxnode_make_sentinel(MOM_PREDEFITM(sequence),
						  declvtree,
						  literal_string(" = "),
						  literal_string("/*nothing*/0;"));
		    else
		      curloctree = //
			mom_boxnode_make_sentinel(MOM_PREDEFITM(sequence),
						  declvtree,
						  literal_string(" = "),
						  literal_string("/*empty*/{};"));
		    MOM_DEBUGPRINTF(gencod,
				    "c-transform_block blkitm=%s lix#%d locitm=%s curloctree=%s",
				    mom_item_cstring(blkitm), lix, mom_item_cstring(locitm),
				    mom_value_cstring(curloctree));
		  }
		MOM_DEBUGPRINTF(gencod,
				"c-transform_block lix#%d locitm=%s curloctree=%s",
				lix, mom_item_cstring(locitm), mom_value_cstring(curloctree));
		if (curloctree == nullptr)
		  MOM_FATAPRINTF("c-transform_block blkitm=%s unhandled local lix#%d locitm:=\n%s",
				 mom_item_cstring(blkitm), lix, mom_item_content_cstring(locitm));
		else
		  vecloctree.push_back(curloctree);
	      }
	    localtree = mom_boxnode_make(MOM_PREDEFITM(out_newline), vecloctree.size(), vecloctree.data());
	    vecloctree.clear();
	  } // end if nblocals>0)
	MOM_DEBUGPRINTF(gencod,
			"c-transform_block blkitm=%s sequence of body %s localtree=%s",
			mom_item_content_cstring(blkitm), mom_value_cstring(bodytup),
			mom_value_cstring(localtree));
	int bodylen = mom_raw_size(bodytup);
	traced_vector_values_t bodyvec;
	bodyvec.reserve(3*bodylen+2);
	if (localtree)
	  bodyvec.push_back(localtree);
	for (int bix=0; bix<bodylen; bix++)
	  {
	    bodyvec.push_back(mom_boxnode_make_va(MOM_PREDEFITM(out_newline),0));
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
	    bodyvec.push_back(instree);
	  }
	bodyvec.push_back(literal_string(";"));
	auto bodytree = mom_boxnode_make(MOM_PREDEFITM(semicolon),bodyvec.size(),bodyvec.data());
	bodyvec.clear();
	auto bracetree =
	  mom_boxnode_make_sentinel(MOM_PREDEFITM(brace),
				    mom_boxnode_make_va(MOM_PREDEFITM(out_newline),0),
				    mom_boxnode_make_va(MOM_PREDEFITM(comment),3,
							literal_string("block"),
							literal_string(" "),
							blkitm),
				    bodytree,
				    mom_boxnode_make_va(MOM_PREDEFITM(out_newline),0),
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
    case CASE_BLOCKROLE_MOM (loop):
      {
	auto whilexpv =  (momvalue_t)mom_unsync_item_get_phys_attr (blkitm, MOM_PREDEFITM(while));
	auto bodytup = mom_dyncast_tuple(mom_unsync_item_get_phys_attr (blkitm, MOM_PREDEFITM(body)));
	long nbcont = _ce_continuecountmap[blkitm];
	long nbbreak = _ce_breakcountmap[blkitm];
	momvalue_t prologtree = nullptr;
	MOM_DEBUGPRINTF(gencod,
			"c-transform_block blkitm=%s loop of while %s, body %s, nbcont=%ld, nbbreak=%ld",
			mom_item_cstring(blkitm), mom_value_cstring(whilexpv), mom_value_cstring(bodytup),
			nbcont, nbbreak);
	if (whilexpv == nullptr || whilexpv == MOM_PREDEFITM(truth))
	  {
	    prologtree = literal_string("for(;;)");
	  }
	else
	  {
	    prologtree = mom_boxnode_make_va(MOM_PREDEFITM(sequence),3,
					     literal_string("while ("),
					     transform_expr(whilexpv,blkitm),
					     literal_string(")"));
	  };
	MOM_DEBUGPRINTF(gencod,
			"c-transform_block blkitm=%s loop prologtree=%s",
			mom_item_cstring(blkitm), mom_value_cstring(prologtree));
	int bodylen = mom_raw_size(bodytup);
	momvalue_t smalbodyarr[8]= {};
	momvalue_t* bodyarr = (bodylen<(int)(sizeof(smalbodyarr)/sizeof(momvalue_t)))
	  ? smalbodyarr
	  : (momvalue_t*) mom_gc_alloc(bodylen*sizeof(momvalue_t));
	for (int bix=0; bix<bodylen; bix++)
	  {
	    struct mom_item_st*insitm = bodytup->seqitem[bix];
	    MOM_DEBUGPRINTF(gencod,
			    "c-transform_block loop blkitm=%s bix#%d insitm:=\n%s",
			    mom_item_cstring(blkitm), bix, mom_item_content_cstring(insitm));
	    assert (is_locked_item(insitm));
	    auto instree = transform_instruction(insitm, blkitm);
	    MOM_DEBUGPRINTF(gencod,
			    "c-transform_block loop insitm=%s instree=%s",
			    mom_item_cstring(insitm), mom_value_cstring(instree));
	    assert (instree != nullptr);
	    bodyarr[bix] = instree;
	  }
	auto bodytree = mom_boxnode_make(MOM_PREDEFITM(semicolon),bodylen,bodyarr);
	auto labcontree = (nbcont>0)
	  ?mom_boxnode_make_va(MOM_PREDEFITM(sequence),4,
			       literal_string(" "),
			       literal_string(CCONTINUELAB_PREFIX),
			       blkitm,
			       literal_string(":;"))
	  :nullptr;
	auto labreaktree = (nbbreak>0)
	  ?mom_boxnode_make_va(MOM_PREDEFITM(sequence),4,
			       literal_string(" "),
			       literal_string(CBREAKLAB_PREFIX),
			       blkitm,
			       literal_string(":;"))
	  :nullptr;
	MOM_DEBUGPRINTF(gencod,
			"c-transform_block blkitm=%s loop prologtree=%s bodytree=%s labcontree=%s labreaktree=%s",
			mom_item_cstring(blkitm),
			mom_value_cstring(prologtree),
			mom_value_cstring(bodytree),
			mom_value_cstring(labcontree),
			mom_value_cstring(labreaktree));
	auto looptree =
	  mom_boxnode_make_va(MOM_PREDEFITM(semicolon),
			      4,
			      labcontree,
			      prologtree,
			      mom_boxnode_make_va(MOM_PREDEFITM(brace),1,bodytree),
			      labreaktree);
	MOM_DEBUGPRINTF(gencod,
			"c-transform_block blkitm=%s loop result looptree=%s",
			mom_item_cstring(blkitm),
			mom_value_cstring(looptree));
	return looptree;
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
  MOM_DEBUGPRINTF(gencod, "c-transform_node_expr start expnod=%s initm=%s",
                  mom_value_cstring(expnod), mom_item_cstring(initm));
  assert (expnod != nullptr && expnod->va_itype==MOMITY_NODE);
  auto connitm = expnod->nod_connitm;
  assert (connitm != nullptr && connitm->va_itype==MOMITY_ITEM);
  unsigned nodarity = mom_size(expnod);
  lock_item(connitm);
  MOM_DEBUGPRINTF(gencod, "c-transform_node_expr connitm=%s", mom_item_cstring(connitm));
#define NBEXPCONN_MOM 131
#define CASE_EXPCONN_MOM(Nam) momhashpredef_##Nam % NBEXPCONN_MOM:	\
  if (connitm == MOM_PREDEFITM(Nam)) goto foundcaseconn_##Nam;		\
  goto defaultcaseconn; foundcaseconn_##Nam
  switch (connitm->hva_hash % NBEXPCONN_MOM)
    {
    case CASE_EXPCONN_MOM(verbatim):
      {
	assert (nodarity == 1);
	auto verbv = expnod->nod_sons[0];
	MOM_DEBUGPRINTF(gencod, "c-transform_node_expr verbatim verbv=%s", mom_value_cstring(verbv));
	auto verbitm = mom_dyncast_item(verbv);
	if (verbitm != nullptr)
	  {
	    auto verbtree = transform_constant_item(verbitm,initm);
	    MOM_DEBUGPRINTF(gencod, "c-transform_node_expr verbatim verbitm=%s gives verbtree=%s",
			    mom_item_cstring(verbitm), mom_value_cstring(verbtree));
	    return verbtree;
	  }
#warning MomCEmitter::transform_node_expr verbatim unimplemented
	MOM_FATAPRINTF("unimplemented c-transform_node_expr of non-item verbatim-expr %s in %s",
		       mom_value_cstring(expnod), mom_item_cstring(initm));
      }
      break;
      ////
    case CASE_EXPCONN_MOM(get):
      {
	assert (nodarity == 2);
	auto ptrexpv = expnod->nod_sons[0];
	auto flditm = mom_dyncast_item(expnod->nod_sons[1]);
	MOM_DEBUGPRINTF(gencod, "c-transform_node_expr get ptrexpv %s flditm %s",
			mom_value_cstring(ptrexpv), mom_item_cstring(flditm));
	assert (is_locked_item(flditm));
	auto fldbind = get_binding(flditm);
	assert (fldbind != nullptr && fldbind->vd_rolitm == MOM_PREDEFITM(field));
	MOM_DEBUGPRINTF(gencod, "c-transform_node_expr get flditm=%s role %s what %s detail %s",
			mom_item_cstring(flditm),
			mom_item_cstring(fldbind->vd_rolitm),
			mom_value_cstring(fldbind->vd_what),
			mom_value_cstring(fldbind->vd_detail));
	auto ptrtree = transform_expr(ptrexpv, initm);
	auto fldtypitm = mom_dyncast_item(fldbind->vd_what);
	assert (fldtypitm != nullptr);
	MOM_DEBUGPRINTF(gencod, "c-transform_node_expr get flditm=%s fldtypitm=%s",
			mom_item_cstring(flditm), mom_item_cstring(fldtypitm));
	auto restree = mom_boxnode_make_sentinel(MOM_PREDEFITM(sequence),
						 literal_string("(/*get*/"),
						 literal_string("(("),
						 literal_string(CTYPE_PREFIX),
						 fldtypitm,
						 literal_string("*)"),
						 literal_string("("),
						 ptrtree,
						 literal_string("))->"),
						 literal_string(CFIELD_PREFIX),
						 flditm,
						 literal_string(")"));
	MOM_DEBUGPRINTF(gencod, "c-transform_node_expr %s gives %s",
			mom_value_cstring(expnod),
			mom_value_cstring(restree));
	return restree;
      }
      break;
      ////
    case CASE_EXPCONN_MOM(at):
      {
	assert (nodarity == 3);
	auto ptrexpv = expnod->nod_sons[0];
	auto flditm = mom_dyncast_item(expnod->nod_sons[1]);
	auto indexpv = expnod->nod_sons[2];
	MOM_DEBUGPRINTF(gencod, "c-transform_node_expr at ptrexpv %s flditm %s indexpv %s",
			mom_value_cstring(ptrexpv), mom_item_cstring(flditm),
			mom_value_cstring(indexpv));
	assert (is_locked_item(flditm));
	auto fldbind = get_binding(flditm);
	assert (fldbind != nullptr && fldbind->vd_rolitm == MOM_PREDEFITM(field));
	MOM_DEBUGPRINTF(gencod, "c-transform_node_expr at flditm %s role %s what %s detail %s",
			mom_item_cstring(flditm),
			mom_item_cstring(fldbind->vd_rolitm),
			mom_value_cstring(fldbind->vd_what),
			mom_value_cstring(fldbind->vd_detail));
	auto intypitm = mom_dyncast_item(fldbind->vd_what);
	auto fldtypv = (fldbind->vd_detail);
	assert (fldtypv != nullptr);
	MOM_DEBUGPRINTF(gencod, "c-transform_node_expr at flditm %s ptrexpv %s fldtypv %s intypitm=%s",
			mom_item_cstring(flditm), mom_value_cstring(ptrexpv),
			mom_value_cstring(fldtypv), mom_item_cstring(intypitm));
	auto ptrtree = transform_expr(ptrexpv, initm);
	MOM_DEBUGPRINTF(gencod, "c-transform_node_expr at flditm %s ptrexpv %s ptrtree %s",
			mom_item_cstring(flditm), mom_value_cstring(ptrexpv),
			mom_value_cstring(ptrtree));
	auto indextree = transform_expr(indexpv, initm);
	MOM_DEBUGPRINTF(gencod, "c-transform_node_expr at flditm=%s fldtypv=%s indextree=%s",
			mom_item_cstring(flditm), mom_value_cstring(fldtypv),
			mom_value_cstring(indextree));
	auto restree = mom_boxnode_make_sentinel(MOM_PREDEFITM(sequence),
						 literal_string("(/*at*/"),
						 literal_string("(("),
						 literal_string(CTYPE_PREFIX),
						 intypitm,
						 literal_string("*)"),
						 literal_string("("),
						 ptrtree,
						 literal_string("))->"),
						 literal_string(CFIELD_PREFIX),
						 flditm,
						 literal_string("["),
						 indextree,
						 literal_string("]"),
						 literal_string(")"));
	MOM_DEBUGPRINTF(gencod, "c-transform_node_expr %s gives %s",
			mom_value_cstring(expnod),
			mom_value_cstring(restree));
	return restree;
      }
      break;
      /////
    case CASE_EXPCONN_MOM(sizeof):
      {
	assert (nodarity == 1);
	auto sztypitm = mom_dyncast_item(expnod->nod_sons[0]);
	MOM_DEBUGPRINTF(gencod, "c-transform_node_expr sizeof sztypitm=%s",
			mom_item_cstring(sztypitm));
	assert (is_locked_item(sztypitm));
	if (_cec_declareditems.find(sztypitm) == _cec_declareditems.end())
	  {
	    auto dtree = declare_type(sztypitm);
	    MOM_DEBUGPRINTF(gencod, "c-transform_node_expr sizeof sztypitm=%s dtree=%s",
			    mom_item_cstring(sztypitm), mom_value_cstring(dtree));
	  }
	auto restree = mom_boxnode_make_va(MOM_PREDEFITM(sequence), 4,
					   literal_string("sizeof("),
					   literal_string(CTYPE_PREFIX),
					   sztypitm,
					   literal_string(")"));
	MOM_DEBUGPRINTF(gencod, "c-transform_node_expr sizeof sztypitm=%s gives restree=%s",
			mom_item_cstring(sztypitm), mom_value_cstring(restree));
	return restree;
      }
      break;
      ////
    case CASE_EXPCONN_MOM(and):
      goto orandcase;
    case CASE_EXPCONN_MOM(or):
    orandcase:
      {
        bool isor = connitm == MOM_PREDEFITM(or);
        assert (nodarity>0);
        traced_vector_values_t vectree;
        vectree.resize(2*nodarity + 4);
        vectree.push_back(literal_string("("));
        for (unsigned ix=0; ix<nodarity; ix++)
          {
            auto cursonexp = expnod->nod_sons[ix];
            MOM_DEBUGPRINTF(gencod, "c-transform_node_expr %s ix#%d cursonexp=%s",
                            isor?"or":"and", ix, mom_value_cstring(cursonexp));
            auto cursontree = transform_expr(cursonexp, initm);
            MOM_DEBUGPRINTF(gencod, "c-transform_node_expr %s ix#%d cursontree=%s",
                            isor?"or":"and", ix, mom_value_cstring(cursontree));
            if (ix>0)
              vectree.push_back(isor?literal_string(") || ("):literal_string(") && ("));
            else
              vectree.push_back(literal_string("("));
            vectree.push_back(cursontree);
          }
        vectree.push_back(literal_string(")"));
        vectree.push_back(literal_string(")"));
        auto orandtree = mom_boxnode_make(MOM_PREDEFITM(sequence), vectree.size(), vectree.data());
        vectree.clear();
        MOM_DEBUGPRINTF(gencod, "c-transform_node_expr %s gives %s",
                        isor?"or":"and", mom_value_cstring(orandtree));
        return orandtree;
      }
    break;
    /////
    case CASE_EXPCONN_MOM(plus):
      goto plusmultcase;
    case CASE_EXPCONN_MOM(mult):
    plusmultcase:
      {
        bool isplus = connitm == MOM_PREDEFITM(plus);
        assert (nodarity>0);
        traced_vector_values_t vectree;
        vectree.resize(2*nodarity + 4);
        vectree.push_back(literal_string("("));
        for (unsigned ix=0; ix<nodarity; ix++)
          {
            auto cursonexp = expnod->nod_sons[ix];
            MOM_DEBUGPRINTF(gencod, "c-transform_node_expr %s ix#%d cursonexp=%s",
                            isplus?"plus":"mult", ix, mom_value_cstring(cursonexp));
            auto cursontree = transform_expr(cursonexp, initm);
            MOM_DEBUGPRINTF(gencod, "c-transform_node_expr %s ix#%d cursontree=%s",
                            isplus?"plus":"mult", ix, mom_value_cstring(cursontree));
            if (ix>0)
              vectree.push_back(isplus?literal_string(") + ("):literal_string(") * ("));
            else
              vectree.push_back(literal_string("("));
            vectree.push_back(cursontree);
          }
        vectree.push_back(literal_string(")"));
        vectree.push_back(literal_string(")"));
        auto plusmulttree = mom_boxnode_make(MOM_PREDEFITM(sequence), vectree.size(), vectree.data());
        vectree.clear();
        MOM_DEBUGPRINTF(gencod, "c-transform_node_expr %s: %s gives %s",
                        isplus?"plus":"mult", mom_value_cstring(expnod), mom_value_cstring(plusmulttree));
        return plusmulttree;
      }
    break;
    case CASE_EXPCONN_MOM(sub):
      {
	assert (nodarity==2);
	MOM_DEBUGPRINTF(gencod, "c-transform_node_expr sub: %s", mom_value_cstring(expnod));
	auto leftree = transform_expr(expnod->nod_sons[0], initm);
	auto rightree = transform_expr(expnod->nod_sons[1], initm);
	auto restree = mom_boxnode_make_va(MOM_PREDEFITM(sequence), 7,
					   literal_string("("),
					   literal_string("("),
					   leftree,
					   literal_string(") - ("),
					   rightree,
					   literal_string(")"),
					   literal_string(")"));
	MOM_DEBUGPRINTF(gencod, "c-transform_node_expr sub gives restree=%s", mom_value_cstring(restree));
	return restree;
      }
      break;
    case CASE_EXPCONN_MOM(div):
      {
	MOM_DEBUGPRINTF(gencod, "c-transform_node_expr div: %s", mom_value_cstring(expnod));
	auto leftree = transform_expr(expnod->nod_sons[0], initm);
	auto rightree = transform_expr(expnod->nod_sons[1], initm);
	auto restree = mom_boxnode_make_va(MOM_PREDEFITM(sequence), 7,
					   literal_string("("),
					   literal_string("("),
					   leftree,
					   literal_string(") / ("),
					   rightree,
					   literal_string(")"),
					   literal_string(")"));
	MOM_DEBUGPRINTF(gencod, "c-transform_node_expr div gives restree=%s", mom_value_cstring(restree));
	return restree;
      }
      break;
    case CASE_EXPCONN_MOM(mod):
      {
	MOM_DEBUGPRINTF(gencod, "c-transform_node_expr mod: %s", mom_value_cstring(expnod));
	auto leftree = transform_expr(expnod->nod_sons[0], initm);
	auto rightree = transform_expr(expnod->nod_sons[1], initm);
	auto restree = mom_boxnode_make_va(MOM_PREDEFITM(sequence), 7,
					   literal_string("("),
					   literal_string("("),
					   leftree,
					   literal_string(") % ("),
					   rightree,
					   literal_string(")"),
					   literal_string(")"));
	MOM_DEBUGPRINTF(gencod, "c-transform_node_expr mod gives restree=%s", mom_value_cstring(restree));
	return restree;
      }
      break;
      ////
    case CASE_EXPCONN_MOM(tuple):
      goto tuplesetcase;
    case CASE_EXPCONN_MOM(set):
    tuplesetcase:
      {
        bool istuple = (connitm == MOM_PREDEFITM(tuple));
        MOM_DEBUGPRINTF(gencod, "c-transform_node_expr %s: %s", istuple?"tuple":"set",
                        mom_value_cstring(expnod));
        traced_vector_values_t vectree;
        vectree.resize(3*nodarity + 4);
        vectree.push_back(istuple
                          ?literal_string("/*tuple*/(mom_boxtuple_make_va(")
                          :literal_string("/*set*/(mom_boxset_make_va("));
        vectree.push_back(mom_int_make(nodarity));
        for (unsigned ix=0; ix<nodarity; ix++)
          {
            auto cursonexp = expnod->nod_sons[ix];
            MOM_DEBUGPRINTF(gencod, "c-transform_node_expr %s ix#%d cursonexp=%s",
                            istuple?"tuple":"set", ix, mom_value_cstring(cursonexp));
            auto cursontree = transform_expr(cursonexp, initm);
            MOM_DEBUGPRINTF(gencod, "c-transform_node_expr %s ix#%d cursontree=%s",
                            istuple?"tuple":"set", ix, mom_value_cstring(cursontree));
            vectree.push_back(literal_string(", ("));
            vectree.push_back(cursontree);
            vectree.push_back(literal_string(")"));
          }
        vectree.push_back(istuple
                          ?literal_string("))/*endtuple*/")
                          :literal_string("))/*endset*/"));
        auto restree = mom_boxnode_make(MOM_PREDEFITM(sequence), vectree.size(), vectree.data());
        vectree.clear();
        MOM_DEBUGPRINTF(gencod, "c-transform_node_expr %s gives %s",
                        istuple?"tuple":"set", mom_value_cstring(restree));
        return restree;
      }
    break;
    ////
    case CASE_EXPCONN_MOM(flatten_tuple):
      goto flatuplesetcase;
    case CASE_EXPCONN_MOM(flatten_set):
    flatuplesetcase:
      {
        bool istuple = (connitm == MOM_PREDEFITM(tuple));
        MOM_DEBUGPRINTF(gencod, "c-transform_node_expr %s: %s", istuple?"flatten_tuple":"dlatten_set",
                        mom_value_cstring(expnod));
        traced_vector_values_t vectree;
        vectree.resize(3*nodarity + 7);
        vectree.push_back(istuple
                          ?literal_string("/*flatten_tuple*/(mom_boxtuple_flatten_make_va((")
                          :literal_string("/*flatten_set*/(mom_boxset_flatten_make_va(("));
        auto itemexp = expnod->nod_sons[0];
        auto itemtree = transform_expr(itemexp,initm);
        vectree.push_back(itemtree);
        vectree.push_back(literal_string("),"));
        vectree.push_back(mom_int_make(nodarity));
        for (unsigned ix=1; ix<nodarity; ix++)
          {
            auto cursonexp = expnod->nod_sons[ix];
            MOM_DEBUGPRINTF(gencod, "c-transform_node_expr %s ix#%d cursonexp=%s",
                            istuple?"flatten_tuple":"flatten_set", ix, mom_value_cstring(cursonexp));
            auto cursontree = transform_expr(cursonexp, initm);
            MOM_DEBUGPRINTF(gencod, "c-transform_node_expr %s ix#%d cursontree=%s",
                            istuple?"flatten_tuple":"flatten_set", ix, mom_value_cstring(cursontree));
            vectree.push_back(literal_string(", ("));
            vectree.push_back(cursontree);
            vectree.push_back(literal_string(")"));
          }
        vectree.push_back(istuple
                          ?literal_string("))/*endflattentuple*/")
                          :literal_string("))/*endflattenset*/"));
        auto restree = mom_boxnode_make(MOM_PREDEFITM(sequence), vectree.size(), vectree.data());
        vectree.clear();
        MOM_DEBUGPRINTF(gencod, "c-transform_node_expr %s gives %s",
                        istuple?"flatten_tuple":"flatten_set", mom_value_cstring(restree));
        return restree;
      }
    break;
    ////
    case CASE_EXPCONN_MOM(node):
      {
	MOM_DEBUGPRINTF(gencod, "c-transform_node_expr node: %s",
			mom_value_cstring(expnod));
	assert (nodarity>0);
	traced_vector_values_t vectree;
	vectree.resize(3*nodarity + 4);
	vectree.push_back(literal_string("/*node*/(mom_boxnode_make_va(("));
	{
	  auto connexp = expnod->nod_sons[0];
	  auto conntree = transform_expr(connexp,initm);
	  MOM_DEBUGPRINTF(gencod,
			  "c-transform_node_expr node connexp=%s conntree=%s",
			  mom_value_cstring(connexp), mom_value_cstring(conntree));
	  vectree.push_back(conntree);
	}
	vectree.push_back(literal_string("),"));
	vectree.push_back(mom_int_make(nodarity-1));
	for (int ix=1; ix<(int)nodarity; ix++)
	  {
	    auto cursonexp = expnod->nod_sons[ix];
	    MOM_DEBUGPRINTF(gencod, "c-transform_node_expr node ix#%d cursonexp=%s",
			    ix, mom_value_cstring(cursonexp));
	    auto cursontree = transform_expr(cursonexp, initm);
	    MOM_DEBUGPRINTF(gencod, "c-transform_node_expr node ix#%d cursontree=%s",
			    ix, mom_value_cstring(cursontree));
	    vectree.push_back(literal_string(", ("));
	    vectree.push_back(cursontree);
	    vectree.push_back(literal_string(")"));
	  }
	vectree.push_back(literal_string(")/*endnode*/"));
	auto restree = mom_boxnode_make(MOM_PREDEFITM(sequence), vectree.size(), vectree.data());
	vectree.clear();
	MOM_DEBUGPRINTF(gencod, "c-transform_node_expr node gives %s",
			mom_value_cstring(restree));
	return restree;
      }
      break;
    default:
    defaultcaseconn:
      {
        MOM_DEBUGPRINTF(gencod, "c-transform_node_expr connitm=%s", mom_item_cstring(connitm));
        auto conndescitm = mom_unsync_item_descr(connitm);
        MOM_DEBUGPRINTF(gencod, "c-transform_node_expr conndescitm=%s", mom_item_cstring(conndescitm));
        if (conndescitm == MOM_PREDEFITM(primitive))
          {
            auto primtree = transform_node_primitive_expr(expnod, initm);
            MOM_DEBUGPRINTF(gencod, "c-transform_node_expr expnod=%s gives primtree=%s",
                            mom_value_cstring(expnod), mom_value_cstring(primtree));
            return primtree;
          }
        else if (conndescitm == MOM_PREDEFITM(inline))
          {
            auto inltree = transform_node_inline_expr(expnod, initm);
            MOM_DEBUGPRINTF(gencod, "c-transform_node_expr expnod=%s gives inltree=%s",
                            mom_value_cstring(expnod), mom_value_cstring(inltree));
            return inltree;
          }
        else if (conndescitm == MOM_PREDEFITM(type))
          {
            auto casttree = transform_node_cast_expr(expnod, initm);
            MOM_DEBUGPRINTF(gencod, "c-transform_node_expr expnod=%s gives casttree=%s",
                            mom_value_cstring(expnod), mom_value_cstring(casttree));
            return casttree;
          }
        else if (conndescitm == MOM_PREDEFITM(field))
          {
            auto fieldtree = transform_node_field_expr(expnod, initm);
            MOM_DEBUGPRINTF(gencod, "c-transform_node_expr expnod=%s gives fieldtree=%s",
                            mom_value_cstring(expnod), mom_value_cstring(fieldtree));
            return fieldtree;
          }
#warning MomCEmitter::transform_node_expr unimplemented
        MOM_FATAPRINTF("unimplemented c-transform_node_expr of default expr %s in %s with conndescitm=%s",
                       mom_value_cstring(expnod), mom_item_cstring(initm), mom_item_cstring(conndescitm));
      }
      break;
    }
#undef NBEXPCONN_MOM
#undef CASE_EXPCONN_MOM
#warning  MomCEmitter::transform_node_expr unimplemented
  MOM_FATAPRINTF("unimplemented c-transform_node_expr of %s in %s",
                 mom_value_cstring(expnod), mom_item_cstring(initm));
} // end MomCEmitter::transform_node_expr




momvalue_t
MomCEmitter::transform_node_primitive_expr(const struct mom_boxnode_st*expnod, struct mom_item_st*insitm)
{
  assert (mom_itype(expnod) == MOMITY_NODE);
  MOM_DEBUGPRINTF(gencod, "c-transform_node_primitive_expr start expnod=%s insitm=%s",
                  mom_value_cstring(expnod), mom_item_cstring(insitm));
  auto connitm = expnod->nod_connitm;
  unsigned nodarity = mom_raw_size(expnod);
  auto sigitm = //
    mom_dyncast_item(mom_unsync_item_get_phys_attr (connitm, MOM_PREDEFITM(signature)));
  assert (is_locked_item(sigitm));
  auto formaltup = //
    mom_dyncast_tuple(mom_unsync_item_get_phys_attr (sigitm, MOM_PREDEFITM(formals)));
  assert (formaltup != nullptr);
  unsigned nbformals = mom_size(formaltup);
  auto cxexpnod = //
    mom_dyncast_node(mom_unsync_item_get_phys_attr (connitm, MOM_PREDEFITM(c_expansion)));
  MOM_DEBUGPRINTF(gencod, "c-transform_node_primitive_expr cxexpnod=%s formaltup=%s",
                  mom_value_cstring(cxexpnod), mom_value_cstring(formaltup));
  if (!cxexpnod || cxexpnod->nod_connitm != MOM_PREDEFITM(code_chunk))
    throw MOM_RUNTIME_PRINTF("c-transform_node_primitive_expr primitive node %s bad cxexpnod=%s",
                             mom_value_cstring(expnod),
                             mom_value_cstring(cxexpnod));
  if (nbformals != nodarity)
    throw MOM_RUNTIME_PRINTF("c-transform_node_primitive_expr primitive node %s arity %d != nbformals %d",
                             mom_value_cstring(expnod), nodarity, nbformals);

  traced_map_item2value_t argmap;
  for (unsigned ix = 0; ix < nbformals; ix++)
    {
      auto curformitm = formaltup->seqitem[ix];
      assert (mom_itype(curformitm) == MOMITY_ITEM);
      assert (is_locked_item(curformitm));
      auto curftypitm =
        mom_dyncast_item(mom_unsync_item_get_phys_attr(curformitm, MOM_PREDEFITM(type)));
      auto curson = expnod->nod_sons[ix];
      MOM_DEBUGPRINTF(gencod, "c-transform_node_primitive_expr expnod=%s ix#%d curson=%s curformitm=%s curftypitm=%s",
                      mom_value_cstring(expnod),
                      ix, mom_value_cstring(curson),
                      mom_item_cstring(curformitm),
                      mom_item_cstring(curftypitm));
      auto curstree = transform_expr(expnod->nod_sons[ix], insitm, curftypitm);
      MOM_DEBUGPRINTF(gencod, "c-transform_node_primitive_expr expnod=%s ix#%d curstree=%s",
                      mom_value_cstring(expnod),
                      ix, mom_value_cstring(curstree));
      argmap[curformitm] = curstree;
    }
  traced_vector_values_t treevec;
  unsigned sizcxexp = mom_raw_size(cxexpnod);
  treevec.reserve(sizcxexp+1);
  for (unsigned cix=0; cix<sizcxexp; cix++)
    {
      auto curxson = cxexpnod->nod_sons[cix];
      auto curxitm = mom_dyncast_item(curxson);
      auto it = argmap.end();
      if (curxitm != nullptr && (it = argmap.find(curxitm)) != argmap.end())
        treevec.push_back(it->second);
      else
        treevec.push_back(curxson);
    }
  auto nodtree = mom_boxnode_make(MOM_PREDEFITM(sequence),
                                  treevec.size(),
                                  treevec.data());
  MOM_DEBUGPRINTF(gencod, "c-transform_node_primitive_expr primitive expnod=%s gives nodtree=%s",
                  mom_value_cstring(expnod), mom_value_cstring(nodtree));
  return nodtree;
} // end MomCEmitter::transform_node_primitive_expr




momvalue_t
MomCEmitter::transform_node_inline_expr(const struct mom_boxnode_st*expnod, struct mom_item_st*insitm)
{
  assert (mom_itype(expnod) == MOMITY_NODE);
  MOM_DEBUGPRINTF(gencod, "c-transform_node_inline_expr start expnod=%s insitm=%s",
                  mom_value_cstring(expnod), mom_item_cstring(insitm));
  auto connitm = expnod->nod_connitm;
  if (_cec_declareditems.find(connitm) == _cec_declareditems.end())
    {
      MOM_DEBUGPRINTF(gencod, "c-transform_node_inline_expr expnod=%s with fresh connitm=%s in insitm=%s",
                      mom_value_cstring(expnod), mom_item_cstring(connitm), mom_item_cstring(insitm));
      /// dont declare the inline now, but postponed after element
      todo_after_element([=](MomEmitter*em)
			 {
			   MomCEmitter*cem = dynamic_cast<MomCEmitter*>(em);
			   assert (cem != nullptr);
			   MOM_DEBUGPRINTF(gencod, "c-transform_node_inline_expr new connitm=%s", mom_item_cstring(connitm));
			   cem->_ce_localvarmap.clear();
			   cem->scan_module_element(connitm);
			   flush_todo_list(__LINE__);
			   auto inldeftree = cem->transform_inline_element(connitm);
			   MOM_DEBUGPRINTF(gencod, "c-transform_node_inline_expr connitm=%s @@@inldeftree=%s",
					   mom_item_cstring(connitm), mom_value_cstring(inldeftree));
			   flush_todo_list(__LINE__);
			   MOM_DEBUGPRINTF(gencod, "c-transform_node_inline_expr handled new connitm=%s", mom_item_cstring(connitm));
			 });
    }
  unsigned arity = mom_raw_size(expnod);
  traced_vector_values_t vectree;
  vectree.reserve(2*arity+5);
  vectree.push_back(mom_boxnode_make_va(MOM_PREDEFITM(sequence),
                                        2, literal_string(MOM_FUNC_PREFIX),  connitm));
  vectree.push_back(literal_string("("));
  for (unsigned ix=0; ix<arity; ix++)
    {
      if (ix>0) vectree.push_back(literal_string(","));
      vectree.push_back(transform_expr(expnod->nod_sons[ix],insitm));
    }
  vectree.push_back(literal_string(")"));
  auto itree = mom_boxnode_make(MOM_PREDEFITM(output), vectree.size(), vectree.data());
  MOM_DEBUGPRINTF(gencod, "c-transform_node_inline_expr expnod=%s gives itree=%s",
                  mom_value_cstring(expnod), mom_value_cstring(itree));
  return itree;
} // end of MomCEmitter::transform_node_inline_expr



momvalue_t
MomCEmitter::transform_node_cast_expr(const struct mom_boxnode_st*expnod, struct mom_item_st*insitm)
{
  assert (mom_itype(expnod) == MOMITY_NODE);
  MOM_DEBUGPRINTF(gencod, "c-transform_node_cast_expr start expnod=%s insitm=%s",
                  mom_value_cstring(expnod), mom_item_cstring(insitm));
  auto connitm = expnod->nod_connitm;
  assert (mom_size(expnod) == 1);
  assert (is_locked_item(connitm));
  assert (mom_unsync_item_descr(connitm)==MOM_PREDEFITM(type));
  auto ccodetree = mom_unsync_item_get_phys_attr(connitm, MOM_PREDEFITM(c_code));
  auto argtree = transform_expr(expnod->nod_sons[0], insitm);
  MOM_DEBUGPRINTF(gencod, "c-transform_node_cast_expr expnod=%s ccodetree=%s argtree=%s",
                  mom_value_cstring(expnod), mom_value_cstring(ccodetree),
                  mom_value_cstring(argtree));
  if (ccodetree != nullptr)
    {
      auto ctree = mom_boxnode_make_va(MOM_PREDEFITM(sequence), 6,
                                       literal_string("("),
                                       mom_boxnode_make_va(MOM_PREDEFITM(comment),
							   2,
							   literal_string("cast "),
							   connitm),
                                       ccodetree,
                                       literal_string(") ("),
                                       argtree,
                                       literal_string(")"));
      MOM_DEBUGPRINTF(gencod, "c-transform_node_cast_expr expnod=%s gives ctree=%s",
                      mom_value_cstring(expnod), mom_value_cstring(ctree));
      return ctree;
    }
  else
    {
      auto dtree = declare_type(connitm);
      MOM_DEBUGPRINTF(gencod, "c-transform_node_cast_expr connitm=%s dtree=%s",
                      mom_item_cstring(connitm), mom_value_cstring(dtree));
      auto catree = mom_boxnode_make_va(MOM_PREDEFITM(sequence), 5,
                                        literal_string("("),
                                        dtree,
                                        literal_string(") ("),
                                        argtree,
                                        literal_string(")"));
      MOM_DEBUGPRINTF(gencod, "c-transform_node_cast_expr expnod=%s gives catree=%s",
                      mom_value_cstring(expnod), mom_value_cstring(catree));
      return catree;
    }
} // end MomCEmitter::transform_node_cast_expr


momvalue_t
MomCEmitter::transform_node_field_expr(const struct mom_boxnode_st*expnod, struct mom_item_st*insitm)
{
  assert (mom_itype(expnod) == MOMITY_NODE);
  MOM_DEBUGPRINTF(gencod, "c-transform_node_field_expr start expnod=%s insitm=%s",
                  mom_value_cstring(expnod), mom_item_cstring(insitm));
  auto connitm = expnod->nod_connitm;
  assert (mom_size(expnod) == 1);
  assert (is_locked_item(connitm));
  assert (mom_unsync_item_descr(connitm)==MOM_PREDEFITM(field));
  auto fldbind = get_binding(connitm);
  assert (fldbind != nullptr);
  MOM_DEBUGPRINTF(gencod, "c-transform_node_field_expr expnod=%s fldbind role=%s what=%s",
                  mom_value_cstring(expnod),
                  mom_item_cstring(fldbind->vd_rolitm),
                  mom_value_cstring(fldbind->vd_what));
  auto intypitm = mom_dyncast_item(fldbind->vd_what);
  auto argtree = transform_expr(expnod->nod_sons[0], insitm);
  MOM_DEBUGPRINTF(gencod, "c-transform_node_field_expr expnod=%s argtree=%s",
                  mom_value_cstring(expnod),
                  mom_value_cstring(argtree));
  auto fldtree = //
    mom_boxnode_make_sentinel(MOM_PREDEFITM(sequence),
                              literal_string("("),
                              literal_string("/*field*/"),
                              literal_string("("),
                              literal_string("("),
                              literal_string(CTYPE_PREFIX),
                              intypitm,
                              literal_string("*"),
                              literal_string(")"),
                              literal_string(" "),
                              literal_string("("),
                              argtree,
                              literal_string(")"),
                              literal_string(")"),
                              literal_string("->"),
                              literal_string(CFIELD_PREFIX),
                              connitm,
                              literal_string(")"));
  MOM_DEBUGPRINTF(gencod, "c-transform_node_field_expr expnod=%s gives fldtree=%s",
                  mom_value_cstring(expnod),
                  mom_value_cstring(fldtree));
  return fldtree;
} // end MomCEmitter::transform_node_field_expr


momvalue_t
MomCEmitter::transform_expr(momvalue_t expv, struct mom_item_st*initm, struct mom_item_st*typitm)
{
  MOM_DEBUGPRINTF(gencod, "c-transform_expr start expv=%s initm=%s typitm=%s",
                  mom_value_cstring(expv), mom_item_cstring(initm), mom_item_cstring(typitm));
  unsigned expty = mom_itype(expv);
  switch (expty)
    {
    case MOMITY_NONE:
      if (typitm==MOM_PREDEFITM(int))
        {
          MOM_DEBUGPRINTF(gencod, "c-transform_expr nil gives 0 since typitm=%s", mom_item_cstring(typitm));
          return mom_int_make(0);
        }
      else if (typitm == MOM_PREDEFITM(bool))
        {
          MOM_DEBUGPRINTF(gencod, "c-transform_expr nil gives false since typitm=%s", mom_item_cstring(typitm));
          return literal_string("false");
        }
      else if (typitm == MOM_PREDEFITM(double))
        {
          MOM_DEBUGPRINTF(gencod, "c-transform_expr nil gives 0.0 since typitm=%s", mom_item_cstring(typitm));
          return mom_boxdouble_make (0.0);
        }
      else
        {
          MOM_DEBUGPRINTF(gencod, "c-transform_expr nil gives NULL since typitm=%s", mom_item_cstring(typitm));
          return literal_string("NULL");
        }
    case MOMITY_INT:
    case MOMITY_BOXDOUBLE:
      return expv;
    case MOMITY_BOXSTRING:
      return mom_boxnode_make_va(MOM_PREDEFITM(string),1,expv);
    case MOMITY_ITEM:
      {
	auto expitm = (struct mom_item_st*)expv;
	assert (is_locked_item(expitm));
	auto descitm = mom_unsync_item_descr(expitm);
	if (descitm == MOM_PREDEFITM(indirect))
	  {
	    auto indexpv = mom_unsync_item_get_phys_attr(expitm, MOM_PREDEFITM(indirect));
	    MOM_DEBUGPRINTF(gencod, "c-transform_expr indirect expitm=%s indexpv=%s",
			    mom_item_cstring(expitm), mom_value_cstring(indexpv));
	    auto indtree = transform_expr(indexpv, initm, typitm);
	    auto restree = mom_boxnode_make_va(MOM_PREDEFITM(sequence),6,
					       literal_string("("),
					       literal_string("/*indirectexp "),
					       expitm,
					       literal_string(":*/"),
					       indtree,
					       literal_string(")"));
	    MOM_DEBUGPRINTF(gencod, "c-transform_expr indirect expitm=%s gives restree=%s",
			    mom_item_cstring(expitm), mom_value_cstring(restree));
	    return restree;
	  }
	if (expitm == MOM_PREDEFITM(truth))
	  {
	    if (typitm == MOM_PREDEFITM(bool))
	      {
		auto truetree = literal_string("true");
		MOM_DEBUGPRINTF(gencod, "c-transform_expr expitm=%s gives truetree=%s",
				mom_item_cstring(expitm), mom_value_cstring(truetree));
		return truetree;
	      }
	    auto xtree = transform_constant_item(MOM_PREDEFITM(truth),initm);
	    MOM_DEBUGPRINTF(gencod, "c-transform_expr expitm=%s got xtree=%s",
			    mom_item_cstring(expitm), mom_value_cstring(xtree));
	    return xtree;
	  };
	auto expbind = get_binding(expitm);
	if (!expbind)
	  throw MOM_RUNTIME_PRINTF("c-transform_item expitm=%s unbound in initm=%s",
				   mom_item_cstring(expitm), mom_value_cstring(initm));
	auto rolitm = expbind?expbind->vd_rolitm:nullptr;
	MOM_DEBUGPRINTF(gencod, "c-transform_expr expitm:=%s\n.. bind rol %s what %s",
			mom_item_content_cstring(expitm), mom_item_cstring(rolitm),
			expbind?mom_value_cstring(expbind->vd_what):"°");
	////
#define NBROLE_MOM 31
#define CASE_ROLE_MOM(Nam) momhashpredef_##Nam % NBROLE_MOM:		\
	if (rolitm == MOM_PREDEFITM(Nam)) goto foundrolcase_##Nam;	\
	goto defaultrole; foundrolcase_##Nam
	switch (rolitm?rolitm->hva_hash % NBROLE_MOM : 0)
	  {
	  case CASE_ROLE_MOM(formal):
	    return transform_var(expitm,initm,expbind);
	  case CASE_ROLE_MOM(locals):
	    return transform_var(expitm,initm,expbind);
	  case CASE_ROLE_MOM(variable):
	    return transform_var(expitm,initm,expbind);
	  case CASE_ROLE_MOM(global):
	    return transform_var(expitm,initm,expbind);
	  case CASE_ROLE_MOM(item):
	    {
	      MOM_DEBUGPRINTF(gencod,
			      "c-transform_expr expitm=%s item",
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
	  case CASE_ROLE_MOM(constant):
	    {
	      MOM_DEBUGPRINTF(gencod,
			      "c-transform_expr expitm:=%s\n.. constant typitm=%s",
			      mom_item_content_cstring(expitm), mom_item_cstring(typitm));
	      auto expnod =
		mom_dyncast_node(mom_unsync_item_get_phys_attr(expitm, MOM_PREDEFITM(c_expansion)));
	      assert (expnod != nullptr && expnod->nod_connitm == MOM_PREDEFITM(code_chunk));
	      momvalue_t exptree = nullptr;
	      unsigned expsize = mom_size(expnod);
	      if (expsize == 1)
		exptree = expnod->nod_sons[0];
	      else
		exptree = mom_boxnode_make(MOM_PREDEFITM(sequence),
					   expsize,
					   (momvalue_t*)expnod->nod_sons);
	      MOM_DEBUGPRINTF(gencod,
			      "c-transform_expr const expitm=%s got exptree=%s",
			      mom_item_cstring(expitm), mom_value_cstring(exptree));
	      if (exptree)
		return exptree;
	      exptree =  transform_constant_item(expitm, initm);
	      MOM_DEBUGPRINTF(gencod,
			      "c-transform_expr const expitm=%s transformed to exptree=%s",
			      mom_item_cstring(expitm), mom_value_cstring(exptree));
	      return exptree;
	    }
	    break;
	  case CASE_ROLE_MOM(enumerator):
	    {
	      auto exptree = mom_boxnode_make_va(MOM_PREDEFITM(sequence), 2,
						 literal_string(CENUVAL_PREFIX),
						 expitm);
	      MOM_DEBUGPRINTF(gencod,
			      "c-transform_expr expitm=%s enumerator typitm=%s gives exptree=%s",
			      mom_item_cstring(expitm), mom_item_cstring(typitm), mom_value_cstring(exptree));
	      return exptree;
	    }
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
MomCEmitter::transform_type_for(momvalue_t typexpv, momvalue_t vartree, bool* scalarp)
{
  MOM_DEBUGPRINTF(gencod, "c-transform_type_for start typexpv=%s vartree=%s",
                  mom_value_cstring(typexpv),
                  mom_value_cstring(vartree));
  unsigned tytyp = mom_itype(typexpv);
  if (tytyp == MOMITY_ITEM)
    {
      auto typitm = (struct mom_item_st*)typexpv;
      lock_item(typitm);
      auto desctitm = mom_unsync_item_descr(typitm);
      if (desctitm == MOM_PREDEFITM(type))
        {
          auto ccodexp = mom_unsync_item_get_phys_attr(typitm, MOM_PREDEFITM(c_code));
          MOM_DEBUGPRINTF(gencod, "c-transform_type_for typitm=%s ccodexp=%s", mom_item_cstring(typitm), mom_value_cstring(ccodexp));
          if (ccodexp)
            {
              if (scalarp)
                *scalarp = true;
              auto tree = mom_boxnode_make_va(MOM_PREDEFITM(sequence), 3,
                                              ccodexp,
                                              literal_string(" "),
                                              vartree);
              MOM_DEBUGPRINTF(gencod, "c-transform_type_for typitm=%s gives %s for primitive c-type",
                              mom_item_cstring(typitm), mom_value_cstring(tree));
              return tree;
            }
          else
            {
              MOM_DEBUGPRINTF(gencod, "c-transform_type_for typitm=%s declaring",
                              mom_item_cstring(typitm));
              auto dtytree = declare_type(typitm, scalarp);
              MOM_DEBUGPRINTF(gencod, "c-transform_type_for typitm=%s so dtytree %s (%s)",
                              mom_item_cstring(typitm),
                              mom_value_cstring(dtytree),
                              scalarp?((*scalarp)?"scalar":"aggregate"):"dontcare");
              auto dtree = mom_boxnode_make_va(MOM_PREDEFITM(sequence), 4,
                                               literal_string(CTYPE_PREFIX),
                                               typitm,
                                               literal_string(" "),
                                               vartree);
              MOM_DEBUGPRINTF(gencod, "c-transform_type_for declared typitm=%s gives dtree=%s",
                              mom_item_cstring(typitm), mom_value_cstring(dtree));
              return dtree;
            }
        }
    }
  else if (tytyp == MOMITY_NODE)
    {
      auto typnod = (const struct mom_boxnode_st*)typexpv;
      unsigned sz = mom_raw_size(typexpv);
      auto connitm = typnod->nod_connitm;
      momvalue_t restree = nullptr;
      if (connitm == MOM_PREDEFITM(pointer) && sz == 1)
        {
          auto starvartree =  mom_boxnode_make_va(MOM_PREDEFITM(sequence), 2,
                                                  literal_string(" *"),
                                                  vartree);
          auto dereftypv = typnod->nod_sons[0];
          MOM_DEBUGPRINTF(gencod, "c-transform_type_for starvartree=%s dereftypv=%s",
                          mom_value_cstring(starvartree), mom_value_cstring(dereftypv));
          if (dereftypv != nullptr && dereftypv != MOM_PREDEFITM(unit))
            restree = transform_type_for(dereftypv, starvartree);
          else
            restree = mom_boxnode_make_va(MOM_PREDEFITM(sequence), 2,
                                          literal_string("void* "),
                                          vartree);
          if (scalarp)
            *scalarp = true;
        }
      else if (connitm == MOM_PREDEFITM(signature) && sz == 1)
        {
          auto sigitm = mom_dyncast_item(typnod->nod_sons[0]);
          assert (is_locked_item(sigitm));
          auto sigvartree =
            mom_boxnode_make_va(MOM_PREDEFITM(sequence), 5,
                                literal_string("/*signaturetype*/"),
                                literal_string(CSIGNTYPE_PREFIX),
                                sigitm,
                                literal_string("* "),
                                vartree);
          if (scalarp)
            *scalarp = true;
          MOM_DEBUGPRINTF(gencod, "c-transform_type_for sigvartree=%s sigitm=%s",
                          mom_value_cstring(sigvartree), mom_item_cstring(sigitm));
          restree = sigvartree;
        }
      else if (connitm == MOM_PREDEFITM(array) && sz == 2)
        {
          auto comptypv = typnod->nod_sons[0];
          auto dimv = typnod->nod_sons[1];
          auto dimitm = mom_dyncast_item(dimv);
          if (dimitm != nullptr)
            {
              dimv = nullptr;
              MOM_DEBUGPRINTF(gencod, "c-transform_type_for dimitm=%s in typnod=%s",
                              mom_item_cstring(dimitm), mom_value_cstring(typnod));
              lock_item(dimitm);
              momvalue_t dimvalv = nullptr;
              if (mom_unsync_item_descr(dimitm) == MOM_PREDEFITM(constant)
                  && (dimvalv = mom_unsync_item_get_phys_attr(dimitm, MOM_PREDEFITM(value))) != nullptr
                  && mom_int_val_def(dimvalv, -1) >=0)
                dimv = mom_boxnode_make_va(MOM_PREDEFITM(sequence),4,
                                           literal_string("/*constdim:"), dimitm, literal_string("*/"), dimvalv);
              else
                throw MOM_RUNTIME_PRINTF("bad dimension %s in array typnod %s",
                                         mom_item_cstring(dimitm), mom_value_cstring(typnod));
            }
          auto arrvartree =  mom_boxnode_make_va(MOM_PREDEFITM(sequence), 4,
                                                 vartree,
                                                 literal_string("["),
                                                 dimv,
                                                 literal_string("]"));
          MOM_DEBUGPRINTF(gencod, "c-transform_type_for arrvartree=%s comptypv=%s",
                          mom_value_cstring(arrvartree), mom_value_cstring(comptypv));
          restree = transform_type_for(comptypv, arrvartree);
          if (scalarp) *scalarp = false;
        }
      else if (connitm == MOM_PREDEFITM(flexible_array) && sz == 1)
        {
          auto comptypv = typnod->nod_sons[0];
          auto flexvartree =  mom_boxnode_make_va(MOM_PREDEFITM(sequence), 2,
                                                  vartree,
                                                  literal_string("[MOM_FLEXIBLE_DIM]"));
          MOM_DEBUGPRINTF(gencod, "c-transform_type_for flexvartree=%s comptypv=%s",
                          mom_value_cstring(flexvartree), mom_value_cstring(comptypv));
          restree = transform_type_for(comptypv, flexvartree);
          if (scalarp) *scalarp = false;
        }
      else goto invalidtype;
      MOM_DEBUGPRINTF(gencod, "c-transform_type_for typexpv=%s gives restree=%s",
                      mom_value_cstring(typexpv), mom_value_cstring(restree));
      if (restree) return restree;
      else goto invalidtype;
    }
  else
  invalidtype:
    throw MOM_RUNTIME_PRINTF("invalid typexpv %s to c-transform",
                             mom_value_cstring(typexpv));
#warning MomCEmitter::transform_type_for unimplemented
  MOM_FATAPRINTF("c-transform_type_for unimplemented typexpv=%s vartree=%s",
                 mom_value_cstring(typexpv),
                 mom_value_cstring(vartree));
} // end of MomCEmitter::transform_type_for



momvalue_t
MomCEmitter::transform_constant_item(struct mom_item_st*cstitm, struct mom_item_st*insitm)
{
  MOM_DEBUGPRINTF(gencod, "c-transform_constant_item start cstitm=%s insitm=%s",
                  mom_item_cstring(cstitm), mom_item_cstring(insitm));
  assert (is_locked_item(cstitm));
  if (mom_item_space(cstitm) == MOMSPA_PREDEF)
    {
      auto prnod =
        mom_boxnode_make_va(MOM_PREDEFITM(sequence),4,
                            literal_string(CPREDEFITEM_MACRO),
                            literal_string("("),
                            cstitm,
                            literal_string(")"));
      MOM_DEBUGPRINTF(gencod,
                      "c-transform_constant_item cstitm=%s predefined gives prnod=%s",
                      mom_item_cstring(cstitm), mom_value_cstring(prnod));
      return prnod;
    }
#warning  MomCEmitter::transform_constant_item not fully implemented
  MOM_FATAPRINTF("c-transform_constant_item non-predefined cstitm=%s initm=%s",
                 mom_item_cstring(cstitm), mom_item_cstring(insitm));
} // end MomCEmitter::transform_constant_item



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
    case CASE_ROLE_MOM(variable):
      vartree =
        mom_boxnode_make_va(MOM_PREDEFITM(sequence),2,
                            literal_string(CLOCAL_PREFIX),
                            varitm);
      break;
    case CASE_ROLE_MOM(global):
      vartree =
        mom_boxnode_make_va(MOM_PREDEFITM(sequence),2,
                            literal_string(CDATA_PREFIX),
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
  if (vartree)
    return vartree;
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
  auto descitm = mom_unsync_item_descr(insitm);
  if (descitm == MOM_PREDEFITM(indirect))
    {
      auto indiritm = mom_dyncast_item(mom_unsync_item_get_phys_attr(insitm, MOM_PREDEFITM(indirect)));
      MOM_DEBUGPRINTF(gencod, "c-transform_instruction indirect insitm=%s indiritm=%s",
                      mom_item_cstring(insitm), mom_item_cstring(indiritm));
      assert (is_locked_item(indiritm));
      auto indirtree = transform_instruction(indiritm, fromitm);
      MOM_DEBUGPRINTF(gencod, "c-transform_instruction indirect insitm=%s indiritm=%s indirtree=%s",
                      mom_item_cstring(insitm), mom_item_cstring(indiritm), mom_value_cstring(indirtree));
      auto restree = mom_boxnode_make_va(MOM_PREDEFITM(sequence), 4,
                                         literal_string("/*indirectinstr "),
                                         insitm,
                                         literal_string(":*/"),
                                         indirtree);
      MOM_DEBUGPRINTF(gencod, "c-transform_instruction indirect insitm=%s gives restree=%s",
                      mom_item_cstring(insitm), mom_value_cstring(restree));
      return restree;
    }
  auto insbind = get_local_binding(insitm);
  assert (insbind != nullptr);
  MOM_DEBUGPRINTF(gencod,
                  "c-transform_instruction insitm=%s insbind rol %s what %s detail %s rank %ld",
                  mom_item_cstring(insitm), mom_item_cstring(insbind->vd_rolitm),
                  mom_value_cstring(insbind->vd_what), mom_value_cstring(insbind->vd_detail), insbind->vd_rank);
  struct mom_item_st*rolitm = insbind->vd_rolitm;
  auto whatv = insbind->vd_what;
  assert (mom_itype(rolitm) == MOMITY_ITEM);
#define NBINSTROLE_MOM 73
#define CASE_INSTROLE_MOM(Nam) momhashpredef_##Nam % NBINSTROLE_MOM:	\
  if (rolitm == MOM_PREDEFITM(Nam)) goto foundcase_##Nam;		\
  goto defaultcaseirole; foundcase_##Nam
  switch (rolitm->hva_hash % NBINSTROLE_MOM)
    {
      ////
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
      ////
    case CASE_INSTROLE_MOM(assign):
      {
	auto whatnod = mom_dyncast_node(whatv);
	assert (whatnod != nullptr && whatnod->nod_connitm == MOM_PREDEFITM(assign) && mom_raw_size(whatnod) == 3);
	auto toexp = whatnod->nod_sons[0];
	auto fromexp = whatnod->nod_sons[1];
	auto totypitm = mom_dyncast_item(whatnod->nod_sons[2]);
	MOM_DEBUGPRINTF(gencod,
			"c-transform_instruction assign insitm=%s toexp=%s fromexp=%s totypitm=%s",
			mom_item_cstring(insitm), mom_value_cstring(toexp),
			mom_value_cstring(fromexp), mom_item_cstring(totypitm));
	assert (toexp != nullptr);
	assert (totypitm != nullptr);
	auto fromtree =  transform_expr(fromexp, insitm);
	// perhaps should handle the lack of fromtree when non-scalar totypitm
	MOM_DEBUGPRINTF(gencod,
			"c-transform_instruction assign insitm=%s fromtree=%s",
			mom_item_cstring(insitm), mom_value_cstring(fromtree));
	auto totree = transform_expr(toexp, insitm);
	MOM_DEBUGPRINTF(gencod,
			"c-transform_instruction assign insitm=%s totree=%s",
			mom_item_cstring(insitm), mom_value_cstring(totree));
	auto assitree = mom_boxnode_make_va(MOM_PREDEFITM(sequence),3,
					    totree, literal_string (" = "), fromtree);
	MOM_DEBUGPRINTF(gencod,
			"c-transform_instruction assign insitm=%s assitree=%s",
			mom_item_cstring(insitm), mom_value_cstring(assitree));
	return assitree;
      }
      break;
      ////
    case CASE_INSTROLE_MOM(break):
      {
	auto outblkitm = mom_dyncast_item(whatv);
	MOM_DEBUGPRINTF(gencod,
			"c-transform_instruction break insitm=%s outblkitm=%s",
			mom_item_cstring(insitm), mom_item_cstring(outblkitm));
	assert (outblkitm != nullptr && is_locked_item(outblkitm));
	assert (_ce_breakcountmap[outblkitm] > 0);
	auto breaktree = MOM_IS_DEBUGGING(gencod)
	  ? mom_boxnode_make_va(MOM_PREDEFITM(sequence),4,
				mom_boxnode_make_va(MOM_PREDEFITM(comment),4,
						    literal_string("break insitm="),
						    insitm,
						    literal_string(" outblkitm="),
						    outblkitm),
				literal_string("goto "),
				literal_string(CBREAKLAB_PREFIX),
				outblkitm)
	  : mom_boxnode_make_va(MOM_PREDEFITM(sequence),3,
				literal_string("goto "),
				literal_string(CBREAKLAB_PREFIX),
				outblkitm);
	MOM_DEBUGPRINTF(gencod,
			"c-transform_instruction break insitm=%s breaktree=%s",
			mom_item_cstring(insitm), mom_value_cstring(breaktree));
	return breaktree;
      }
      break;
      ////
    case CASE_INSTROLE_MOM(continue):
      {
	auto outblkitm = mom_dyncast_item(whatv);
	MOM_DEBUGPRINTF(gencod,
			"c-transform_instruction continue insitm=%s outblkitm=%s",
			mom_item_cstring(insitm), mom_item_cstring(outblkitm));
	assert (outblkitm != nullptr && is_locked_item(outblkitm));
	assert (_ce_continuecountmap[outblkitm] > 0);
	auto continuetree = MOM_IS_DEBUGGING(gencod)
	  ? mom_boxnode_make_va(MOM_PREDEFITM(sequence),4,
				mom_boxnode_make_va(MOM_PREDEFITM(comment),4,
						    literal_string("continue insitm="),
						    insitm,
						    literal_string(" outblkitm="),
						    outblkitm),
				literal_string("goto "),
				literal_string(CCONTINUELAB_PREFIX),
				outblkitm)
	  : mom_boxnode_make_va(MOM_PREDEFITM(sequence),3,
				literal_string("goto "),
				literal_string(CCONTINUELAB_PREFIX),
				outblkitm);
	MOM_DEBUGPRINTF(gencod,
			"c-transform_instruction continue insitm=%s continuetree=%s",
			mom_item_cstring(insitm), mom_value_cstring(continuetree));
	return continuetree;
      }
      break;
      ////
    case CASE_INSTROLE_MOM(switch):
      {
	MOM_DEBUGPRINTF(gencod,
			"c-transform_instruction switch insitm=%s whatv=%s",
			mom_item_cstring(insitm), mom_value_cstring(whatv));
	auto switchtree = transform_switchinstr(insitm, whatv, fromitm);
	MOM_DEBUGPRINTF(gencod,
			"c-transform_instruction switch insitm=%s gives switchtree=%s",
			mom_item_cstring(insitm), mom_value_cstring(switchtree));
	return switchtree;
      }
      break;
      ////
    case CASE_INSTROLE_MOM(cond):
      {
	MOM_DEBUGPRINTF(gencod,
			"c-transform_instruction cond insitm:=%s\n... whatv=%s",
			mom_item_content_cstring(insitm),
			mom_value_cstring(whatv));
	auto condtup = mom_dyncast_tuple(whatv);
	assert (condtup != nullptr);
	unsigned nbcond = mom_size(condtup);
	traced_vector_values_t vecondtree;
	vecondtree.reserve(2*nbcond+4);
	vecondtree.push_back(mom_boxnode_make_va(MOM_PREDEFITM(out_newline),0));
	vecondtree.push_back(mom_boxnode_make_va(MOM_PREDEFITM(comment),3,
						 literal_string("cond"),
						 literal_string(" "),
						 insitm));
	for (int cix=0; cix<(int)nbcond; cix++)
	  {
	    auto curconditm = condtup->seqitem[cix];
	    MOM_DEBUGPRINTF(gencod,
			    "c-transform_instruction cond insitm=%s cix#%d curconditm:=%s",
			    mom_item_cstring(insitm), cix, mom_item_content_cstring(curconditm));
	    assert (is_locked_item(curconditm));
	    assert (mom_unsync_item_descr(curconditm) == MOM_PREDEFITM(test));
	    momvalue_t iftree = nullptr;
	    momvalue_t thentree = nullptr;
	    auto testexpv = mom_unsync_item_get_phys_attr(curconditm, MOM_PREDEFITM(test));
	    auto thenitm = mom_dyncast_item(mom_unsync_item_get_phys_attr(curconditm, MOM_PREDEFITM(then)));
	    assert (is_locked_item(thenitm));
	    MOM_DEBUGPRINTF(gencod,
			    "c-transform_instruction cond insitm=%s cix#%d testexpv=%s thenitm=%s",
			    mom_item_cstring(insitm), cix,
			    mom_value_cstring(testexpv), mom_item_cstring(thenitm));
	    if (cix+1 < (int)nbcond
		&& testexpv != (momvalue_t)MOM_PREDEFITM(unit)
		&& testexpv != (momvalue_t)MOM_PREDEFITM(truth))
	      {
		auto testtree = transform_expr(testexpv, insitm);
		MOM_DEBUGPRINTF(gencod,
				"c-transform_instruction cond insitm=%s cix#%d testtree=%s",
				mom_item_cstring(insitm), cix,
				mom_value_cstring(testtree));
		iftree =  mom_boxnode_make_va(MOM_PREDEFITM(sequence),7,
					      mom_boxnode_make_va(MOM_PREDEFITM(out_newline),0),
					      mom_boxnode_make_va(MOM_PREDEFITM(comment), 2, literal_string("test:"), curconditm),
					      mom_boxnode_make_va(MOM_PREDEFITM(out_newline),0),
					      (cix==0)?(literal_string("if ")):literal_string("else if "),
					      literal_string("("),
					      testtree,
					      literal_string(")"));

	      }
	    else
	      {
		MOM_DEBUGPRINTF(gencod,
				"c-transform_instruction cond insitm=%s cix#%d lastestexpv=%s curconditm=%s thenitm=%s",
				mom_item_cstring(insitm), cix,
				mom_value_cstring(testexpv),
				mom_item_cstring(curconditm),
				mom_item_cstring(thenitm));
		if (testexpv == nullptr || testexpv == (momvalue_t)MOM_PREDEFITM(unit) || testexpv == (momvalue_t)MOM_PREDEFITM(truth))
		  {
		    iftree =
		      mom_boxnode_make_va(MOM_PREDEFITM(sequence),3,
					  mom_boxnode_make_va(MOM_PREDEFITM(out_newline),0),
					  mom_boxnode_make_va(MOM_PREDEFITM(comment), 2, literal_string("lastunitest:"), curconditm),
					  literal_string ("else")
					  );
		  }
		else
		  {
		    auto latestree = transform_expr(testexpv, insitm);
		    MOM_DEBUGPRINTF(gencod,
				    "c-transform_expr cond insitm=%s cix#%d latestree= %s",
				    mom_item_cstring(insitm), cix, mom_value_cstring(latestree));
		    iftree =  mom_boxnode_make_va(MOM_PREDEFITM(sequence),7,
						  mom_boxnode_make_va(MOM_PREDEFITM(out_newline),0),
						  mom_boxnode_make_va(MOM_PREDEFITM(comment), 2, literal_string("latest:"), curconditm),
						  mom_boxnode_make_va(MOM_PREDEFITM(out_newline),0),
						  (cix==0)?(literal_string("if ")):literal_string("else if "),
						  literal_string("("),
						  latestree,
						  literal_string(")"));
		  }
	      }
	    MOM_DEBUGPRINTF(gencod,
			    "c-transform_instruction cond insitm=%s cix#%d iftree=%s",
			    mom_item_cstring(insitm), cix,
			    mom_value_cstring(iftree));
	    vecondtree.push_back(iftree);
	    auto thendescitm = mom_unsync_item_descr(thenitm);
	    if (thendescitm==MOM_PREDEFITM(sequence) || thendescitm==MOM_PREDEFITM(loop))
	      thentree = transform_block(thenitm, insitm);
	    else
	      {
		auto theninstree = transform_instruction(thenitm, insitm);
		thentree = mom_boxnode_make_va(MOM_PREDEFITM(brace),2,
					       theninstree,
					       literal_string(";"));
	      }
	    MOM_DEBUGPRINTF(gencod,
			    "c-transform_instruction cond insitm=%s cix#%d thentree=%s",
			    mom_item_cstring(insitm), cix,
			    mom_value_cstring(thentree));
	    vecondtree.push_back(thentree);
	  }
	auto condtree = mom_boxnode_make(MOM_PREDEFITM(brace),vecondtree.size(),vecondtree.data());
	vecondtree.clear();
	MOM_DEBUGPRINTF(gencod,
			"c-transform_instruction cond insitm=%s gives condtree=%s",
			mom_item_cstring(insitm), mom_value_cstring(condtree));
	return condtree;
      }
      break;
      ////
    case CASE_INSTROLE_MOM(return):
      {
	MOM_DEBUGPRINTF(gencod,
			"c-transform_instruction return insitm=%s whatv=%s",
			mom_item_cstring(insitm), mom_value_cstring(whatv));
	auto retypitm = mom_dyncast_item(whatv);
	assert (is_locked_item(retypitm));
	auto retexpv = mom_unsync_item_get_phys_attr(insitm, MOM_PREDEFITM(return));
	MOM_DEBUGPRINTF(gencod,
			"c-transform_instruction return insitm=%s retexpv=%s retypitm=%s",
			mom_item_cstring(insitm), mom_value_cstring(retexpv),
			mom_item_cstring(retypitm));
	momvalue_t retexptree = nullptr;
	if (retypitm != nullptr && retypitm != MOM_PREDEFITM(unit))
	  retexptree = transform_expr(retexpv, insitm, retypitm);
	MOM_DEBUGPRINTF(gencod,
			"c-transform_instruction return insitm=%s retexptree=%s",
			mom_item_cstring(insitm), mom_value_cstring(retexptree));
	auto retstmtree = retexptree
	  ? mom_boxnode_make_va(MOM_PREDEFITM(sequence),3,
				literal_string("return ("),
				retexptree,
				literal_string(");"))
	  : literal_string("return;");
	MOM_DEBUGPRINTF(gencod,
			"c-transform_instruction return insitm=%s gives retstmtree=%s",
			mom_item_cstring(insitm), mom_value_cstring(retstmtree));
	return retstmtree;
      }
      break;
    default:
    defaultcaseirole:
      MOM_FATAPRINTF("c-transform_instruction unexpected role %s in insitm %s",
                     mom_item_cstring(rolitm), mom_item_cstring(insitm));
      break;
    }
#undef NBINSTROLE_MOM
#undef CASE_INSTROLE_MOM
#warning unimplemented MomCEmitter::transform_instruction
  MOM_FATAPRINTF("unimplemented c-transform_instruction insitm=%s", mom_item_cstring(insitm));
} // end of MomCEmitter::transform_instruction




momvalue_t
MomCEmitter::transform_runinstr(struct mom_item_st*insitm, struct mom_item_st*runitm,
                                struct mom_item_st*fromitm)
{
  MOM_DEBUGPRINTF(gencod, "c-transform_runinstr fromitm=%s runitm:=\n%s insitm:=\n%s",
                  mom_item_cstring(fromitm), mom_item_content_cstring(runitm), mom_item_content_cstring(insitm));
  assert (is_locked_item(insitm));
  assert (is_locked_item(runitm));
  auto desrunitm = mom_unsync_item_descr(runitm);
  auto inscomp = insitm->itm_pcomp;
  unsigned nbcomp = mom_vectvaldata_count(inscomp);
  auto resexp = mom_unsync_item_get_phys_attr(insitm, MOM_PREDEFITM(result));
  if (desrunitm == MOM_PREDEFITM(primitive))
    {
      auto sigitm = //
        mom_dyncast_item(mom_unsync_item_get_phys_attr (runitm, MOM_PREDEFITM(signature)));
      MOM_DEBUGPRINTF(gencod, "c-transform_runinstr runitm=%s sigitm:=\n%s",
                      mom_item_cstring(runitm), mom_item_content_cstring(sigitm));
      assert (is_locked_item(sigitm));
      auto formaltup = //
        mom_dyncast_tuple(mom_unsync_item_get_phys_attr (sigitm, MOM_PREDEFITM(formals)));
      assert (formaltup != nullptr);
      unsigned nbformals = mom_size(formaltup);
      traced_map_item2value_t argmap;
      momvalue_t treev = nullptr;
      auto expnod = //
        mom_dyncast_node(mom_unsync_item_get_phys_attr (runitm, MOM_PREDEFITM(c_expansion)));
      MOM_DEBUGPRINTF(gencod, "c-transform_runinstr formaltup=%s expnod=%s", mom_value_cstring(formaltup),
                      mom_value_cstring(expnod));
      if (!expnod || expnod->nod_connitm != MOM_PREDEFITM(code_chunk))
        throw MOM_RUNTIME_PRINTF("c-transform_runinstr insitm=%s runitm=%s bad expnod=%s",
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
      momvalue_t* treearr = (exparity+1<(int)(sizeof(smalltreearr)/sizeof(momvalue_t)))
	? smalltreearr
	: (momvalue_t*) mom_gc_alloc((exparity+2)*sizeof(momvalue_t));
      int treecnt = 0;
      if (MOM_IS_DEBUGGING(gencod))
        treearr[treecnt++] = //
          mom_boxnode_make_sentinel(MOM_PREDEFITM(comment),
                                    literal_string("run insitm="),
                                    insitm,
                                    literal_string(" runitm="),
                                    runitm);
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
          treearr[treecnt++] = curtreev;
        }
      treev = mom_boxnode_make(MOM_PREDEFITM(sequence),treecnt,treearr);
      MOM_DEBUGPRINTF(gencod, "c-transform_runinstr insitm=%s treev=%s",
                      mom_item_cstring(insitm), mom_value_cstring(treev));
      if (resexp != nullptr)
        {
          MOM_DEBUGPRINTF(gencod, "c-transform_runinstr insitm=%s resexp=%s",
                          mom_item_cstring(insitm), mom_value_cstring(resexp));
          auto restree = transform_expr(resexp, insitm);
          MOM_DEBUGPRINTF(gencod, "c-transform_runinstr insitm=%s restree=%s",
                          mom_item_cstring(insitm), mom_value_cstring(restree));
          treev = mom_boxnode_make_va(MOM_PREDEFITM(sequence), 3,
                                      restree, literal_string(" = "),
                                      restree);
          MOM_DEBUGPRINTF(gencod, "c-transform_runinstr insitm=%s resulting treev=%s",
                          mom_item_cstring(insitm), mom_value_cstring(treev));
        }
      return treev;
    } // end if desrunitm is primitive
  else  if (desrunitm == MOM_PREDEFITM(routine))
    {
      auto sigitm = //
        mom_dyncast_item(mom_unsync_item_get_phys_attr (runitm, MOM_PREDEFITM(signature)));
      MOM_DEBUGPRINTF(gencod, "c-transform_runinstr routine runitm=%s sigitm:=\n%s",
                      mom_item_cstring(runitm), mom_item_content_cstring(sigitm));
      assert (is_locked_item(sigitm));

      if (_cec_declareditems.find(sigitm) == _cec_declareditems.end())
        {
          _cec_declareditems.insert(sigitm);
          auto sigtypnod = declare_signature_type(sigitm);
          MOM_DEBUGPRINTF(gencod, "c-transform_runinstr routine insitm=%s sigitm %s sigtypnod %s",
                          mom_item_cstring(insitm),
                          mom_item_cstring(sigitm), mom_value_cstring(sigtypnod));
          add_global_decl(sigtypnod);
        }
      if (_cec_declareditems.find(runitm) == _cec_declareditems.end())
        {
          _cec_declareditems.insert(runitm);
          auto rtdecl =
            mom_boxnode_make_sentinel(MOM_PREDEFITM(sequence),
                                      literal_string("extern"),
                                      literal_string(" "),
                                      literal_string(CSIGNTYPE_PREFIX),
                                      sigitm,
                                      literal_string(" "),
                                      literal_string(MOM_FUNC_PREFIX),
                                      runitm,
                                      literal_string(";"));
          add_global_decl(rtdecl);
          MOM_DEBUGPRINTF(gencod, "c-transform_runinstr routine insitm=%s runitm %s rtdecl %s",
                          mom_item_cstring(insitm),
                          mom_item_cstring(runitm),
                          mom_value_cstring(rtdecl));
        }
      auto formaltup = //
        mom_dyncast_tuple(mom_unsync_item_get_phys_attr (sigitm, MOM_PREDEFITM(formals)));
      MOM_DEBUGPRINTF(gencod, "c-transform_runinstr routine runitm=%s formaltup=%s", mom_item_cstring(runitm), mom_value_cstring(formaltup));
      assert (formaltup != nullptr);
      unsigned nbformals = mom_size(formaltup);
      assert (nbformals == nbcomp);
      traced_vector_values_t vecargtree;
      vecargtree.reserve (nbformals+1);
      for (unsigned aix=0; aix<nbcomp; aix++)
        {
          auto curarg = mom_vectvaldata_nth(inscomp, aix);
          MOM_DEBUGPRINTF(gencod, "c-transform_runinstr routine insitm=%s aix#%d curarg: %s",
                          mom_item_cstring(insitm),
                          aix,
                          mom_value_cstring(curarg));
          auto argtree =  transform_expr(curarg, insitm);
          MOM_DEBUGPRINTF(gencod, "c-transform_runinstr routine insitm=%s aix#%d curarg: %s argtree=%s",
                          mom_item_cstring(insitm),
                          aix,
                          mom_value_cstring(curarg), mom_value_cstring(argtree));
          vecargtree.push_back(argtree);
        }
      auto argstree = mom_boxnode_make(MOM_PREDEFITM(comma),
                                       vecargtree.size(),
                                       vecargtree.data());
      MOM_DEBUGPRINTF(gencod, "c-transform_runinstr routine insitm=%s argstree=%s",
                      mom_item_cstring(insitm),
                      mom_value_cstring(argstree));
      auto calltree = mom_boxnode_make_va(MOM_PREDEFITM(sequence),
                                          6,
                                          literal_string(MOM_FUNC_PREFIX),
                                          runitm,
                                          literal_string("("),
                                          argstree,
                                          literal_string(")"),
                                          literal_string(";"));
      momvalue_t treev = calltree;
      if (resexp != nullptr)
        {
          MOM_DEBUGPRINTF(gencod, "c-transform_runinstr insitm=%s resexp=%s",
                          mom_item_cstring(insitm), mom_value_cstring(resexp));
          auto restree = transform_expr(resexp, insitm);
          MOM_DEBUGPRINTF(gencod, "c-transform_runinstr insitm=%s restree=%s",
                          mom_item_cstring(insitm), mom_value_cstring(restree));
          treev = mom_boxnode_make_va(MOM_PREDEFITM(sequence), 3,
                                      restree, literal_string(" = "),
                                      restree);
          MOM_DEBUGPRINTF(gencod, "c-transform_runinstr insitm=%s resulting treev=%s",
                          mom_item_cstring(insitm), mom_value_cstring(treev));
        }
      else
        MOM_DEBUGPRINTF(gencod, "c-transform_runinstr insitm=%s gives treev=%s",
                        mom_item_cstring(insitm), mom_value_cstring(treev));
      return treev;
    } // end if desrunitm is routine
#warning unimplemented MomCEmitter::transform_runinstr
  MOM_FATAPRINTF("unimplemented c-transform_runinstr insitm=%s", mom_item_cstring(insitm));
} // end of MomCEmitter::transform_runinstr



momvalue_t
MomCEmitter::transform_switchinstr(struct mom_item_st*insitm,  momvalue_t whatv,
                                   struct mom_item_st*fromitm)
{
  MOM_DEBUGPRINTF(gencod, "c-transform_switchinstr insitm:=\n%s\n.. whatv=%s\n.. fromitm=%s",
                  mom_item_content_cstring(insitm),
                  mom_value_cstring(whatv),
                  mom_item_cstring(fromitm));
  auto whatnod = mom_dyncast_node(whatv);
  assert (whatnod != nullptr);
  assert (whatnod->nod_connitm == MOM_PREDEFITM(switch));
  assert (mom_raw_size(whatnod) == 4);
  auto swtypitm = mom_dyncast_item(whatnod->nod_sons[0]);
  auto argexp = whatnod->nod_sons[1];
  auto caseset = mom_dyncast_set(whatnod->nod_sons[2]);
  auto otherwitm = mom_dyncast_item(whatnod->nod_sons[3]);
  assert (swtypitm && swtypitm->va_itype == MOMITY_ITEM);
  assert (caseset && caseset->va_itype == MOMITY_SET);
  unsigned nbcases = mom_size(caseset);
  assert (otherwitm == nullptr || otherwitm->va_itype == MOMITY_ITEM);
  std::unique_ptr<CaseScannerData> casdata {make_case_scanner_data(swtypitm,insitm,-1,fromitm)};
  assert (casdata != nullptr);
  auto argtree = transform_expr(argexp, insitm);
  MOM_DEBUGPRINTF(gencod, "argexp=%s argtree=%s",
                  mom_value_cstring(argexp), mom_value_cstring(argtree));
#define NBSWTYPE_MOM 43
#define CASE_SWTYPE_MOM(Nam) momhashpredef_##Nam % NBSWTYPE_MOM:	\
  if (swtypitm == MOM_PREDEFITM(Nam)) goto foundcaseswtyp_##Nam;	\
  goto defaultcaseswtyp; foundcaseswtyp_##Nam
  switch (swtypitm->hva_hash % NBSWTYPE_MOM)
    {
#warning should fill cases in MomCEmitter::transform_switchinstr
    case CASE_SWTYPE_MOM(item):
      {
	auto itemcasdata = dynamic_cast<ItemCaseScannerData*>(casdata.get());
	assert (itemcasdata != nullptr);
	auto switemtree =
	  mom_boxnode_make_va(MOM_PREDEFITM(sequence),2,
			      literal_string(CITEMSW_PREFIX),
			      insitm);
	MOM_DEBUGPRINTF(gencod, "switemtree=%s", mom_value_cstring(switemtree));
	for (unsigned cix=0; cix<nbcases; cix++)
	  {
	    auto casitm = caseset->seqitem[cix];
	    assert (is_locked_item(casitm));
	    auto runitm =
	      mom_dyncast_item(mom_unsync_item_get_phys_attr(casitm,MOM_PREDEFITM(run)));
	    auto casev =
	      mom_unsync_item_get_phys_attr(casitm,MOM_PREDEFITM(case));
	    MOM_DEBUGPRINTF(gencod, "cix#%d casitm:=%s\n .. runitm=%s casev=%s",
			    cix, mom_item_content_cstring(casitm),
			    mom_item_cstring(runitm), mom_value_cstring(casev));
	    itemcasdata->process_itemcase(casev,casitm,runitm);
	    MOM_DEBUGPRINTF(gencod, "cix#%d processed casitm=%s runitm=%s casev=%s",
			    cix, mom_item_cstring(casitm), mom_item_cstring(runitm),
			    mom_value_cstring(casev));
	  };
	unsigned nbdiscases = itemcasdata->nb_item_cases ();
	unsigned primsiz = //
	  mom_prime_above(3*nbdiscases/2+nbdiscases/4+nbcases/2+15);
	MOM_DEBUGPRINTF(gencod, "nbdiscases=%d primsiz=%d", nbdiscases, primsiz);
	std::multimap<momhash_t,struct mom_item_st*,std::less<long>,
		      traceable_allocator<std::pair<momhash_t,struct mom_item_st*>>>
	  casemultimap;
	itemcasdata->each_item_case([=,&casemultimap](struct mom_item_st*discritm,struct mom_item_st*casitm)
				    {
				      assert (mom_itype(discritm) == MOMITY_ITEM);
				      auto h = mom_hash(discritm);
				      MOM_DEBUGPRINTF(gencod, "discritm=%s casitm=%s h=%u=%#x h%%prim=%d",
						      mom_item_cstring(discritm),
						      mom_item_cstring(casitm),
						      h,h, h%primsiz);
				      casemultimap.insert( {h%primsiz,discritm});
				    },
				    "fill casemultimap", __LINE__);
	unsigned nbemitcases = casemultimap.size();
	MOM_DEBUGPRINTF(gencod, "nbemitcases=%u", nbemitcases);
	traced_vector_values_t vectree;
	int nboutcases = 0;
	momhash_t prevk = 0;
	for (auto p : casemultimap)
	  {
	    momhash_t kh = p.first;
	    struct mom_item_st* kitm=p.second;
	    MOM_DEBUGPRINTF(gencod,
			    "kh=%u kitm=%s",
			    kh, mom_item_cstring(kitm));
	    if (nboutcases==0 || prevk != kh)
	      {
		if (nboutcases > 0)
		  {
		    auto gotothertree =
		      mom_boxnode_make_va(MOM_PREDEFITM(sequence),3,
					  literal_string("goto "),
					  literal_string(COTHERWISELAB_PREFIX),
					  insitm
					  );
		    MOM_DEBUGPRINTF(gencod, "gotothertree=%s",
				    mom_value_cstring(gotothertree));
		    vectree.push_back(gotothertree);
		  }
		else
		  {
		    auto defgotothtree =
		      mom_boxnode_make_va(MOM_PREDEFITM(sequence),5,
					  literal_string("default:"),
					  literal_string("goto "),
					  literal_string(COTHERWISELAB_PREFIX),
					  insitm,
					  literal_string(";")
					  );
		    MOM_DEBUGPRINTF(gencod, "defgotothtree=%s",
				    mom_value_cstring(defgotothtree));
		    vectree.push_back(defgotothtree);
		  }
		auto ccastree =
		  mom_boxnode_make_va(MOM_PREDEFITM(out_newline),3,
				      literal_string("case "),
				      mom_int_make(kh),
				      literal_string(":")
				      );
		MOM_DEBUGPRINTF(gencod, "ccastree=%s",
				mom_value_cstring(ccastree));
		vectree.push_back(ccastree);
		nboutcases++;
	      };
	    auto curitmtree = transform_constant_item(kitm,insitm);
	    MOM_DEBUGPRINTF(gencod, "curitmtree=%s", mom_value_cstring(curitmtree));
	    auto curcasitm = itemcasdata->case_for_item(kitm);
	    auto testitmtree =
	      mom_boxnode_make_sentinel(MOM_PREDEFITM(sequence),
					literal_string("if ("),
					switemtree,
					literal_string(" == "),
					curitmtree,
					literal_string(") goto "),
					literal_string(CCASELAB_PREFIX),
					curcasitm,
					literal_string(";"));
	    MOM_DEBUGPRINTF(gencod, "testitmtree=%s", mom_value_cstring(testitmtree));
	    vectree.push_back(testitmtree);
	  }
	MOM_DEBUGPRINTF(gencod, "vectreesize %d", (int) vectree.size());
	auto swbodytree = mom_boxnode_make(MOM_PREDEFITM(semicolon),
					   vectree.size(),
					   vectree.data());
	vectree.clear();
	MOM_DEBUGPRINTF(gencod, "swbodytree=%s", mom_value_cstring(swbodytree));
	auto defswitemtree =
	  mom_boxnode_make_sentinel(MOM_PREDEFITM(sequence),
				    literal_string("struct mom_item_st* "),
				    switemtree,
				    literal_string(" = "),
				    argtree
				    );
	MOM_DEBUGPRINTF(gencod, "defswitemtree=%s", mom_value_cstring(defswitemtree));
	auto swheadtree=
	  mom_boxnode_make_sentinel(MOM_PREDEFITM(sequence),
				    literal_string("switch(mom_item_hash("),
				    switemtree,
				    literal_string(") % "),
				    mom_int_make(primsiz),
				    literal_string(")"));
	MOM_DEBUGPRINTF(gencod, "swheadtree=%s",
			mom_value_cstring(swheadtree));
	vectree.push_back(defswitemtree);
	auto swblocktree = mom_boxnode_make_va(MOM_PREDEFITM(brace),
					       1,
					       swbodytree);
	auto switchtree =
	  mom_boxnode_make_sentinel (MOM_PREDEFITM(sequence),
				     swheadtree,
				     swblocktree);
	MOM_DEBUGPRINTF(gencod, "switchtree=%s", mom_value_cstring(switchtree));
	vectree.push_back(switchtree);
	auto gotoendtree =
	  mom_boxnode_make_sentinel (MOM_PREDEFITM(sequence),
				     literal_string("goto "),
				     literal_string(CENDCASELAB_PREFIX),
				     insitm);
	MOM_DEBUGPRINTF(gencod, "gotoendtree=%s", mom_value_cstring(gotoendtree));
	/* output each case of caseset and the otherwitm */
	for (unsigned cix=0; cix<nbcases; cix++)
	  {
	    auto casitm = caseset->seqitem[cix];
	    assert (is_locked_item(casitm));
	    auto runitm =
	      mom_dyncast_item(mom_unsync_item_get_phys_attr(casitm,MOM_PREDEFITM(run)));
	    assert (is_locked_item(runitm));
	    auto caselabtree =
	      mom_boxnode_make_sentinel (MOM_PREDEFITM(out_newline),
					 literal_string(CCASELAB_PREFIX),
					 casitm,
					 literal_string(":")
					 );
	    MOM_DEBUGPRINTF(gencod, "cix#%d caselabtree=%s runitm=%s",
			    cix, mom_value_cstring(caselabtree),
			    mom_item_cstring(runitm));
	    vectree.push_back(caselabtree);
	    auto curuntree = transform_instruction(runitm, insitm);
	    MOM_DEBUGPRINTF(gencod, "cix#%d runitm=%s curuntree=%s",
			    cix, mom_item_cstring(runitm),
			    mom_value_cstring(curuntree));
	    vectree.push_back(curuntree);
	    vectree.push_back(gotoendtree);
	  }
	{
	  auto otherwlabtree =
	    mom_boxnode_make_sentinel (MOM_PREDEFITM(out_newline),
				       literal_string(COTHERWISELAB_PREFIX),
				       insitm,
				       literal_string(":"));
	  MOM_DEBUGPRINTF(gencod, "otherwlabtree=%s",
			  mom_value_cstring(otherwlabtree));
	  vectree.push_back(otherwlabtree);
	}
	if (otherwitm)
	  {
	    MOM_DEBUGPRINTF(gencod, "otherwitm=%s",
			    mom_item_cstring(otherwitm));
	    auto otherwtree = transform_instruction(otherwitm, insitm);
	    MOM_DEBUGPRINTF(gencod, "otherwtree=%s",
			    mom_value_cstring(otherwtree));
	    vectree.push_back(otherwtree);
	  }
	vectree.push_back(gotoendtree);
	{
	  auto endlabtree =
	    mom_boxnode_make_sentinel (MOM_PREDEFITM(out_newline),
				       literal_string(CENDCASELAB_PREFIX),
				       insitm,
				       literal_string(":;"));
	  MOM_DEBUGPRINTF(gencod, "endlabtree=%s",
			  mom_value_cstring(endlabtree));
	  vectree.push_back(endlabtree);
	}
	auto fullswitchtree =
	  mom_boxnode_make_sentinel (MOM_PREDEFITM(brace),
				     mom_boxnode_make (MOM_PREDEFITM(semicolon),
						       vectree.size(),
						       vectree.data()));
	MOM_DEBUGPRINTF(gencod, "insitm=%s gives fullswitchtree=%s",
			mom_item_cstring(insitm),
			mom_value_cstring(fullswitchtree));
	return fullswitchtree;
      }
      break;
    case CASE_SWTYPE_MOM(int):
      {
	auto intcasdata = dynamic_cast<IntCaseScannerData*>(casdata.get());
	assert (intcasdata != nullptr);
	traced_vector_values_t casvectree;
	for (unsigned cix=0; cix<nbcases; cix++)
	  {
	    auto casitm = caseset->seqitem[cix];
	    assert (is_locked_item(casitm));
	    auto runitm =
	      mom_dyncast_item(mom_unsync_item_get_phys_attr(casitm,MOM_PREDEFITM(run)));
	    auto casev =
	      mom_unsync_item_get_phys_attr(casitm,MOM_PREDEFITM(case));
	    MOM_DEBUGPRINTF(gencod, "cix#%d casitm:=%s\n .. runitm=%s casev=%s",
			    cix, mom_item_content_cstring(casitm),
			    mom_item_cstring(runitm), mom_value_cstring(casev));
	    intcasdata->process_intcase(casev,casitm,runitm);
	    MOM_DEBUGPRINTF(gencod, "cix#%d processed casitm=%s runitm=%s casev=%s",
			    cix, mom_item_cstring(casitm), mom_item_cstring(runitm),
			    mom_value_cstring(casev));
	    auto castree = intcasdata->c_transform_intcase(this,casev,casitm,runitm);
	    MOM_DEBUGPRINTF(gencod, "cix#%d castree=%s", cix, mom_value_cstring(castree));
	    casvectree.push_back(castree);
	    auto gotocastree = mom_boxnode_make_va(MOM_PREDEFITM(sequence),
						   3,
						   literal_string("goto "),
						   literal_string(CCASELAB_PREFIX),
						   casitm);
	    casvectree.push_back(gotocastree);
	    MOM_DEBUGPRINTF(gencod, "cix#%d gotocastree=%s", cix, mom_value_cstring(gotocastree));
	  };
	auto defgototree = mom_boxnode_make_va(MOM_PREDEFITM(sequence),
					       5,
					       literal_string("default:"),
					       literal_string("goto "),
					       literal_string(COTHERWISELAB_PREFIX),
					       insitm,
					       literal_string(";"));
	casvectree.push_back(defgototree);
	MOM_DEBUGPRINTF(gencod, "insitm %s defgototree %s",
			mom_item_cstring(insitm), mom_value_cstring(defgototree));
	auto swbratree =mom_boxnode_make_va(MOM_PREDEFITM(brace), 1,
					    mom_boxnode_make(MOM_PREDEFITM(semicolon),
							     casvectree.size(),
							     casvectree.data()));
	MOM_DEBUGPRINTF(gencod, "insitm %s swbratree %s", mom_item_cstring(insitm),
			mom_value_cstring(swbratree));
	auto swintree = //
	  mom_boxnode_make_sentinel(MOM_PREDEFITM(sequence),
				    literal_string("switch ("),
				    argtree,
				    literal_string(")"),
				    swbratree);
	MOM_DEBUGPRINTF(gencod, "insitm %s swintree %s",
			mom_item_cstring(insitm), mom_value_cstring(swintree));
	casvectree.clear();
	traced_vector_values_t swinvectree;
	swinvectree.push_back(swintree);
	auto gotoendtree = mom_boxnode_make_va(MOM_PREDEFITM(sequence),
					       3,
					       literal_string("goto "),
					       literal_string(CENDCASELAB_PREFIX),
					       insitm);
	MOM_DEBUGPRINTF(gencod, "insitm %s gotoendtree %s",
			mom_item_cstring(insitm), mom_value_cstring(gotoendtree));
	auto newlinetree = mom_boxnode_make(MOM_PREDEFITM(out_newline),
					    0, NULL);
	for (unsigned cix=0; cix<nbcases; cix++)
	  {
	    auto casitm = caseset->seqitem[cix];
	    if (cix>0)
	      swinvectree.push_back(newlinetree);
	    assert (is_locked_item(casitm));
	    auto runitm =
	      mom_dyncast_item(mom_unsync_item_get_phys_attr(casitm,MOM_PREDEFITM(run)));
	    auto labcastree = mom_boxnode_make_va(MOM_PREDEFITM(sequence),
						  3,
						  literal_string(CCASELAB_PREFIX),
						  casitm,
						  literal_string(":"));
	    MOM_DEBUGPRINTF(gencod, "insitm %s cix#%d labcastree=%s runitm=%s",
			    mom_item_cstring(insitm), cix, mom_value_cstring(labcastree),
			    mom_item_cstring(runitm));
	    swinvectree.push_back(labcastree);
	    auto runtree = transform_instruction(runitm, insitm);
	    MOM_DEBUGPRINTF(gencod, "cix#%d runitm=%s runtree=%s",
			    cix, mom_item_cstring(runitm), mom_value_cstring(runtree));
	    swinvectree.push_back(runtree);
	    swinvectree.push_back(gotoendtree);
	  };
	auto labotherwisetree = mom_boxnode_make_va(MOM_PREDEFITM(sequence),
						    3,
						    literal_string(COTHERWISELAB_PREFIX),
						    insitm,
						    literal_string(":"));
	swinvectree.push_back(labotherwisetree);
	if (otherwitm)
	  {
	    MOM_DEBUGPRINTF(gencod, "otherwitm=%s",
			    mom_item_cstring(otherwitm));
	    auto otherwtree = transform_instruction(otherwitm, insitm);
	    MOM_DEBUGPRINTF(gencod, "otherwtree=%s",
			    mom_value_cstring(otherwtree));
	    swinvectree.push_back(otherwtree);
	  }
	swinvectree.push_back(gotoendtree);
	auto labendtree = mom_boxnode_make_va(MOM_PREDEFITM(sequence),
					      3,
					      literal_string(CENDCASELAB_PREFIX),
					      insitm,
					      literal_string(":;"));
	swinvectree.push_back(labendtree);
	auto fullswintree = mom_boxnode_make_va(MOM_PREDEFITM(brace),1,
						mom_boxnode_make(MOM_PREDEFITM(semicolon),
								 swinvectree.size(),
								 swinvectree.data()));
	MOM_DEBUGPRINTF(gencod, "insitm=%s gives fullswintree=%s",
			mom_item_cstring(insitm), mom_value_cstring(fullswintree));
	return fullswintree;
      }
      break;
    case CASE_SWTYPE_MOM(string):
      {
	auto strcasdata = dynamic_cast<StringCaseScannerData*>(casdata.get());
	assert (strcasdata != nullptr);
	MOM_FATAPRINTF("c-transform_switchinstr string insitm=%s unimplemented",
		       mom_item_cstring(insitm));
#warning c-transform_switchinstr string switch unimplemented
      }
      break;
    default:
    defaultcaseswtyp:
      MOM_FATAPRINTF("c-transform_switchinstr impossible swtypitm %s in insitm %s",
		     mom_item_cstring(swtypitm), mom_item_cstring(insitm));
    }
#warning unimplemented MomCEmitter::transform_switchinstr
  MOM_FATAPRINTF("unimplemented c-transform_switchinstr insitm=%s whatv=%s",
		 mom_item_cstring(insitm), mom_value_cstring(whatv));
} // end of MomCEmitter::transform_switchinstr



////////////////
const struct mom_boxnode_st*
MomCEmitter::transform_routine_element(struct mom_item_st*rtitm)
{
  assert (is_locked_item(rtitm));
  MOM_DEBUGPRINTF(gencod, "c-transform_routine_element start rtitm:=\n%s\n", mom_item_content_cstring(rtitm));
  auto sigitm = mom_dyncast_item(mom_unsync_item_get_phys_attr (rtitm, MOM_PREDEFITM(signature)));
  auto bdyitm = mom_dyncast_item(mom_unsync_item_get_phys_attr (rtitm, MOM_PREDEFITM(body)));
  MOM_DEBUGPRINTF(gencod, "c-transform_routine_element rtitm %s sigitm %s bdyitm %s",
		  mom_item_cstring(rtitm), mom_item_cstring(sigitm), mom_item_cstring(bdyitm));
  assert (is_locked_item(sigitm));
  assert (is_locked_item(bdyitm));
  auto routdnod = declare_routheader_for(sigitm,rtitm);
  MOM_DEBUGPRINTF(gencod, "c-transform_routine_element rtitm %s routdnod %s",
		  mom_item_cstring(rtitm), mom_value_cstring(routdnod));
  if (_cec_declareditems.find(sigitm) == _cec_declareditems.end())
    {
      _cec_declareditems.insert(sigitm);
      auto sigtypnod = declare_signature_type(sigitm);
      MOM_DEBUGPRINTF(gencod, "c-transform_routine_element sigitm %s sigtypnod %s",
		      mom_item_cstring(sigitm), mom_value_cstring(sigtypnod));
      add_global_decl(sigtypnod);
    }
  if (_cec_declareditems.find(rtitm) == _cec_declareditems.end())
    {
      _cec_declareditems.insert(rtitm);
      auto rtdecl =
	mom_boxnode_make_sentinel(MOM_PREDEFITM(sequence),
				  literal_string("extern"),
				  literal_string(" "),
				  literal_string(CSIGNTYPE_PREFIX),
				  sigitm,
				  literal_string(" "),
				  literal_string(MOM_FUNC_PREFIX),
				  rtitm,
				  literal_string(";"));
      add_global_decl(rtdecl);
      MOM_DEBUGPRINTF(gencod, "c-transform_routine_element rtitm %s rtdecl %s",
		      mom_item_cstring(rtitm),
		      mom_value_cstring(rtdecl));
    }
  auto bdynod = transform_body_element(bdyitm,rtitm);
  MOM_DEBUGPRINTF(gencod, "c-transform_routine_element rtitm %s bdyitm %s bdynod %s\n ... routdnod=%s",
		  mom_item_cstring(rtitm), mom_item_cstring(bdyitm),
		  mom_value_cstring(bdynod), mom_value_cstring(routdnod));
  auto rtdef = mom_boxnode_make_va(MOM_PREDEFITM(sequence),4,
				   mom_boxnode_make_va(MOM_PREDEFITM(comment),2,literal_string("routine:"), rtitm),
				   routdnod,
				   bdynod,
				   mom_boxnode_make_va(MOM_PREDEFITM(comment),2,literal_string("endroutine:"), rtitm));
  MOM_DEBUGPRINTF(gencod, "c-transform_routine_element rtitm %s result rtdef=%s",  mom_item_cstring(rtitm),
		  mom_value_cstring(rtdef));
  return rtdef;
} // end MomCEmitter::transform_routine_element




const struct mom_boxnode_st*
MomCEmitter::declare_header_for (struct mom_item_st*sigitm, struct mom_item_st*ilitm, bool inlined)
{
  assert (is_locked_item(sigitm));
  assert (is_locked_item(ilitm));
  MOM_DEBUGPRINTF(gencod, "c-emitter declare_header_for start sigitm=%s %s ilitm:=%s",
		  mom_item_cstring(sigitm), inlined?"inlined":"plain", mom_item_content_cstring(ilitm));
  auto formtup = mom_dyncast_tuple(mom_unsync_item_get_phys_attr (sigitm, MOM_PREDEFITM(formals)));
  assert (formtup != nullptr);
  auto restyv = mom_unsync_item_get_phys_attr (sigitm, MOM_PREDEFITM(result));
  MOM_DEBUGPRINTF(gencod, "c-declare_header_for sigitm=%s formtup=%s restyv=%s",
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
      MOM_DEBUGPRINTF(gencod, "c-declare_header_for sigitm=%s  ix#%d curformitm=%s",
		      mom_item_cstring(sigitm), ix, mom_item_cstring(curformitm));
      assert (is_locked_item(curformitm));
      struct mom_item_st*formtypitm =
	mom_dyncast_item(mom_unsync_item_get_phys_attr (curformitm, MOM_PREDEFITM(type)));
      MOM_DEBUGPRINTF(gencod, "c-declare_header_for curformitm=%s formtypitm=%s",
		      mom_item_cstring(curformitm),
		      mom_item_cstring(formtypitm));
      assert (is_locked_item(formtypitm));
      auto formtypnod = declare_type(formtypitm);
      MOM_DEBUGPRINTF(gencod, "c-declare_header_for formtypitm=%s formtypnod=%s",
		      mom_item_cstring(formtypitm), mom_value_cstring(formtypnod));
      curformdeclv = mom_boxnode_make_va(MOM_PREDEFITM(sequence),4,
					 formtypnod, literal_string(" "),
					 literal_string(CFORMAL_PREFIX),
					 curformitm);
      MOM_DEBUGPRINTF(gencod, "c-declare_header_for ix#%d curformitm=%s curformdeclv=%s",
		      ix,
		      mom_item_cstring(curformitm), mom_value_cstring(curformdeclv));
      formdeclarr[ix] = curformdeclv;
    }
  for (int j=0; j<nbform; j++)
    MOM_DEBUGPRINTF(gencod, "c-declare_header_for formdeclarr[%d] :@%p %s", j, formdeclarr[j], mom_value_cstring(formdeclarr[j]));
  auto formtreev =  mom_boxnode_make_va(MOM_PREDEFITM(parenthesis),1,
					mom_boxnode_make(MOM_PREDEFITM(comma),nbform,formdeclarr));
  if (formdeclarr != smallformdeclarr) GC_FREE(formdeclarr);
  formdeclarr = nullptr;
  MOM_DEBUGPRINTF(gencod, "c-declare_header_for sigitm=%s formtreev=%s restyv=%s",
		  mom_item_cstring(sigitm), mom_value_cstring(formtreev), mom_value_cstring(restyv));
  momvalue_t restytree = nullptr;
  switch(mom_itype(restyv))
    {
    case MOMITY_ITEM:
      restytree = declare_type((struct mom_item_st*)restyv);
      break;
    default:
      throw MOM_RUNTIME_PRINTF("bad result type %s for signature %s inline %s",
			       mom_value_cstring(restyv),
			       mom_item_cstring(sigitm),
			       mom_item_cstring(ilitm));
    }
  MOM_DEBUGPRINTF(gencod, "c-declare_header_for sigitm=%s restyv=%s restytree=%s",
		  mom_item_cstring(sigitm), mom_value_cstring(restyv), mom_value_cstring(restytree));
  auto funheadv =  mom_boxnode_make_sentinel(MOM_PREDEFITM(sequence),
					     inlined?literal_string("static inline"):literal_string("/*plain*/"),
					     literal_string(" "),
					     restytree,
					     literal_string(" "),
					     literal_string(MOM_FUNC_PREFIX),
					     ilitm,
					     formtreev );
  MOM_DEBUGPRINTF(gencod, "c-declare_header_for sigitm=%s ilitm=%s result funheadv=%s",
		  mom_item_cstring(sigitm),
		  mom_item_cstring(ilitm),
		  mom_value_cstring(funheadv));
  return funheadv;
} // end MomCEmitter::declare_header_for



////////////////
const struct mom_boxnode_st*
MomCEmitter::transform_inline_element(struct mom_item_st*ilitm)
{
  assert (is_locked_item(ilitm));
  auto prevfunitm = _ce_curfunctionitm;
  MOM_DEBUGPRINTF(gencod, "c-transform_inline_element start ilitm:=\n%s\n.. prevfunitm=%s",
		  mom_item_content_cstring(ilitm), mom_item_cstring(prevfunitm));
  assert (mom_unsync_item_descr(ilitm) == MOM_PREDEFITM(inline));
  _ce_curfunctionitm = ilitm;
  auto sigitm = mom_dyncast_item(mom_unsync_item_get_phys_attr (ilitm, MOM_PREDEFITM(signature)));
  if (!sigitm)
    throw MOM_RUNTIME_PRINTF("missing signature in inline %s", mom_item_cstring(ilitm));
  lock_item(sigitm);
  auto bdyitm = mom_dyncast_item(mom_unsync_item_get_phys_attr (ilitm, MOM_PREDEFITM(body)));
  if (!bdyitm)
    throw MOM_RUNTIME_PRINTF("missing body in inline %s", mom_item_cstring(ilitm));
  lock_item(bdyitm);
  MOM_DEBUGPRINTF(gencod, "c-transform_inline_element ilitm %s sigitm %s bdyitm %s",
		  mom_item_cstring(ilitm), mom_item_cstring(sigitm), mom_item_cstring(bdyitm));
  auto inlhnod = declare_inlineheader_for(sigitm,ilitm);
  MOM_DEBUGPRINTF(gencod, "c-transform_inline_element ilitm %s sigitm %s inlhnod %s",
		  mom_item_cstring(ilitm), mom_item_cstring(sigitm), mom_value_cstring(inlhnod));
  if (_cec_declareditems.find(sigitm) == _cec_declareditems.end())
    {
      _cec_declareditems.insert(sigitm);
      auto sigtypnod = declare_signature_type(sigitm);
      MOM_DEBUGPRINTF(gencod, "c-transform_inline_element sigitm %s sigtypnod %s",
		      mom_item_cstring(sigitm), mom_value_cstring(sigtypnod));
      add_global_decl(sigtypnod);
    }
  if (_cec_declareditems.find(ilitm) == _cec_declareditems.end())
    {
      _cec_declareditems.insert(ilitm);
      auto inldecl =
	mom_boxnode_make_sentinel(MOM_PREDEFITM(sequence),
				  literal_string("static inline"),
				  literal_string(" "),
				  literal_string(CSIGNTYPE_PREFIX),
				  sigitm,
				  literal_string(" "),
				  literal_string(MOM_FUNC_PREFIX),
				  ilitm,
				  literal_string(";"));
      add_global_decl(inldecl);
      MOM_DEBUGPRINTF(gencod, "c-transform_inline_element ilitm %s inldecl %s", mom_item_cstring(ilitm),
		      mom_value_cstring(inldecl));
    }
  auto bdynod = transform_body_element(bdyitm,ilitm);
  MOM_DEBUGPRINTF(gencod, "c-transform_inline_element ilitm %s bdyitm %s bdynod %s\n ... funhnod=%s",
		  mom_item_cstring(ilitm), mom_item_cstring(bdyitm),
		  mom_value_cstring(bdynod), mom_value_cstring(inlhnod));
  auto inldef = mom_boxnode_make_va(MOM_PREDEFITM(sequence),4,
				    mom_boxnode_make_va(MOM_PREDEFITM(comment),2,literal_string("inline:"), ilitm),
				    inlhnod,
				    bdynod,
				    mom_boxnode_make_va(MOM_PREDEFITM(comment),2,literal_string("endinline:"), ilitm));
  MOM_DEBUGPRINTF(gencod, "c-transform_inline_element ilitm %s result inldef=%s",  mom_item_cstring(ilitm),
		  mom_value_cstring(inldef));
  _ce_curfunctionitm = prevfunitm;
  return inldef;
} // end MomCEmitter::transform_inline_element





////////////////
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
  MOM_DEBUGPRINTF(gencod, "c-case_scanner start swtypitm=%s insitm=%s rk=%d blkitm=%s",
		  mom_item_cstring(swtypitm),
		  mom_item_cstring(insitm),
		  rk,
		  mom_item_cstring(blkitm));
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
	       lock_item(runitm);
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
	       lock_item(runitm);
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


momvalue_t
MomEmitter::IntCaseScannerData::c_transform_intcase(MomCEmitter*cem, const void*expv,
						    struct mom_item_st*casitm,
						    struct mom_item_st*runitm)
{
  assert (cem != nullptr);
  momvalue_t restree = nullptr;
  MOM_DEBUGPRINTF(gencod, "c_transform_intcase expv=%s casitm:=%s\n.. runtim=%s",
		  mom_value_cstring(expv),
		  mom_item_content_cstring(casitm),
		  mom_item_cstring(runitm));
  auto exptyp = mom_itype(expv);
  switch (exptyp)
    {
    case MOMITY_INT:
      {
	restree = mom_boxnode_make_va(MOM_PREDEFITM(sequence), 3,
				      cem->literal_string("case "),
				      expv,
				      cem->literal_string(":"));
	MOM_DEBUGPRINTF(gencod, "c_transform_intcase expv=%s gives %s",
			mom_value_cstring(expv),
			mom_value_cstring(restree));
	return restree;
      }
      break;
    case MOMITY_NODE:
      {
	auto expnod = (const struct mom_boxnode_st*)expv;
	auto connitm = expnod->nod_connitm;
	auto expsiz = mom_raw_size(expv);
	if (connitm == MOM_PREDEFITM(or))
	  {
	    traced_vector_values_t vectree;
	    for (unsigned ix=0; ix<expsiz; ix++)
	      {
		if (ix>0)
		  vectree.push_back(mom_boxnode_make_va(MOM_PREDEFITM(out_newline), 0));
		auto curson = expnod->nod_sons[ix];
		MOM_DEBUGPRINTF(gencod, "c_transform_intcase disjonction ix#%d curson=%s", ix,
				mom_value_cstring(curson));
		auto cursubtree = c_transform_intcase(cem,curson,casitm,runitm);
		MOM_DEBUGPRINTF(gencod, "c_transform_intcase disjonction ix#%d cursubtree=%s", ix,
				mom_value_cstring(cursubtree));
		vectree.push_back(cursubtree);
	      }
	    restree = mom_boxnode_make(MOM_PREDEFITM(sequence),
				       vectree.size(),
				       vectree.data());
	    MOM_DEBUGPRINTF(gencod,
			    "c_transform_intcase disjonction gives %s",
			    mom_value_cstring(restree));
	    return restree;
	  }
	else if (connitm == MOM_PREDEFITM(range))
	  {
	    assert (expsiz==2);
	    auto left = expnod->nod_sons[0];
	    auto right = expnod->nod_sons[1];
	    assert (mom_itype(left)==MOMITY_INT);
	    assert (mom_itype(right)==MOMITY_INT);
	    assert (mom_int_val_def (left, -2) <= mom_int_val_def (right, -1));
	    restree = mom_boxnode_make_va(MOM_PREDEFITM(sequence), 5,
					  cem->literal_string("case "),
					  left,
					  cem->literal_string(" ... "),
					  right,
					  cem->literal_string(":"));
	    MOM_DEBUGPRINTF(gencod, "c_transform_intcase range gives %s",
			    mom_value_cstring(restree));
	    return restree;
	  }
	else
	  MOM_FATAPRINTF("c_transform_intcase bad node expv=%s casitm=%s runtim=%s",
			 mom_value_cstring(expv),
			 mom_item_cstring(casitm),
			 mom_item_cstring(runitm));

      }
      break;
    default: /// should never happen
      break;
    }
  MOM_FATAPRINTF("c_transform_intcase bad expv=%s casitm=%s runtim=%s",
		 mom_value_cstring(expv),
		 mom_item_cstring(casitm),
		 mom_item_cstring(runitm));
} // end MomEmitter::IntCaseScannerData::c_transform_intcase



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
  MOM_DEBUGPRINTF(gencod, "itemcase start expv=%s casitm:=%s\n.. runitm=%s",
		  mom_value_cstring(expv),
		  mom_item_content_cstring(casitm),
		  mom_item_content_cstring(runitm));
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
      MOM_DEBUGPRINTF(gencod, "itemcase casitm %s item %s",
		      mom_item_cstring(casitm), mom_item_cstring(itm));
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
      MOM_DEBUGPRINTF(gencod, "itemcase casitm %s set %s",
		      mom_item_cstring(casitm), mom_value_cstring(set));
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
	  MOM_DEBUGPRINTF(gencod, "itemcase ix#%d itm=%s casitm=%s", ix, mom_item_cstring(itm),
			  mom_item_cstring(casitm));
	  cas_item2casemap[itm] = casitm;
	}
      return;
    }
  else if (expty != MOMITY_NODE)
    throw MOM_RUNTIME_PRINTF("non-node item case %s in case item %s"
			     " run item %s insitm %s #%d blkitm %s",
			     mom_value_cstring(expv), mom_item_cstring(casitm),
			     mom_item_cstring(runitm),
			     mom_item_cstring(cas_insitm),
			     cas_rank, mom_item_cstring(cas_blkitm));
  auto nodexp = (const struct mom_boxnode_st*)expv;
  MOM_DEBUGPRINTF(gencod, "itemcase casitm %s nodexp %s",
		  mom_item_cstring(casitm), mom_value_cstring(nodexp));
  assert (mom_itype(nodexp) == MOMITY_NODE);
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
      formdeclarr[ix] =
	mom_boxnode_make_va(MOM_PREDEFITM(sequence),2,
			    literal_string(JSFORMAL_PREFIX),
			    curformitm);
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
  auto funtree = mom_boxnode_make_sentinel(MOM_PREDEFITM(sequence),
					   funhnod,
					   bdynod,
					   literal_string("\n"));
  MOM_DEBUGPRINTF(gencod, "JS-emitter transform func fuitm %s gives funtree %s",
		  mom_item_cstring(fuitm), mom_value_cstring(funtree));
  return funtree;
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
		  "js-transform_block blkitm=%s bodytup=%s",
		  mom_item_cstring(blkitm), mom_value_cstring(bodytup));
#define NBBLOCKROLE_MOM 31
#define CASE_BLOCKROLE_MOM(Nam) momhashpredef_##Nam % NBBLOCKROLE_MOM:  \
  if (rolitm == MOM_PREDEFITM(Nam)) goto foundcase_##Nam;		\
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
		locarr[lix] = mom_boxnode_make_va(MOM_PREDEFITM(sequence), 2,
						  literal_string(JSLOCAL_PREFIX),
						  locitm);
	      }
	    auto commatree =  mom_boxnode_make(MOM_PREDEFITM(comma), nblocals, locarr);
	    localtree = mom_boxnode_make_va(MOM_PREDEFITM(sequence), 2,
					    literal_string("let "),
					    commatree);
	  }
	MOM_DEBUGPRINTF(gencod,
			"js-transform_block blkitm=%s sequence of body %s localtree=%s",
			mom_item_content_cstring(blkitm), mom_value_cstring(bodytup),
			mom_value_cstring(localtree));
	int bodylen = mom_raw_size(bodytup);
	int bodycnt = 0;
	momvalue_t smalbodyarr[8]= {};
	momvalue_t* bodyarr = ((bodylen+1)<(int)(sizeof(smalbodyarr)/sizeof(momvalue_t)))
	  ? smalbodyarr
	  : (momvalue_t*) mom_gc_alloc((bodylen+1)*sizeof(momvalue_t));
	if (localtree != nullptr)
	  bodyarr[bodycnt++] = localtree;
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
	    bodyarr[bodycnt++] = instree;
	  }
	auto bodytree = mom_boxnode_make(MOM_PREDEFITM(semicolon),bodycnt,bodyarr);
	auto bracetree =
	  mom_boxnode_make_va(MOM_PREDEFITM(brace),
			      5,
			      mom_boxnode_make_va(MOM_PREDEFITM(out_newline),0),
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
    case CASE_BLOCKROLE_MOM (loop):
      {
	auto whilexpv =  (momvalue_t)mom_unsync_item_get_phys_attr (blkitm, MOM_PREDEFITM(while));
	auto bodytup = mom_dyncast_tuple(mom_unsync_item_get_phys_attr (blkitm, MOM_PREDEFITM(body)));
	long nbcont = _ce_continuecountmap[blkitm];
	long nbbreak = _ce_breakcountmap[blkitm];
	momvalue_t prologtree = nullptr;
	MOM_DEBUGPRINTF(gencod,
			"js-transform_block blkitm=%s loop of while %s, body %s, nbcont=%ld, nbbreak=%ld",
			mom_item_cstring(blkitm), mom_value_cstring(whilexpv), mom_value_cstring(bodytup),
			nbcont, nbbreak);
	if (whilexpv == nullptr || whilexpv == MOM_PREDEFITM(truth))
	  {
	    prologtree = literal_string("for(;;)");
	  }
	else
	  {
	    prologtree = mom_boxnode_make_va(MOM_PREDEFITM(sequence),3,
					     literal_string("while ("),
					     transform_expr(whilexpv,blkitm),
					     literal_string(")"));
	  };
	MOM_DEBUGPRINTF(gencod,
			"js-transform_block blkitm=%s loop prologtree=%s",
			mom_item_cstring(blkitm), mom_value_cstring(prologtree));
	int bodylen = mom_raw_size(bodytup);
	momvalue_t smalbodyarr[8]= {};
	momvalue_t* bodyarr = (bodylen<(int)(sizeof(smalbodyarr)/sizeof(momvalue_t)))
	  ? smalbodyarr
	  : (momvalue_t*) mom_gc_alloc(bodylen*sizeof(momvalue_t));
	for (int bix=0; bix<bodylen; bix++)
	  {
	    struct mom_item_st*insitm = bodytup->seqitem[bix];
	    MOM_DEBUGPRINTF(gencod,
			    "js-transform_block loop blkitm=%s bix#%d insitm:=\n%s",
			    mom_item_cstring(blkitm), bix, mom_item_content_cstring(insitm));
	    assert (is_locked_item(insitm));
	    auto instree = transform_instruction(insitm, blkitm);
	    MOM_DEBUGPRINTF(gencod,
			    "js-transform_block loop insitm=%s instree=%s",
			    mom_item_cstring(insitm), mom_value_cstring(instree));
	    assert (instree != nullptr);
	    bodyarr[bix] = instree;
	  }
	auto bodytree = mom_boxnode_make(MOM_PREDEFITM(semicolon),bodylen,bodyarr);
	auto labtree = (nbcont>0 || nbbreak>0)
	  ? mom_boxnode_make_va(MOM_PREDEFITM(sequence),4,
				literal_string(" "),
				literal_string(JSLABEL_PREFIX),
				blkitm,
				literal_string(":"))
	  : nullptr;
	MOM_DEBUGPRINTF(gencod,
			"js-transform_block loop blkitm=%s prologtree=%s bodytree=%s labtree=%s",
			mom_item_cstring(blkitm), mom_value_cstring(prologtree),
			mom_value_cstring(bodytree), mom_value_cstring(labtree));
	auto looptree = //
	  mom_boxnode_make_va(MOM_PREDEFITM(sequence),5,
			      mom_boxnode_make_va(MOM_PREDEFITM(comment),2,
						  literal_string("loop "),
						  blkitm),
			      labtree,
			      prologtree,
			      mom_boxnode_make_va(MOM_PREDEFITM(brace),3,
						  bodytree,
						  literal_string(";")),
			      mom_boxnode_make_va(MOM_PREDEFITM(comment),2,
						  literal_string("endloop "),
						  blkitm)
			      );
	MOM_DEBUGPRINTF(gencod,
			"js-transform_block loop blkitm=%s result looptree=%s",
			mom_item_cstring(blkitm), mom_value_cstring(looptree));
	return looptree;
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
  auto whatv = insbind->vd_what;
#define NBINSTROLE_MOM 73
#define CASE_INSTROLE_MOM(Nam) momhashpredef_##Nam % NBINSTROLE_MOM:	\
  if (rolitm == MOM_PREDEFITM(Nam)) goto foundcase_##Nam;		\
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
    case CASE_INSTROLE_MOM(assign):
      {
	auto whatnod = mom_dyncast_node(whatv);
	assert (whatnod != nullptr && whatnod->nod_connitm == MOM_PREDEFITM(assign) && mom_raw_size(whatnod) == 3);
	auto tovaritm = mom_dyncast_item(whatnod->nod_sons[0]);
	auto fromexp = whatnod->nod_sons[1];
	auto totypitm = mom_dyncast_item(whatnod->nod_sons[2]);
	MOM_DEBUGPRINTF(gencod,
			"c-transform_instruction assign insitm=%s tovaritm=%s fromexp=%s totypitm=%s",
			mom_item_cstring(insitm), mom_item_cstring(tovaritm),
			mom_value_cstring(fromexp), mom_item_cstring(totypitm));
	assert (tovaritm != nullptr);
	assert (totypitm != nullptr);
	auto fromtree =  transform_expr(fromexp, insitm);
	if (fromtree == nullptr)
	  fromtree = literal_string("null");
	MOM_DEBUGPRINTF(gencod,
			"js-transform_instruction assign insitm=%s fromtree=%s",
			mom_item_cstring(insitm), mom_value_cstring(fromtree));
	auto totree = transform_var(tovaritm, insitm);
	MOM_DEBUGPRINTF(gencod,
			"js-transform_instruction assign insitm=%s totree=%s",
			mom_item_cstring(insitm), mom_value_cstring(totree));
	auto assitree = mom_boxnode_make_va(MOM_PREDEFITM(sequence),3,
					    totree, literal_string (" = "), fromtree);
	MOM_DEBUGPRINTF(gencod,
			"js-transform_instruction assign insitm=%s assitree=%s",
			mom_item_cstring(insitm), mom_value_cstring(assitree));
	return assitree;
      }
      break;
    case CASE_INSTROLE_MOM(break):
      {
	auto outblkitm = mom_dyncast_item(whatv);
	MOM_DEBUGPRINTF(gencod,
			"js-transform_instruction break insitm=%s outblkitm=%s",
			mom_item_cstring(insitm), mom_item_cstring(outblkitm));
	assert (outblkitm != nullptr && is_locked_item(outblkitm));
	assert (_ce_breakcountmap[outblkitm] > 0);
	auto breaktree =
	  mom_boxnode_make_va(MOM_PREDEFITM(sequence),4,
			      literal_string("break "),
			      literal_string(JSLABEL_PREFIX),
			      outblkitm,
			      literal_string(";"));
	MOM_DEBUGPRINTF(gencod,
			"js-transform_instruction break insitm=%s result breaktree=%s",
			mom_item_cstring(insitm), mom_value_cstring(breaktree));
	return breaktree;
      }
      break;
    case CASE_INSTROLE_MOM(continue):
      {
	auto outblkitm = mom_dyncast_item(whatv);
	MOM_DEBUGPRINTF(gencod,
			"js-transform_instruction continue insitm=%s outblkitm=%s",
			mom_item_cstring(insitm), mom_item_cstring(outblkitm));
	assert (outblkitm != nullptr && is_locked_item(outblkitm));
	assert (_ce_continuecountmap[outblkitm] > 0);
	auto contree =
	  mom_boxnode_make_va(MOM_PREDEFITM(sequence),4,
			      literal_string("continue "),
			      literal_string(JSLABEL_PREFIX),
			      outblkitm,
			      literal_string(";"));
	MOM_DEBUGPRINTF(gencod,
			"js-transform_instruction continue insitm=%s result contree=%s",
			mom_item_cstring(insitm), mom_value_cstring(contree));
	return contree;
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
  assert (is_locked_item(insitm));
  assert (is_locked_item(runitm));
  auto desrunitm = mom_unsync_item_descr(runitm);
  if (desrunitm == MOM_PREDEFITM(primitive))
    {
      auto sigitm = mom_dyncast_item(mom_unsync_item_get_phys_attr (runitm, MOM_PREDEFITM(signature)));
      MOM_DEBUGPRINTF(gencod, "js-transform_runinstr runitm=%s sigitm:=\n%s",
		      mom_item_cstring(runitm), mom_item_content_cstring(sigitm));
      assert (is_locked_item(sigitm));
      auto formaltup = mom_dyncast_tuple(mom_unsync_item_get_phys_attr (sigitm, MOM_PREDEFITM(formals)));
      assert (formaltup != nullptr);
      unsigned nbformals = mom_size(formaltup);
      traced_map_item2value_t argmap;
      auto inscomp = insitm->itm_pcomp;
      momvalue_t treev = nullptr;
      auto expnod = mom_dyncast_node(mom_unsync_item_get_phys_attr (runitm, MOM_PREDEFITM(js_expansion)));
      MOM_DEBUGPRINTF(gencod, "js-transform_runinstr formaltup=%s expnod=%s", mom_value_cstring(formaltup),
		      mom_value_cstring(expnod));
      if (!expnod || expnod->nod_connitm != MOM_PREDEFITM(code_chunk))
	throw MOM_RUNTIME_PRINTF("js-transform_runinstr insitm=%s runitm=%s bad expnod=%s",
				 mom_item_cstring(insitm), mom_item_cstring(runitm),
				 mom_value_cstring(expnod));
      for (int aix=0; aix<(int)nbformals; aix++)
	{
	  auto curfitm = formaltup->seqitem[aix];
	  auto curarg = mom_vectvaldata_nth(inscomp, aix);
	  MOM_DEBUGPRINTF(gencod, "js-transform_runinstr aix#%d curfitm=%s curarg=%s", aix,
			  mom_item_cstring(curfitm), mom_value_cstring(curarg));
	  assert (mom_itype(curfitm)==MOMITY_ITEM);
	  auto argtree =  transform_expr(curarg, insitm);
	  MOM_DEBUGPRINTF(gencod, "js-transform_runinstr insitm=%s aix#%d curarg=%s argtree=%s",
			  mom_item_cstring(fromitm), aix, mom_value_cstring(curarg), mom_value_cstring(argtree));
	  argmap[curfitm] = argtree;
	}
      int exparity = mom_raw_size(expnod);
      momvalue_t smalltreearr[8]= {};
      momvalue_t* treearr = (exparity<(int)(sizeof(smalltreearr)/sizeof(momvalue_t)))
	? smalltreearr
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
      MOM_DEBUGPRINTF(gencod, "js-transform_runinstr insitm=%s treev=%s",
		      mom_item_cstring(insitm), mom_value_cstring(treev));
      return treev;
    }
#warning unimplemented MomJavascriptEmitter::transform_runinstr
  MOM_FATAPRINTF("unimplemented js-transform_runinstr insitm=%s", mom_item_cstring(insitm));
} // end of MomJavascriptEmitter::transform_runinstr


momvalue_t
MomJavascriptEmitter::transform_expr(momvalue_t expv, struct mom_item_st*initm)
{
  MOM_DEBUGPRINTF(gencod, "js-transform_expr expv=%s initm=%s",
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
	MOM_DEBUGPRINTF(gencod, "js-transform_expr expitm=%s bind rol %s what %s",
			mom_item_cstring(expitm), mom_item_cstring(rolitm),
			expbind?mom_value_cstring(expbind->vd_what):"°");
#define NBROLE_MOM 31
#define CASE_ROLE_MOM(Nam) momhashpredef_##Nam % NBROLE_MOM:		\
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
			      "js-transform_expr expitm=%s constant",
			      mom_item_cstring(expitm));
#warning js-transform_expr does not handle yet items
	      MOM_FATAPRINTF("js-transform_expr constant expitm=%s initm=%s unhandled",
			     mom_item_cstring(expitm), mom_item_cstring(initm));
	    }
	    break;
	  defaultrole:
	  default:
	    MOM_FATAPRINTF("js-transform_expr bad expitm=%s rolitm=%s initm=%s",
			   mom_item_cstring(expitm), mom_item_cstring(rolitm), mom_item_cstring(initm));
	  }
#undef NBROLE_MOM
#undef CASE_ROLE_MOM
      }
      break;
    case MOMITY_NODE:
      return transform_node_expr((struct mom_boxnode_st*)expv, initm);
    default:
      MOM_FATAPRINTF("js-transform_expr bad expv=%s initm=%s",
                     mom_value_cstring(expv), mom_item_cstring(initm));

    }
#warning unimplemented MomJavascriptEmitter::transform_expr
  MOM_FATAPRINTF("unimplemented MomJavascriptEmitter::transform_expr expv=%s initm=%s",
                 mom_value_cstring(expv), mom_item_cstring(initm));
} // end of MomJavascriptEmitter::transform_expr


momvalue_t
MomJavascriptEmitter::transform_var(struct mom_item_st*varitm, struct mom_item_st*insitm, const vardef_st*varbind)
{
  if (!varbind)
    varbind = get_binding(varitm);
  if (MOM_UNLIKELY(!varbind)) // should never happen
    MOM_FATAPRINTF("js-transform_var varitm=%s in insitm %s lacking binding",
                   mom_item_cstring(varitm), mom_item_cstring(insitm));
  auto rolitm = varbind->vd_rolitm;
  MOM_DEBUGPRINTF(gencod, "js-transform_var varitm:=\n%s ... insitm=%s, rolitm=%s what=%s",
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
                            literal_string(JSFORMAL_PREFIX),
                            varitm);
      break;
    defaultrole:
    default:
      break;
    }
#undef NBROLE_MOM
#undef CASE_ROLE_MOM
  MOM_DEBUGPRINTF(gencod, "js-transform_var varitm=%s insitm=%s rolitm=%s vartree=%s",
                  mom_item_content_cstring(varitm), mom_item_cstring(insitm),
                  mom_item_cstring(rolitm), mom_value_cstring(vartree));
  if (vartree) return vartree;
#warning unimplemented MomJavascriptEmitter::transform_var
  MOM_FATAPRINTF("unimplemented MomJavascriptEmitter::transform_var varitm=%s insitm=%s rolitm=%s",
                 mom_item_cstring(varitm), mom_item_cstring(insitm), mom_item_cstring(rolitm));
} // end of MomJavascriptEmitter::transform_var

momvalue_t
MomJavascriptEmitter:: transform_node_expr(const struct mom_boxnode_st*expnod, struct mom_item_st*insitm)
{
  MOM_DEBUGPRINTF(gencod, "js-transform_node_expr expnod=%s insitm=%s",
                  mom_value_cstring(expnod), mom_item_cstring(insitm));
#warning unimplemented MomJavascriptEmitter::transform_node_expr
  MOM_FATAPRINTF("unimplemented MomJavascriptEmitter::transform_node_expr expnod=%s insitm=%s",
                 mom_value_cstring(expnod), mom_item_cstring(insitm));
} // end of MomJavascriptEmitter::transform_node_expr


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


// end of file emitc.cc

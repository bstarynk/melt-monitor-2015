// file microedit.js - Javascript & Jquery for microedit.html.

/**   Copyright (C)  2015 Basile Starynkevitch, later FSF
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

var $editdiv;
var $editlog;
var $cleareditbut;


////////////////////////////////////////////////////////////////
///// utility functions

// from http://stackoverflow.com/a/1219983/841108
function htmlEncode(value){
    //create a in-memory div, set it's inner text(which jQuery automatically encodes)
    //then grab the encoded contents back out.  The div never exists on the page.
    return $('<div/>').text(value).html();
};

function htmlDecode(value){
    return $('<div/>').html(value).text();
};

var momc_count=0;
var momv_entriesvar=null;
var mom_name_regexp = /^[A-Za-z0-9_]*$/;

var mom_name_cache = new Object();

/// check if a name is some known item, with memoization & AJAX do_knownitem
function mom_known_item(name) {
    if (!name.match(mom_name_regexp))
        return false;
    if (mom_name_cache[name])
        return true;
    var res = null;
    console.log("mom_known_item name=", name, " before ajaxing");
    $.ajax
    ({url: "/microedit",
      method: "POST",
      async: false,
      data: {"do_knownitem": name},
      dataType: "json",
      success: function (data, stat, jh) {
          console.log("mom_known_item success data=", data, " stat=", stat, " jh=", jh);
          if (data) {
              mom_known_item[name] = data;
              res = true;
          }
          else res = false;
      },
      error: function (jq, stat, err) {
          console.warn("mom_known_item error jq=", jq, " stat=", stat, " err=", err);
          res = false;
      }
     });
    console.log ("mom_known_item name=", name, " res=", res);
    return res;
};                              // end mom_known_item

//// give the completion of some name string, using AJAX do_completename
function mom_complete_name(name) {
    var res = null;
    if (!name.match(mom_name_regexp)) {
        console.log ("mom_complete_name bad name=", name);
        return false;
    }
    else 
        console.log("mom_complete_name name=", name, " before ajaxing");
    $.ajax
    ({url: "/microedit",
      method: "POST",
      async: false,
      data: {"do_completename": name},
      dataType: "json",
      success: function (data) {
          console.log("mom_complete_name success data=", data);
          console.trace();
          res = data;
      },
      error: function (jq, stat, err) {
          console.warn("mom_complete_name error jq=", jq, " stat=", stat, " err=", err);
          res = false;
      }
     });
    console.log ("mom_complete_name name=", name, " res=", res);
    return res;
};                              // end mom_complete_name



// this utility function is assigning a unique number
function mom_numbered(obj) {
    momc_count = momc_count+1;
    obj.inum = momc_count;
    return momc_count;
};
/// this function is called by /microedit AJAX for do_fillpage at document loading
function mom_ajaxfillscript(script) {
    console.log("mom_ajaxfillscript:\n", script, "\n### endajaxfillscript\n");
    console.trace();
};

function mome_begin_fill(statitmid) {
    console.log("mome_begin_fill statitmid=", statitmid, " $editdiv=", $editdiv);
    $editdiv.empty();
    $("<p class='mombeginfill_cl'>" + statitmid + "</p>").appendTo($editdiv);
    console.log("mome_begin_fill $editdiv=", $editdiv, " this=", this);
    mom_name_cache = new Object();
};

function mome_mtime(timestr) {
    console.log("mome_mtime timestr=", timestr);
    $("<p class='mommtime_cl'>" + htmlEncode(timestr) + "</p>").appendTo($editdiv);
    console.log("mome_mtime $editdiv=", $editdiv, " this=", this);
};

function mome_entries(entarr) {
    console.log("mome_entries entarr=", entarr, " $editdiv=", $editdiv,
                " this=", this);
    momv_entriesvar = entarr;
    console.assert(Array.isArray(entarr),
                   "mome_entries bad entarr=", entarr);
    var nbent = entarr.length;
    var entdl = $("<dl class='momentrieslist_cl'></dl>");
    entdl.appendTo($editdiv);
    entarr.mom_ast = "entries";
    console.log("mome_entries entdl=", entdl, " $editdiv=", $editdiv, " entarr=", entarr);
};


function mome_generated(msg) {
    console.log("mome_generated start msg=", msg, "; $editdiv=", $editdiv, " this=", this);
    console.log("mome_generated ending $editlog=", $editlog, " msg=", msg);
    $editlog.append(msg);
};                              // end mome_generated

////////////////
function MomeEntry(entitm,entval) {
    this.mom_entry_item= entitm;
    this.mom_entry_val= entval;
    this.mom_ast = "entry";
    mom_numbered(this);
};
var momp_entry = {
    name: "MomeEntry",
    realize: function (incont) {
	console.error("MomeEntry-realize unimplemented incont=", incont);
    }
};
MomeEntry.prototype = momp_entry;
function mome_entry(entitm,entval) {
    console.log("mome_entry entitm=", entitm, " entval=", entval);
    var res = new MomeEntry(entitm, entval);
    console.log("mome_entry res=", res);
    return res;
};


var momp_value = {
    name: "MompValue",
    realize: function (incont,fromast) {
	console.error("MomeValue-realize bad incont=", incont, " fromast=", fromast);
    }
};
////////////////////////////////////////////////////////////////


var momp_item_ref = {
    name: "MomeItemRef",
    realize: function (incont,fromast) {
	console.error("MomeItemRef-realize unimplemented incont=", incont, " fromast=", fromast);
    }
};
function MomeItemRef(iname) {
    this.mom_item_name = iname;
    this.mom_ast = "item_ref";
    mom_numbered(this);
};
MomeItemRef.prototype = momp_item_ref;
function mome_item_ref(itmname) {
    var res = new MomeItemRef(itmname);
    console.log ("mome_item_ref res=", res);
    return res;
};


////////////////
var momp_nil_value = {
    name: "MomeNilValue",
    realize: function (incont,fromast) {
	console.error("MomeNilValue-realize unimplemented incont=", incont, " fromast=", fromast);
    }
};
function MomeNilValue() {
    mom_numbered(this);
    this.mom_ast = "nilval";
};
MomeNilValue.prototype = momp_nil_value;

function mome_nil_val() { /// a nil value
    var res = new MomeNilValue();
    console.log ("mome_nil_val res=", res);
    return res;
};

////////////////
var momp_nil_ref = {
    name: "MomeNilRef",
    realize: function (incont,fromast) {
	console.error("MomeNilRef-realize unimplemented incont=", incont, " fromast=", fromast);
    }
};
function MomeNilRef() {
    mom_numbered(this);
    this.mom_ast = "nilref";
};
MomeNilRef.prototype = momp_nil_ref;

function mome_nil_ref() {       /// a nil item reference
    var res = new MomeNilRef();
    console.log ("mome_nil_ref res=", res);
    return res;
};
////////////////

console.warn("incomplete new microedit.js, see also _old_microedit.js");
